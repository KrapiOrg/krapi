//
// Created by mythi on 12/11/22.
//

#pragma once

#include <unordered_set>
#include <utility>
#include "nlohmann/json.hpp"

namespace krapi {
    enum class PeerMessageType {
        PeerTypeRequest,
        PeerTypeResponse,
        AddTransaction,
        RemoveTransactions,
        SetTransactionStatus,
        SyncBlockchain,
        RequestBlocks,
        BlocksResponse,
        BlockHeadersRequest,
        BlockHeadersResponse
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(PeerMessageType, {
        { PeerMessageType::PeerTypeRequest, "peer_type_request" },
        { PeerMessageType::PeerTypeResponse, "peer_type_response" },
        { PeerMessageType::AddTransaction, "add_transaction" },
        { PeerMessageType::RemoveTransactions, "remove_transactions" },
        { PeerMessageType::SetTransactionStatus, "set_transaction_status" },
        { PeerMessageType::SyncBlockchain, "sync_blockchain_request" },
        { PeerMessageType::RequestBlocks, "request_blocks" },
        { PeerMessageType::BlocksResponse, "blocks_response" },
        { PeerMessageType::BlockHeadersRequest, "block_headers_request" },
        { PeerMessageType::BlockHeadersResponse, "block_headers_response" }
    })

    class PeerMessage {
        static inline std::atomic<int> tag_ = 0;
        PeerMessageType m_type;
        int m_peer_id;
        int m_tag;
        nlohmann::json m_content;

    public:

        explicit PeerMessage(
                PeerMessageType type,
                int peer_id,
                int tag,
                nlohmann::json content = {}
        ) : m_type(type),
            m_peer_id(peer_id),
            m_tag(tag),
            m_content(std::move(content)) {

        }

        [[nodiscard]]
        PeerMessageType type() const {

            return m_type;
        }

        [[nodiscard]]
        int peer_id() const {

            return m_peer_id;
        }

        [[nodiscard]]
        int tag() const {

            return m_tag;
        }

        [[nodiscard]]
        nlohmann::json content() const {

            return m_content;
        }

        [[nodiscard]]
        nlohmann::json to_json() const {

            return {
                    {"type",    m_type},
                    {"peer_id", m_peer_id},
                    {"tag",     m_tag},
                    {"content", m_content}
            };
        }

        inline operator std::string() {

            return to_string();
        }

        [[nodiscard]]
        inline std::string to_string() const {

            return to_json().dump();
        }

        static int create_tag() {

            return tag_++;
        }

        static inline PeerMessage from_json(nlohmann::json json) {

            return PeerMessage{
                    json["type"].get<PeerMessageType>(),
                    json["peer_id"].get<int>(),
                    json["tag"].get<int>(),
                    json["content"].get<nlohmann::json>()
            };
        }

        static inline PeerMessage from_json(const std::string &str) {

            auto json = nlohmann::json::parse(str);
            return from_json(json);
        }
    };


}
