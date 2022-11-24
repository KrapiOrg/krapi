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



    std::vector<int> LightNodeManager::peer_ids_of_type(PeerType type) {

        std::vector<int> ans;
        for (const auto &[id, tp]: peer_type_map)  {
            if(tp == type)
                ans.push_back(id);
        }
        return ans;
    }
} // krapi