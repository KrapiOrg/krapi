//
// Created by mythi on 20/10/22.
//

#ifndef RSPNS_MODELS_MESSAGE_H
#define RSPNS_MODELS_MESSAGE_H

#include "nlohmann/json.hpp"

namespace krapi {
    enum class Message {
        Acknowledge,
        CreateTransaction,
        DiscoverNodes,
        DiscoverTxPools,
        DiscoverIdentity
    };

    NLOHMANN_JSON_SERIALIZE_ENUM( Message, {
        { Message::Acknowledge, "acknowledge" },
        { Message::CreateTransaction, "create_tx" },
        { Message::DiscoverNodes, "discover_nodes" },
        { Message::DiscoverTxPools, "discover_pools" },
        { Message::DiscoverIdentity, "discover_identity" }
    })
}

#endif //RSPNS_MODELS_MESSAGE_H
