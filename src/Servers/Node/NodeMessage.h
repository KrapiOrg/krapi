//
// Created by mythi on 20/10/22.
//

#ifndef NODE_NODEMESSAGE_H
#define NODE_NODEMESSAGE_H

#include "nlohmann/json.hpp"

namespace krapi {

    enum class NodeMessageType {
        Stop,
        Broadcast
    };

    struct NodeMessage {
        NodeMessageType type;
        nlohmann::json content;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(NodeMessage, type, content)
    };

} // krapi

#endif //NODE_NODEMESSAGE_H
