
#include "PeerConnection.h"
#include "PeerMessage.h"
#include "nlohmann/json_fwd.hpp"
#include "rtc/candidate.hpp"
#include "rtc/description.hpp"
#include "spdlog/spdlog.h"

#include <chrono>

using namespace std::chrono_literals;

namespace krapi {

  PeerConnection::PeerConnection(
    EventQueuePtr event_queue,
    SignalingClientPtr signaling_client,
    std::string identity
  )
      : m_signaling_client(std::move(signaling_client)),
        m_event_queue(event_queue),
        m_subscription_remover(event_queue->internal_queue()),
        m_peer_connection(
          std::make_shared<rtc::PeerConnection>(rtc::Configuration())
        ),
        m_peer_type(PeerType::Unknown),
        m_peer_state(PeerState::Unknown),
        m_identity(std::move(identity)) {


    m_peer_connection->onLocalCandidate(
      std::bind_front(&PeerConnection::on_local_candidate, this)
    );
    m_peer_connection->onLocalDescription(
      std::bind_front(&PeerConnection::on_local_description, this)
    );
    m_datachannel = m_peer_connection->createDataChannel("krapi");
    initialize_channel(m_datachannel);

    m_subscription_remover.appendListener(
      PeerMessageType::PeerStateUpdate,
      std::bind_front(&PeerConnection::on_peer_state_update, this)
    );
    m_subscription_remover.appendListener(
      SignalingMessageType::RTCCandidate,
      std::bind_front(&PeerConnection::on_rtc_candidate, this)
    );
  }

  PeerConnection::PeerConnection(
    EventQueuePtr event_queue,
    SignalingClientPtr signaling_client,
    std::string identity,
    std::string sdp,
    std::string type
  )
      : m_signaling_client(signaling_client),
        m_event_queue(event_queue),
        m_subscription_remover(event_queue->internal_queue()),
        m_peer_connection(
          std::make_shared<rtc::PeerConnection>(rtc::Configuration())
        ),
        m_peer_type(PeerType::Unknown),
        m_peer_state(PeerState::Unknown),
        m_identity(std::move(identity)) {

    m_peer_connection->onLocalCandidate(
      std::bind_front(&PeerConnection::on_local_candidate, this)
    );
    m_peer_connection->onLocalDescription(
      std::bind_front(&PeerConnection::on_local_description, this)
    );
    m_peer_connection->onDataChannel(
      std::bind_front(&PeerConnection::initialize_channel, this)
    );
    m_peer_connection->setRemoteDescription(
      rtc::Description(sdp, type)
    );

    m_subscription_remover.appendListener(
      PeerMessageType::PeerStateUpdate,
      std::bind_front(&PeerConnection::on_peer_state_update, this)
    );

    m_subscription_remover.appendListener(
      SignalingMessageType::RTCCandidate,
      std::bind_front(&PeerConnection::on_rtc_candidate, this)
    );
  }

  void PeerConnection::initialize_channel(RTCDataChannel datachannel) {

    if (m_datachannel == nullptr) {
      m_datachannel = datachannel;
    }

    m_datachannel->onMessage([this](rtc::message_variant rtc_message) {
      auto message_str = std::get<std::string>(std::move(rtc_message));

      auto message_json = nlohmann::json::parse(message_str);
      auto peer_message = PeerMessage::from_json(message_json);
      m_event_queue->enqueue(peer_message);
    });

    m_datachannel->onOpen([this]() {
      m_event_queue->enqueue<InternalNotification<std::string>>(
        InternalNotificationType::DataChannelOpened,
        m_identity
      );
    });
    m_datachannel->onClosed([this]() {
      m_event_queue->enqueue<InternalNotification<std::string>>(
        InternalNotificationType::DataChannelClosed,
        m_identity
      );
    });
  }

  bool PeerConnection::send_and_forget(Box<PeerMessage> message) const {

    if (m_datachannel == nullptr) return false;

    if (m_datachannel->isOpen() && !m_datachannel->isClosed()) {

      m_datachannel->send(message->to_string());
      return true;
    }
    return false;
  }

  void PeerConnection::set_remote_description(std::string sdp, std::string type_string) {

    m_peer_connection->setRemoteDescription(
      rtc::Description(
        sdp,
        type_string
      )
    );
  }

  void PeerConnection::on_local_candidate(rtc::Candidate candidate) {

    auto candidate_json = nlohmann::json();
    candidate_json["candidate"] = candidate.candidate();
    candidate_json["mid"] = candidate.mid();

    m_event_queue->enqueue<InternalMessage<SignalingMessage>>(
      InternalMessageType::SendSignalingMessage,
      SignalingMessage::create(
        SignalingMessageType::RTCCandidate,
        m_signaling_client->identity(),
        m_identity,
        SignalingMessage::create_tag(),
        candidate_json
      )

    );
  }

  void PeerConnection::on_local_description(rtc::Description description) {

    auto description_json = nlohmann::json();
    description_json["sdp"] = description.generateSdp();
    description_json["type"] = description.typeString();

    m_event_queue->enqueue<InternalMessage<SignalingMessage>>(
      InternalMessageType::SendSignalingMessage,
      SignalingMessage::create(
        SignalingMessageType::RTCSetup,
        m_signaling_client->identity(),
        m_identity,
        SignalingMessage::create_tag(),
        description_json
      )
    );
  }

  concurrencpp::result<PeerState> PeerConnection::state() {

    if (m_peer_state == PeerState::Unknown) {

      auto result =
        co_await m_event_queue->submit<InternalMessage<PeerMessage>>(
          InternalMessageType::SendPeerMessage,
          PeerMessage::create(
            PeerMessageType::PeerStateRequest,
            m_signaling_client->identity(),
            m_identity,
            PeerMessage::create_tag(),
            nlohmann::json()
          )
        );
      m_peer_state = result.get<PeerMessage>()->content().get<PeerState>();
    }

    co_return m_peer_state;
  }

  concurrencpp::result<PeerType> PeerConnection::type() {

    if (m_peer_type == PeerType::Unknown) {


      auto result =
        co_await m_event_queue->submit<InternalMessage<PeerMessage>>(
          InternalMessageType::SendPeerMessage,
          PeerMessage::create(
            PeerMessageType::PeerTypeRequest,
            m_signaling_client->identity(),
            m_identity,
            PeerMessage::create_tag(),
            nlohmann::json()
          )
        );

      m_peer_type = result.get<PeerMessage>()->content().get<PeerType>();
    }

    co_return m_peer_type;
  }

  void PeerConnection::on_peer_state_update(Event event) {

    auto message = event.get<PeerMessage>();

    auto peer_id = message->sender_identity();
    if (peer_id == m_identity) {

      auto new_state = message->content().get<PeerState>();
      spdlog::info(
        "Updated state of peer {} to {}",
        peer_id,
        to_string(new_state)
      );
    }
  }

  void PeerConnection::on_rtc_candidate(Event event) {

    auto message = event.get<SignalingMessage>();
    if (m_identity == message->sender_identity()) {

      auto candidate_sdp = message->content();
      auto candidate = candidate_sdp["candidate"].get<std::string>();
      auto mid = candidate_sdp["mid"].get<std::string>();
      m_peer_connection->addRemoteCandidate(rtc::Candidate(candidate, mid));
    }
  }

  PeerConnection::~PeerConnection() {
    m_datachannel->close();
  }
}// namespace krapi