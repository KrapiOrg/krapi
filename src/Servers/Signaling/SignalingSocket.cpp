//
// Created by mythi on 17/12/22.
//

#include "SignalingSocket.h"

namespace krapi {

    concurrencpp::result<void> SignalingSocket::wait_for_open() {

        auto open_promise = std::make_shared<concurrencpp::result_promise<void>>();
        m_socket->onOpen([=]() { open_promise->set_result(); });
        co_await open_promise->get_result();
    }

    concurrencpp::result<std::string> SignalingSocket::wait_for_identity() {

        auto identity_promise = std::make_shared<concurrencpp::result_promise<std::string>>();
        m_socket->onMessage(
                [=](rtc::message_variant msg) {

                    auto msg_str = std::get<std::string>(msg);
                    auto msg_json = nlohmann::json::parse(msg_str);
                    auto signaling_msg = SignalingMessage::from_json(msg_json);
                    identity_promise->set_result(signaling_msg.content().get<std::string>());
                }
        );
        m_socket->send(
                SignalingMessage{
                        SignalingMessageType::IdentityRequest,
                        "signaling_server",
                        "",
                        SignalingMessage::create_tag(),
                }.to_string()
        );
        co_return co_await identity_promise->get_result();
    }

    void SignalingSocket::send_identity_acknowledgement() {

        m_socket->send(
                SignalingMessage{
                        SignalingMessageType::Acknowledgement,
                        "signaling_server",
                        m_identity,
                        SignalingMessage::create_tag()
                }.to_string()
        );
    }

    SignalingSocket::SignalingSocket(std::shared_ptr<rtc::WebSocket> socket) :
            m_socket(std::move(socket)),
            m_initialized(false) {

    }

    concurrencpp::result<std::string> SignalingSocket::initialize(std::function<void(SignalingMessage)> on_message,
                                                                  std::function<void(std::string)> on_closed) {

        assert(!m_initialized && "Tried to call SignalingSocket::initialize() more than once");
        m_on_message = std::move(on_message);

        co_await wait_for_open();
        m_identity = co_await wait_for_identity();
        send_identity_acknowledgement();
        spdlog::info("{} has connected", m_identity);

        m_socket->onMessage(
                [this](rtc::message_variant msg) {

                    auto msg_str = std::get<std::string>(msg);
                    auto msg_json = nlohmann::json::parse(msg_str);
                    auto signaling_msg = SignalingMessage::from_json(msg_json);

                    m_on_message(signaling_msg);
                }
        );
        m_socket->onClosed([on_closed = std::move(on_closed), this]() { on_closed(m_identity); });
        m_initialized = true;
        co_return m_identity;
    }

    void SignalingSocket::send(const SignalingMessage &message) {

        assert(m_initialized && "SignalingSocket::initialized() has not been called");
        m_socket->send(message.to_string());
    }

    std::string SignalingSocket::identity() const {

        return m_identity;
    }
}