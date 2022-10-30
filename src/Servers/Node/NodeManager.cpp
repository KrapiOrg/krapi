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
            ServerHosts network_hosts
    ) :
            m_ws_server_host(std::move(ws_server_host)),
            m_http_server_host(std::move(http_server_host)),
            m_identity_server_host(std::move(identity_server_host)),
            m_network_hosts(std::move(network_hosts)),
            m_eq(create_message_queue()),
            m_txq(create_tx_queue()),
            m_identity_manager(std::make_shared<IdentityManager>(m_identity_server_host)),
            m_ws_server(m_ws_server_host, m_txq),
            m_http_server(m_http_server_host, m_eq, m_txq, m_identity_manager) {

        setup_listeners();
    }

    void NodeManager::connect_to_nodes() {

        for (const auto &host: m_network_hosts) {
            if (host != m_ws_server_host) {
                m_connections.push_back(std::make_unique<NetworkConnection>(host, m_eq));
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

        m_txq->appendListener(0, [this](Transaction tx) {


            if (!contains_tx(tx)) {
                spdlog::info("TxPool recieved transaction");

                m_txpool.push_back(tx);

                for (auto &connection: m_connections) {

                    auto msg = NodeMessage{
                            NodeMessageType::BroadcastTx,
                            tx,
                            m_identity_manager->identity(),
                            connection->identity(),
                            {m_ws_server_host},
                    };
                    m_eq->dispatch(NodeMessageType::BroadcastTx, msg);
                }
            }

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

        return std::any_of(
                m_txpool.begin(),
                m_txpool.end(),
                [&transaction](const Transaction &tx) {
                    return tx == transaction;
                }
        );

    }
    void NodeManager::start_ws_server() {

        m_ws_server.start();
    }

    void NodeManager::start_http_server() {

        m_http_server.start();
    }
} // krapi