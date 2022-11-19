//
// Created by mythi on 12/11/22.
//

#pragma once

#include <condition_variable>
#include <future>

#include "spdlog/spdlog.h"
#include "eventpp/eventdispatcher.h"
#include "nlohmann/json.hpp"
#include "ixwebsocket/IXWebSocketMessage.h"
#include "ixwebsocket/IXWebSocket.h"

#include "PeerMap.h"
#include "Message.h"
#include "Response.h"
#include "PeerMessage.h"
#include "Block.h"
#include "TransactionPool.h"

namespace krapi {

    class NodeManager {

    private:

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

        void onRemoteMessage(int id, const PeerMessage &);


    public:

        NodeManager();

        void wait();

        void broadcast_message(const PeerMessage &);

        void send_message(int, PeerMessage);

        void append_listener(PeerMessageType, std::function<void(PeerMessage)> listener);

        void append_listener(PeerMap::Event event, std::function<void(int)> listener);

        PeerType get_peer_type(int id);

        [[nodiscard]]
        int id() const;
    };

} // krapi
