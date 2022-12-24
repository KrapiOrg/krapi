//
// Created by mythi on 24/11/22.
//

#pragma once

#include <set>
#include <utility>
#include "ixwebsocket/IXWebSocket.h"
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

    class NodeManager : public std::enable_shared_from_this<NodeManager> {
    protected:

        std::atomic<bool> m_blocking_bool;

        std::unique_ptr<SignalingClient> m_signaling_client;

        NotNull<EventQueue *> m_event_queue;
        std::unordered_map<std::string, std::shared_ptr<PeerConnection>> m_connection_map;

        PeerState m_peer_state;
        PeerType m_peer_type;
        bool m_initialized;

    public:

        explicit NodeManager(
                NotNull<EventQueue *> event_queue,
                PeerType pt
        );

        [[nodiscard]]
        concurrencpp::result<void> send(
                std::string id,
                const PeerMessage &message
        );

        concurrencpp::result<std::vector<std::string>> connect_to_peers();

        void append_listener(
                PeerMessageType,
                const std::function<void(PeerMessage)> &listener
        );

        [[nodiscard]]
        PeerState get_state() const;

        [[nodiscard]]
        std::string id() const;

        concurrencpp::result<void> initialize();

        void on_rtc_setup(Event);

        void on_rtc_candidate(Event);

        ~NodeManager();
    };

} // krapi
