//
// Created by mythi on 12/10/22.
//
#include "NodeManager.h"
#include "Blockchain/Blockchain.h"
#include "Miner.h"
#include "TransactionPool.h"
#include "Content/BlocksResponseContent.h"
#include "Content/SetTransactionStatusContent.h"

using namespace krapi;
using namespace std::chrono_literals;

int main(int argc, char *argv[]) {

    constexpr int BATCH_SZE = 20;
    std::string path;

    if (argc == 2) {
        path = std::string{argv[1]};
    } else {
        path = "blockchain";
    }

    auto blockchain = Blockchain::from_disk(path);
    auto miner = Miner();
    miner.set_latest_hash(blockchain.last().hash());
    auto transaction_pool = TransactionPool(BATCH_SZE);

    NodeManager manager;

    manager.append_listener(
            PeerMessageType::AddTransaction,
            [&transaction_pool](PeerMessage message) {
                auto transaction = Transaction::from_json(message.content());
                spdlog::info("Main: Received Transaction {}", transaction.hash().substr(0, 10));
                transaction_pool.add(transaction);
            }
    );

    transaction_pool.append_listener(
            TransactionPool::Event::BatchSizeReached,
            [&](std::unordered_set<Transaction> transactions) {
                spdlog::info("Main: BatchSizeReached");

                transaction_pool.remove(transactions);
                miner.mine(std::move(transactions));
            }
    );

    miner.append_listener(
            Miner::Event::BlockMined,
            [&, path](Block block) {
                spdlog::info("Main: BlockMined, {}", block.hash().substr(0, 10));

                miner.set_latest_hash(block.hash());
                blockchain.add(block);
                block.to_disk(path);


                for (const auto &transaction: block.transactions()) {

                    manager.send(
                            transaction.from(),
                            PeerMessage{
                                    PeerMessageType::SetTransactionStatus,
                                    manager.id(),
                                    PeerMessage::create_tag(),
                                    SetTransactionStatusContent{
                                            TransactionStatus::Verified,
                                            transaction.hash()
                                    }.to_json()
                            }
                    );
                    manager.send(
                            transaction.to(),
                            PeerMessage{
                                    PeerMessageType::AddTransaction,
                                    manager.id(),
                                    PeerMessage::create_tag(),
                                    transaction.to_json()
                            }
                    );
                }
            }
    );

    manager.wait_for(PeerType::Full, 1);

    manager.wait();
}
