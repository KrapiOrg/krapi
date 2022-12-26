//
// Created by mythi on 16/11/22.
//

#pragma once

#include "nlohmann/json.hpp"

namespace krapi {
    enum class PeerType {
        Light,
        Full,
        Observer,
        Unknown
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(PeerType, {
        { PeerType::Full, "full" },
        { PeerType::Light, "light" },
        { PeerType::Observer, "observer" },
        { PeerType::Unknown, "unknown" }
    })

    inline std::string to_string(PeerType type) {

        if (type == PeerType::Full)
            return "full";
        if(type == PeerType::Light)
            return "light";
        if(type == PeerType::Observer)
            return "observer";

        return "unknown";
    }

}// krapi
