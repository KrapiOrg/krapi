//
// Created by mythi on 12/11/22.
//

#include "TransactionPool.h"

#include "spdlog/spdlog.h"
#include <utility>

namespace krapi {


  TransactionPool::TransactionPool(int batchsize) : m_batchsize(batchsize) {}

  bool TransactionPool::add(const Transaction &transaction) {

    bool added;
    {
      std::lock_guard l(m_pool_mutex);
      added = m_pool.insert(transaction).second;
    }

    if (added) {
      if (m_pool.size() >= m_batchsize) {
        std::set<Transaction> batch;
        {
          std::lock_guard l(m_pool_mutex);
          batch = m_pool;
          m_pool.clear();
        }
        m_batch_queue.push_task([batch = std::move(batch), this]() {
          m_on_batch_callback(batch);
        });
      }
    }

    return added;
  }

  void TransactionPool::remove(const std::set<Transaction> &transactions) {

    std::lock_guard l(m_pool_mutex);
    for (const auto &transaction: transactions) { m_pool.erase(transaction); }
  }

  void TransactionPool::on_batch(OnBatchCallback callback) {

    m_on_batch_callback = std::move(callback);
  }

}// namespace krapi