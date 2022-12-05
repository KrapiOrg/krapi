//
// Created by mythi on 12/11/22.
//

#include "LightNodeManager.h"
#include "effolkronium/random.hpp"

namespace krapi {

    LightNodeManager::LightNodeManager() : NodeManager(PeerType::Light) {
        m_peer_state = PeerState::Open;
    }
    ErrorOr<int> LightNodeManager::random_light_node() {

        using Random = effolkronium::random_static;
        auto ids = TRY(get_peers({PeerType::Light}));

        if (!ids.empty()) {

            auto random_index = Random::get(0, static_cast<int>(ids.size()) - 1);
            return std::get<0>(ids[random_index]);
        }
        return tl::make_unexpected(KrapiErr{"There are no light nodes in the network"});
    }

} // krapi