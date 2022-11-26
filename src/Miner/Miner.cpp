//
// Created by mythi on 12/11/22.
//

#include "Miner.h"

#include <utility>
#include "cryptopp/hex.h"
#include "spdlog/spdlog.h"


using namespace std::chrono;

namespace krapi {

    void Miner::mine(std::string previous_hash, std::unordered_set<Transaction> batch) {

        spdlog::info("Miner: using {} as tip", previous_hash.substr(0, 10));
        spdlog::info("Miner: Batch...");
        for (const auto &tx: batch) {
            spdlog::info("=== TX#{}", tx.hash().substr(0, 10));
        }

        merkle::TreeT<32, krapi_hash_function> tree;
        for (const auto &tx: batch) {
            tree.insert(tx.hash());
        }

        auto merkle_root = tree.root().to_string(32, false);

        auto timestamp = (uint64_t) duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        for (uint64_t nonce = 0;; nonce++) {

            auto block_hash = std::string{};
            StringSource s2(
                    fmt::format("{}{}{}{}", previous_hash, merkle_root, timestamp, nonce),
                    true,
                    new HashFilter(Miner::sha_256, new HexEncoder(new StringSink(block_hash)))
            );

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