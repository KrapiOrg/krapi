//
// Created by mythi on 13/10/22.
//

#include "NodeManager.h"
#include "spdlog/spdlog.h"

#include <thread>

namespace krapi {
    using namespace eventpp;

    NodeManager::NodeManager(
            std::string my_uri,
            std::vector<std::string> network_hosts,
            std::vector<std::string> pool_hosts
    )
            : m_my_uri(std::move(my_uri)),
              m_network_node_hosts(std::move(network_hosts)),
              m_pool_hosts(std::move(pool_hosts)),
              m_eq(std::make_shared<EventDispatcher<NodeMessageType, void(const NodeMessage &)>>()) {
    }

    void NodeManager::connect_to_nodes() {

        spdlog::warn("Connecting to nodes!");
        for (const auto &uri: m_network_node_hosts) {
            if (uri != m_my_uri) {
                spdlog::info("Attempting to connect to {}", uri);
                m_nodes.emplace_back(uri, m_eq);
            }
        }
    }

    void NodeManager::connect_to_tx_pools() {

    }

    void NodeManager::wait() {

        spdlog::warn("Here1");
        for (auto &server: m_nodes) {
            server.wait();
        }
    }

    void NodeManager::set_network_node_hosts(const std::vector<std::string> &hosts) {

        m_network_node_hosts = hosts;
    }

    void NodeManager::set_pool_hosts(const std::vector<std::string> &hosts) {

        m_pool_hosts = hosts;
    }


} // krapi