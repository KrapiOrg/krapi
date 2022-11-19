//
// Created by mythi on 12/11/22.
//

#include "Miner.h"

#include <utility>
#include "cryptopp/hex.h"
#include "spdlog/spdlog.h"


using namespace std::chrono;

namespace krapi {

    Miner::Miner() : m_cancelled(false) {

    }

    void Miner::async_mine(std::unordered_set<Transaction> batch) {

        spdlog::info("Miner: starting miner to mine the following transactions...");
        for (const auto &tx: batch) {
            spdlog::info("Miner: {}", tx.to_json().dump());
        }
        auto previous_hash = std::string{};
        {
            std::lock_guard l(m_mutex);
            previous_hash = m_latest_hash;
        }
        spdlog::info("Miner: PreviousHash: {}\n", previous_hash);

        merkle::TreeT<32, krapi_hash_function> tree;
        for (const auto &tx: batch) {
            tree.insert(tx.hash());
        }

        auto merkle_root = tree.root().to_string(32, false);

        spdlog::info("Miner: MerkleRoot: {}\n", merkle_root);

        auto timestamp = (uint64_t) duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        spdlog::info("Miner: TimeStamp: {}\n", timestamp);


        for (uint64_t nonce = 0;; nonce++) {

            if (m_cancelled) break;

            auto block_hash = std::string{};
            StringSource s2(
                    fmt::format("{}{}{}{}", previous_hash, merkle_root, timestamp, nonce),
                    true,
                    new HashFilter(Miner::sha_256, new HexEncoder(new StringSink(block_hash)))
            );

            spdlog::info("Miner: Produced Hash: {}", block_hash);
            if (block_hash.starts_with("0000")) {

                for (auto &transaction: batch)
                    transaction.set_status(TransactionStatus::Verified);

                auto block = Block{
                        BlockHeader{
                                block_hash,
                                previous_hash,
                                merkle_root,
                                timestamp,
                                nonce
                        },
                        batch
                };
                if (!m_cancelled) {
                    m_dispatcher.dispatch(Event::BlockMined, block);
                    break;
                }
            }
        }
    }

    void Miner::mine(std::unordered_set<Transaction> batch) {

        {
            std::lock_guard l(m_mutex);
            m_futures.emplace_back(
                    std::async(std::launch::async, std::bind_front(&Miner::async_mine, this, batch))
            );
            m_batches.push_back(batch);
        }
        m_batch_dispatcher.dispatch(BatchEvent::BatchSubmitted, batch);
    }

    void Miner::cancel_all() {

        m_cancelled = true;
        {
            std::lock_guard l(m_mutex);
            for (int i = 0; auto &f: m_futures) {
                f.get();
                m_batch_dispatcher.dispatch(BatchEvent::BatchCancelled, m_batches[i++]);
            }
            m_futures.clear();
        }
        m_cancelled = false;
        spdlog::info("Miner: Cancelled all batches!");
    }

    void Miner::set_latest_hash(std::string hash) {

        std::lock_guard l(m_mutex);
        m_latest_hash = std::move(hash);
    }

    void Miner::append_listener(Event event, const std::function<OnBlockMinedCallback> &callback) {

        m_dispatcher.appendListener(event, callback);
    }

    void Miner::append_listener(BatchEvent event, const std::function<OnBatchEventCallback> &callback) {

        m_batch_dispatcher.appendListener(event, callback);
    }

    void Miner::krapi_hash_function(
            const merkle::HashT<32> &l,
            const merkle::HashT<32> &r,
            merkle::HashT<32> &out
    ) {

        Miner::sha_256.Update(l.bytes, 32);
        Miner::sha_256.Update(r.bytes, 32);
        Miner::sha_256.Final(out.bytes);
    }


} // krapi