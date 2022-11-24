//
// Created by mythi on 24/11/22.
//

#pragma once

#include <condition_variable>
#include "ixwebsocket/IXWebSocket.h"
#include "rtc/peerconnection.hpp"
#include "eventpp/eventdispatcher.h"
#include "TransactionPool.h"
#include "PeerMessage.h"
#include "KrapiRTCDataChannel.h"
#include "PeerType.h"
#include "SignalingMessage.h"

namespace krapi {

    class NodeManager {
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

        void add_peer_connection(
                int id,
                std::shared_ptr<rtc::PeerConnection> peer_connection
        );

        void add_channel(
                int id,
                std::shared_ptr<rtc::DataChannel> channel,
                std::optional<PeerMessageCallback> callback = std::nullopt
        );

    public:

        explicit NodeManager(PeerType peer_type = PeerType::Full);

        std::vector<std::shared_ptr<KrapiRTCDataChannel>> get_channels();

        void broadcast(
                const PeerMessage& message,
                const std::optional<PeerMessageCallback>& callback = std::nullopt,
                bool include_light_nodes = false
        );

        std::shared_future<PeerMessage> send(
                int id,
                PeerMessage message,
                std::optional<PeerMessageCallback> callback = std::nullopt
        );

        void wait();

        void append_listener(
                PeerMessageType,
                const std::function<void(PeerMessage)>& listener
        );

        [[nodiscard]]
        int id() const;

    };

} // krapi
