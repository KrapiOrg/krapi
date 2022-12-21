//
// Created by mythi on 02/12/22.
//

#include "SignalingClient.h"

#include <utility>
#include "uuid.h"

namespace krapi {
    SignalingClient::SignalingClient(
            const RTCMessageCallback &rtc_setup_callback,
            const RTCMessageCallback &rtc_candidate_callback
    ) :
            m_identity(uuids::to_string(uuids::uuid_system_generator{}())),
            m_ws(std::make_unique<rtc::WebSocket>()),
            m_initialized(false) {

        m_dispatcher.appendListener(SignalingMessageType::RTCSetup, rtc_setup_callback);
        m_dispatcher.appendListener(SignalingMessageType::RTCCandidate, rtc_candidate_callback);
    }

    concurrencpp::result<void> SignalingClient::wait_for_open() {

        m_ws->open("ws://127.0.0.1:8080");
        auto open_promise = std::make_shared<concurrencpp::result_promise<void>>();
        m_ws->onOpen([=]() { open_promise->set_result(); });
        return open_promise->get_result();
    }

    concurrencpp::result<void> SignalingClient::wait_for_identity_request() {

        auto promise = std::make_shared<concurrencpp::result_promise<void>>();

        m_ws->onMessage(
                [=](rtc::message_variant message) {
                    auto message_str = std::get<std::string>(message);
                    auto message_json = nlohmann::json::parse(message_str);
                    auto signaling_message = SignalingMessage::from_json(message_json);
                    if (signaling_message.type() == SignalingMessageType::IdentityRequest)
                        promise->set_result();
                }
        );
        return promise->get_result();
    }

    concurrencpp::result<void> SignalingClient::send_identity() {

        auto promise = std::make_shared<concurrencpp::result_promise<void>>();
        m_ws->onMessage(
                [=](rtc::message_variant message) {
                    auto message_str = std::get<std::string>(message);
                    auto message_json = nlohmann::json::parse(message_str);
                    auto signaling_message = SignalingMessage::from_json(message_json);
                    if (signaling_message.type() == SignalingMessageType::Acknowledgement)
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
        m_ws->onMessage([this](rtc::message_variant message) {
            auto message_str = std::get<std::string>(message);
            auto message_json = nlohmann::json::parse(message_str);
            auto signaling_message = SignalingMessage::from_json(message_json);

            bool contains_tag;
            SignalingMessageType type;
            {
                std::lock_guard l(m_promises_mutex);
                contains_tag = m_promises.contains(signaling_message.tag());
                type = signaling_message.type();
            }
            if (contains_tag)
                m_promises[signaling_message.tag()].set_result(signaling_message);
            else
                m_dispatcher.dispatch(signaling_message.type(), signaling_message);

        });
        m_initialized = true;
    }

    std::string SignalingClient::identity() const noexcept {

        return m_identity;
    }

    concurrencpp::result<SignalingMessage> SignalingClient::send(SignalingMessage message) {

        assert(m_initialized && "start() was not called on Signaling Client");
        {
            std::lock_guard l(m_promises_mutex);
            m_promises.emplace(message.tag(), concurrencpp::result_promise<SignalingMessage>{});
        }
        m_ws->send(message.to_string());
        co_return co_await m_promises[message.tag()].get_result();
    }

    void SignalingClient::send_async(const SignalingMessage &message) {

        assert(m_initialized && "start() was not called on Signaling Client");
        m_ws->send(message.to_string());
    }
} // krapi