//
// Created by mythi on 12/11/22.
//

#include "LightNodeManager.h"
#include "effolkronium/random.hpp"

namespace krapi {

    LightNodeManager::LightNodeManager() : NodeManager(PeerType::Light) {

    }
    std::optional<int> LightNodeManager::random_light_node() {

        using Random = effolkronium::random_static;
        auto ids = peer_ids_of_type(PeerType::Light);

        if (!ids.empty()) {

            auto random_index = Random::get(0, static_cast<int>(ids.size()) - 1);
            return ids[random_index];
        }
        return {};
    }

} // krapi