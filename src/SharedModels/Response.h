//
// Created by mythi on 20/10/22.
//

#ifndef RSPNS_MODELS_RESPONSE_H
#define RSPNS_MODELS_RESPONSE_H

#include "nlohmann/json.hpp"

namespace krapi {
    enum class ResponseType {
        NodesDiscovered,
        TxPoolsDiscovered,
        IdentityDiscovered,
        IdentityFound,
        IdentityError,
        Error
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(ResponseType, {
        { ResponseType::NodesDiscovered, "nodes_discovered" },
        { ResponseType::TxPoolsDiscovered, "pools_discovered" },
        { ResponseType::IdentityDiscovered, "identity_discovered" },
        { ResponseType::IdentityFound, "identity_found" },
        { ResponseType::IdentityError, "identity_error" },
        { ResponseType::Error, "error" }
    })

    struct Response {
        ResponseType type;
        nlohmann::json content;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Response, type, content)
    };
}

#endif //RSPNS_MODELS_RESPONSE_H
