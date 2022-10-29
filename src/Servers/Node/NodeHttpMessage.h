//
// Created by mythi on 26/10/22.
//

#ifndef NODE_NODEHTTPMESSAGE_H
#define NODE_NODEHTTPMESSAGE_H

#include "nlohmann/json.hpp"

namespace krapi {
    enum class NodeHttpMessageType {
        AddTx
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(NodeHttpMessageType, {
        { NodeHttpMessageType::AddTx, "add_tx" }
    })

    struct NodeHttpMessage {
        NodeHttpMessageType type;
        nlohmann::json content;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(NodeHttpMessage, type, content)
    };
}

#endif //NODE_NODEHTTPMESSAGE_H
