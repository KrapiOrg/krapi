//
// Created by mythi on 12/11/22.
//

#include "TransactionPool.h"

namespace krapi {


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
        }
        return added;
    }

    void TransactionPool::append_listener(TransactionPool::Event event, std::function<void(Transaction)> listener) {

        m_tx_events.appendListener(event, listener);
    }

    void TransactionPool::remove(const std::unordered_set<Transaction> &transactions) {

        std::lock_guard l(m_pool_mutex);
        for (const auto &transaction: transactions) {
            m_pool.erase(transaction);
        }
    }

    std::optional<std::unordered_set<Transaction>> TransactionPool::get_a_batch() {

        std::lock_guard l(m_pool_mutex);

        if (m_pool.size() < m_batchsize)
            return {};

        auto batch = std::unordered_set<Transaction>{
                m_pool.begin(),
                std::next(m_pool.begin(), m_batchsize)
        };
        for (const auto &tx: batch)
            m_pool.erase(tx);

        return batch;
    }
} // krapi