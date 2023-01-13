#include "SignalingClient.h"
#include "InternalMessage.h"
#include "InternalNotification.h"
#include "SignalingMessage.h"
#include "nlohmann/json_fwd.hpp"
#include "spdlog/spdlog.h"

#include <thread>
#include <utility>

using namespace std::chrono_literals;

#include "uuid.h"

namespace krapi {
  SignalingClient::SignalingClient(
    std::string identity,
    EventQueuePtr event_queue
  )
      : m_event_queue(event_queue),
        m_ws(std::make_unique<rtc::WebSocket>()),
        m_identity(std::move(identity)),
        m_subscription_remover(event_queue->internal_queue()) {
  }

  concurrencpp::result<void> SignalingClient::initialize() {

    m_ws->onOpen([this]() {
      m_event_queue->enqueue<InternalNotification<void>>(
        InternalNotificationType::SignalingServerOpened
      );
    });

    m_ws->onMessage([this](rtc::message_variant message) {
      auto message_str = std::get<std::string>(message);
      auto message_json = nlohmann::json::parse(message_str);
      auto signaling_message = SignalingMessage::from_json(message_json);
      m_event_queue->enqueue(signaling_message);
    });

    m_subscription_remover.appendListener(
      InternalMessageType::SendSignalingMessage,
      [this](Event event) {
        auto internal_message = event.get<InternalMessage<SignalingMessage>>();
        auto content = internal_message->content();
        content->set_sender_identity(m_identity);
        m_ws->send(content->to_string());
      }
    );

    m_ws->onClosed([this]() {
      m_event_queue->enqueue<InternalNotification<void>>(
        InternalNotificationType::SignalingServerClosed
      );
    });

    m_ws->open("ws://127.0.0.1:8080");

    co_await m_event_queue->event_of_type(InternalNotificationType::SignalingServerOpened);

    spdlog::info(
      "SignalingClient: connected to signaling server {}",
      *m_ws->remoteAddress()
    );

    co_await m_event_queue->submit<InternalMessage<SignalingMessage>>(
      InternalMessageType::SendSignalingMessage,
      SignalingMessageType::SetIdentityRequest,
      m_identity,
      "signaling_server",
      SignalingMessage::create_tag(),
      nlohmann::json(m_identity)
    );

    spdlog::info("SignalingClient: identity {}", m_identity);
  }

  std::string SignalingClient::identity() const noexcept {
    return m_identity;
  }
}// namespace krapi