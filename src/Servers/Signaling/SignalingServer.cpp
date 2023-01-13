//
// Created by mythi on 17/12/22.
//

#include "nlohmann/json_fwd.hpp"
#include "spdlog/spdlog.h"

#include "SignalingMessage.h"
#include "SignalingServer.h"
#include <mutex>

namespace krapi {

  void SignalingServer::on_client_message(
    RawWebSocket ws,
    rtc::message_variant rtc_message
  ) {

    std::lock_guard l(m_mutex);

    auto message_str = std::get<std::string>(rtc_message);
    auto message =
      SignalingMessage::from_json(nlohmann::json::parse(message_str));

    std::string my_identity;
    my_identity = m_socket_identity_map[ws];

    if (message->type() == SignalingMessageType::SetIdentityRequest) {

      auto identity = message->content().get<std::string>();
      m_socket_identity_map[ws] = identity;
      spdlog::info("{} client has connected", identity);

      ws->send(
        SignalingMessage(
          SignalingMessageType::SetIdentityResponse,
          "signaling_server",
          identity,
          message->tag(),
          nlohmann::json()
        )
          .to_string()
      );

      for (const auto &socket: m_sockets) {

        if (socket.get() != ws) {
          spdlog::info("here2");
          socket->send(
            SignalingMessage(
              SignalingMessageType::PeerAvailable,
              "signaling_server",
              m_socket_identity_map[socket.get()],
              message->tag(),
              nlohmann::json(identity)
            )
              .to_string()
          );
        }
      }

    } else if (message->type() == SignalingMessageType::AvailablePeersRequest) {

      std::vector<std::string> identities;

      for (const auto &socket: m_sockets) {
        if (m_socket_identity_map[socket.get()] != my_identity) {
          identities.push_back(m_socket_identity_map[socket.get()]);
        }
      }

      ws->send(
        SignalingMessage(
          SignalingMessageType::AvailablePeersResponse,
          "signaling_server",
          message->sender_identity(),
          message->tag(),
          identities
        )
          .to_string()
      );

    } else if (message->type() == SignalingMessageType::RTCSetup || message->type() == SignalingMessageType::RTCCandidate) {
      
      auto receiver_identity = message->receiver_identity();

      for (const auto &socket: m_sockets) {

        if (m_socket_identity_map[socket.get()] == receiver_identity) {

          socket->send(message->to_string());
        }
      }
    }
  }

  void SignalingServer::on_client_closed(RawWebSocket ws) {


    auto identity = m_socket_identity_map[ws];
    bool erased;

    std::lock_guard l(m_mutex);

    erased = std::erase_if(
               m_sockets,
               [=](WebSocket socket) {
                 return socket.get() == ws;
               }
             ) == 1 &&
             m_socket_identity_map.erase(ws);

    if (!erased) return;

    spdlog::info("connection with {} closed", identity);
    for (auto socket: m_sockets) {
      socket->send(
        SignalingMessage(
          SignalingMessageType::PeerClosed,
          "signaling_server",
          m_socket_identity_map[socket.get()],
          SignalingMessage::create_tag(),
          identity
        )
          .to_string()
      );
    }
  }

  SignalingServer::SignalingServer()
      : m_blocking_bool(false) {

    m_server.onClient([this](std::shared_ptr<rtc::WebSocket> ws) {
      {
        std::lock_guard l(m_mutex);
        m_sockets.emplace(ws);
        m_socket_identity_map.emplace(ws.get(), std::string{});
      }

      ws->onMessage(
        [=, this](rtc::message_variant message) {
          on_client_message(ws.get(), std::move(message));
        }
      );

      ws->onClosed(
        [=, this]() {
          on_client_closed(ws.get());
        }
      );
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