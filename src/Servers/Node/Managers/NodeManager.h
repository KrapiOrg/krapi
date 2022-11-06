//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_NETWORKMANAGER_H
#define KRAPI_MODELS_NETWORKMANAGER_H


#include <vector>
#include <string>

#include "NetworkConnectionManager.h"
#include "Transaction.h"
#include "MessageQueue.h"
#include "IdentityManager.h"
#include "WebSocketServer.h"
#include "ParsingUtils.h"
#include "HttpServer.h"
#include "Blockchain.h"
#include "TransactionPool.h"
#include "Miner.h"

namespace krapi {

    class NodeManager {

        ServerHost m_ws_server_host;
        ServerHost m_http_server_host;
        ServerHost m_identity_server_host;
        ServerHosts m_network_hosts;

        MessageQueuePtr m_eq;
        std::shared_ptr<Blockchain> m_blockchain;
        std::shared_ptr<Miner> m_miner;
        TransactionPoolPtr m_transaction_pool;

        std::shared_ptr<IdentityManager> m_identity_manager;
        krapi::WebSocketServer m_ws_server;
        krapi::HttpServer m_http_server;

        std::vector<std::unique_ptr<NetworkConnectionManager>> m_connections;

        void setup_listeners();

    public:
        explicit NodeManager(
                ServerHost ws_server_host,
                ServerHost http_server_host,
                ServerHost identity_server_host,
                ServerHosts network_hosts,
                const std::string& blockchain_path
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
