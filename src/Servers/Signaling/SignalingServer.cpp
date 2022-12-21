//
// Created by mythi on 17/12/22.
//

#include "range/v3/view.hpp"
#include "spdlog/spdlog.h"

#include "SignalingServer.h"

namespace krapi {
    std::weak_ptr<SignalingSocket> SignalingServer::get_socket(const std::string &identity) const {

        std::lock_guard l(m_mutex);
        if (m_sockets.contains(identity))
            return m_sockets.at(identity);

        return {};
    }

    std::vector<std::string> SignalingServer::get_identities(std::string_view identity_to_filter_for) const {

        std::lock_guard l(m_mutex);
        return m_sockets
               | ranges::views::keys
               | ranges::views::filter([=](std::string_view identity) { return identity != identity_to_filter_for; })
               | ranges::to<std::vector<std::string>>();
    }

    void SignalingServer::on_client_message(const SignalingMessage &message) const {

        if (message.type() == SignalingMessageType::AvailablePeersRequest) {
            auto wsocket = get_socket(message.sender_identity());
            if (auto socket = wsocket.lock()) {

                socket->send(
                        SignalingMessage{
                                SignalingMessageType::AvailablePeersResponse,
                                "signaling_server",
                                message.sender_identity(),
                                message.tag(),
                                get_identities(message.sender_identity())
                        }
                );
            }
        } else if (
                message.type() == SignalingMessageType::RTCSetup ||
                message.type() == SignalingMessageType::RTCCandidate) {
            spdlog::info("{}", message.to_string());
            if (auto socket = get_socket(message.receiver_identity()).lock()) {
                socket->send(message);
            }
        }
    }

    void SignalingServer::on_client_closed(const std::string &identity) {

        std::lock_guard l(m_mutex);
        if (m_sockets.erase(identity)) {
            spdlog::info("connection with {} closed", identity);
        }
    }

    SignalingServer::SignalingServer() :
            m_blocking_bool(false) {

        m_server.onClient(
                [this](std::shared_ptr<rtc::WebSocket> ws) {
                    auto signaling_socket = std::make_shared<SignalingSocket>(
                            std::move(ws)
                    );
                    auto identity = signaling_socket->initialize(
                            std::bind_front(&SignalingServer::on_client_message, this),
                            std::bind_front(&SignalingServer::on_client_closed, this)
                    ).get();
                    std::lock_guard l(m_mutex);
                    m_sockets.emplace(identity, signaling_socket);
                }
        );
    }

    int SignalingServer::port() const {

        return m_server.port();
    }

    SignalingServer::~SignalingServer() {

        m_blocking_bool.wait(false);
        m_server.stop();
    }
} // krapi