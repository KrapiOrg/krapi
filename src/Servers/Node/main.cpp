//
// Created by mythi on 12/10/22.
//
#include "NodeManager.h"
#include "Blockchain/Blockchain.h"
#include "Miner.h"
#include "TransactionPool.h"

using namespace krapi;
using namespace std::chrono_literals;

int main(int argc, char *argv[]) {

    std::string path;

    if (argc == 2) {
        path = std::string{argv[1]};
    } else {
        path = "blockchain";
    }

    auto blockchain = Blockchain::from_disk(path);
    auto miner = Miner(blockchain.last());
    auto transaction_pool = TransactionPool();

    NodeManager manager;

    manager.append_listener(
            NodeManager::Event::TransactionReceived,
            [&transaction_pool](Transaction transaction) {
                spdlog::info("Main: Received Transaction {}", transaction.to_json().dump(4));
                transaction_pool.add(transaction);
            }
    );

    manager.append_listener(
            NodeManager::Event::BlockReceived,
            [path, &blockchain](Block block) {
                spdlog::info("Main: Received Block {}", block.to_json().dump(4));
                blockchain.add(block);
                block.to_disk(path);
            }
    );

    transaction_pool.append_listener(
            TransactionPool::Event::BatchSizeReached,
            [&miner](std::unordered_set<Transaction> transactions) {
                spdlog::info("Main: Batchsize Reached");
                miner.mine(std::move(transactions));
            }
    );

    miner.append_listener(
            Miner::Event::BlockMined, [&](Block block) {
                spdlog::info("Main: Mined block {}", block.to_json().dump(4));
                blockchain.add(block);
                block.to_disk(path);
                manager.broadcast_message(PeerMessage{PeerMessageType::AddBlock, block.to_json()});
            }
    );

    manager.wait();
}
