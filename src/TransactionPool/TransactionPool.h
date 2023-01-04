//
// Created by mythi on 12/11/22.
//

#pragma once

#include "AsyncQueue.h"
#include "eventpp/eventdispatcher.h"
#include <set>
#include <thread>

#include "Transaction.h"

namespace krapi {

  class TransactionPool {

    using OnBatchCallback = std::function<void(std::set<Transaction>)>;

    std::mutex m_pool_mutex;
    std::set<Transaction> m_pool;
    AsyncQueue m_batch_queue;
    OnBatchCallback m_on_batch_callback;

    int m_batchsize;

   public:
    explicit TransactionPool(int batchsize = 3);

    bool add(const Transaction &transaction);

    void remove(const std::set<Transaction> &transactions);

    void on_batch(OnBatchCallback callback);
  };

}// namespace krapi
