//
// Created by mythi on 26/10/22.
//

#ifndef SHARED_MODELS_HTTPSERVER_H
#define SHARED_MODELS_HTTPSERVER_H

#include <utility>

#include "nlohmann/json.hpp"
#include "eventpp/eventdispatcher.h"
#include "eventpp/utilities/counterremover.h"
#include "ixwebsocket/IXHttpServer.h"
#include "spdlog/spdlog.h"

#include "HttpMessage.h"
#include "Transaction.h"
#include "MessageQueue.h"
#include "TransactionQueue.h"
#include "IdentityManager.h"
#include "ParsingUtils.h"
#include "InternalMessageQueue.h"

namespace krapi {

    class HttpServer {

        ServerHost m_host;
        std::shared_ptr<IdentityManager> m_identity_manager;
        InternalMessageQueue m_internal_queue;
        MessageQueuePtr m_node_message_queue;
        TransactionQueuePtr m_tx_queue;
        ix::HttpServer m_server;


        void setup_listeners();

    public:

        explicit HttpServer(
                ServerHost host,
                MessageQueuePtr mq,
                TransactionQueuePtr tq,
                std::shared_ptr<IdentityManager> identity_manager
        );

        void start();

        void wait();

        void stop();

        ~HttpServer();
    };

} // krapi

#endif //SHARED_MODELS_HTTPSERVER_H
