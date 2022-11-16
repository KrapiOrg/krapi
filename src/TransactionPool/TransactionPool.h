//
// Created by mythi on 12/11/22.
//

#pragma once

#include <thread>
#include <unordered_set>
#include "eventpp/eventdispatcher.h"

#include "Transaction.h"

namespace krapi {

    class TransactionPool {
    public:
        enum class Event {
            TransactionAdded,
            TransactionRemoved,
            PoolCleared,
            BatchAdded,
            BatchRemoved,
            BatchSizeReached,
        };
    private:
        std::mutex m_pool_mutex;
        std::unordered_set<Transaction> m_pool;
        using TxEventDispatcher = eventpp::EventDispatcher<Event, void(Transaction)>;
        using BatchEventDispatcher = eventpp::EventDispatcher<Event, void(std::unordered_set<Transaction>)>;

        TxEventDispatcher m_tx_events;
        BatchEventDispatcher m_batch_events;

        int m_batchsize;

        void dispatch_if_batchsize_reached();

    public:

        explicit TransactionPool(int batchsize = 3);

        bool add(const Transaction &transaction);

        bool add(const std::unordered_set<Transaction> &transactions);

        bool remove(const Transaction &transaction);

        bool remove(const std::unordered_set<Transaction> &transactions);

        void append_listener(Event event, std::function<void(Transaction)> listener);

        void append_listener(Event event, std::function<void(std::unordered_set<Transaction>)> listener);

        int batchsize() const;
    };

} // krapi
