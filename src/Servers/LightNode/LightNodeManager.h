//
// Created by mythi on 12/11/22.
//

#pragma once

#include <condition_variable>
#include <future>

#include "spdlog/spdlog.h"

#include "PeerMap.h"
#include "Message.h"
#include "Response.h"
#include "PeerMessage.h"
#include "Block.h"
#include "TransactionPool.h"
#include "ixwebsocket/IXWebSocket.h"

namespace krapi {

    class LightNodeManager {

        PeerMap peer_map;
        std::mutex blocking_mutex;
        std::condition_variable blocking_cv;

        rtc::Configuration rtc_config;
        ix::WebSocket ws;
        int my_id;

        std::shared_ptr<rtc::PeerConnection> create_connection(int);

        void onWsResponse(const Response &);

        void onRemoteMessage(int id, const PeerMessage &);


    public:

        LightNodeManager();

        void wait();

        void broadcast_message(const PeerMessage &);

        [[nodiscard]]
        int id() const;

        std::optional<int> get_random_light_node();
    };

} // krapi
