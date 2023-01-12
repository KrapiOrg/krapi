//
// Created by mythi on 17/12/22.
//

#include "nlohmann/json_fwd.hpp"
#include "spdlog/spdlog.h"

#include "SignalingMessage.h"
#include "SignalingServer.h"

namespace krapi {
  std::weak_ptr<rtc::WebSocket>
  SignalingServer::get_socket(const std::string &identity) const {

    std::lock_guard l(m_mutex);
    if (m_sockets.contains(identity)) return m_sockets.at(identity);

    return {};
  }

  std::unordered_set<std::string>
  SignalingServer::get_identities(std::string identity_to_filter_for) const {

    std::lock_guard l(m_mutex);

    auto identities = m_identities;
    identities.erase(identity_to_filter_for);

    return identities;
  }

  void SignalingServer::broadcast(
    std::string identity_to_filter_for,
    SignalingMessageType type,
    nlohmann::json content
  ) {

    auto identities = get_identities(identity_to_filter_for);

    for (const auto &iden: identities) {
      auto wws = get_socket(iden);
      if (auto ws = wws.lock()) {
        ws->send(
          SignalingMessage(
            SignalingMessageType::PeerAvailable,
            "signaling_server",
            iden,
            SignalingMessage::create_tag(),
            content
          )
            .to_string()
        );
      }
    }
  }

  void SignalingServer::on_client_message(
    std::string sender_identity,
    rtc::message_variant rtc_message
  ) {

    auto message_str = std::get<std::string>(rtc_message);
    auto message =
      SignalingMessage::from_json(nlohmann::json::parse(message_str));

    if (message->type() == SignalingMessageType::IdentityRequest) {
      auto wsocket = get_socket(sender_identity);
      if (auto socket = wsocket.lock()) {

        socket->send(
          SignalingMessage(
            SignalingMessageType::IdentityResponse,
            "signaling_server",
            "",
            message->tag(),
            sender_identity
          )
            .to_string()
        );

        auto has_requested_identity_before = m_identities.contains(sender_identity);
        if (has_requested_identity_before)
          return;
        m_identities.insert(sender_identity);

        broadcast(
          sender_identity,
          SignalingMessageType::PeerAvailable,
          nlohmann::json(sender_identity)
        );
      }
    } else if (message->type() == SignalingMessageType::AvailablePeersRequest) {

      auto wsocket = get_socket(message->sender_identity());
      if (auto socket = wsocket.lock()) {

        socket->send(SignalingMessage(
                       SignalingMessageType::AvailablePeersResponse,
                       "signaling_server",
                       message->sender_identity(),
                       message->tag(),
                       get_identities(message->sender_identity())
        )
                       .to_string());
      }
    } else if (message->type() == SignalingMessageType::RTCSetup || message->type() == SignalingMessageType::RTCCandidate) {

      if (auto socket = get_socket(message->receiver_identity()).lock()) {
        socket->send(message->to_string());
      }
    }
  }

  void SignalingServer::on_client_closed(std::string identity) {

    bool erased;

    {
      std::lock_guard l(m_mutex);
      erased = m_sockets.erase(identity) && m_identities.erase(identity);
    }

    if (!erased) return;

    spdlog::info("connection with {} closed", identity);
    for (auto &[receiver, socket]: m_sockets) {
      socket->send(
        SignalingMessage(
          SignalingMessageType::PeerClosed,
          "signaling_server",
          receiver,
          SignalingMessage::create_tag(),
          identity
        )
          .to_string()
      );
    }
  }

  void SignalingServer::on_client_open(std::string identity) const {

    spdlog::info("{} Connected", identity);
  }

  SignalingServer::SignalingServer()
      : m_blocking_bool(false) {

    m_server.onClient([this](std::shared_ptr<rtc::WebSocket> ws) {
      auto identity = uuids::to_string(uuids::uuid_system_generator{}());

      ws->onOpen([identity, this]() { on_client_open(identity); });
      ws->onMessage([identity, this](rtc::message_variant message) {
        on_client_message(identity, std::move(message));
      });
      ws->onClosed([identity, this]() {
        auto iden = identity;
        on_client_closed(iden);
      });
      std::lock_guard l(m_mutex);
      m_sockets.emplace(identity, std::move(ws));
    });
  }

  int SignalingServer::port() const {
    return m_server.port();
  }

  SignalingServer::~SignalingServer() {

    m_blocking_bool.wait(false);
    m_server.stop();
  }
}// namespace krapi