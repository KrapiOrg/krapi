//
// Created by mythi on 12/11/22.
//

#include "TransactionPool.h"

namespace krapi {
    void TransactionPool::dispatch_if_batchsize_reached() {

        std::lock_guard l(m_pool_mutex);
        if (m_pool.size() == m_batchsize) {

            m_batch_events.dispatch(Event::BatchSizeReached, m_pool);
            m_pool.clear();
            m_batch_events.dispatch(Event::PoolCleared, m_pool);
        }
    }

    TransactionPool::TransactionPool(int batchsize) : m_batchsize(batchsize) {

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

    bool TransactionPool::add(const std::unordered_set<Transaction> &transactions) {

        {
            std::lock_guard l(m_pool_mutex);
            for (const auto &transaction: transactions) {
                if (m_pool.contains(transaction))
                    return false;
            }

            for (const auto &transaction: transactions) {
                m_pool.insert(transaction);
            }
        }
        m_batch_events.dispatch(Event::BatchAdded, transactions);
        dispatch_if_batchsize_reached();
        return true;
    }

    bool TransactionPool::remove(const Transaction &transaction) {

        bool removed{false};
        {
            std::lock_guard l(m_pool_mutex);
            removed = m_pool.erase(transaction) == 1;
        }
        if (removed) {
            m_tx_events.dispatch(Event::TransactionRemoved, transaction);
        }
        return removed;
    }

    bool TransactionPool::remove(const std::unordered_set<Transaction> &transactions) {

        {
            std::lock_guard l(m_pool_mutex);
            for (const auto &transaction: transactions) {
                if (m_pool.contains(transaction))
                    return false;
            }

            for (const auto &transaction: transactions) {
                m_pool.erase(transaction);
            }
        }

        m_batch_events.dispatch(Event::BatchRemoved, transactions);

        return true;
    }

    void TransactionPool::append_listener(TransactionPool::Event event, std::function<void(Transaction)> listener) {

        m_tx_events.appendListener(event, listener);
    }

    void TransactionPool::append_listener(TransactionPool::Event event,
                                          std::function<void(std::unordered_set<Transaction>)> listener) {

        m_batch_events.appendListener(event, listener);
    }

    int TransactionPool::batchsize() const {

        return m_batchsize;
    }
} // krapi