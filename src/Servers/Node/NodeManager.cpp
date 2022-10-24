//
// Created by mythi on 13/10/22.
//

#include <future>
#include <thread>
#include "NodeManager.h"
#include "spdlog/spdlog.h"
#include "Response.h"


namespace krapi {
    using namespace eventpp;

    NodeManager::NodeManager(
            std::string my_uri,
            std::vector<std::string> network_hosts,
            int identity
    )
            : m_my_uri(std::move(my_uri)),
              m_identity(identity),
              m_network_node_hosts(std::move(network_hosts)),
              m_eq(std::make_shared<EventDispatcher<NodeMessageType, void(const NodeMessage &)>>()) {
    }

    void NodeManager::connect_to_nodes() {

        for (const auto &uri: m_network_node_hosts) {
            if (uri != m_my_uri) {
                m_nodes.emplace_back(uri, m_eq);
            }
        }
    }

    void NodeManager::wait() {

        for (auto &server: m_nodes) {
            server.wait();
        }
    }


    int NodeManager::identity() {

        return m_identity;
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