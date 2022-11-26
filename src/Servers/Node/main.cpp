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

    constexpr int BATCH_SZE = 5;
    std::string path;

    if (argc == 2) {
        path = std::string{argv[1]};
    } else {
        path = "blockchain";
    }

    auto blockchain = Blockchain::from_disk(path);
    auto miner = Miner{};
    auto transaction_pool = TransactionPool(BATCH_SZE);

    auto manager = std::make_shared<NodeManager>();

    manager->append_listener(
            PeerMessageType::AddBlock,
            [&](const PeerMessage &message) {

                auto block = Block::from_json(message.content());
                auto local_hash = blockchain->last().hash();
                auto local_prev_hash = blockchain->last().header().previous_hash();
                auto incoming = block.hash();

                // Converge on the block with the higher number of confirmations
                if (auto incoming_prev = blockchain->get_block(block.header().previous_hash())) {
                    if (auto incoming_prev_prev = blockchain->get_block(
                            incoming_prev.value().header().previous_hash())) {
                        auto incoming_prev_prev_hash = incoming_prev_prev->hash();

                        if (local_prev_hash == incoming_prev_prev_hash) {
                            blockchain->remove(local_hash);
                            Block::remove_from_disk(path, local_hash);
                            blockchain->add(block);
                            block.to_disk(path);
                            spdlog::info(
                                    "Main: AddBlock, Confirmed {} and Abandoned {}",
                                    block.hash().substr(0, 10),
                                    local_hash.substr(0, 10)
                            );
                        } else {
                            blockchain->add(block);
                            block.to_disk(path);
                            spdlog::info(
                                    "Main: AddBlock, Confirmed {}",
                                    block.hash().substr(0, 10)
                            );
                        }
                    }

                }


            }
    );

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
                                blockchain->get_block(hash).value().to_json()
                        }
                );
            }
    );

    manager->append_listener(
            PeerMessageType::BlockHeadersRequest,
            [&](const PeerMessage &message) {

                auto latest_remote_header = BlockHeader::from_json(message.content());

                auto headers = blockchain->get_all_after(latest_remote_header);

                manager->send(
                        message.peer_id(),
                        PeerMessage{
                                PeerMessageType::BlockHeadersResponse,
                                manager->id(),
                                message.tag(),
                                BlockHeadersResponseContent{
                                        blockchain->headers()
                                }.to_json()
                        }
                );
            }
    );

    manager->append_listener(
            PeerMessageType::AddTransaction,
            [&](const PeerMessage &message) {

                auto transaction = Transaction::from_json(message.content());
                spdlog::info("Main: Received Transaction {}", transaction.hash().substr(0, 10));

                transaction_pool.add(transaction);
                auto batch = transaction_pool.get_a_batch();
                if (batch.has_value()) {

                    miner.mine(blockchain->last().hash(), batch.value());
                }
            }
    );

    miner.append_listener(
            Miner::Event::BlockMined,
            [&, path](Block block) {

                spdlog::info("Main: BlockMined, {}", block.hash().substr(0, 10));

                blockchain->add(block);
                block.to_disk(path);

                manager->broadcast(
                        PeerMessage{
                                PeerMessageType::AddBlock,
                                manager->id(),
                                PeerMessage::create_tag(),
                                block.to_json()
                        }
                );
                spdlog::info("Main: BlockMined, Broadcasted {}", block.hash().substr(0, 10));
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
                            blockchain->last().header().to_json()
                    }
            ).get();

            auto content = BlockHeadersResponseContent::from_json(resp.content());
            header_cache[id] = content.headers();
        }

        if (!ids.empty()) {
            int longest_chain_peer_id = ids.front();
            for (const auto &[id, headers]: header_cache) {
                if (headers.size() > header_cache[longest_chain_peer_id].size()) {
                    longest_chain_peer_id = id;
                }
            }

            for (const auto &header: header_cache[longest_chain_peer_id]) {
                if (blockchain->contains(header.hash()))
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

                blockchain->add(block);
                block.to_disk(path);
            }
        }

        manager->update_state_to(PeerState::Open);
    }

    manager->wait();
}
