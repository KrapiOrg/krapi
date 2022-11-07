//
// Created by mythi on 26/10/22.
//

#ifndef NODE_NODEHTTPMESSAGE_H
#define NODE_NODEHTTPMESSAGE_H

#include "nlohmann/json.hpp"

namespace krapi {
    enum class NodeHttpMessageType {
        AddTx,
        TxAdded,
        RequestIdentity,
        IdentityResponse
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(NodeHttpMessageType, {
        { NodeHttpMessageType::AddTx, "add_tx" },
        { NodeHttpMessageType::TxAdded, "tx_added" },
        { NodeHttpMessageType::RequestIdentity, "request_identity" },
        { NodeHttpMessageType::IdentityResponse, "identity_response" }
    })

    struct HttpMessage {
        NodeHttpMessageType type;
        nlohmann::json content;

        static HttpMessage fromJson(const nlohmann::json &json) {

            return HttpMessage{
                    json["type"].get<NodeHttpMessageType>(),
                    json["content"].get<nlohmann::json>()
            };
        }

        [[nodiscard]]
        nlohmann::json to_json() const {

            return {
                    {"type",    type},
                    {"content", content}
            };
        }
    };
}

#endif //NODE_NODEHTTPMESSAGE_H
