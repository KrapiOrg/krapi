//
// Created by mythi on 13/10/22.
//

#include "NetworkManager.h"
#include "spdlog/spdlog.h"

namespace krapi {
    NetworkManager::NetworkManager(std::vector<std::string> uris)
            : m_uris(std::move(uris)) {
    }

    void NetworkManager::start() {

        spdlog::info("NETWORK MANAGER: Started");

        for (const auto &uri: m_uris) {
            spdlog::info("NETWORK MANAGER: Opening Socket to {}", uri);

            m_nodes.push_back(NetworkNode::create(uri));
            m_nodes.back()->start();
        }
    }
} // krapi