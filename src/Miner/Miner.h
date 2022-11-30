//
// Created by mythi on 12/11/22.
//

#pragma once

#include <future>
#include <functional>
#include <unordered_set>
#include <chrono>
#include "Block.h"
#include "merklecpp.h"
#include "sha.h"
#include "AsyncQueue.h"
#include "fmt/core.h"

using namespace std::chrono;


namespace krapi {

    class MiningJob {
        std::string m_previous_hash;
        std::set<Transaction> m_transactions;

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

    public:

        MiningJob(
                std::string previous_hash,
                std::set<Transaction> transactions
        ) : m_previous_hash(std::move(previous_hash)),
            m_transactions(std::move(transactions)) {

        }


        Block operator()() {

            CryptoPP::SHA256 sha_256;
            merkle::TreeT<32, hash_function> tree;
            for (const auto &tx: m_transactions) {
                tree.insert(tx.hash());
            }

            auto merkle_root = tree.root().to_string(32, false);

            auto timestamp = (uint64_t) duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

            for (uint64_t nonce = 0;; nonce++) {

                auto block_hash = std::string{};
                StringSource s2(
                        fmt::format("{}{}{}{}", m_previous_hash, merkle_root, timestamp, nonce),
                        true,
                        new HashFilter(sha_256, new HexEncoder(new StringSink(block_hash)))
                );

                if (block_hash.starts_with("000")) {

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
            }
        }

        [[nodiscard]]
        std::string previous_hash() const {

            return m_previous_hash;
        }

        [[nodiscard]]
        std::set<Transaction> transactions() const {

            return m_transactions;
        }

        [[nodiscard]]
        bool operator==(const MiningJob &other) const {

            return m_previous_hash == other.m_previous_hash;
        }
    };

    class Miner {

    private:

        AsyncQueue m_queue;
        std::unordered_set<std::string> m_used;


    public:

        explicit Miner();

        std::optional<std::future<Block>> mine(std::string previous_hash, std::set<Transaction>);

        bool was_used(std::string hash);
    };

} // krapi
