//
// Created by mythi on 13/10/22.
//

#include "NodeManager.h"

#include <utility>
#include "spdlog/spdlog.h"


namespace krapi {
    using namespace eventpp;

    NodeManager::NodeManager(
            ServerHost ws_server_host,
            ServerHost http_server_host,
            ServerHost identity_server_host,
            ServerHosts network_hosts,
            const std::string &blockchain_path
    ) :
            m_ws_server_host(std::move(ws_server_host)),
            m_http_server_host(std::move(http_server_host)),
            m_identity_server_host(std::move(identity_server_host)),
            m_network_hosts(std::move(network_hosts)),
            m_transaction_pool(create_transaction_pool()),
            m_eq(create_message_queue()),
            m_identity_manager(std::make_shared<IdentityManager>(m_identity_server_host)),
            m_ws_server(m_ws_server_host, m_transaction_pool),
            m_http_server(m_http_server_host, m_eq, m_transaction_pool, m_identity_manager),
            m_blockchain(std::move(Blockchain::from_disk(blockchain_path))) {

        setup_listeners();
    }

    void NodeManager::connect_to_nodes() {

        for (const auto &host: m_network_hosts) {
            if (host != m_ws_server_host) {
                m_connections.push_back(std::make_unique<NetworkConnectionManager>(host, m_eq));
            }
        }
        for (auto &connection: m_connections) {
            connection->start();
        }
    }

    void NodeManager::wait() {

        for (auto &connection: m_connections) {
            connection->wait();
        }
        m_ws_server.wait();
        m_http_server.wait();
    }

    void NodeManager::setup_listeners() {

        m_transaction_pool->append_listener(
                TransactionPool::Event::Add,
                [this](const Transaction &tx) {

                    for (auto &connection: m_connections) {

                        auto msg = NodeMessage{
                                NodeMessageType::BroadcastTx,
                                tx.to_json(),
                                m_identity_manager->identity(),
                                connection->identity(),
                                {m_identity_manager->identity()},
                        };
                        m_eq->dispatch(NodeMessageType::BroadcastTx, msg);
                    }
                }
        );
    }

    NodeManager::~NodeManager() {

        for (auto &node: m_connections) {
            node->stop();
        }
        m_http_server.stop();
        m_ws_server.stop();
        spdlog::info("Successfully terminated");
    }


    bool NodeManager::contains_tx(const Transaction &transaction) {

        return m_transaction_pool->contains(transaction);
    }

    void NodeManager::start_ws_server() {

        m_ws_server.start();
    }

    void NodeManager::start_http_server() {

        m_http_server.start();
    }
} // krapi