#pragma once

#include "Transaction.h"
#include <set>
#include <span>

namespace krapi {
  struct PoolResponseContent {
    std::set<Transaction> m_transactions;

   public:
    explicit PoolResponseContent(std::set<Transaction> transactions)
        : m_transactions(std::move(transactions)) {
    }

    [[nodiscard]] std::set<Transaction> transactions() const {

      return m_transactions;
    }

    static PoolResponseContent from_json(nlohmann::json json) {

      auto transactions = std::set<Transaction>{};

      for (const auto &json_block: json["transactions"]) {
        transactions.insert(Transaction::from_json(json_block));
      }

      return PoolResponseContent{std::move(transactions)};
    }

    [[nodiscard]] nlohmann::json to_json() const {

      auto json = nlohmann::json::array();
      for (const auto &block: m_transactions) {
        json.push_back(block.to_json());
      }

      return {{"transactions", json}};
    }
  };
}// namespace krapi