//
// Created by mythi on 13/10/22.
//

#include "NodeManager.h"
#include "spdlog/spdlog.h"


namespace krapi {
    using namespace eventpp;

    NodeManager::NodeManager(
            const std::string &server_host,
            int ws_server_port,
            int http_server_port,
            std::vector<std::string> node_uris,
            const std::string &identity_server_uri
    )
            : m_server_host(server_host),
              m_server_port(ws_server_port),
              m_node_uris(std::move(node_uris)),
              m_identity_manager(identity_server_uri),
              m_eq(create_message_queue()),
              m_txq(create_tx_queue()),
              m_ws_server(server_host, ws_server_port, m_identity_manager.identity(), m_txq),
              m_http_server(server_host, http_server_port, m_eq, m_txq) {

        setup_listeners();
    }

    void NodeManager::connect_to_nodes() {

        static auto my_uri = fmt::format("{}:{}", m_server_host, m_server_port);

        for (const auto &uri: m_node_uris) {
            if (uri != my_uri) {
                m_nodes.emplace_back(uri, m_eq);
            }
        }
    }

    void NodeManager::wait() {

        for (auto &server: m_nodes) {
            server.wait();
        }
    }

    void NodeManager::setup_listeners() {

        static auto my_uri = fmt::format("{}:{}", m_server_host, m_server_port);
        m_txq->appendListener(0, [this](Transaction tx) {


            if (!contains_tx(tx)) {
                spdlog::info("TxPool recieved transaction");

                m_txpool.push_back(tx);

                for (auto &node: m_nodes) {

                    auto msg = NodeMessage{
                            NodeMessageType::BroadcastTx,
                            tx,
                            m_identity_manager.identity(),
                            node.identity(),
                            {my_uri},
                    };
                    m_eq->dispatch(NodeMessageType::BroadcastTx, msg);
                }
            }

        });
    }

    NodeManager::~NodeManager() {

        for (auto &node: m_nodes) {
            node.stop();
        }
        spdlog::info("Successfully terminated");
    }

    TransactionQueuePtr NodeManager::get_tx_queue() {

        return m_txq;
    }

    bool NodeManager::contains_tx(const Transaction &tx) {

        for (const auto &Tx: m_txpool) {
            if (tx == Tx) {
                return true;
            }
        }
        return false;
    }


} // krapi