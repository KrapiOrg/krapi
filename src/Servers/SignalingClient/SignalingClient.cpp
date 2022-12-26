//
// Created by mythi on 02/12/22.
//

#include "SignalingClient.h"
#include "InternalMessage.h"

#include <utility>

using namespace std::chrono_literals;

#include "uuid.h"

namespace krapi {
    SignalingClient::SignalingClient(
            NotNull<EventQueue *> event_queue
    ) :
            m_event_queue(event_queue),
            m_ws(std::make_unique<rtc::WebSocket>()) {
    }

    concurrencpp::result<void> SignalingClient::for_open() const {

        auto promise = std::make_shared<concurrencpp::result_promise<void>>();
        m_ws->onOpen([promise]() { promise->set_result(); });
        m_ws->open("ws://127.0.0.1:8080");

        return promise->get_result();
    }

    concurrencpp::shared_result<std::string> SignalingClient::request_identity() const {

        auto promise = std::make_shared<concurrencpp::result_promise<std::string>>();
        m_ws->onMessage(
                [promise](rtc::message_variant message) {
                    auto message_str = std::get<std::string>(message);
                    auto message_json = nlohmann::json::parse(message_str);
                    auto signaling_message = SignalingMessage::from_json(message_json);
                    if (signaling_message->type() == SignalingMessageType::IdentityResponse)
                        promise->set_result(signaling_message->content().get<std::string>());
                }
        );

        m_ws->send(
                SignalingMessage(
                        SignalingMessageType::IdentityRequest,
                        "unknown_identity",
                        "signaling_server",
                        SignalingMessage::create_tag()
                ).to_string()
        );

        return promise->get_result();
    }


    concurrencpp::result<void> SignalingClient::initialize() {

        co_await for_open();
        spdlog::info("SignalingClient: connected to signaling server {}", *m_ws->remoteAddress());
        m_identity = co_await request_identity();
        spdlog::info("SignalingClient: identity {}", m_identity);

        m_ws->onMessage(
                [this](rtc::message_variant message) {

                    auto message_str = std::get<std::string>(message);
                    auto message_json = nlohmann::json::parse(message_str);
                    auto signaling_message = SignalingMessage::from_json(message_json);
                    m_event_queue->enqueue(signaling_message);
                }
        );

        m_event_queue->append_listener(
                InternalMessageType::SendSignalingMessage,
                [this](Event event) {

                    auto internal_message = event.get<InternalMessage>();
                    auto message_json = internal_message->content();
                    auto message_str = message_json.dump();
                    m_ws->send(message_str);
                }
        );

        m_ws->onClosed(
                [this]() {
                    m_event_queue->enqueue<InternalMessage>(InternalMessageType::SignalingServerClosed);
                }
        );


    }

    std::string SignalingClient::identity() const noexcept {

        return m_identity;
    }

    concurrencpp::shared_result<Event> SignalingClient::available_peers() const {

        return send(
                SignalingMessage::create(
                        SignalingMessageType::AvailablePeersRequest,
                        m_identity,
                        "signaling_server",
                        SignalingMessage::create_tag()
                )
        );
    }

    concurrencpp::shared_result<Event> SignalingClient::send(Box<SignalingMessage> message) const {

        return m_event_queue->submit<InternalMessage>(
                InternalMessageType::SendSignalingMessage,
                message->to_json()
        );
    }

    void SignalingClient::send_and_forget(Box<SignalingMessage> message) const {

        m_event_queue->enqueue<InternalMessage>(
                        InternalMessageType::SendSignalingMessage,
                        message->to_json()
        );
    }
} // krapi