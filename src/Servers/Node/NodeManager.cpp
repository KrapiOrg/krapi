//
// Created by mythi on 13/10/22.
//

#include "NodeManager.h"
#include "spdlog/spdlog.h"


namespace krapi {
    using namespace eventpp;

    NodeManager::NodeManager(
            const std::string &server_host,
            int server_port,
            std::vector<std::string> node_uris,
            const std::string &identity_server_uri
    )
            : m_server_host(server_host),
              m_server_port(server_port),
              m_node_uris(std::move(node_uris)),
              m_identity_manager(identity_server_uri),
              m_server(server_host, server_port, m_identity_manager.identity()),
              m_eq(create_message_queue()) {
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

    }

    NodeManager::~NodeManager() {

        for (auto &node: m_nodes) {
            node.stop();
        }
        spdlog::info("Successfully terminated");
    }


} // krapi