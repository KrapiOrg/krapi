#pragma once

#include "Helpers.h"
#include "nlohmann/json.hpp"
#include "uuid.h"
#include <cstdint>
#include <unordered_set>
#include <utility>

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
    GetLastBlockResponse,
    SyncPoolRequest
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(
    PeerMessageType,
    {{PeerMessageType::PeerTypeRequest, "peer_type_request"},
     {PeerMessageType::PeerTypeResponse, "peer_type_response"},
     {PeerMessageType::AddTransaction, "add_transaction"},
     {PeerMessageType::RemoveTransactions, "remove_transactions"},
     {PeerMessageType::SetTransactionStatus, "set_transaction_status"},
     {PeerMessageType::BlockHeadersRequest, "block_headers_request"},
     {PeerMessageType::BlockHeadersResponse, "block_headers_response"},
     {PeerMessageType::BlockRequest, "block_request"},
     {PeerMessageType::BlockResponse, "block_response"},
     {PeerMessageType::BlockNotFoundResponse, "block_not_found_response"},
     {PeerMessageType::PeerStateRequest, "peer_state_request"},
     {PeerMessageType::PeerStateResponse, "peer_state_response"},
     {PeerMessageType::PeerStateUpdate, "peer_state_update"},
     {PeerMessageType::AddBlock, "add_block"},
     {PeerMessageType::BlockRejected, "block_rejected"},
     {PeerMessageType::BlockAccepted, "block_accepted"},
     {PeerMessageType::GetLastBlockRequest, "get_last_block_request"},
     {PeerMessageType::GetLastBlockResponse, "get_last_block_response"},
     {PeerMessageType::SyncPoolRequest, "sync_pool_request"}}
  )

  inline std::string to_string(PeerMessageType type) {

    switch (type) {
      case PeerMessageType::PeerTypeRequest:
        return "peer_type_request";
      case PeerMessageType::PeerTypeResponse:
        return "peer_type_response";
      case PeerMessageType::AddTransaction:
        return "add_transaction";
      case PeerMessageType::RemoveTransactions:
        return "remove_transactions";
      case PeerMessageType::SetTransactionStatus:
        return "set_transaction_status";
      case PeerMessageType::BlockHeadersRequest:
        return "block_headers_request";
      case PeerMessageType::BlockHeadersResponse:
        return "block_headers_response";
      case PeerMessageType::BlockRequest:
        return "block_request";
      case PeerMessageType::BlockResponse:
        return "block_response";
      case PeerMessageType::BlockNotFoundResponse:
        return "block_not_found_response";
      case PeerMessageType::PeerStateRequest:
        return "peer_state_request";
      case PeerMessageType::PeerStateResponse:
        return "peer_state_response";
      case PeerMessageType::PeerStateUpdate:
        return "peer_state_update";
      case PeerMessageType::AddBlock:
        return "add_block";
      case PeerMessageType::BlockRejected:
        return "block_rejected";
      case PeerMessageType::BlockAccepted:
        return "block_accepted";
      case PeerMessageType::GetLastBlockRequest:
        return "get_last_block_request";
      case PeerMessageType::GetLastBlockResponse:
        return "get_last_block_response";
      case PeerMessageType::SyncPoolRequest:
        return "sync_pool_request";
      case PeerMessageType::DEFAULT:
        return "default";
    }
  }

  class PeerMessage {

    PeerMessageType m_type;
    std::string m_sender_identity;
    std::string m_receiver_identity;
    std::string m_tag;
    nlohmann::json m_content;
    uint64_t m_timestamp;

   public:
    explicit PeerMessage(
      PeerMessageType type,
      std::string sender_identity,
      std::string receiver_identity,
      std::string tag,
      nlohmann::json content = {}
    )
        : m_type(type),
          m_sender_identity(std::move(sender_identity)),
          m_receiver_identity(std::move(receiver_identity)),
          m_tag(std::move(tag)),
          m_content(std::move(content)),
          m_timestamp(get_krapi_timestamp()) {
    }

    explicit PeerMessage(
      PeerMessageType type,
      std::string sender_identity,
      std::string receiver_identity,
      std::string tag,
      uint64_t timestamp,
      nlohmann::json content = {}
    )
        : m_type(type),
          m_sender_identity(std::move(sender_identity)),
          m_receiver_identity(std::move(receiver_identity)),
          m_tag(std::move(tag)),
          m_content(std::move(content)),
          m_timestamp(timestamp) {
    }

    template<typename... UU>
    static Box<PeerMessage> create(UU &&...params) {

      return make_box<PeerMessage>(std::forward<UU>(params)...);
    }

    [[nodiscard]] PeerMessageType type() const {
      return m_type;
    }

    [[nodiscard]] std::string sender_identity() const {

      return m_sender_identity;
    }

    [[nodiscard]] std::string receiver_identity() const {

      return m_receiver_identity;
    }

    [[nodiscard]] std::string tag() const {
      return m_tag;
    }

    [[nodiscard]] nlohmann::json content() const {
      return m_content;
    }

    [[nodiscard]] nlohmann::json to_json() const {

      return {
        {"type", m_type},
        {"sender_identity", m_sender_identity},
        {"receiver_identity", m_receiver_identity},
        {"tag", m_tag},
        {"content", m_content},
        {"timestamp", m_timestamp}};
    }

    [[nodiscard]] inline std::string to_string() const {

      return to_json().dump();
    }

    uint64_t timestamp() const {
      return m_timestamp;
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
        json["timestamp"].get<uint64_t>(),
        json["content"].get<nlohmann::json>()
      );
    }
  };


}// namespace krapi
