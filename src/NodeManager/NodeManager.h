//
// Created by mythi on 24/11/22.
//

#pragma once

#include <condition_variable>
#include <set>
#include "ixwebsocket/IXWebSocket.h"
#include "rtc/peerconnection.hpp"
#include "eventpp/eventdispatcher.h"
#include "tl/expected.hpp"
#include "PeerMessage.h"
#include "PeerType.h"
#include "SignalingMessage.h"
#include "PeerState.h"
#include "AsyncQueue.h"
#include "SignalingClient.h"
#include "ErrorOr.h"

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
        std::string my_id;

        std::recursive_mutex m_connection_map_mutex;
        std::unordered_map<std::string, std::shared_ptr<rtc::PeerConnection>> m_connection_map;
        std::recursive_mutex m_channel_map_mutex;
        std::unordered_map<std::string, std::shared_ptr<rtc::DataChannel>> m_channel_map;
        std::recursive_mutex m_peer_types_map_mutex;
        std::unordered_map<std::string, PeerType> m_peer_types_map;
        std::recursive_mutex m_peer_states_map_mutex;
        std::unordered_map<std::string, PeerState> m_peer_states_map;

        mutable std::mutex m_peer_state_mutex;
        PeerState m_peer_state;

        PeerType m_peer_type;

        AsyncQueue m_send_queue;
        AsyncQueue m_receive_queue;
        AsyncQueue m_peer_handler_queue;
        std::mutex m_promise_map_mutex;

        using PromiseType = tl::expected<PeerMessage, KrapiErr>;
        using Promise = std::promise<PromiseType>;
        using PromisePtr = std::shared_ptr<Promise>;
        using Future = std::shared_future<PromiseType>;
        using PromiseMap = std::map<std::string, PromisePtr>;
        using PromiseMapPtr = std::shared_ptr<PromiseMap>;
        using PerPeerPromiseMap = std::map<std::string, PromiseMapPtr>;
        std::shared_ptr<PerPeerPromiseMap> promise_map;

        std::mutex m_future_map_mutex;
        using FutureMap = std::map<std::string, Future>;
        using FutureMapPtr = std::shared_ptr<FutureMap>;
        using PerPeerFutureMap = std::map<std::string, FutureMapPtr>;
        std::shared_ptr<PerPeerFutureMap> future_map;


        void on_channel_close(std::string id);

        std::shared_ptr<rtc::PeerConnection> create_connection(std::string);

        std::future<std::string> set_up_datachannel(std::string id, std::shared_ptr<rtc::DataChannel> channel);

    public:

        explicit NodeManager(
                PeerType pt
        );

        [[nodiscard]]
        MultiFuture<PromiseType> broadcast(
                PeerMessage message,
                const std::set<PeerType> &excluded_types = {PeerType::Light},
                const std::set<PeerState> &excluded_states = {
                        PeerState::Closed,
                        PeerState::WaitingForPeers
                }
        );

        [[nodiscard]]
        ErrorOr<Future> send(
                std::string id,
                const PeerMessage &message
        );

        void wait();

        MultiFuture<std::string> connect_to_peers();

        void append_listener(
                PeerMessageType,
                const std::function<void(PeerMessage)> &listener
        );

        [[nodiscard]]
        ErrorOr<PeerState> request_peer_state(std::string id);

        [[nodiscard]]
        ErrorOr<PeerType> request_peer_type(std::string id);

        void set_state(PeerState new_state);

        [[nodiscard]]
        PeerState get_state() const;

        [[nodiscard]]
        std::string id() const;

        [[nodiscard]]
        ErrorOr<std::vector<std::tuple<std::string, PeerType, PeerState>>> get_peers(
                const std::set<PeerType>& types,
                const std::set<PeerState>& states = {PeerState::Open}
        );

    };

} // krapi
