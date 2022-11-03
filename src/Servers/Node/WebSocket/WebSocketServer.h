//
// Created by mythi on 24/10/22.
//

#ifndef NODE_NODEWEBSOCKETSERVER_H
#define NODE_NODEWEBSOCKETSERVER_H

#include "ixwebsocket/IXWebSocketServer.h"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"

#include "NodeMessage.h"
#include "MessageQueue.h"
#include "IdentityManager.h"
#include "ParsingUtils.h"
#include "InternalMessageQueue.h"
#include "TransactionPool.h"

namespace krapi {

    class WebSocketServer {

        InternalMessageQueue m_internal_queue;

        ServerHost m_host;
        ix::WebSocketServer m_server;

        TransactionPoolPtr m_transaction_pool;

        void setup_listeners();


    public:
        explicit WebSocketServer(
                ServerHost host,
                TransactionPoolPtr transaction_pool
        );

        void start();

        void wait();

        void stop();

        ~WebSocketServer();
    };

} // krapi

#endif //NODE_NODEWEBSOCKETSERVER_H
