//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_NETWORKMANAGER_H
#define KRAPI_MODELS_NETWORKMANAGER_H


#include <vector>
#include <string>

#include "ixwebsocket/IXWebSocket.h"

#include "NodeServer.h"
#include "Transaction.h"
#include "NodeMessageQueue.h"
#include "NodeIdentityManager.h"
#include "NodeWebSocketServer.h"
#include "ParsingUtils.h"
#include "TransactionQueue.h"
#include "NodeHttpServer.h"

namespace krapi {

    class NodeManager {

        NodeMessageQueuePtr m_eq;
        TransactionQueuePtr m_txq;

        NodeIdentityManager m_identity_manager;
        NodeWebSocketServer m_ws_server;
        NodeHttpServer m_http_server;

        std::vector<NodeServer> m_nodes;
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
