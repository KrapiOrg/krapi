//
// Created by mythi on 12/11/22.
//

#ifndef SHARED_MODELS_P2PNODEMANAGER_H
#define SHARED_MODELS_P2PNODEMANAGER_H

#include <condition_variable>
#include <future>

#include "spdlog/spdlog.h"

#include "PeerMap.h"
#include "nlohmann/json.hpp"
#include "Message.h"
#include "Response.h"
#include "ixwebsocket/IXWebSocketMessage.h"
#include "ixwebsocket/IXWebSocket.h"

namespace krapi {

    class P2PNodeManager {

        PeerMap peer_map;
        std::mutex blocking_mutex;
        std::condition_variable blocking_cv;

        rtc::Configuration rtc_config;
        ix::WebSocket ws;
        int my_id;

        std::shared_ptr<rtc::PeerConnection> create_connection(int id);

        void onWsResponse(const Response &rsp);

    public:

        P2PNodeManager();
        void wait();
    };

} // krapi

#endif //SHARED_MODELS_P2PNODEMANAGER_H
