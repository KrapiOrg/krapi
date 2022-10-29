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

        MessageQueuePtr m_eq;
        TransactionQueuePtr m_txq;

        IdentityManager m_identity_manager;
        WebSocketServer m_ws_server;
        HttpServer m_http_server;

        std::vector<NetworkConnection> m_nodes;
        std::vector<Transaction> m_txpool;

        std::string m_server_host;
        int m_server_port;
        std::vector<std::string> m_node_uris;

        void setup_listeners();

    public:
        explicit NodeManager(
                const std::string &server_host,
                int ws_server_port,
                int http_server_port,
                std::vector<std::string> node_uris,
                const std::string& identity_server_uri
        );

        void connect_to_nodes();

        TransactionQueuePtr get_tx_queue();

        void wait();

        inline bool contains_tx(const Transaction &tx);

        ~NodeManager();
    };

} // krapi

#endif //KRAPI_MODELS_NETWORKMANAGER_H
