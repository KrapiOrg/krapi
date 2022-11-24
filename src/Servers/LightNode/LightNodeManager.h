//
// Created by mythi on 12/11/22.
//

#pragma once

#include "NodeManager.h"

namespace krapi {

    class LightNodeManager : public NodeManager {


    public:
        LightNodeManager();

        std::vector<int> peer_ids_of_type(PeerType type);

        std::optional<int> random_light_node();
    };

} // krapi
