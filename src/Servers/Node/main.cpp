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
#include "Content/PoolResponseContent.h"


using namespace krapi;
using namespace std::chrono_literals;
namespace fs = std::filesystem;

void block_download(
        const BlockHeader &block_header,
        const std::shared_ptr<Blockchain> &blockchain,
        const std::shared_ptr<NodeManager> &manager
) {

    manager->set_state(PeerState::InitialBlockDownload);
    spdlog::info("Set state to {}", to_string(manager->get_state()));

    spdlog::info(
            "Downloading headers for blocks after Block #{}",
            block_header.contrived_hash()
    );

    auto block_headers_resp = manager->broadcast(
            PeerMessage{
                    PeerMessageType::BlockHeadersRequest,
                    manager->id(),
                    PeerMessage::create_tag(),
                    block_header.to_json()
            }
    ).get();

    auto header_cache = std::unordered_map<int, std::vector<BlockHeader>>{};
    for (const auto &resp: block_headers_resp) {
        spdlog::info("{} Replied with the following headers", resp.peer_id());
        auto content = BlockHeadersResponseContent::from_json(resp.content());
        for (const auto &header: content.headers()) {
            spdlog::info("#{}", header.contrived_hash());
        }
        header_cache[resp.peer_id()] = content.headers();
    }

    if (!header_cache.empty()) {

        int longest_chain_peer_id = header_cache.begin()->first;
        for (const auto &[id, headers]: header_cache) {
            if (headers.size() > header_cache[longest_chain_peer_id].size()) {
                longest_chain_peer_id = id;
            }
        }
        spdlog::info(
                "Downloading blocks from {} because it has the longest chain",
                longest_chain_peer_id
        );

        for (const auto &header: header_cache[longest_chain_peer_id]) {
            if (blockchain->contains(header.hash()))
                continue;

            spdlog::info(
                    "Requesting Block #{} from {}",
                    header.contrived_hash(),
                    longest_chain_peer_id
            );

            auto resp = manager->send(
                    longest_chain_peer_id,
                    PeerMessage{
                            PeerMessageType::BlockRequest,
                            manager->id(),
                            PeerMessage::create_tag(),
                            header.hash()
                    }
            ).get();

            if (resp.type() != PeerMessageType::BlockNotFoundResponse) {

                auto block = Block::from_json(resp.content());
                spdlog::info("Received Block #{} from {}", block.contrived_hash(), longest_chain_peer_id);

                blockchain->put(block);
            } else {

                spdlog::info("Block #{} not found in {}", header.contrived_hash(), longest_chain_peer_id);
            }
        }
    }

    manager->set_state(PeerState::Open);
    spdlog::info("Set state to {}", to_string(manager->get_state()));
}

