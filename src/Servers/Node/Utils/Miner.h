//
// Created by mythi on 04/11/22.
//

#ifndef SHARED_MODELS_MINER_H
#define SHARED_MODELS_MINER_H

#include <unordered_set>
#include <utility>
#include <future>

#include "eventpp/eventdispatcher.h"
#include "eventpp/utilities/counterremover.h"
#include "fmt/format.h"
#include "date/date.h"
#include "merklecpp.h"

#include "Transaction.h"
#include "Block.h"
#include "Blockchain.h"

namespace krapi {
    class Miner {
    public:
        enum class Event {
            BatchMined
        };
    private:
        using MinerEventDispatcher = eventpp::EventDispatcher<Event, void(Block)>;
        MinerEventDispatcher miner_event_dispatcher;

        std::shared_ptr<Blockchain> m_blockchain;

    public:


        explicit Miner(std::shared_ptr<Blockchain> blockchain) : m_blockchain(std::move(blockchain)) {

        }

        void mine(const std::unordered_set<Transaction> &transactions) {

            spdlog::info("Miner: Starting mining job");
            merkle::Tree tree;

            spdlog::info("Miner: Creating MerkleTree");
            for (const auto &transaction: transactions) {
                tree.insert(merkle::Hash(transaction.hashcode()));
            }
            auto merkle_root = tree.root().to_string(32, false);
            spdlog::info("Miner: The merkle root is: {}", merkle_root);

            using namespace std::chrono;
            using namespace CryptoPP;

            SHA256 hash;
            auto digest = std::string{};
            int nonce = 0;
            auto timestamp = std::to_string(date::sys_time<nanoseconds>().time_since_epoch().count());
            auto previous_hash = m_blockchain->last().hash();
            auto msg_no_nonce = fmt::format("{}{}{}", previous_hash, timestamp, merkle_root);

            digest.reserve(256);
            while (!digest.starts_with("00")) {
                digest.clear();
                auto msg_with_nonce = fmt::format("{}{}", msg_no_nonce, nonce++);
                StringSource(msg_with_nonce, true, new HashFilter(hash, new HexEncoder(new StringSink(digest))));
            }

            spdlog::info("Miner: Sucessfully solved task, digest is {}, nonce is {}", digest, nonce);

            auto block = Block{
                    BlockHeader{
                            digest,
                            previous_hash,
                            timestamp,
                            merkle_root,
                            nonce - 1
                    },
                    transactions
            };

            m_blockchain->add(block);
            miner_event_dispatcher.dispatch(Event::BatchMined, block);
        }

        void append_listener(Event event, std::function<void(Block)> listener) {

            miner_event_dispatcher.appendListener(event, listener);
        }

        ~Miner() {

            spdlog::error("DROPPED MINER!!!!!!!!!!!!!!");
        }
    };
}

#endif //SHARED_MODELS_MINER_H
