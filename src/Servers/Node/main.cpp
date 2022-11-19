//
// Created by mythi on 12/10/22.
//
#include "NodeManager.h"
#include "Blockchain/Blockchain.h"
#include "Miner.h"
#include "TransactionPool.h"
#include "Content/SetTransactionStatusContent.h"
#include "Content/SyncBlockchainResponseContent.h"

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
    auto miner = Miner();
    miner.set_latest_hash(blockchain.last().hash());
    auto transaction_pool = TransactionPool(5);

    NodeManager manager;

    transaction_pool.append_listener(
            TransactionPool::Event::TransactionRejectedPoolClosed,
            [&manager](Transaction transaction) {
                spdlog::warn(
                        "Main: Transaction pool is closed, transaction {} has been rejected",
                        transaction.hash().substr(0, 6)
                );
                manager.send_message(
                        transaction.from(),
                        PeerMessage{
                                PeerMessageType::SetTransactionStatus,
                                manager.id(),
                                SetTransactionStatusContent{
                                        TransactionStatus::Rejected,
                                        transaction.hash()
                                }.to_json()
                        }
                );
            }
    );

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
            [&, path](PeerMessage message) {
                auto received_block = Block::from_json(message.content);

                spdlog::info("Main: AddBlock, {}", received_block.to_json().dump(4));

                spdlog::info("Main: AddBlock, Closing pool");
                transaction_pool.close_pool();
                spdlog::info("Main: AddBlock, Cancelling mining jobs");
                miner.cancel_all();

                auto current_last_hash = blockchain.last().hash();

                if (current_last_hash != received_block.header().previous_hash()) {
                    spdlog::warn(
                            "Main: AddBlock, requesting blockchain sync"
                    );
                    manager.broadcast_message(
                            PeerMessage{
                                    PeerMessageType::SyncBlockchainRequest,
                                    manager.id(),
                                    current_last_hash
                            }
                    );
                } else {
                    spdlog::warn(
                            "Main: AddBlock, setting miner latest to {}",
                            received_block.hash().substr(0, 10)
                    );
                    miner.set_latest_hash(received_block.hash());
                    blockchain.add(received_block);
                    received_block.to_disk(path);
                    transaction_pool.open_pool();
                }
            }
    );

    manager.append_listener(
            PeerMessageType::SyncBlockchainRequest,
            [&blockchain, &manager](PeerMessage message) {

                auto block_hash = message.content.get<std::string>();
                spdlog::info("Main: SyncBlockchainRequest from {}, with {}", message.peer_id, block_hash);

                auto blocks = blockchain.get_after(block_hash);

                spdlog::info(
                        "Main: SyncBlockchainRequest, Responding with {} blocks after {}",
                        blocks.size(),
                        block_hash.substr(0, 10)
                );
                for (const auto &blk: blocks) {
                    spdlog::info("Main: SyncBlockchainRequest, {}", blk.hash());
                }

                manager.send_message(
                        message.peer_id,
                        PeerMessage{
                                PeerMessageType::SyncBlockchainResponse,
                                manager.id(),
                                SyncBlockchainResponseContent{
                                        std::move(blocks)
                                }.to_json()
                        }
                );
            }
    );

    manager.append_listener(
            PeerMessageType::SyncBlockchainResponse,
            [&, path](PeerMessage message) {
                auto content = SyncBlockchainResponseContent::from_json(message.content);
                spdlog::warn("Main: SyncBlockchainResponse, closing pool");
                transaction_pool.close_pool();
                spdlog::warn("Main: SyncBlockchainResponse, cancelling all jobs in the miner");
                miner.cancel_all();

                if (content.blocks().empty()) {
                    transaction_pool.open_pool();
                } else if (!blockchain.append_to_end(content.blocks())) {

                    spdlog::warn("Main: SyncBlockchainResponse, Failed to append...");

                    for (const auto &block: content.blocks()) {

                        spdlog::warn("Main: SyncBlockchainResponse, Block {}", block.hash().substr(0, 10));
                    }
                    spdlog::warn("Main: Blockchain State");
                    blockchain.dump();
                } else {
                    spdlog::info("Main: SyncBlockchainResponse, State after appending");
                    blockchain.dump();

                    for (const auto &block: content.blocks())
                        block.to_disk(path);
                    auto last_hash = blockchain.last().hash();
                    spdlog::info("Main: SyncBlockchainResponse, Setting miners hash to: {}", last_hash);
                    miner.set_latest_hash(last_hash);
                    spdlog::info("Main: SyncBlockchainResponse, Opening Transaction Pool");
                    transaction_pool.open_pool();
                }
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
                spdlog::info("Main: BlockMined, Settings latest hash");
                miner.set_latest_hash(block.hash());
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
            Miner::BatchEvent::BatchCancelled,
            [&](std::unordered_set<Transaction> cancelled_transactions) {
                spdlog::warn("Main: A batch is being cancelled");
                for (const auto &cancelled_transaction: cancelled_transactions) {
                    manager.send_message(
                            cancelled_transaction.from(),
                            PeerMessage{
                                    PeerMessageType::SetTransactionStatus,
                                    manager.id(),
                                    SetTransactionStatusContent{
                                            TransactionStatus::Rejected,
                                            cancelled_transaction.hash()
                                    }.to_json()
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
