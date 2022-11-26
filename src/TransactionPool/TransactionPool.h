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
            TransactionAdded
        };
    private:
        std::mutex m_pool_mutex;
        std::unordered_set<Transaction> m_pool;
        using TxEventDispatcher = eventpp::EventDispatcher<Event, void(Transaction)>;

        TxEventDispatcher m_tx_events;

        int m_batchsize;

    public:

        explicit TransactionPool(int batchsize = 3);

        bool add(const Transaction &transaction);

        void remove(const std::unordered_set<Transaction> &transactions);

        void append_listener(Event event, std::function<void(Transaction)> listener);

        std::optional<std::unordered_set<Transaction>> get_a_batch();
    };

} // krapi
