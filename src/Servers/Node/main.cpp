//
// Created by mythi on 12/10/22.
//
#include "Blockchain/Blockchain.h"
#include "Content/BlockHeadersResponseContent.h"
#include "EventLoop.h"
#include "EventQueue.h"
#include "InternalNotification.h"
#include "Miner.h"
#include "NodeManager.h"
#include "PeerMessage.h"
#include "PeerType.h"
#include "eventpp/utilities/scopedremover.h"
#include "fmt/format.h"
#include "spdlog/spdlog.h"
#include <concurrencpp/executors/thread_executor.h>
#include <concurrencpp/results/result.h>
#include <concurrencpp/results/result_fwd_declarations.h>
#include <memory>
#include <string>
#include <vector>

using namespace krapi;
using namespace std::chrono_literals;

concurrencpp::result<std::pair<std::vector<BlockHeader>, std::string>>
request_headers(
  NodeManagerPtr manager,
  std::vector<std::string> identities,
  BlockHeader last_known_header
) {

  struct SizeSorter {
    bool operator()(
      const std::vector<BlockHeader> &a,
      const std::vector<BlockHeader> &b
    ) const {

      return a.size() > b.size();
    }
  };

  auto header_peer_map =
    std::map<std::vector<BlockHeader>, std::string, SizeSorter>(SizeSorter{});

  for (const auto identity: identities) {
    auto result = co_await manager->send(
      PeerMessageType::BlockHeadersRequest,
      manager->id(),
      identity,
      PeerMessage::create_tag(),
      last_known_header.to_json()
    );
    auto message = result.get<PeerMessage>();
    auto respose_content =
      BlockHeadersResponseContent::from_json(message->content());
    header_peer_map.emplace(
      respose_content.headers(),
      message->sender_identity()
    );
  }

  co_return *header_peer_map.begin();
}

concurrencpp::null_result initial_block_download(
  concurrencpp::executor_tag,
  std::shared_ptr<concurrencpp::thread_executor> executor,
  EventQueuePtr event_queue,
  NodeManagerPtr manager,
  BlockchainPtr blockchain
) {

  auto last_known_header = blockchain->last().header();
  auto blocks = std::vector<Block>{};
  auto peers =
    co_await manager
      ->wait_for_peers(executor, 1, {PeerType::Full}, {PeerState::Open});

  spdlog::info("Requesting headers from {}", fmt::join(peers, ", "));
  auto [headers, peer_id] =
    co_await request_headers(manager, peers, last_known_header);
  if (headers.empty()) {
    spdlog::info("There were no headers to sync");
    co_return;
  }
  spdlog::info(
    "## Syncing Headers after {} from peer {}",
    last_known_header.contrived_hash(),
    peer_id
  );
  for (const auto &header: headers) {
    spdlog::info(
      "## Downloading Block associated with header {}",
      header.contrived_hash()
    );
    auto event = co_await manager->send(
      PeerMessageType::BlockRequest,
      manager->id(),
      peer_id,
      PeerMessage::create_tag(),
      header.to_json()
    );
    auto response = event.get<PeerMessage>();

    if (response->type() == PeerMessageType::BlockResponse) {

      auto block = Block::from_json(response->content());
      spdlog::info("## {} Downloaded", header.contrived_hash());
      blocks.push_back(block);
    } else {

      spdlog::error(
        "## Failed to download block {} from peer {}",
        header.contrived_hash(),
        peer_id
      );
    }
  }
  for (const auto &block: blocks) {
    blockchain->put(block);
    spdlog::info("Added {} to blockchain", block.contrived_hash());
  }
}

int main(int argc, char *argv[]) {

  auto runtime = std::make_shared<concurrencpp::runtime>();

  auto event_loop = EventLoop::create(runtime->make_worker_thread_executor());

  constexpr int BATCH_SZE = 10;
  std::string path;

  if (argc == 2) {
    path = std::string{argv[1]};
  } else {
    path = "blockchain";
  }

  auto blockchain = Blockchain::from_path(path);
  auto signaling_client = SignalingClient::create(event_loop->event_queue());

  auto manager = NodeManager::create(
    runtime->make_worker_thread_executor(),
    event_loop->event_queue(),
    signaling_client,
    PeerType::Full
  );
  auto miner =
    Miner(runtime->make_worker_thread_executor(), event_loop->event_queue());

  signaling_client->initialize().wait();

  event_loop->append_listener(
    InternalNotificationType::SignalingServerClosed,
    [=](Event) {
      spdlog::warn("Signaling Server Closed...");
      event_loop->end();
    }
  );

  event_loop->append_listener(
    PeerMessageType::BlockHeadersRequest,
    [=](Event event) {
      auto peer_message = event.get<PeerMessage>();
      auto last_header = BlockHeader::from_json(peer_message->content());

      manager->send_and_forget(
        PeerMessageType::BlockHeadersResponse,
        peer_message->receiver_identity(),
        peer_message->sender_identity(),
        peer_message->tag(),
        BlockHeadersResponseContent(blockchain->get_all_after(last_header))
          .to_json()
      );
    }
  );

  event_loop->append_listener(PeerMessageType::BlockRequest, [=](Event event) {
    auto peer_message = event.get<PeerMessage>();
    auto block_header = BlockHeader::from_json(peer_message->content());
    spdlog::info(
      "{} requested block {}",
      peer_message->sender_identity(),
      block_header.contrived_hash()
    );
    auto block = blockchain->get(block_header.hash());
    if (block) {

      manager->send_and_forget(
        PeerMessageType::BlockResponse,
        manager->id(),
        peer_message->sender_identity(),
        peer_message->tag(),
        block->to_json()
      );
    } else {

      manager->send_and_forget(
        PeerMessageType::BlockNotFoundResponse,
        manager->id(),
        peer_message->sender_identity(),
        peer_message->tag()
      );
    }
  });

  event_loop->append_listener(PeerMessageType::AddTransaction, [](Event event) {
    auto peer_message = event.get<PeerMessage>();
    auto transaction = Transaction::from_json(peer_message->content());

    spdlog::info(
      "{} wants to send {} a transaction",
      transaction.from(),
      transaction.to()
    );
  });

  spdlog::info("Starting initial block download");
  initial_block_download(
    {},
    runtime->thread_executor(),
    event_loop->event_queue(),
    manager,
    blockchain
  );
  manager->set_state(PeerState::Open);
  event_loop->wait();
}