int main(int argc, char *argv[]) {

    constexpr int BATCH_SZE = 10;
    std::string path;

    if (argc == 2) {
        path = std::string{argv[1]};
    } else {
        path = "blockchain";
    }

    if (!fs::exists(path)) {
        spdlog::error("Could not find {}, creating...", path);
        fs::create_directory(path);
    }

    auto blockchain = std::make_shared<Blockchain>(path);

    auto miner = Miner();
    auto transaction_pool = TransactionPool(BATCH_SZE);
    auto manager = std::make_shared<NodeManager>(PeerType::Full);

    transaction_pool.on_batch(
            [&](std::set<Transaction> batch) {
                spdlog::info("== Batch \n{}", to_string(batch));

                auto block = miner.mine(blockchain->last().hash(), batch)->get();

                (void) manager->broadcast(
                        PeerMessage{
                                PeerMessageType::AddBlock,
                                manager->id(),
                                block.to_json()
                        }
                );
                if (blockchain->put(block)) {
                    spdlog::info(
                            "Mined Block #{}, using {}",
                            block.contrived_hash(),
                            block.header().previous_hash().substr(0, 10)
                    );
                    spdlog::info("\n{}", to_string(block.transactions()));
                }
            }
    );

    manager->append_listener(
            PeerMessageType::BlockRequest,
            [&](const PeerMessage &message) {
                auto hash = message.content().get<std::string>();

                auto block = blockchain->get(hash);

                if (block) {
                    spdlog::info(
                            "{} Requested Block #{}",
                            message.peer_id(),
                            hash.substr(0, 10)
                    );
                    (void) manager->send(
                            message.peer_id(),
                            PeerMessage{
                                    PeerMessageType::BlockResponse,
                                    manager->id(),
                                    message.tag(),
                                    block.value().to_json()
                            }
                    );
                } else {
                    spdlog::info(
                            "{} Requested Block #{}, but it is not found",
                            message.peer_id(),
                            hash.substr(0, 10)
                    );
                    (void) manager->send(
                            message.peer_id(),
                            PeerMessage{
                                    PeerMessageType::BlockNotFoundResponse,
                                    manager->id(),
                                    message.tag(),
                            }
                    );
                }

            }
    );

    manager->append_listener(
            PeerMessageType::BlockHeadersRequest,
            [&](const PeerMessage &message) {

                auto latest_remote_header = BlockHeader::from_json(message.content());

                spdlog::info(
                        "{} Requested all headers after #{}",
                        message.peer_id(),
                        latest_remote_header.contrived_hash()
                );

                auto headers = blockchain->get_all_after(latest_remote_header);

                spdlog::info("Replying with the following headers...");

                for (const auto &header: headers) {
                    if (header.hash().empty()) {

                        spdlog::info("#{}", header.contrived_hash());
                    }
                }

                (void) manager->send(
                        message.peer_id(),
                        PeerMessage{
                                PeerMessageType::BlockHeadersResponse,
                                manager->id(),
                                message.tag(),
                                BlockHeadersResponseContent{
                                        std::move(headers)
                                }.to_json()
                        }
                );
            }
    );

    manager->append_listener(
            PeerMessageType::AddTransaction,
            [&](const PeerMessage &message) {

                auto transaction = Transaction::from_json(message.content());
                transaction_pool.add(transaction);
            }
    );

    manager->append_listener(
            PeerMessageType::BlockAccepted,
            [&](const PeerMessage &message) {
                auto block = Block::from_json(message.content());
                spdlog::info(
                        "Peer {} accepted block {}",
                        message.peer_id(),
                        block.contrived_hash()
                );
            }
    );

    manager->append_listener(
            PeerMessageType::BlockRejected,
            [&](const PeerMessage &message) {
                manager->set_state(PeerState::Closed);
                miner.wait();

                auto removed_block = Block::from_json(message.content());
                spdlog::warn(
                        "{} Rejected block {}",
                        message.peer_id(),
                        removed_block.contrived_hash()
                );

                auto removed_blocks = blockchain->remove_all_after(removed_block.hash());

                for (const auto &block: removed_blocks) {
                    spdlog::warn("Removed block #{}", block.contrived_hash());
                    for (const auto &transaction: block.transactions()) {
                        if (!blockchain->contains_transaction(transaction.hash())) {
                            if (transaction_pool.add(transaction)) {
                                spdlog::warn(
                                        "== TX#{} from Block#{} from peer {}, restored",
                                        transaction.contrived_hash(),
                                        removed_block.contrived_hash(),
                                        message.peer_id()
                                );
                            }
                        }
                    }
                }

                block_download(
                        blockchain->last().header(),
                        blockchain,
                        manager
                );
            }
    );

    manager->append_listener(
            PeerMessageType::AddBlock,
            [&](const PeerMessage &message) {
                auto block = Block::from_json(message.content());
                auto last = blockchain->last().hash();
                auto Lprev = blockchain->last().header().previous_hash();
                auto Rprev = block.header().previous_hash();

                auto reject = [&]() {
                    spdlog::info("Rejected {} ancestor is {}, needs to be {}",
                                 block.contrived_hash(),
                                 Rprev.substr(0, 10),
                                 Lprev.substr(0, 10)
                    );
                    (void) manager->send(
                            message.peer_id(),
                            PeerMessage{
                                    PeerMessageType::BlockRejected,
                                    manager->id(),
                                    PeerMessage::create_tag(),
                                    block.to_json()
                            }
                    );
                };
                auto accept = [&]() {
                    spdlog::info("AddBlock, Accepted {}, with {} ancestory",
                                 block.contrived_hash(),
                                 Rprev.substr(0, 10)
                    );
                    blockchain->put(block);
                    transaction_pool.remove(block.transactions());
                    (void) manager->send(
                            message.peer_id(),
                            PeerMessage{
                                    PeerMessageType::BlockAccepted,
                                    manager->id(),
                                    PeerMessage::create_tag(),
                                    block.to_json()
                            }
                    );
                };


                if (Rprev == last) {
                    spdlog::info("Here1");
                    accept();
                    return;
                }

                if (Lprev == Rprev) {
                    spdlog::info("Here2");
                    accept();
                    return;
                } else {

                    reject();
                    return;
                }

            }
    );

    spdlog::info("Connecting to network");
    manager->set_state(PeerState::WaitingForPeers);
    auto peers_connected = manager->connect_to_peers().get();
    spdlog::info("Connected to [{}]", fmt::join(peers_connected, ", "));

    block_download(
            blockchain->last().header(),
            blockchain,
            manager
    );

    manager->wait();
}
