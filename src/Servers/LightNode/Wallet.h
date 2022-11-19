//
// Created by mythi on 18/11/22.
//

#pragma once

#include <mutex>
#include <unordered_map>

#include "cryptopp/sha.h"
#include "effolkronium/random.hpp"
#include "eventpp/eventdispatcher.h"

#include "Transaction.h"

namespace krapi {

    class Wallet {
    public:
        enum class Event {
            TransactionStatusChanged
        };

    private:
        using Random = effolkronium::random_local;
        using TransactionChangeCallback = void(Transaction, TransactionStatus, TransactionStatus);
        using TransactionStatusChangeDispatcher = eventpp::EventDispatcher<Event, TransactionChangeCallback>;

        TransactionStatusChangeDispatcher m_transaction_status_change_dispatcher;

        CryptoPP::SHA256 m_sha_256;
        Random m_random;

        std::mutex m_mutex;
        std::unordered_map<std::string, Transaction> m_transactions;
        std::unordered_map<std::string, int> m_confirmations;



    public:

        bool add_transaction(Transaction);
        void set_transaction_status(TransactionStatus, std::string);

        Transaction create_transaction(int my_id, int receiver_id);

        void append_listener(Event, std::function<TransactionChangeCallback>);
    };

} // krapi
