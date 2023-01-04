//
// Created by mythi on 12/11/22.
//

#pragma once

#include <unordered_set>
#include "Block.h"
#include "merklecpp.h"
#include "sha.h"
#include "fmt/core.h"

using namespace std::chrono;


namespace krapi {

    struct MiningJob {

        eventpp::ScopedRemover<EventQueueType> m_event_queue;
        std::string m_previous_hash;
        std::set<Transaction> m_transactions;
        bool m_stop;

        static void hash_function(
                const merkle::HashT<32> &l,
                const merkle::HashT<32> &r,
                merkle::HashT<32> &out
        ) {

            CryptoPP::SHA256 sha_256;
            sha_256.Update(l.bytes, 32);
            sha_256.Update(r.bytes, 32);
            sha_256.Final(out.bytes);
        }

        std::string obtain_merkle_root() {

            merkle::TreeT<32, hash_function> tree;
            for (const auto &tx: m_transactions) {
                tree.insert(tx.hash());
            }
            return tree.root().to_string(32, false);
        }

        uint64_t obtain_current_timestamp() {

            return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        }

    public:

        explicit MiningJob(
                EventQueuePtr event_queue,
                std::string hash,
                std::set<Transaction> transactions
        ) :
                m_stop(false),
                m_event_queue(event_queue->internal_queue()),
                m_previous_hash(std::move(hash)),
                m_transactions(std::move(transactions)) {

            m_event_queue.appendListener(
                    PeerMessageType::BlockAccepted,
                    [this](Event event) {

                        m_stop = true;
                    }
            );

        }

        std::optional<Block> operator()() {

            CryptoPP::SHA256 sha_256;

            auto merkle_root = obtain_merkle_root();

            auto timestamp = obtain_current_timestamp();

            uint64_t nonce = 0;
            while (!m_stop) {

                auto block_hash = std::string{};

                StringSource s2(
                        fmt::format("{}{}{}{}", m_previous_hash, merkle_root, timestamp, nonce),
                        true,
                        new HashFilter(sha_256, new HexEncoder(new StringSink(block_hash)))
                );

                if (block_hash.starts_with("00000")) {

                    return Block{
                            BlockHeader{
                                    block_hash,
                                    m_previous_hash,
                                    merkle_root,
                                    timestamp,
                                    nonce
                            },
                            m_transactions
                    };
                }
                nonce++;
            }
            return {};
        }
    };

    class Miner {
        std::shared_ptr<concurrencpp::worker_thread_executor> m_worker;

        EventQueuePtr m_event_queue;

    public:
        Miner(
                std::shared_ptr<concurrencpp::worker_thread_executor> worker,
                EventQueuePtr event_queue
        ) : m_event_queue(event_queue),
            m_worker(std::move(worker)) {

        }

        concurrencpp::shared_result<std::optional<Block>> mine(
                std::string previous_hash,
                std::set<Transaction> transactions
        ) {

            return
                    m_worker->submit(
                            MiningJob{
                                    m_event_queue,
                                    std::move(previous_hash),
                                    std::move(transactions)
                            }
                    );
        }
    };

} // krapi
