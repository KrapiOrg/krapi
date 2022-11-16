//
// Created by mythi on 12/11/22.
//

#pragma once

#include <unordered_set>
#include "nlohmann/json.hpp"

namespace krapi {
    enum class PeerMessageType {
        PeerTypeRequest,
        PeerTypeResponse,
        AddTransaction,
        RemoveTransactions,
        AddBlock
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(PeerMessageType, {
        { PeerMessageType::PeerTypeRequest, "peer_type_request" },
        { PeerMessageType::PeerTypeResponse, "peer_type_response" },
        { PeerMessageType::AddTransaction, "add_transaction" },
        { PeerMessageType::RemoveTransactions, "remove_transactions" },
        { PeerMessageType::AddBlock, "add_block" },
    })

    struct PeerMessage {
        PeerMessageType type;
        nlohmann::json content;
        std::unordered_set<int> ignore_list;

        [[nodiscard]]
        nlohmann::json to_json() const {

            return {
                    {"type",        type},
                    {"content",     content},
                    {"ignore_list", ignore_list}
            };
        }

        inline operator std::string() {

            return to_string();
        }

        [[nodiscard]]
        inline std::string to_string() const {

            return to_json().dump();
        }

        static inline PeerMessage from_json(nlohmann::json json) {

            return PeerMessage{
                    json["type"].get<PeerMessageType>(),
                    json["content"].get<nlohmann::json>(),
                    json["ignore_list"].get<std::unordered_set<int>>()
            };
        }

        static inline PeerMessage from_json(const std::string &str) {

            auto json = nlohmann::json::parse(str);
            return from_json(json);
        }
    };
}
