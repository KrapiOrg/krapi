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
        static inline std::shared_ptr<NodeManager> create(
                EventQueuePtr event_queue,
                SignalingClientPtr signaling_client,
                PeerType pt
        ) {

            return std::shared_ptr<NodeManager>(new NodeManager(std::move(event_queue), std::move(signaling_client) ,pt));
        }

        [[nodiscard]]
        concurrencpp::result<void> connect_to_peers();

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
                SignalingClientPtr,
                PeerType pt
        );

        EventQueuePtr m_event_queue;
        SignalingClientPtr m_signaling_client;
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
    };

} // krapi
