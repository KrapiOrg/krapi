//
// Created by mythi on 12/11/22.
//

#include "LightNodeManager.h"
#include "PeerType.h"
#include "effolkronium/random.hpp"

namespace krapi {


    std::optional<int> LightNodeManager::get_random_light_node() {

        using Random = effolkronium::random_static;
        auto ids = std::vector<int>{};
        for (auto &channel: peer_map.get_channels()) {

            if(!channel->is_open())
                continue;

            auto resp = channel->send(
                    PeerMessage{
                            PeerMessageType::PeerTypeRequest,
                            my_id,
                            PeerMessage::create_tag()
                    }
            ).get();

            auto type = resp.content().get<PeerType>();

            if (type == PeerType::Light) {
                ids.push_back(resp.peer_id());
            }
        }

        if (!ids.empty()) {

            auto random_index = Random::get(0, static_cast<int>(ids.size()) - 1);
            return ids[random_index];
        }
        return {};
    }

    LightNodeManager::LightNodeManager() : NodeManager(PeerType::Light) {

    }
} // krapi