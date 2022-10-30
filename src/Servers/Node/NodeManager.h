//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_NETWORKMANAGER_H
#define KRAPI_MODELS_NETWORKMANAGER_H


#include <vector>
#include <string>

#include "ixwebsocket/IXWebSocket.h"

#include "NetworkConnection.h"
#include "Transaction.h"
#include "MessageQueue.h"
#include "IdentityManager.h"
#include "WebSocketServer.h"
#include "ParsingUtils.h"
#include "TransactionQueue.h"
#include "HttpServer.h"

namespace krapi {

    class NodeManager {

        ServerHost m_ws_server_host;
        ServerHost m_http_server_host;
        ServerHost m_identity_server_host;
        ServerHosts m_network_hosts;

        MessageQueuePtr m_eq;
        TransactionQueuePtr m_txq;

        std::shared_ptr<IdentityManager> m_identity_manager;
        krapi::WebSocketServer m_ws_server;
        krapi::HttpServer m_http_server;

        std::vector<std::unique_ptr<NetworkConnection>> m_connections;
        std::vector<Transaction> m_txpool;



        void setup_listeners();

    public:
        explicit NodeManager(
                ServerHost ws_server_host,
                ServerHost http_server_host,
                ServerHost identity_server_host,
                ServerHosts network_hosts
        );

        void connect_to_nodes();

        void start_http_server();
        void start_ws_server();

        void wait();

        inline bool contains_tx(const Transaction &transaction);

        ~NodeManager();
    };

} // krapi

#endif //KRAPI_MODELS_NETWORKMANAGER_H
