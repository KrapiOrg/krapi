//
// Created by mythi on 20/10/22.
//

#ifndef NODE_NODEMESSAGE_H
#define NODE_NODEMESSAGE_H

#include "nlohmann/json.hpp"

namespace krapi {

    enum class NodeMessageType {
        Stop,
        NodeIdentityRequest,
        NodeIdentityReply,
        BroadcastTx
    };

    struct NodeMessage {
        NodeMessageType type{};
        nlohmann::json content{};
        int sender_idenetity{};
        int receiver_identity{};
        std::vector<std::string> blacklist{};

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
