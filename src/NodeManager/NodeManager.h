//
// Created by mythi on 24/11/22.
//

#pragma once

#include <condition_variable>
#include "ixwebsocket/IXWebSocket.h"
#include "rtc/peerconnection.hpp"
#include "eventpp/eventdispatcher.h"
#include "TransactionPool.h"
#include "PeerMap.h"
#include "PeerMessage.h"
#include "Response.h"

namespace krapi {

    class NodeManager {
    protected:

        using PeerMessageEventDispatcher = eventpp::EventDispatcher<PeerMessageType, void(PeerMessage)>;

        PeerMessageEventDispatcher m_dispatcher;

        PeerMap peer_map;
        std::mutex blocking_mutex;
        std::condition_variable blocking_cv;

        rtc::Configuration rtc_config;
        ix::WebSocket ws;
        int my_id;

        TransactionPool m_transaction_pool;

        std::shared_ptr<rtc::PeerConnection> create_connection(int);

        void onWsResponse(const Response &);


    public:

        NodeManager(PeerType peer_type = PeerType::Full);

        void wait();

        void broadcast_message(const PeerMessage &);

        std::shared_future<PeerMessage> send_message(
                int,
                PeerMessage,
                std::optional<PeerMessageCallback> = std::nullopt
        );

        void append_listener(
                PeerMessageType,
                std::function<void(PeerMessage)> listener
        );

        [[nodiscard]]
        int id() const;

    };

} // krapi
