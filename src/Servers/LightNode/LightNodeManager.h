//
// Created by mythi on 12/11/22.
//

#pragma once

#include "NodeManager.h"

namespace krapi {

    class LightNodeManager : public NodeManager {


    public:
        LightNodeManager();

        std::optional<int> get_random_light_node();
    };

} // krapi
