//
// Created by mythi on 02/11/22.
//

#ifndef NODE_TRANSACTIONPOOL_H
#define NODE_TRANSACTIONPOOL_H

#include "eventpp/eventdispatcher.h"
#include "Transaction.h"
#include "spdlog/spdlog.h"
#include <unordered_set>

namespace krapi {

    class TransactionPool {
    public:
        enum class Event {
            Add,
            Remove
        };
    private:

        using EventDispatcher = eventpp::EventDispatcher<Event, void(const Transaction &,
                                                                     const std::unordered_set<int> &identity_blacklist)>;

        EventDispatcher dispatcher;
        std::unordered_set<Transaction> m_transactions;
        std::mutex m_transactions_mutex;

    public:

        void add_transaction(const Transaction &transaction, const std::unordered_set<int> &identity_blacklist = {}) {

            std::lock_guard l(m_transactions_mutex);
            if (m_transactions.contains(transaction)) {
                spdlog::warn("TransactionPool: Did not add transaction because it already exists");
                return;
            }
            m_transactions.insert(transaction);
            dispatcher.dispatch(Event::Add, transaction, identity_blacklist);
        }

        void remove_transaction(const Transaction &transaction, const std::unordered_set<int> &identity_blacklist = {}) {

            std::lock_guard l(m_transactions_mutex);
            if (m_transactions.contains(transaction)) {
                m_transactions.erase(transaction);
                dispatcher.dispatch(Event::Remove, transaction, identity_blacklist);
            }
        }

        [[nodiscard]]
        bool contains(const Transaction &transaction) const {

            return m_transactions.contains(transaction);
        }

        void append_listener(
                Event event,
                const std::function<void(const Transaction &,
                                         const std::unordered_set<int> &identity_blacklist)> &listener
        ) {

            dispatcher.appendListener(event, listener);
        }

    };

    using TransactionPoolPtr = std::shared_ptr<TransactionPool>;

    inline TransactionPoolPtr create_transaction_pool() {

        return std::make_shared<TransactionPool>();
    }

} // krapi

#endif //NODE_TRANSACTIONPOOL_H
