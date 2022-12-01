//
// Created by mythi on 12/11/22.
//

#include "TransactionPool.h"

#include <utility>
#include "spdlog/spdlog.h"

namespace krapi {


    TransactionPool::TransactionPool(int batchsize) :
            m_batchsize(batchsize) {
    }

    bool TransactionPool::add(const Transaction &transaction) {

        bool added;

        std::lock_guard l(m_pool_mutex);
        added = m_pool.insert(transaction).second;

        if (added) {

            m_blocking_cv.notify_all();
        }
        return added;
    }

    void TransactionPool::add(const std::set<Transaction> &transactions) {

        std::lock_guard l(m_pool_mutex);

        bool added = true;

        for (const auto &transaction: transactions) {
            if (!m_pool.insert(transaction).second) {
                added = false;
            }
        }
        if (added) {

            m_blocking_cv.notify_all();
        }
    }

    void TransactionPool::remove(const std::set<Transaction> &transactions) {

        std::lock_guard l(m_pool_mutex);
        for (const auto &transaction: transactions) {
            m_pool.erase(transaction);
        }
    }

    std::optional<std::set<Transaction>> TransactionPool::get_a_batch() {

        std::lock_guard l(m_pool_mutex);

        if (m_pool.size() < m_batchsize)
            return {};

        auto batch = std::set<Transaction>{
                m_pool.begin(),
                std::next(m_pool.begin(), m_batchsize)
        };
        for (const auto &tx: batch)
            m_pool.erase(tx);

        return batch;
    }

    std::set<Transaction> TransactionPool::get_pool() {

        std::lock_guard l(m_pool_mutex);
        return m_pool;
    }

    void TransactionPool::wait() {

        std::unique_lock l(m_pool_mutex);
        m_blocking_cv.wait(l, [&]() {
            return m_pool.size() >= m_batchsize;
        });
    }
} // krapi