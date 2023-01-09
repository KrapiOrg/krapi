//
// Created by mythi on 12/11/22.
//

#pragma once

#include "DBInterface.h"
#include "Transaction.h"
#include "readerwriterqueue.h"
#include "spdlog/spdlog.h"
#include <concurrencpp/executors/thread_executor.h>
#include <concurrencpp/results/shared_result.h>
#include <memory>

namespace krapi {

  enum class PoolEndReason {
    SignalingClosed
  };

  class TransactionPool : public DBInternface<Transaction>,
                          public std::enable_shared_from_this<TransactionPool> {

   public:
    static inline std::shared_ptr<TransactionPool> create(std::string path) {
      return std::shared_ptr<TransactionPool>(new TransactionPool(std::move(path)));
    }

    bool add(Transaction transaction);
    
    void end(PoolEndReason end_reason) {
      m_queue->enqueue(end_reason);
    }

    concurrencpp::result<Transactions>
    wait(std::shared_ptr<concurrencpp::thread_executor>, int);

   private:
    explicit TransactionPool(std::string path);
    using Queue = moodycamel::BlockingReaderWriterQueue<std::variant<Transaction, PoolEndReason>>;
    using QueuePtr = std::shared_ptr<Queue>;
    QueuePtr m_queue;
  };

  using TransactionPoolPtr = std::shared_ptr<TransactionPool>;
}// namespace krapi
