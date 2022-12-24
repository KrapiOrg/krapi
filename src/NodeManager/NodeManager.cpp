//
// Created by mythi on 24/11/22.
//

#include "NodeManager.h"
#include "spdlog/spdlog.h"
#include "ErrorOr.h"

using namespace std::chrono_literals;

namespace krapi {

    NodeManager::NodeManager(
            NotNull<EventQueue *> event_queue,
            PeerType pt
    ) :
            m_blocking_bool(false),
            m_signaling_client(std::make_unique<SignalingClient>(event_queue)),
            m_event_queue(event_queue),
            m_peer_state(PeerState::Closed),
            m_peer_type(pt),
            m_initialized(false) {

    }

    std::string NodeManager::id() const {

        return m_signaling_client->identity();
    }

    concurrencpp::result<void> NodeManager::send(
            std::string id,
            Box<PeerMessage> message
    ) {


    }

    void NodeManager::send_and_forget(
            std::string id,
            Box<PeerMessage> message
    ) {


    }

    concurrencpp::result<std::vector<std::string>> NodeManager::connect_to_peers() {

        assert(m_initialized && "NodeManager::initialize() was not called");
        auto available_peers_response = co_await m_signaling_client->available_peers();

        auto available_peers = available_peers_response
                .get<SignalingMessage>()
                ->content()
                .get<std::vector<std::string>>();

        for (const auto &peer: available_peers) {

            spdlog::info("Creating connection to {}", peer);
            auto peer_connection = PeerConnection::create(
                    m_event_queue,
                    make_not_null(m_signaling_client.get()),
                    peer
            );
            spdlog::info("Creating Channel to {}", peer);
            co_await peer_connection->create_datachannel();
            spdlog::info("Channel with {} created", peer);
            m_connection_map.emplace(peer, std::move(peer_connection));
        }
        co_return available_peers;
    }

    PeerState NodeManager::get_state() const {

        return m_peer_state;
    }

    concurrencpp::result<void> NodeManager::initialize() {

        assert(!m_initialized && "Called NodeManager::initialize() more than once");
        m_event_queue->append_listener(
                SignalingMessageType::RTCCandidate,
                std::bind_front(&NodeManager::on_rtc_candidate, this)
        );

        m_event_queue->append_listener(
                SignalingMessageType::RTCSetup,
                std::bind_front(&NodeManager::on_rtc_setup, this)
        );

        m_event_queue->append_listener(
                PeerMessageType::PeerTypeRequest,
                std::bind_front(&NodeManager::on_peer_type_request, this)
        );

        m_event_queue->append_listener(
                PeerMessageType::PeerStateRequest,
                std::bind_front(&NodeManager::on_peer_state_request, this)
        );

        co_await m_signaling_client->initialize();
        m_initialized = true;
    }

    void NodeManager::on_rtc_setup(Event event) {

        assert(m_initialized && "NodeManager::initialize() was not called");
        auto message = event.get<SignalingMessage>();

        if (m_connection_map.contains(message->sender_identity())) {

            auto description = message->content().get<std::string>();
            m_connection_map[message->sender_identity()]->set_remote_description(description);
        } else {

            m_connection_map.emplace(
                    message->sender_identity(),
                    PeerConnection::create(
                            m_event_queue,
                            make_not_null(m_signaling_client.get()),
                            message->sender_identity(),
                            message->content().get<std::string>()
                    )
            );
        }

    }

    void NodeManager::on_rtc_candidate(Event event) {

        assert(m_initialized && "NodeManager::initialize() was not called");
        auto message = event.get<SignalingMessage>();
        auto spd = message->content().get<std::string>();
        m_connection_map.at(message->sender_identity())->add_remote_candidate(spd);
    }

    void NodeManager::on_peer_state_request(Event event) {

        assert(m_initialized && "NodeManager::initialize() was not called");
        auto message = event.get<PeerMessage>();
        send_and_forget(
                message->sender_identity(),
                PeerMessage::create(
                        PeerMessageType::PeerStateResponse,
                        m_signaling_client->identity(),
                        message->sender_identity(),
                        message->tag(),
                        PeerState::Open
                )
        );
    }

    void NodeManager::on_peer_type_request(Event event) {

        assert(m_initialized && "NodeManager::initialize() was not called");
        auto message = event.get<PeerMessage>();
        send_and_forget(
                message->sender_identity(),
                PeerMessage::create(
                        PeerMessageType::PeerTypeResponse,
                        m_signaling_client->identity(),
                        message->sender_identity(),
                        message->tag(),
                        PeerType::Full
                )
        );
    }

    NodeManager::~NodeManager() {

        m_blocking_bool.wait(false);
    }
} // krapi