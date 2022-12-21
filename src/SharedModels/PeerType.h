//
// Created by mythi on 16/11/22.
//

#pragma once

#include "nlohmann/json.hpp"

namespace krapi {
    enum class PeerType {
        Full,
        Light,
        Observer,
        Unknown
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(PeerType, {
        { PeerType::Full, "peer_type_full" },
        { PeerType::Light, "peer_type_light" },
        { PeerType::Observer, "peer_type_observer" },
        { PeerType::Observer, "peer_type_unknown" }
    })

    inline std::string to_string(PeerType type) {

        if (type == PeerType::Full)
            return "Full";
        if(type == PeerType::Light)
            return "Light";
        if(type == PeerType::Observer)
            return "Observer";

        return "Unknown";
    }

}// krapi
