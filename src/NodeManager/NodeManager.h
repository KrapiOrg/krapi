//
// Created by mythi on 24/11/22.
//

#pragma once

#include <condition_variable>
#include <set>
#include "ixwebsocket/IXWebSocket.h"
#include "rtc/peerconnection.hpp"
#include "eventpp/eventdispatcher.h"
#include "PeerMessage.h"
#include "PeerType.h"
#include "SignalingMessage.h"
#include "PeerState.h"
#include "AsyncQueue.h"
#include "ErrorOr.h"
#include "SignalingClient.h"

namespace krapi {

    class NodeManager : public std::enable_shared_from_this<NodeManager> {
    protected:

        using PeerMessageEventDispatcher = eventpp::EventDispatcher<PeerMessageType, void(PeerMessage)>;

        PeerMessageEventDispatcher m_dispatcher;

        std::mutex m_blocking_mutex;
        std::condition_variable m_blocking_cv;

        std::atomic<int> m_peer_count;

        rtc::Configuration m_rtc_config;
        SignalingClient m_signaling_client;
        int my_id;


        std::unordered_map<int, std::shared_ptr<rtc::PeerConnection>> m_connection_map;
        std::unordered_map<int, std::shared_ptr<rtc::DataChannel>> m_channel_map;
        std::unordered_map<int, PeerType> m_peer_types_map;
        std::unordered_map<int, PeerState> m_peer_states_map;

        mutable std::mutex m_peer_state_mutex;
        PeerState m_peer_state;

        PeerType m_peer_type;

        AsyncQueue m_send_queue;
        AsyncQueue m_receive_queue;
        AsyncQueue m_peer_handler_queue;
        std::map<int, std::map<std::string, std::promise<PeerMessage>>> promise_map;

        void on_channel_close(int id);

        std::shared_ptr<rtc::PeerConnection> create_connection(int);

        std::future<int> set_up_datachannel(int id, std::shared_ptr<rtc::DataChannel> channel);

    public:

        explicit NodeManager(
                PeerType pt
        );

        std::vector<int> peer_ids_of_type(PeerType type);

        MultiFuture<PeerMessage> broadcast(
                PeerMessage message,
                const std::set<PeerType> &excluded_types = {PeerType::Light},
                const std::set<PeerState> &excluded_states = {
                        PeerState::Closed,
                        PeerState::WaitingForPeers
                }
        );

        std::future<PeerMessage> send(
                int id,
                const PeerMessage &message
        );

        void wait();

        MultiFuture<int> connect_to_peers();

        void append_listener(
                PeerMessageType,
                const std::function<void(PeerMessage)> &listener
        );

        PeerState request_peer_state(int id);

        PeerType request_peer_type(int id);

        void set_state(PeerState new_state);

        [[nodiscard]]
        PeerState get_state() const;

        [[nodiscard]]
        int id() const;

    };

} // krapi
