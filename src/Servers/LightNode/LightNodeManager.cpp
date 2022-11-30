//
// Created by mythi on 12/11/22.
//

#include "LightNodeManager.h"
#include "effolkronium/random.hpp"

namespace krapi {

    LightNodeManager::LightNodeManager() : NodeManager(PeerType::Light) {
        m_peer_state = PeerState::Open;
    }
    std::optional<int> LightNodeManager::random_light_node() {

        using Random = effolkronium::random_static;
        auto ids = peer_ids_of_type(PeerType::Light);

        if (!ids.is_error()) {

            auto random_index = Random::get(0, static_cast<int>(ids.value().size()) - 1);
            return ids.value()[random_index];
        }
        return {};
    }

} // krapi