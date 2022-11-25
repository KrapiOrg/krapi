//
// Created by mythi on 12/10/22.
//
#include "NodeManager.h"
#include "Blockchain/Blockchain.h"
#include "Miner.h"
#include "TransactionPool.h"
#include "Content/BlocksResponseContent.h"
#include "Content/SetTransactionStatusContent.h"
#include "Content/BlockHeadersResponseContent.h"

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
    auto transaction_pool = TransactionPool(BATCH_SZE);

    auto manager = std::make_shared<NodeManager>();

    manager->append_listener(
            PeerMessageType::PeerStateRequest,
            [&](const PeerMessage &message) {

                manager->send(
                        message.peer_id(),
                        PeerMessage{
                                PeerMessageType::PeerStateResponse,
                                manager->id(),
                                message.tag(),
                                manager->get_state()
                        }
                );
            }
    );

    manager->append_listener(
            PeerMessageType::BlockRequest,
            [&](const PeerMessage &message) {
                auto hash = message.content().get<std::string>();

                manager->send(
                        message.peer_id(),
                        PeerMessage{
                                PeerMessageType::BlockResponse,
                                manager->id(),
                                message.tag(),
                                blockchain.get_block(hash).to_json()
                        }
                );
            }
    );

    manager->append_listener(
            PeerMessageType::BlockHeadersRequest,
            [&](const PeerMessage &message) {

                auto latest_remote_header = BlockHeader::from_json(message.content());

                auto headers = blockchain.get_all_after(latest_remote_header);

                manager->send(
                        message.peer_id(),
                        PeerMessage{
                                PeerMessageType::BlockHeadersResponse,
                                manager->id(),
                                message.tag(),
                                BlockHeadersResponseContent{
                                        blockchain.headers()
                                }.to_json()
                        }
                );
            }
    );

    manager->append_listener(
            PeerMessageType::AddTransaction,
            [&transaction_pool](const PeerMessage &message) {
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

                    manager->send(
                            transaction.from(),
                            PeerMessage{
                                    PeerMessageType::SetTransactionStatus,
                                    manager->id(),
                                    PeerMessage::create_tag(),
                                    SetTransactionStatusContent{
                                            TransactionStatus::Verified,
                                            transaction.hash()
                                    }.to_json()
                            }
                    );
                    manager->send(
                            transaction.to(),
                            PeerMessage{
                                    PeerMessageType::AddTransaction,
                                    manager->id(),
                                    PeerMessage::create_tag(),
                                    transaction.to_json()
                            }
                    );
                }
            }
    );

    manager->update_state_to(PeerState::WaitingForPeers);
    manager->wait_for(PeerType::Full, 1);

    {
        manager->update_state_to(PeerState::InitialBlockDownload);
        auto ids = manager->peer_ids_of_type(PeerType::Full);

        auto header_cache = std::unordered_map<int, std::vector<BlockHeader>>{};
        for (const auto &id: ids) {

            auto resp = manager->send(
                    id,
                    PeerMessage{
                            PeerMessageType::BlockHeadersRequest,
                            manager->id(),
                            PeerMessage::create_tag(),
                            blockchain.last().header().to_json()
                    }
            ).get();

            auto content = BlockHeadersResponseContent::from_json(resp.content());
            header_cache[id] = content.headers();
        }

        int longest_chain_peer_id = ids.front();
        for (const auto &[id, headers]: header_cache) {
            if (headers.size() > header_cache[longest_chain_peer_id].size()) {
                longest_chain_peer_id = id;
            }
        }

        for (const auto &header: header_cache[longest_chain_peer_id]) {
            if (blockchain.contains(header.hash()))
                continue;

            auto resp = manager->send(
                    longest_chain_peer_id,
                    PeerMessage{
                            PeerMessageType::BlockRequest,
                            manager->id(),
                            PeerMessage::create_tag(),
                            header.hash()
                    }
            ).get();
            auto block = Block::from_json(resp.content());

            blockchain.add(block);
            block.to_disk(path);
        }
        miner.set_latest_hash(blockchain.last().hash());
        manager->update_state_to(PeerState::Open);
    }

    manager->wait();
}
