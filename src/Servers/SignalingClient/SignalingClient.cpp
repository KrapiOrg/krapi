//
// Created by mythi on 02/12/22.
//

#include "SignalingClient.h"
#include "uuid.h"

namespace krapi {
    SignalingClient::SignalingClient() {

        m_identity = uuids::to_string(uuids::uuid_system_generator{}());
        spdlog::info("Assigned myself {}", m_identity);
        m_ws.setUrl("ws://127.0.0.1:8080");
        std::promise<void> signaling_open_barrier;
        m_ws.setOnMessageCallback(
                [&, this](const ix::WebSocketMessagePtr &message) {
                    if (message->type == ix::WebSocketMessageType::Open) {

                        signaling_open_barrier.set_value();
                    }
                    if (message->type == ix::WebSocketMessageType::Message) {

                        auto msg_json = nlohmann::json::parse(message->str);
                        auto msg = SignalingMessage::from_json(msg_json);

                        if (msg.type == SignalingMessageType::RTCSetup) {

                            m_rtc_setup_callback(msg);
                        } else {

                            std::lock_guard l(m_mutex);
                            m_promises[msg.tag].set_value(msg);
                        }
                    }
                }
        );
        m_ws.start();
        spdlog::info("SignalingClient: Waiting for signaling server...");
        signaling_open_barrier.get_future().wait();
        spdlog::info("SignalingClient: Sending my identity to signaling server...");
        send(
                SignalingMessage{
                        SignalingMessageType::SetIdentityRequest,
                        SignalingMessage::create_tag(),
                        m_identity
                }
        ).get();
        spdlog::info("SignalingClient: Server Acknowledged my identity...");
    }

    std::string SignalingClient::get_identity() {

        return m_identity;
    }

    std::future<SignalingMessage> SignalingClient::send(SignalingMessage message) {

        m_ws.send(message.to_string());

        std::lock_guard l(m_mutex);
        m_promises[message.tag] = std::promise<SignalingMessage>{};
        return m_promises[message.tag].get_future();
    }

    std::future<SignalingMessage> SignalingClient::send(SignalingMessageType message_type) {

        auto message = SignalingMessage{
                message_type
        };
        m_ws.send(message.to_string());

        std::lock_guard l(m_mutex);
        m_promises[message.tag] = std::promise<SignalingMessage>{};
        return m_promises[message.tag].get_future();
    }
} // krapi