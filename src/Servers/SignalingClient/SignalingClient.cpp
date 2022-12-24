//
// Created by mythi on 02/12/22.
//

#include "SignalingClient.h"

#include <utility>
#include "uuid.h"

namespace krapi {
    SignalingClient::SignalingClient(
            NotNull<EventQueue *> event_queue
    ) :
            m_event_queue(event_queue),
            m_identity(uuids::to_string(uuids::uuid_system_generator{}())),
            m_ws(std::make_unique<rtc::WebSocket>()),
            m_initialized(false) {
    }

    concurrencpp::result<void> SignalingClient::wait_for_open() const {

        m_ws->open("ws://127.0.0.1:8080");
        auto open_promise = std::make_shared<concurrencpp::result_promise<void>>();
        m_ws->onOpen([=]() { open_promise->set_result(); });
        return open_promise->get_result();
    }

    concurrencpp::result<void> SignalingClient::wait_for_identity_request() const {

        auto promise = std::make_shared<concurrencpp::result_promise<void>>();

        m_ws->onMessage(
                [=](rtc::message_variant message) {
                    auto message_str = std::get<std::string>(message);
                    auto message_json = nlohmann::json::parse(message_str);
                    auto signaling_message = SignalingMessage::from_json(message_json);
                    if (signaling_message->type() == SignalingMessageType::IdentityRequest)
                        promise->set_result();
                }
        );
        return promise->get_result();
    }

    concurrencpp::result<void> SignalingClient::send_identity() const {

        auto promise = std::make_shared<concurrencpp::result_promise<void>>();
        m_ws->onMessage(
                [=](rtc::message_variant message) {
                    auto message_str = std::get<std::string>(message);
                    auto message_json = nlohmann::json::parse(message_str);
                    auto signaling_message = SignalingMessage::from_json(message_json);
                    if (signaling_message->type() == SignalingMessageType::Acknowledgement)
                        promise->set_result();
                }
        );
        m_ws->send(
                SignalingMessage{
                        SignalingMessageType::IdentityResponse,
                        m_identity,
                        "signaling_server",
                        SignalingMessage::create_tag(),
                        m_identity
                }.to_string()
        );
        return promise->get_result();
    }


    concurrencpp::result<void> SignalingClient::initialize() {

        assert(!m_initialized && "Tried to call initialize more than once");
        spdlog::info("SignalingClient: Waiting for server");
        co_await wait_for_open();
        spdlog::info("SignalingClient: Connected to server");
        co_await wait_for_identity_request();
        spdlog::info("SignalingClient: Sending identity {}", m_identity);
        co_await send_identity();
        spdlog::info("SignalingClient: identity acknowledged");
        m_ws->onMessage(
                [this](rtc::message_variant message) {
                    auto message_str = std::get<std::string>(message);
                    auto message_json = nlohmann::json::parse(message_str);
                    auto signaling_message = SignalingMessage::from_json(message_json);
                    spdlog::info("{} from {}", message_json["type"], signaling_message->sender_identity());
                    m_event_queue->enqueue(signaling_message->type(), signaling_message);
                }
        );
        m_initialized = true;
    }

    std::string SignalingClient::identity() const noexcept {

        return m_identity;
    }

    concurrencpp::result<Event> SignalingClient::available_peers() const {

        auto tag = SignalingMessage::create_tag();
        auto awaitable = m_event_queue->create_awaitable(tag);

        m_ws->send(
                SignalingMessage(
                        SignalingMessageType::AvailablePeersRequest,
                        m_identity,
                        "signaling_server",
                        tag
                ).to_string()
        );
        return awaitable;
    }

    concurrencpp::result<Event> SignalingClient::send(Box<SignalingMessage> message) const {

        assert(m_initialized && "start() was not called on Signaling Client");

        auto awaitable = m_event_queue->create_awaitable(message->tag());
        m_ws->send(message->to_string());
        return awaitable;
    }

    void SignalingClient::send_and_forget(Box<SignalingMessage> message) const {

        assert(m_initialized && "start() was not called on Signaling Client");

        spdlog::info("sending {} to {}", message->to_json()["type"], message->receiver_identity());
        m_ws->send(message->to_string());
    }
} // krapi