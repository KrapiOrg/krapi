//
// Created by mythi on 18/11/22.
//

#include <chrono>
#include "fmt/format.h"
#include "spdlog/spdlog.h"

#include "Wallet.h"

using namespace CryptoPP;
using namespace std::chrono;

namespace krapi {

    void Wallet::set_transaction_status(TransactionStatus status, std::string hash) {

        std::lock_guard l(m_mutex);
        auto transaction_itr = m_transactions.find(hash);

        if (transaction_itr != m_transactions.end()) {
            auto &transaction = transaction_itr->second;
            auto status_before = transaction.status();

            if (status_before != status) {

                if (transaction.set_status(status)) {

                    m_transaction_status_change_dispatcher.dispatch(
                            Event::TransactionStatusChanged,
                            transaction,
                            status_before, status
                    );
                    m_confirmations[hash]++;
                    spdlog::info(
                            "Wallet: Transaction {} has {} confirmations",
                            transaction.hash().substr(0, 6),
                            m_confirmations[hash]
                    );
                }
            }
        }
    }

    Transaction Wallet::create_transaction(int my_id, int receiver_id) {

        auto timestamp = (uint64_t) duration_cast<microseconds>(
                system_clock::now().time_since_epoch()
        ).count();

        std::string tx_hash;
        StringSource s(
                fmt::format(
                        "{}{}{}{}",
                        "send_tx",
                        timestamp,
                        my_id,
                        receiver_id
                ),
                true,
                new HashFilter(m_sha_256, new HexEncoder(new StringSink(tx_hash)))
        );
        auto tx = Transaction{
                TransactionType::Send,
                TransactionStatus::Pending,
                tx_hash,
                timestamp,
                my_id,
                receiver_id
        };
        std::lock_guard l(m_mutex);

        m_transactions.emplace(tx_hash, tx);

        return tx;
    }

    void Wallet::append_listener(Event event, std::function<TransactionChangeCallback> callback) {

        m_transaction_status_change_dispatcher.appendListener(event, callback);
    }

    bool Wallet::add_transaction(Transaction transaction) {

        std::lock_guard l(m_mutex);
        return m_transactions.emplace(transaction.hash(), transaction).second;
    }
} // krapi