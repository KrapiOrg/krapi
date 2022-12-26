//
// Created by mythi on 24/11/22.
//

#include "NodeManager.h"
#include "spdlog/spdlog.h"
#include "ErrorOr.h"
#include "range/v3/view.hpp"

using namespace std::chrono_literals;


namespace krapi {

    NodeManager::NodeManager(
            std::shared_ptr<concurrencpp::worker_thread_executor> worker,
            PeerType pt
    ) :
            m_blocking_bool(false),
            m_worker(std::move(worker)),
            m_peer_state(PeerState::Closed),
            m_peer_type(pt),
            m_initialized(false) {

    }

    std::string NodeManager::id() const {

        return m_signaling_client->identity();
    }

    concurrencpp::shared_result<Event> NodeManager::send(Box<PeerMessage> message) {

        return m_event_queue->submit<InternalMessage>(
                InternalMessageType::SendPeerMessage,
                message->to_json()
        );
    }

    void NodeManager::send_and_forget(Box<PeerMessage> message) {

        m_event_queue->enqueue(
                InternalMessage::create(
                        InternalMessageType::SendPeerMessage,
                        message->to_json()
                )
        );
    }

    concurrencpp::result<void> NodeManager::connect_to_peers() {

        auto available_peers_response = co_await m_signaling_client->available_peers();

        auto available_peers = available_peers_response
                .get<SignalingMessage>()
                ->content()
                .get<std::vector<std::string>>();
        std::vector<concurrencpp::shared_result<Event>> datachannel_open_awaitables;

        for (const auto &peer: available_peers) {

            datachannel_open_awaitables.push_back(m_event_queue->create_awaitable(peer));

            auto peer_connection = PeerConnection::create(
                    make_not_null(m_event_queue.get()),
                    make_not_null(m_signaling_client.get()),
                    peer
            );

            m_connection_map.emplace(peer, std::move(peer_connection));
        }

        for (auto &event: datachannel_open_awaitables) {
            co_await event;
        }
        spdlog::info("Created connections to : [{}]", fmt::join(available_peers, ", "));
    }

    PeerState NodeManager::get_state() const {

        return m_peer_state;
    }

    concurrencpp::result<void> NodeManager::initialize(std::shared_ptr<concurrencpp::timer_queue> timer_queue) {

        assert(!m_initialized && "Called NodeManager::initialize() more than once");

        m_event_queue = EventQueue::create();
        m_signaling_client = SignalingClient::create(make_not_null(m_event_queue.get()));
        m_event_loop = timer_queue->make_timer(
                0ms,
                30ms,
                m_worker,
                [this]() {
                    m_event_queue->wait();
                    m_event_queue->process_one();
                }
        );

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

        m_event_queue->append_listener(
                InternalMessageType::SendPeerMessage,
                std::bind_front(&NodeManager::on_send_peer_message, this)
        );

        m_event_queue->append_listener(
                InternalMessageType::DataChannelOpened,
                std::bind_front(&NodeManager::on_datachannel_opened, this)
        );

        m_event_queue->append_listener(
                InternalMessageType::DataChannelClosed,
                std::bind_front(&NodeManager::on_datachannel_closed, this)
        );

        m_event_queue->append_listener(
                SignalingMessageType::PeerClosed,
                std::bind_front(&NodeManager::on_peer_closed, this)
        );

        m_event_queue->append_listener(
                InternalMessageType::SignalingServerClosed,
                std::bind_front(&NodeManager::on_signaling_server_closed, this)
        );

        m_initialized = true;
        co_await m_signaling_client->initialize();
        co_await connect_to_peers();

        for (const auto &[peer_id, connection]: m_connection_map) {
            auto state = co_await connection->state();
            auto type = co_await connection->type();
            spdlog::info("{}: {} {}", peer_id, to_string(type), to_string(state));
        }
    }

    void NodeManager::on_rtc_setup(Event event) {

        auto message = event.get<SignalingMessage>();

        if (m_connection_map.contains(message->sender_identity())) {

            auto description = message->content().get<std::string>();
            m_connection_map[message->sender_identity()]->set_remote_description(description);
        } else {

            m_connection_map.emplace(
                    message->sender_identity(),
                    PeerConnection::create(
                            make_not_null(m_event_queue.get()),
                            make_not_null(m_signaling_client.get()),
                            message->sender_identity(),
                            message->content().get<std::string>()
                    )
            );
        }

    }

