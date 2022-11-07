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
            m_eq(create_message_queue()),
            m_blockchain(Blockchain::from_disk(blockchain_path)),
            m_miner(std::make_shared<Miner>(m_blockchain)),
            m_transaction_pool(create_transaction_pool(m_miner)),
            m_identity_manager(std::make_shared<IdentityManager>(m_identity_server_host)),
            m_ws_server(m_ws_server_host, m_transaction_pool),
            m_http_server(m_http_server_host, m_transaction_pool, m_identity_manager) {

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
                [this](const Transaction &tx, const std::unordered_set<int> &old_blacklist) {

                    // The new blacklist contains the identites of the nodes that the tx passed to
                    // And the identity of this node and the nodes it's connected to
                    auto new_blacklist = old_blacklist;
                    new_blacklist.insert(m_identity_manager->identity());
                    for (const auto &connection: m_connections) {
                        new_blacklist.insert(connection->identity());
                    }

                    // pass this tx if it still hasn't passed to a particular node
                    for (const auto &connection: m_connections) {
                        if (!old_blacklist.contains(connection->identity())) {
                            auto msg = NodeMessage{
                                    NodeMessageType::AddTransactionToPool,
                                    tx.to_json(),
                                    m_identity_manager->identity(),
                                    connection->identity(),
                                    new_blacklist,
                            };
                            m_eq->dispatch(NodeMessageType::AddTransactionToPool, msg);
                        }

                    }
                }
        );

        m_miner->append_listener(Miner::Event::BatchMined, [](Block block) {
            spdlog::info("NodeManager: block {} was just mined", block.to_json().dump());
        });
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