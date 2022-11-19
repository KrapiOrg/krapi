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
            m_batchsize(batchsize),
            m_closed(false) {

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

    void TransactionPool::restore(const Transaction &transaction) {


        std::lock_guard l(m_pool_mutex);
        m_pool.insert(transaction);
    }

    void TransactionPool::restore(const std::unordered_set<Transaction> &transactions) {


        std::lock_guard l(m_pool_mutex);

        for (const auto &transaction: transactions) {
            m_pool.insert(transaction);
        }
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

    bool TransactionPool::is_pool_closed() const {

        return m_closed;
    }

    void TransactionPool::open_pool() {

        m_closed = false;
        dispatch_if_batchsize_reached();
    }

    void TransactionPool::close_pool() {

        m_closed = true;
    }

    void TransactionPool::clear_pool() {

        std::lock_guard l(m_pool_mutex);
        m_pool.clear();
    }

    void TransactionPool::remove(const std::unordered_set<Transaction> &transactions) {

        std::lock_guard l(m_pool_mutex);
        for (const auto &transaction: transactions) {
            m_pool.erase(transaction);
        }
    }
} // krapi