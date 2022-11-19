//
// Created by mythi on 12/10/22.
//
#include "NodeManager.h"
#include "Blockchain/Blockchain.h"
#include "Miner.h"
#include "TransactionPool.h"
#include "Content/SetTransactionStatusContent.h"
#include "Content/BlocksResponseContent.h"

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
                auto transaction = Transaction::from_json(message.content);
                spdlog::info("Main: Received Transaction {}", transaction.hash().substr(0, 10));
                transaction_pool.add(transaction);
            }
    );

    manager.append_listener(
            PeerMessageType::SyncBlockchain,
            [&](PeerMessage message) {
                auto offered_hashes = message.content.get<std::vector<std::string>>();

                auto needed_hashes = std::vector<std::string>();
                for (const auto &hash: offered_hashes) {

                    if (!blockchain.contains(hash)) {
                        needed_hashes.push_back(hash);
                    }
                }

                manager.broadcast_message(

                        PeerMessage{
                                PeerMessageType::RequestBlocks,
                                manager.id(),
                                needed_hashes
                        }
                );
            }
    );

    manager.append_listener(
            PeerMessageType::RequestBlocks,
            [&](PeerMessage message) {
                auto needed_hashes = message.content.get<std::vector<std::string>>();

                auto response_blocks = std::list<Block>();
                for (const auto &hash: needed_hashes) {
                    if (blockchain.contains(hash)) {
                        response_blocks.push_back(blockchain.get_block(hash));
                    }
                }
                manager.send_message(
                        message.peer_id,
                        PeerMessage{
                                PeerMessageType::BlocksResponse,
                                manager.id(),
                                BlocksResponseContent{
                                        response_blocks
                                }.to_json()
                        }
                );
            }
    );


    manager.append_listener(
            PeerMessageType::BlocksResponse,
            [&, path](PeerMessage message) {
                auto blocks = BlocksResponseContent::from_json(message.content).blocks();
                for (const auto &block: blocks) {
                    if (!blockchain.contains(block.hash())) {

                        blockchain.add(block);
                        block.to_disk(path);
                    }
                }
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

                manager.broadcast_message(
                        PeerMessage{
                                PeerMessageType::SyncBlockchain,
                                manager.id(),
                                blockchain.get_hashes()
                        }
                );


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

    manager.wait();
}
