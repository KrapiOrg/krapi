//
// Created by mythi on 12/11/22.
//

#include "TransactionPool.h"

namespace krapi {
    void TransactionPool::dispatch_if_batchsize_reached() {


        auto pool = std::unordered_set<Transaction>{};
        {
            std::lock_guard l(m_pool_mutex);
            pool = m_pool;
        }

        if (pool.size() >= m_batchsize) {

            m_batch_events.dispatch(Event::BatchSizeReached, pool);
        }
    }

    TransactionPool::TransactionPool(int batchsize) :
            m_batchsize(batchsize) {

    }

    bool TransactionPool::add(const Transaction &transaction) {

        bool added{false};

        {
            std::lock_guard l(m_pool_mutex);
            added = m_pool.insert(transaction).second;
        }

        if (added) {

            m_tx_events.dispatch(Event::TransactionAdded, transaction);
            dispatch_if_batchsize_reached();
        }
        return added;
    }

    void TransactionPool::append_listener(TransactionPool::Event event, std::function<void(Transaction)> listener) {

        m_tx_events.appendListener(event, listener);
    }

    void TransactionPool::append_listener(TransactionPool::Event event,
                                          std::function<void(std::unordered_set<Transaction>)> listener) {

        m_batch_events.appendListener(event, listener);
    }

    void TransactionPool::remove(const std::unordered_set<Transaction> &transactions) {

        std::lock_guard l(m_pool_mutex);
        for (const auto &transaction: transactions) {
            m_pool.erase(transaction);
        }
    }
} // krapi