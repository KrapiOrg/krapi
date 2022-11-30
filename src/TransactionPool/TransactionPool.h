//
// Created by mythi on 12/11/22.
//

#pragma once

#include <thread>
#include <set>
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
        std::set<Transaction> m_pool;
        using TxEventDispatcher = eventpp::EventDispatcher<Event, void(Transaction)>;

        TxEventDispatcher m_tx_events;

        int m_batchsize;

        std::condition_variable m_blocking_cv;

    public:

        explicit TransactionPool(int batchsize = 3);

        bool add(const Transaction &transaction);

        void remove(const std::set<Transaction> &transactions);

        void append_listener(Event event, std::function<void(Transaction)> listener);

        std::optional<std::set<Transaction>> get_a_batch();

        void add(const std::set<Transaction> &transactions);

        std::set<Transaction> get_pool();

        void wait();
    };

} // krapi
