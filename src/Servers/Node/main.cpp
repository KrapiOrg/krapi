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

Block create_genesis_block() {

    CryptoPP::SHA256 sha_256;
    spdlog::info("Blockchain: Creating genesis block");
    auto previous_hash = std::string{"0"};
    auto merkle_root = std::string{"0"};
    auto timestamp = static_cast<uint64_t>(1668542625);
    auto nonce = (uint64_t) 0;
    auto block_hash = std::string{};

    StringSource s2(
            fmt::format("{}{}{}{}", previous_hash, merkle_root, timestamp, nonce),
            true,
            new HashFilter(sha_256, new HexEncoder(new StringSink(block_hash)))
    );

    return Block{
            BlockHeader{
                    block_hash,
                    previous_hash,
                    merkle_root,
                    timestamp,
                    nonce
            },
            {}
    };
}

int main(int argc, char *argv[]) {

    constexpr int BATCH_SZE = 3;
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

    auto blockchain = Blockchain::create();
    auto genesis = create_genesis_block();
    blockchain->add(genesis);
    genesis.to_disk(path);

    auto miner = Miner();
    auto transaction_pool = TransactionPool(BATCH_SZE);

    auto manager = std::make_shared<NodeManager>(PeerType::Full);


    manager->append_listener(
            PeerMessageType::SyncPoolRequest,
            [&](const PeerMessage &message) {
                (void) manager->send(
                        message.peer_id(),
                        PeerMessage{
                                PeerMessageType::SyncPoolResponse,
                                manager->id(),
                                message.tag(),
                                PoolResponseContent{
                                        transaction_pool.get_pool()
                                }.to_json()
                        }
                );
            }
    );

    manager->append_listener(
            PeerMessageType::BlockRequest,
            [&](const PeerMessage &message) {
                auto hash = message.content().get<std::string>();

                (void) manager->send(
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

                (void) manager->send(
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
                transaction_pool.add(transaction);
                if (auto batch = transaction_pool.get_a_batch()) {
                    auto block = miner.mine(blockchain->last().hash(), batch.value())->get();
                    spdlog::info("Mined {}", block.hash().substr(0, 10));
                    spdlog::info("\n{}", to_string(block.transactions()));
                    blockchain->add(block);
                    block.to_disk(path);
                }
            }
    );

    manager->append_listener(
            PeerMessageType::BlockAccepted,
            [&](const PeerMessage &message) {
                auto block = Block::from_json(message.content());
                spdlog::info(
                        "Peer {} accepted block {}",
                        message.peer_id(),
                        block.hash().substr(0, 10)
                );
            }
    );

    manager->append_listener(
            PeerMessageType::BlockRejected,
            [&](const PeerMessage &message) {
                auto block = Block::from_json(message.content());
                Block::remove_from_disk(path, block.hash());

                spdlog::warn("{} Rejected block {}", message.peer_id(), block.hash().substr(0, 10));
                for (const auto &transaction: block.transactions()) {
                    if (transaction_pool.add(transaction)) {

                        spdlog::warn(
                                "== TX#{} from Block#{} from peer {}, restored",
                                transaction.hash().substr(0, 10),
                                block.hash().substr(0, 10),
                                message.peer_id()
                        );
                    }
                }
            }
    );

    manager->append_listener(
            PeerMessageType::AddBlock,
            [&](const PeerMessage &message) {
                spdlog::info("=================");
                auto block = Block::from_json(message.content());
                auto last = blockchain->last().hash();
                auto Lprev = blockchain->last().header().previous_hash();
                auto Rprev = block.header().previous_hash();

                auto reject = [&]() {
                    spdlog::info("Rejected {} ancestor is {}, needs to be {}",
                                 block.hash().substr(0, 10),
                                 Rprev.substr(0, 10),
                                 Lprev.substr(0, 10)
                    );
                    (void) manager->send(
                            message.peer_id(),
                            PeerMessage{
                                    PeerMessageType::BlockRejected,
                                    manager->id(),
                                    PeerMessage::create_tag()
                            }
                    );
                };
                auto accept = [&]() {
                    spdlog::info("AddBlock, Accepted {}, with {} ancestory",
                                 block.hash().substr(0, 10),
                                 Lprev.substr(0, 10)
                    );
                    blockchain->add(block);
                    block.to_disk(path);
                    transaction_pool.remove(block.transactions());
                    (void) manager->send(
                            message.peer_id(),
                            PeerMessage{
                                    PeerMessageType::BlockAccepted,
                                    manager->id(),
                                    PeerMessage::create_tag()
                            }
                    );
                };


                if (Rprev == last) {
                    accept();
                    return;
                }

                if (Lprev == Rprev) {

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

    for (auto id: peers_connected) {
        auto peer_type = manager->request_peer_type(id);
        auto peer_state = manager->request_peer_state(id);
        if (peer_type == krapi::PeerType::Full && peer_state == krapi::PeerState::Open) {
            spdlog::info("Downloading Transactions from {}", id);
            manager->set_state(PeerState::InitialBlockDownload);

            auto response = manager->send(
                    id,
                    PeerMessage{
                            PeerMessageType::SyncPoolRequest,
                            manager->id()
                    }
            ).get();

            auto content = PoolResponseContent::from_json(response.content());
            transaction_pool.add(content.transactions());
            spdlog::info("Received the following batch from {}", response.peer_id());
            spdlog::info(to_string(content.transactions()));
        }
    }

    auto block_headers_resp = manager->broadcast(
            PeerMessage{
                    PeerMessageType::BlockHeadersRequest,
                    manager->id(),
                    PeerMessage::create_tag(),
                    blockchain->last().header().to_json()
            }
    ).get();

    auto header_cache = std::unordered_map<int, std::vector<BlockHeader>>{};
    for (const auto &resp: block_headers_resp) {
        auto content = BlockHeadersResponseContent::from_json(resp.content());
        header_cache[resp.peer_id()] = content.headers();
    }

    if (!header_cache.empty()) {

        int longest_chain_peer_id = header_cache.begin()->first;
        for (const auto &[id, headers]: header_cache) {
            if (headers.size() >= header_cache[longest_chain_peer_id].size()) {
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
    manager->set_state(PeerState::Open);

    // Wait for tx pool to have transactions
    spdlog::info("Waiting for transactions...");
    transaction_pool.wait();
    auto first_batch = transaction_pool.get_a_batch();

    spdlog::info("Starting miner with first batch");
    miner.mine(blockchain->last().hash(), first_batch.value());

    manager->wait();
}
