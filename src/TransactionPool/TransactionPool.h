//
// Created by mythi on 12/11/22.
//

#pragma once

#include "DBInterface.h"
#include "EventQueue.h"
#include "Transaction.h"
#include <concurrencpp/results/result.h>
#include <concurrencpp/results/shared_result.h>
#include <memory>
#include <queue>
#include <valarray>

namespace krapi {

  using TransactionsPromise = concurrencpp::result_promise<Transactions>;
  using TransactionsResult = concurrencpp::shared_result<Transactions>;

  class TransactionPool : public DBInternface<Transaction>,
                          public std::enable_shared_from_this<TransactionPool> {

   public:
    static inline std::shared_ptr<TransactionPool>
    create(std::string path, EventQueuePtr event_queue) {

      return std::shared_ptr<TransactionPool>(
        new TransactionPool(std::move(path), std::move(event_queue))
      );
    }

    bool add(Transaction);

    concurrencpp::shared_result<Transactions> wait_for(int) const;

   private:
    Transactions m_pool;
    EventQueuePtr m_event_queue;
    mutable std::queue<Transaction> m_transaction_queue;

    Transactions take_from_queue(int n) const;
    explicit TransactionPool(std::string, EventQueuePtr);
  };

  using TransactionPoolPtr = std::shared_ptr<TransactionPool>;
}// namespace krapi
