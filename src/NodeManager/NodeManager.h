//
// Created by mythi on 24/11/22.
//

#pragma once

#include <condition_variable>
#include "ixwebsocket/IXWebSocket.h"
#include "rtc/peerconnection.hpp"
#include "eventpp/eventdispatcher.h"
#include "PeerMessage.h"
#include "KrapiRTCDataChannel.h"
#include "PeerType.h"
#include "SignalingMessage.h"
#include "PeerState.h"

namespace krapi {

    class NodeManager : public std::enable_shared_from_this<NodeManager> {
    protected:

        using PeerMessageEventDispatcher = eventpp::EventDispatcher<PeerMessageType, void(PeerMessage)>;

        PeerMessageEventDispatcher m_dispatcher;

        std::mutex blocking_mutex;
        std::condition_variable blocking_cv;

        rtc::Configuration rtc_config;
        ix::WebSocket ws;
        int my_id;

        std::shared_ptr<rtc::PeerConnection> create_connection(int);

        void on_signaling_message(const SignalingMessage &rsp);

        std::unordered_map<int, std::shared_ptr<rtc::PeerConnection>> peer_map;
        std::unordered_map<int, std::shared_ptr<KrapiRTCDataChannel>> channel_map;
        std::unordered_map<int, PeerType> peer_type_map;

        std::atomic<int> full_peer_count;
        std::atomic<int> light_peer_count;
        std::mutex peer_threshold_mutex;
        std::condition_variable peer_threshold_cv;

        std::mutex peer_state_mutex;
        PeerState peer_state;

        void add_peer_connection(
                int id,
                std::shared_ptr<rtc::PeerConnection> peer_connection
        );

        void add_channel(
                int id,
                std::shared_ptr<rtc::DataChannel> channel
        );

    public:

        explicit NodeManager(
                PeerType peer_type = PeerType::Full
        );
        ~NodeManager();

        std::vector<int> peer_ids_of_type(PeerType type);

        std::vector<std::shared_ptr<KrapiRTCDataChannel>> get_channels();

        void broadcast(
                const PeerMessage &message,
                const std::optional<PeerMessageCallback> &callback = std::nullopt,
                bool include_light_nodes = false
        );

        std::shared_future<PeerMessage> send(
                int id,
                PeerMessage message,
                std::optional<PeerMessageCallback> callback = std::nullopt
        );

        void wait();

        void wait_for(PeerType, int);

        void append_listener(
                PeerMessageType,
                const std::function<void(PeerMessage)> &listener
        );

        [[nodiscard]]
        int id() const;

    };

} // krapi
