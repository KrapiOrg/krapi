//
// Created by mythi on 24/11/22.
//

#pragma once

#include <utility>
#include "rtc/peerconnection.hpp"
#include "eventpp/eventdispatcher.h"
#include "concurrencpp/concurrencpp.h"

#include "PeerConnection.h"
#include "PeerState.h"
#include "PeerType.h"
#include "SignalingMessage.h"
#include "SignalingClient.h"
#include "PeerMessage.h"
#include "PeerConnection.h"

namespace krapi {

    class [[nodiscard]] NodeManager final {

    public:

        [[nodiscard]]
        static inline concurrencpp::result<std::shared_ptr<NodeManager>> create(
                EventQueuePtr event_queue,
                PeerType pt
        ) {

            auto manager = std::shared_ptr<NodeManager>(new NodeManager(std::move(event_queue), pt));

            co_await manager->m_signaling_client->initialize();
            co_await manager->connect_to_peers();

            for (const auto &[peer_id, connection]: manager->m_connection_map) {
                auto state = co_await connection->state();
                auto type = co_await connection->type();
                spdlog::info("{}: {} {}", peer_id, to_string(type), to_string(state));
            }
            co_return manager;
        }

        void set_state(PeerState state);

        [[nodiscard]]
        PeerState get_state() const;

        [[nodiscard]]
        std::string id() const;

        concurrencpp::shared_result<Event> send(Box<PeerMessage>);

        void send_and_forget(Box<PeerMessage>);

        std::vector<concurrencpp::shared_result<Event>> broadcast(PeerMessageType, nlohmann::json = {});

        void broadcast_and_forget(PeerMessageType, nlohmann::json = {});

    private:

        explicit NodeManager(
                EventQueuePtr,
                PeerType pt
        );

        EventQueuePtr m_event_queue;
        std::unique_ptr<SignalingClient> m_signaling_client;
        std::unordered_map<std::string, std::shared_ptr<PeerConnection>> m_connection_map;
        eventpp::ScopedRemover<EventQueueType> m_subscription_remover;

        PeerState m_peer_state;
        PeerType m_peer_type;

        void on_rtc_setup(Event);

        void on_rtc_candidate(Event);

        void on_peer_state_request(Event);

        void on_peer_type_request(Event);

        void on_send_peer_message(Event);

        concurrencpp::result<void> on_datachannel_opened(Event);

        void on_datachannel_closed(Event);

        void on_peer_closed(Event);

        [[nodiscard]]
        concurrencpp::result<void> connect_to_peers();
    };

} // krapi
