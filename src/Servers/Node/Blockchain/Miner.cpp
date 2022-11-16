//
// Created by mythi on 12/11/22.
//

#include "Miner.h"
#include "cryptopp/sha.h"
#include "cryptopp/hex.h"

using namespace std::chrono;

namespace krapi {

    Miner::Miner(Block last_mined) : m_last_mined(std::move(last_mined)) {

    }

    void Miner::mine(std::unordered_set<Transaction> batch) {

        spdlog::info("Miner: starting miner to mine the following transactions...");
        for (const auto &tx: batch) {
            spdlog::info("Miner: {}", tx.to_json().dump());
        }

        auto previous_hash = m_last_mined.hash();
        spdlog::info("Miner: PreviousHash: {}\n", m_last_mined.hash());

        merkle::TreeT<32, krapi_hash_function> tree;
        for (const auto &tx: batch) {
            tree.insert(tx.hash());
        }

        auto merkle_root = tree.root().to_string(32, false);

        spdlog::info("Miner: MerkleRoot: {}\n", merkle_root);

        auto timestamp = (uint64_t) duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        spdlog::info("Miner: TimeStamp: {}\n", timestamp);


        for (uint64_t nonce = 0;; nonce++) {
            auto block_hash = std::string{};
            StringSource s2(
                    fmt::format("{}{}{}{}", previous_hash, merkle_root, timestamp, nonce),
                    true,
                    new HashFilter(Miner::sha_256, new HexEncoder(new StringSink(block_hash)))
            );

            spdlog::info("Miner: Produced Hash: {}", block_hash);
            if (block_hash.starts_with("00")) {
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

                {
                    std::lock_guard l(miner_mutex);
                    m_last_mined = block;
                }
                m_dispatcher.dispatch(Event::BlockMined, block);
                break;
            }
        }


    }

    void Miner::append_listener(Event event, const std::function<OnBlockMinedCallback> &callback) {

        m_dispatcher.appendListener(event, callback);
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