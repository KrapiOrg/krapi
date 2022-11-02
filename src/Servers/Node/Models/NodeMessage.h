//
// Created by mythi on 20/10/22.
//

#ifndef NODE_NODEMESSAGE_H
#define NODE_NODEMESSAGE_H

#include "../../../../libs/json/include/nlohmann/json.hpp"
#include "../../ServerUtils/ParsingUtils.h"

namespace krapi {

    enum class NodeMessageType {
        BroadcastTx
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(NodeMessageType, {
        { NodeMessageType::BroadcastTx, "boradcast_tx" }
    })

    struct NodeMessage {
        NodeMessageType type{};
        nlohmann::json content{};
        int sender_idenetity{};
        int receiver_identity{};
        std::vector<ServerHost> blacklist{};

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
                NodeMessage,
                sender_idenetity,
                receiver_identity,
                type,
                content,
                blacklist
        )
    };

} // krapi

#endif //NODE_NODEMESSAGE_H
