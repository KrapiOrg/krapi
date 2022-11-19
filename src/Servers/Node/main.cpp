//
// Created by mythi on 12/10/22.
//
#include "NodeManager.h"
#include "Blockchain/Blockchain.h"
#include "Miner.h"
#include "TransactionPool.h"
#include "Content/SetTransactionStatusContent.h"

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
    auto transaction_pool = TransactionPool(10);

    NodeManager manager;

    manager.append_listener(
            PeerMessageType::AddTransaction,
            [&transaction_pool](PeerMessage message) {
                auto transaction = Transaction::from_json(message.content);

                spdlog::info("Main: Received Transaction {}", transaction.to_json().dump(4));
                transaction_pool.add(transaction);
            }
    );

    manager.append_listener(
            PeerMessageType::AddBlock,
            [path, &blockchain](PeerMessage message) {
                auto block = Block::from_json(message.content);

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
            Miner::Event::BlockMined,
            [&](Block block) {

                for (const auto &transaction: block.transactions()) {

                    manager.send_message(
                            transaction.from(),
                            PeerMessage{
                                    PeerMessageType::SetTransactionStatus,
                                    manager.id(),
                                    SetTransactionStatusContent{
                                            TransactionStatus::Verified,
                                            transaction.hash()
                                    }.to_json()
                            }
                    );
                    manager.send_message(
                            transaction.to(),
                            PeerMessage{
                                    PeerMessageType::AddTransaction,
                                    manager.id(),
                                    transaction.to_json()
                            }
                    );
                }
            }
    );

    miner.append_listener(
            Miner::Event::BlockMined, [&](Block block) {
                spdlog::info("Main: Mined block {}", block.to_json().dump(4));
                blockchain.add(block);
                block.to_disk(path);
                manager.broadcast_message(
                        PeerMessage{
                                PeerMessageType::AddBlock,
                                manager.id(),
                                block.to_json()
                        }
                );
            }
    );

    manager.wait();
}
