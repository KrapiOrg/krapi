//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_NETWORKMANAGER_H
#define KRAPI_MODELS_NETWORKMANAGER_H

#include <vector>
#include <string>
#include "ixwebsocket/IXWebSocket.h"
#include "NodeServer.h"

namespace krapi {

    class NodeManager {
        std::string m_my_uri;
        std::shared_ptr<eventpp::EventQueue<NodeMessageType, void(const NodeMessage &)>> m_eq;
        std::vector<NodeServer> m_nodes;
        std::vector<std::string> m_network_node_hosts;
        std::vector<std::string> m_pool_hosts;

    public:
        explicit NodeManager(
                std::string my_uri,
                std::vector<std::string> network_node_hosts,
                std::vector<std::string> pool_hosts
        );

        void set_network_node_hosts(const std::vector<std::string> &hosts);

        void set_pool_hosts(const std::vector<std::string> &hosts);

        void connect_to_nodes();

        void connect_to_tx_pools();

        void wait();
    };

} // krapi

#endif //KRAPI_MODELS_NETWORKMANAGER_H
