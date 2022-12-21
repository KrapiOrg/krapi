//
// Created by mythi on 24/11/22.
//

#include "NodeManager.h"
#include "spdlog/spdlog.h"
#include "ErrorOr.h"

using namespace std::chrono_literals;

namespace krapi {

    NodeManager::NodeManager(
            PeerType pt
    ) : m_peer_state(PeerState::Closed),
        m_peer_type(pt),
        m_initialized(false),
        m_signaling_client(
                std::make_shared<SignalingClient>(
                        std::bind_front(&NodeManager::on_rtc_setup, this),
                        std::bind_front(&NodeManager::on_rtc_candidate, this)
                )
        ) {

    }

    void NodeManager::append_listener(
            PeerMessageType type,
            const std::function<void(PeerMessage)> &listener
    ) {


    }

    std::string NodeManager::id() const {

        return m_signaling_client->identity();
    }

    concurrencpp::result<void> NodeManager::send(
            std::string id,
            const PeerMessage &message
    ) {


    }

    concurrencpp::result<std::vector<std::string>> NodeManager::connect_to_peers() {

        auto available_peers_response = co_await m_signaling_client->send(
                SignalingMessage{
                        SignalingMessageType::AvailablePeersRequest,
                        m_signaling_client->identity(),
                        "signaling_server",
                        SignalingMessage::create_tag()
                }
        );
        auto available_peers = available_peers_response.content().get<std::vector<std::string >>();

        for (const auto &peer: available_peers) {

            auto peer_connection = std::make_shared<PeerConnection>(peer, m_signaling_client);
            co_await peer_connection->create_datachannel();

            m_connection_map.insert({peer, std::move(peer_connection)});
        }
        co_return available_peers;
    }

    PeerState NodeManager::get_state() const {

        return m_peer_state;
    }

    concurrencpp::result<void> NodeManager::initialize() {

        assert(!m_initialized && "Called NodeManager::initialize() more than once");
        co_await m_signaling_client->initialize();
        m_initialized = true;
    }

    void NodeManager::on_rtc_setup(SignalingMessage message) {

        auto type = message.content().get<std::string>();
        auto description = rtc::Description(type);

        std::shared_ptr<PeerConnection> peer_connection;
        {
            std::lock_guard l(m_connection_map_mutex);
            if (m_connection_map.contains(message.sender_identity())) {

                peer_connection = m_connection_map[message.sender_identity()];
            }
        }
        if (!peer_connection) {

            peer_connection = std::make_shared<PeerConnection>(
                    message.sender_identity(),
                    m_signaling_client,
                    message.content().get<std::string>()
            );

            std::lock_guard l(m_connection_map_mutex);
            m_connection_map.emplace(message.sender_identity(), std::move(peer_connection));
        } else {

            peer_connection->set_remote_description(message.content().get<std::string>());
        }
    }

    void NodeManager::on_rtc_candidate(const SignalingMessage &message) {

        std::lock_guard l(m_connection_map_mutex);
        m_connection_map.at(message.sender_identity())->add_remote_candidate(
                message.content().get<std::string>()
        );
    }

    NodeManager::~NodeManager() {

        m_blocking_bool.wait(false);
    }
} // krapi