    void NodeManager::on_rtc_candidate(Event event) {

        auto message = event.get<SignalingMessage>();
        auto spd = message->content().get<std::string>();
        m_connection_map.at(message->sender_identity())->add_remote_candidate(spd);
    }

    void NodeManager::on_peer_state_request(Event event) {

        auto message = event.get<PeerMessage>();

        m_event_queue->enqueue<InternalMessage>(
                InternalMessageType::SendPeerMessage,
                PeerMessage(
                        PeerMessageType::PeerStateResponse,
                        m_signaling_client->identity(),
                        message->sender_identity(),
                        message->tag(),
                        m_peer_state
                ).to_json()
        );
    }

    void NodeManager::on_peer_type_request(Event event) {

        auto message = event.get<PeerMessage>();
        m_event_queue->enqueue<InternalMessage>(
                InternalMessageType::SendPeerMessage,
                PeerMessage(
                        PeerMessageType::PeerTypeResponse,
                        m_signaling_client->identity(),
                        message->sender_identity(),
                        message->tag(),
                        m_peer_type
                ).to_json()
        );
    }

    void NodeManager::on_send_peer_message(Event event) {

        auto internal_message = event.get<InternalMessage>();
        auto message_json = internal_message->content();
        auto message = PeerMessage::from_json(message_json);
        auto message_str = message_json.dump();

        if (!m_connection_map.contains(message->receiver_identity()))
            return;

        if (!m_connection_map[message->receiver_identity()]->send_and_forget(message)) {
            spdlog::error(
                    "{} to {} was not sent, re-enqueing",
                    to_string(message->type()),
                    message->receiver_identity()
            );
            m_event_queue->enqueue<InternalMessage>(
                    InternalMessageType::SendPeerMessage,
                    message_json
            );
        }
    }

    std::vector<concurrencpp::shared_result<Event>> NodeManager::broadcast(
            PeerMessageType type,
            nlohmann::json content
    ) {

        std::vector<concurrencpp::shared_result<Event>> results;

        for (const auto &peer: m_connection_map | ranges::views::keys) {
            results.push_back(
                    send(
                            PeerMessage::create(
                                    type,
                                    m_signaling_client->identity(),
                                    peer,
                                    PeerMessage::create_tag(),
                                    content
                            )
                    )
            );
        }

        return results;
    }

    void NodeManager::broadcast_and_forget(
            PeerMessageType type,
            nlohmann::json content
    ) {

        for (const auto &peer: m_connection_map | ranges::views::keys) {

            send_and_forget(
                    PeerMessage::create(
                            type,
                            m_signaling_client->identity(),
                            peer,
                            PeerMessage::create_tag(),
                            content
                    )
            );
        }
    }

    concurrencpp::result<void> NodeManager::on_datachannel_opened(Event event) {

        auto id = event.get<InternalMessage>()->content().get<std::string>();
        spdlog::info("DataChannel with {} opened", id);
        auto connection = m_connection_map[id];
        auto state = co_await connection->state();
        auto type = co_await connection->type();
        spdlog::info("{}: {} {}", id, to_string(type), to_string(state));
    }

    void NodeManager::on_datachannel_closed(Event event) {

        auto id = event.get<InternalMessage>()->content().get<std::string>();
        spdlog::warn("DataChannel with {} closed", id);
    }

    void NodeManager::on_peer_closed(Event event) {

        auto id = event.get<SignalingMessage>()->content().get<std::string>();
        spdlog::info("PeerConnection with {} closed", id);
        m_connection_map.erase(id);
    }

    void NodeManager::on_signaling_server_closed(Event event) {

        spdlog::info("NodeManager: signaling server closed");
        m_event_queue->close();
        m_event_loop.cancel();
        m_blocking_bool = true;
        m_blocking_bool.notify_one();
    }

    void NodeManager::set_state(PeerState state) {

        m_peer_state = state;
    }

    NodeManager::~NodeManager() {

        m_blocking_bool.wait(false);
    }
} // krapi