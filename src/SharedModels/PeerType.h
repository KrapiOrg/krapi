//
// Created by mythi on 16/11/22.
//

#ifndef SHARED_MODELS_PEERTYPE_H
#define SHARED_MODELS_PEERTYPE_H

#include "nlohmann/json.hpp"

namespace krapi {
    enum class PeerType {
        Full,
        Light
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(PeerType, {
        { PeerType::Full, "peer_type_full" },
        { PeerType::Full, "peer_type_light" },
    })

}// krapi

#endif //SHARED_MODELS_PEERTYPE_H
