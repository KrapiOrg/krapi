#pragma once

#include "Transaction.h"

namespace krapi {

  class SetTransactionStatusContent {

    TransactionStatus m_status;
    std::string m_hash;

   public:
    explicit SetTransactionStatusContent(
      TransactionStatus status,
      std::string hash
    )
        : m_status(status), m_hash(std::move(hash)) {
    }

    [[nodiscard]] TransactionStatus status() const {
      return m_status;
    }

    [[nodiscard]] std::string hash() const {
      return m_hash;
    }

    static SetTransactionStatusContent from_json(nlohmann::json json) {

      return SetTransactionStatusContent{
        json["status"].get<TransactionStatus>(),
        json["hash"].get<std::string>()};
    }

    [[nodiscard]] nlohmann::json to_json() const {

      return {{"status", m_status}, {"hash", m_hash}};
    }
  };
}// namespace krapi
