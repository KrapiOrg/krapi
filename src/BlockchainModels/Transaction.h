//
// Created by mythi on 24/10/22.
//

#ifndef NODE_BLOCKCHAIN_TRANSACTION_H
#define NODE_BLOCKCHAIN_TRANSACTION_H

#include "cryptopp/hex.h"
#include "nlohmann/json.hpp"

namespace krapi {

  enum class TransactionType { Send };

  NLOHMANN_JSON_SERIALIZE_ENUM(
    TransactionType,
    {{TransactionType::Send, "send"}}
  )

  enum class TransactionStatus { Pending, Verified, Rejected };

  inline std::string to_string(TransactionStatus status) {

    switch (status) {

      case TransactionStatus::Pending:
        return "Pending";
      case TransactionStatus::Verified:
        return "Verified";
      case TransactionStatus::Rejected:
        return "Rejected";
    }
    return {};
  }

  NLOHMANN_JSON_SERIALIZE_ENUM(
    TransactionStatus,
    {{TransactionStatus::Pending, "pending"},
     {TransactionStatus::Verified, "verified"},
     {TransactionStatus::Rejected, "rejected"}}
  )

  class Transaction {
   public:
    explicit Transaction() = default;
    explicit Transaction(
      TransactionType type,
      TransactionStatus status,
      std::string hash,
      uint64_t timestamp,
      std::string from,
      std::string to
    );

    static Transaction from_json(const nlohmann::json &json);

    [[nodiscard]] nlohmann::json to_json() const;

    bool operator==(const Transaction &) const;

    [[nodiscard]] TransactionType type() const;

    [[nodiscard]] TransactionStatus status() const;

    [[nodiscard]] uint64_t timestamp() const;

    [[nodiscard]] std::string from() const;

    [[nodiscard]] std::string to() const;

    [[nodiscard]] std::string hash() const;

    [[nodiscard]] std::string contrived_hash() const;

    [[nodiscard]] std::array<CryptoPP::byte, 32> byte_hash() const;

    bool operator<(const Transaction &other) const;

   private:
    TransactionType m_type;
    mutable TransactionStatus m_status;
    std::string m_hash;
    uint64_t m_timestamp;
    std::string m_from;
    std::string m_to;

    std::array<CryptoPP::byte, 32> m_byte_hash{};
  };

  using Transactions = std::vector<Transaction>;
}// namespace krapi

namespace std {
  template<>
  struct hash<krapi::Transaction> {
    size_t operator()(const krapi::Transaction &tx) const {

      auto byte_hash = tx.byte_hash();

      size_t x = 0;
      x |= static_cast<unsigned long>(byte_hash[0]) << 0;
      x |= static_cast<unsigned long>(byte_hash[1]) << 8;
      x |= static_cast<unsigned long>(byte_hash[2]) << 16;
      x |= static_cast<unsigned long>(byte_hash[3]) << 24;
      x |= static_cast<unsigned long>(byte_hash[4]) << 32;
      x |= static_cast<unsigned long>(byte_hash[5]) << 40;
      x |= static_cast<unsigned long>(byte_hash[6]) << 48;
      x |= static_cast<unsigned long>(byte_hash[7]) << 56;
      return x;
    }
  };
}// namespace std

#endif//NODE_BLOCKCHAIN_TRANSACTION_H
