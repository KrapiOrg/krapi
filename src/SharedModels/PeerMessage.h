//
// Created by mythi on 12/11/22.
//

#pragma once

#include <unordered_set>
#include <utility>
#include "nlohmann/json.hpp"
#include "uuid.h"

namespace krapi {
    enum class PeerMessageType {
        DEFAULT,
        PeerTypeRequest,
        PeerTypeResponse,
        AddTransaction,
        RemoveTransactions,
        SetTransactionStatus,
        SyncBlockchain,
        RequestBlocks,
        BlocksResponse,
        BlockHeadersRequest,
        BlockHeadersResponse,
        BlockRequest,
        BlockResponse,
        BlockNotFoundResponse,
        PeerStateRequest,
        PeerStateResponse,
        PeerStateUpdate,
        AddBlock,
        BlockRejected,
        BlockAccepted,
        GetLastBlockRequest,
        GetLastBlockResponse
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
        { PeerMessageType::BlockHeadersResponse, "block_headers_response" },
        { PeerMessageType::BlockRequest, "block_request" },
        { PeerMessageType::BlockResponse, "block_response" },
        { PeerMessageType::BlockNotFoundResponse, "block_not_found_response" },
        { PeerMessageType::PeerStateRequest, "peer_state_request" },
        { PeerMessageType::PeerStateResponse, "peer_state_response" },
        { PeerMessageType::PeerStateUpdate, "peer_state_update" },
        { PeerMessageType::AddBlock, "add_block" },
        { PeerMessageType::BlockRejected, "block_rejected" },
        { PeerMessageType::BlockAccepted, "block_accepted" },
        { PeerMessageType::GetLastBlockRequest, "get_last_block_request" },
        { PeerMessageType::GetLastBlockResponse, "get_last_block_response" }
    })

    class PeerMessage {

        PeerMessageType m_type;
        int m_peer_id;
        std::string m_tag;
        nlohmann::json m_content;

    public:
        explicit PeerMessage() :
                m_type(PeerMessageType::DEFAULT),
                m_peer_id(0) {

        }

        explicit PeerMessage(
                PeerMessageType type,
                int peer_id,
                std::string tag,
                nlohmann::json content = {}
        ) : m_type(type),
            m_peer_id(peer_id),
            m_tag(std::move(tag)),
            m_content(std::move(content)) {

        }

        explicit PeerMessage(
                PeerMessageType type,
                int peer_id,
                nlohmann::json content = {}
        ) : m_type(type),
            m_peer_id(peer_id),
            m_content(std::move(content)) {

        }

        void randomize_tag() {

            m_tag = create_tag();
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
        std::string tag() const {

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

        static std::string create_tag() {

            return uuids::to_string(uuids::uuid_system_generator{}());
        }

        static inline PeerMessage from_json(nlohmann::json json) {

            return PeerMessage{
                    json["type"].get<PeerMessageType>(),
                    json["peer_id"].get<int>(),
                    json["tag"].get<std::string>(),
                    json["content"].get<nlohmann::json>()
            };
        }

        static inline PeerMessage from_json(const std::string &str) {

            auto json = nlohmann::json::parse(str);
            return from_json(json);
        }
    };


}
