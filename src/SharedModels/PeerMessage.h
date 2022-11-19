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
        AddBlock,
        SetTransactionStatus,
        SyncBlockchainRequest,
        SyncBlockchainResponse
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(PeerMessageType, {
        { PeerMessageType::PeerTypeRequest, "peer_type_request" },
        { PeerMessageType::PeerTypeResponse, "peer_type_response" },
        { PeerMessageType::AddTransaction, "add_transaction" },
        { PeerMessageType::RemoveTransactions, "remove_transactions" },
        { PeerMessageType::AddBlock, "add_block" },
        { PeerMessageType::SetTransactionStatus, "set_transaction_status" },
        { PeerMessageType::SyncBlockchainRequest, "sync_blockchain_request" },
        { PeerMessageType::SyncBlockchainResponse, "sync_blockchain_response" },
    })

    struct PeerMessage {
        PeerMessageType type;
        int peer_id;
        nlohmann::json content;
        std::unordered_set<int> ignore_list;

        [[nodiscard]]
        nlohmann::json to_json() const {

            return {
                    {"type",        type},
                    {"peer_id",     peer_id},
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
                    json["peer_id"].get<int>(),
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
