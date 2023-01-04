//
// Created by mythi on 18/11/22.
//

#pragma once

#include <memory>
#include <unordered_map>

#include "cryptopp/sha.h"

#include "Transaction.h"

namespace krapi {

  class Wallet {

   private:
    std::unordered_map<std::string, Transaction> m_transactions;
    std::unordered_map<std::string, int> m_confirmations;

    Wallet() = default;

   public:
    bool add_transaction(Transaction);
    void set_transaction_status(TransactionStatus, std::string);

    static std::shared_ptr<Wallet> create() {

      return std::shared_ptr<Wallet>(new Wallet());
    }

    Transaction create_transaction(std::string my_id, std::string receiver_id);
  };
  using WalletPtr = std::shared_ptr<Wallet>;
}// namespace krapi
