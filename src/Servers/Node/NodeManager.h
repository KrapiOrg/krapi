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

namespace krapi {

    class NodeManager {

        int m_identity;

        std::string m_my_uri;
        std::shared_ptr<eventpp::EventDispatcher<NodeMessageType, void(const NodeMessage &)>> m_eq;
        std::vector<NodeServer> m_nodes;
        std::vector<Transaction> m_txpool;
        std::vector<std::string> m_network_node_hosts;

        void setup_listeners();

    public:
        explicit NodeManager(
                std::string my_uri,
                std::vector<std::string> network_node_hosts,
                int identity
        );

        void connect_to_nodes();

        int identity();

        void wait();

        ~NodeManager();
    };

} // krapi

#endif //KRAPI_MODELS_NETWORKMANAGER_H
