//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_NETWORKMANAGER_H
#define KRAPI_MODELS_NETWORKMANAGER_H

#include "NetworkNode.h"

namespace krapi {

    class NetworkManager {
        std::vector<NetworkNode::Ptr> m_nodes;
        std::vector<std::string> m_uris;

    public:
        explicit NetworkManager(
                std::vector<std::string> uris
        );

        void start();
    };

} // krapi

#endif //KRAPI_MODELS_NETWORKMANAGER_H
