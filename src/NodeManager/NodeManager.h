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

    class NodeManager {
    protected:

        std::atomic<bool> m_blocking_bool;
        std::shared_ptr<concurrencpp::worker_thread_executor> m_worker;
        EventQueuePtr m_event_queue;
        concurrencpp::timer m_event_loop;
        std::unique_ptr<SignalingClient> m_signaling_client;
        std::unordered_map<std::string, std::shared_ptr<PeerConnection>> m_connection_map;
        eventpp::ScopedRemover<EventQueueType> m_subscription_remover;

        std::atomic<PeerState> m_peer_state;
        PeerType m_peer_type;
        bool m_initialized;

        void on_rtc_setup(Event);

        void on_rtc_candidate(Event);

        void on_peer_state_request(Event);

        void on_peer_type_request(Event);

        void on_peer_state_update(Event);

        void on_send_peer_message(Event);

        concurrencpp::result<void> on_datachannel_opened(Event);

        void on_datachannel_closed(Event);

        void on_peer_closed(Event);

        void on_signaling_server_closed(Event);

        [[nodiscard]]
        concurrencpp::result<void> connect_to_peers();

    public:

        explicit NodeManager(
                std::shared_ptr<concurrencpp::worker_thread_executor> worker,
                PeerType pt
        );

        void set_state(PeerState state);

        [[nodiscard]]
        PeerState get_state() const;

        [[nodiscard]]
        std::string id() const;

        [[nodiscard]]
        concurrencpp::result<void> initialize(std::shared_ptr<concurrencpp::timer_queue>);

        concurrencpp::shared_result<Event> send(Box<PeerMessage>);

        void send_and_forget(Box<PeerMessage>);

        std::vector<concurrencpp::shared_result<Event>> broadcast(PeerMessageType, nlohmann::json = {});

        void broadcast_and_forget(PeerMessageType, nlohmann::json = {});

        ~NodeManager();
    };

} // krapi
