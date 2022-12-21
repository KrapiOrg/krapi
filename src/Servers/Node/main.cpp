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
namespace fs = std::filesystem;

//void block_download(
//        const BlockHeader &block_header,
//        const std::shared_ptr<Blockchain> &blockchain,
//        const std::shared_ptr<NodeManager> &manager
//) {
//
//    manager->set_state(PeerState::InitialBlockDownload);
//    spdlog::info("Set state to {}", to_string(manager->get_state()));
//
//    spdlog::info(
//            "Downloading headers for blocks after Block #{}",
//            block_header.contrived_hash()
//    );
//
//    auto block_headers_resp =
//
//            manager->broadcast(
//                    PeerMessage{
//                            PeerMessageType::BlockHeadersRequest,
//                            manager->id(),
//                            block_header.to_json()
//                    },
//                    {PeerType::Light, PeerType::Observer}
//            ).get();
//
//
//    auto header_cache = std::unordered_map<std::string, std::vector<BlockHeader>>{};
//    for (const auto &resp: block_headers_resp) {
//        if (!resp.has_value())
//            continue;
//        spdlog::info("{} Replied with the following headers", resp.value().peer_id());
//        auto content = BlockHeadersResponseContent::from_json(resp.value().content());
//        for (const auto &header: content.headers()) {
//            spdlog::info("#{}", header.contrived_hash());
//        }
//        header_cache[resp.value().peer_id()] = content.headers();
//    }
//
//    if (!header_cache.empty()) {
//
//        std::string longest_chain_peer_id = header_cache.begin()->first;
//        for (const auto &[id, headers]: header_cache) {
//            if (headers.size() > header_cache[longest_chain_peer_id].size()) {
//                longest_chain_peer_id = id;
//            }
//        }
//        spdlog::info(
//                "Downloading blocks from {} because it has the longest chain",
//                longest_chain_peer_id
//        );
//
//        for (const auto &header: header_cache[longest_chain_peer_id]) {
//            if (blockchain->contains(header.hash()))
//                continue;
//
//            spdlog::info(
//                    "Requesting Block #{} from {}",
//                    header.contrived_hash(),
//                    longest_chain_peer_id
//            );
//
//            auto resp_future = manager->send(
//                    longest_chain_peer_id,
//                    PeerMessage{
//                            PeerMessageType::BlockRequest,
//                            manager->id(),
//                            PeerMessage::create_tag(),
//                            header.to_json()
//                    }
//            );
//
//            if (!resp_future.has_value()) {
//                continue;
//            }
//            auto resp = resp_future->get();
//
//            if (!resp.has_value()) {
//
//                continue;
//            }
//
//            if (resp.value().type() != PeerMessageType::BlockNotFoundResponse) {
//
//                auto block = Block::from_json(resp.value().content());
//                spdlog::info("Received Block #{} from {}", block.contrived_hash(), longest_chain_peer_id);
//
//                blockchain->put(block);
//            } else {
//
//                spdlog::info("Block #{} not found in {}", header.contrived_hash(), longest_chain_peer_id);
//            }
//        }
//    }
//
//    manager->set_state(PeerState::Open);
//    spdlog::info("Set state to {}", to_string(manager->get_state()));
//}

int main(int argc, char *argv[]) {

    concurrencpp::runtime runtime;

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

    manager->initialize().wait();

    auto worker = runtime.make_worker_thread_executor();

    spdlog::info("Connecting to network");
    // manager->set_state(PeerState::WaitingForPeers);
    auto peers_connected = manager->connect_to_peers().get();
    spdlog::info("Connected to [{}]", fmt::join(peers_connected, ", "));

//    block_download(
//            blockchain->last().header(),
//            blockchain,
//            manager
//    );
}
