//
// Created by mythi on 26/10/22.
//

#ifndef SHARED_MODELS_HTTPSERVER_H
#define SHARED_MODELS_HTTPSERVER_H

#include <utility>

#include "nlohmann/json.hpp"
#include "ixwebsocket/IXHttpServer.h"
#include "spdlog/spdlog.h"

#include "HttpMessage.h"
#include "Transaction.h"
#include "IdentityManager.h"
#include "ParsingUtils.h"
#include "InternalMessageQueue.h"
#include "TransactionPool.h"

namespace krapi {

    class HttpServer {

        ServerHost m_host;
        std::shared_ptr<IdentityManager> m_identity_manager;
        TransactionPoolPtr m_transaction_pool;
        ix::HttpServer m_server;

        ix::HttpResponsePtr  onMessage(
                const ix::HttpRequestPtr &req,
                const std::shared_ptr<ix::ConnectionState> &state
        );

    public:

        explicit HttpServer(
                ServerHost host,
                TransactionPoolPtr txp,
                std::shared_ptr<IdentityManager> identity_manager
        );

        void start();

        void wait();

        void stop();

        ~HttpServer();
    };

} // krapi

#endif //SHARED_MODELS_HTTPSERVER_H
