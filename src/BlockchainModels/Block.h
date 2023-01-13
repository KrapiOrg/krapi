#pragma once

#include <cstdint>
#include <set>
#include <string>

#include "nlohmann/json.hpp"

#include "BlockHeader.h"
#include "Transaction.h"

namespace krapi {

  class Block {

   public:
    explicit Block(
      BlockHeader header = BlockHeader{},
      Transactions transactions = {}
    );

    static Block from_json(const nlohmann::json &json);

    [[nodiscard]] std::string hash() const;

    [[nodiscard]] std::array<CryptoPP::byte, 32> hash_bytes() const;

    [[nodiscard]] nlohmann::json to_json() const;

    [[nodiscard]] BlockHeader header() const;

    [[nodiscard]] Transactions transactions() const;

    [[nodiscard]] std::string contrived_hash() const;

    [[nodiscard]] uint64_t timestamp() const;

    [[nodiscard]] std::string previous_hash() const;

    [[nodiscard]] bool contains_transaction(Transaction transaction) const {
      return std::find(
               m_transactions.begin(),
               m_transactions.end(),
               transaction
             ) != m_transactions.end();
    }

    bool operator==(const Block &) const;

    bool operator<(const Block &) const;

    void set_transactions_status(TransactionStatus status) {

      for (auto &transaction: m_transactions) {
        transaction.set_status(status);
      }
    }

    std::string tag() const {
      return m_header.m_hash;
    }

   private:
    BlockHeader m_header;
    Transactions m_transactions;
  };
}// namespace krapi
namespace std {
  template<>
  struct hash<krapi::Block> {
    size_t operator()(const krapi::Block &block) const {

      auto byte_hash = block.hash_bytes();

      size_t x = 0;
      x |= static_cast<unsigned long>(byte_hash[10]) << 0;
      x |= static_cast<unsigned long>(byte_hash[11]) << 8;
      x |= static_cast<unsigned long>(byte_hash[12]) << 16;
      x |= static_cast<unsigned long>(byte_hash[13]) << 24;
      x |= static_cast<unsigned long>(byte_hash[14]) << 32;
      x |= static_cast<unsigned long>(byte_hash[15]) << 40;
      x |= static_cast<unsigned long>(byte_hash[16]) << 48;
      x |= static_cast<unsigned long>(byte_hash[17]) << 56;

      return x;
    }
  };
}// namespace std
