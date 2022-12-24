//
// Created by mythi on 12/11/22.
//

#pragma once

#include <unordered_set>
#include <utility>
#include "nlohmann/json.hpp"
#include "uuid.h"

#include "Box.h"

namespace krapi {
    enum class PeerMessageType {
        DEFAULT,
        PeerTypeRequest,
        PeerTypeResponse,
        AddTransaction,
        RemoveTransactions,
        SetTransactionStatus,
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
        std::string m_sender_identity;
        std::string m_receiver_identity;
        std::string m_tag;
        nlohmann::json m_content;

    public:


        explicit PeerMessage(
                PeerMessageType type,
                std::string sender_identity,
                std::string receiver_identity,
                std::string tag,
                nlohmann::json content = {}
        ) : m_type(type),
            m_sender_identity(std::move(sender_identity)),
            m_receiver_identity(std::move(receiver_identity)),
            m_tag(std::move(tag)),
            m_content(std::move(content)) {

        }

        template<typename ...UU>
        static Box<PeerMessage> create(UU &&... params) {

            return make_box<PeerMessage>(std::forward<UU>(params)...);
        }

        [[nodiscard]]
        PeerMessageType type() const {

            return m_type;
        }

        [[nodiscard]]
        std::string sender_identity() const {

            return m_sender_identity;
        }

        [[nodiscard]]
        std::string receiver_identity() const {

            return m_receiver_identity;
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
                    {"type",              m_type},
                    {"sender_identity",   m_sender_identity},
                    {"receiver_identity", m_receiver_identity},
                    {"tag",               m_tag},
                    {"content",           m_content}
            };
        }

        [[nodiscard]]
        inline std::string to_string() const {

            return to_json().dump();
        }

        static std::string create_tag() {

            return uuids::to_string(uuids::uuid_system_generator{}());
        }

        static inline Box<PeerMessage> from_json(nlohmann::json json) {

            return make_box<PeerMessage>(
                    json["type"].get<PeerMessageType>(),
                    json["sender_identity"].get<std::string>(),
                    json["receiver_identity"].get<std::string>(),
                    json["tag"].get<std::string>(),
                    json["content"].get<nlohmann::json>()
            );
        }
    };


}
