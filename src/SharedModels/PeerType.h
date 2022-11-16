//
// Created by mythi on 16/11/22.
//

#pragma once

#include "nlohmann/json.hpp"

namespace krapi {
    enum class PeerType {
        Full,
        Light
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(PeerType, {
        { PeerType::Full, "peer_type_full" },
        { PeerType::Light, "peer_type_light" },
    })

}// krapi
