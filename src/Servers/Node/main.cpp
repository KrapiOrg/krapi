//
// Created by mythi on 12/10/22.
//
#include "BlockHeader.h"
#include "Blockchain/Blockchain.h"
#include "Content/BlockHeadersResponseContent.h"
#include "EventLoop.h"
#include "EventQueue.h"
#include "InternalNotification.h"
#include "Miner.h"
#include "NodeManager.h"
#include "PeerMessage.h"
#include "PeerType.h"
#include "SignalingClient.h"
#include "Transaction.h"
#include "TransactionPool.h"
#include "ValidationState.h"
#include "Validator.h"
#include "eventpp/utilities/scopedremover.h"
#include "fmt/format.h"
#include "mqueue.h"
#include "nlohmann/json_fwd.hpp"
#include "spdlog/spdlog.h"
#include <concurrencpp/executors/executor.h>
#include <concurrencpp/executors/thread_executor.h>
#include <concurrencpp/executors/worker_thread_executor.h>
#include <concurrencpp/runtime/runtime.h>
#include <memory>
#include <string>
#include <vector>

using namespace krapi;
using namespace std::chrono_literals;


concurrencpp::result<std::pair<std::vector<BlockHeader>, std::string>> request_headers(
  NodeManagerPtr manager,
  std::vector<std::string> identities,
  BlockHeader last_known_header
) {
  spdlog::info("## Syncing Headers after {}", last_known_header.contrived_hash());
  struct SizeSorter {
    bool operator()(const std::vector<BlockHeader> &a, const std::vector<BlockHeader> &b)
      const {

      return a.size() > b.size();
    }
  };
  using Headers = std::vector<BlockHeader>;
  using Map = std::map<Headers, std::string, SizeSorter>;

  auto header_peer_map = Map(SizeSorter{});

  for (const auto identity: identities) {

    auto reason = co_await manager->send(
      PeerMessageType::BlockHeadersRequest,
      manager->id(),
      identity,
      PeerMessage::create_tag(),
      last_known_header.to_json()
    );

    auto message = reason.get<PeerMessage>();

    auto respose_content = BlockHeadersResponseContent::from_json(message->content());

    header_peer_map.emplace(respose_content.headers(), message->sender_identity());
  }

  co_return *header_peer_map.begin();
}

concurrencpp::result<std::vector<Block>> download_blocks(
  NodeManagerPtr manager,
  std::string peer_id,
  std::vector<BlockHeader> block_headers
) {

  auto blocks = std::vector<Block>{};

  for (const auto &header: block_headers) {

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
  co_return blocks;
}

concurrencpp::null_result initial_block_download(
  concurrencpp::executor_tag,
  std::shared_ptr<concurrencpp::thread_executor> executor,
  SignalingClientPtr signaling_client,
  EventQueuePtr event_queue,
  NodeManagerPtr manager,
  BlockchainPtr blockchain
) {

  co_await signaling_client->initialize();

  auto last_known_header = blockchain->last().header();

  auto peers =
    co_await manager->wait_for_peers(executor, 1, {PeerType::Full}, {PeerState::Open});

  spdlog::info("Requesting headers from {}", fmt::join(peers, ", "));

  auto [headers, peer_id] = co_await request_headers(manager, peers, last_known_header);

  if (!headers.empty()) {
    spdlog::info("Downloading blocks from {}", peer_id);
    auto blocks = co_await download_blocks(manager, peer_id, headers);

    for (const auto &block: blocks) {
      blockchain->put(block);
      spdlog::info("Added {} to blockchain", block.contrived_hash());
    }
  }

  spdlog::info("Broadcasting SyncPoolRequest");
  manager->broadcast_to_peers_of_type_and_forget(
    executor,
    PeerType::Full,
    PeerMessageType::SyncPoolRequest
  );
  spdlog::info("Broadcasted SyncPoolRequest");

  spdlog::info("Initial Block download compeleted");
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
  auto pool_path = fmt::format("{}/txpool", path);
  auto transaction_pool = TransactionPool::create(pool_path);
  auto signaling_client = SignalingClient::create(event_loop->event_queue());

  auto manager = NodeManager::create(
    runtime->make_worker_thread_executor(),
    event_loop->event_queue(),
    signaling_client,
    PeerType::Full
  );
  auto miner = Miner(
    runtime->thread_executor(),
    event_loop->event_queue(),
    transaction_pool,
    blockchain,
    manager
  );

  event_loop->append_listener(
    InternalNotificationType::SignalingServerClosed,
    [=](Event) {
      spdlog::warn("Signaling Server Closed...");
      transaction_pool->end(PoolEndReason::SignalingClosed);
      event_loop->end();
    }
  );

  event_loop->append_listener(PeerMessageType::BlockHeadersRequest, [=](Event event) {
    auto peer_message = event.get<PeerMessage>();
    auto last_header = BlockHeader::from_json(peer_message->content());

    manager->send_and_forget(
      PeerMessageType::BlockHeadersResponse,
      peer_message->receiver_identity(),
      peer_message->sender_identity(),
      peer_message->tag(),
      BlockHeadersResponseContent(blockchain->get_all_after(last_header)).to_json()
    );
  });

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

  event_loop->append_listener(PeerMessageType::AddTransaction, [=](Event event) {
    auto peer_message = event.get<PeerMessage>();
    auto transaction = Transaction::from_json(peer_message->content());

    spdlog::info(
      "Validating transaction #{} from peer {}",
      transaction.contrived_hash(),
      peer_message->sender_identity()
    );

    auto validation_state = Validator::validate_transaction(blockchain, transaction);

    if (validation_state != ValidationState::Success) {

      spdlog::warn(
        "Validaiton failed for transaction {}, {}",
        transaction.contrived_hash(),
        to_string(validation_state)
      );
      return;
    }

    if (transaction_pool->add(transaction)) {

      spdlog::info(
        "Added transaction #{} from {}",
        transaction.contrived_hash(),
        peer_message->sender_identity()
      );
    } else {

      spdlog::info(
        "Did not Add transaction #{} from {} because it is already in the pool",
        transaction.contrived_hash(),
        peer_message->sender_identity()
      );
    }
  });
  event_loop->append_listener(PeerMessageType::SyncPoolRequest, [=](Event e) {
    auto peer_message = e.get<PeerMessage>();

    spdlog::info(
      "{} Requested to sync transaction pools",
      peer_message->sender_identity()
    );

    for (auto transaction: transaction_pool->data()) {

      spdlog::info(
        "Sending pool transaction #{} to {}",
        transaction.contrived_hash(),
        peer_message->sender_identity()
      );

      manager->send_and_forget(
        PeerMessageType::AddTransaction,
        manager->id(),
        peer_message->sender_identity(),
        peer_message->tag(),
        transaction.to_json()
      );
    }
  });

  event_loop->append_listener(PeerMessageType::AddBlock, [=](Event e) {
    auto message = e.get<PeerMessage>();
    auto block = Block::from_json(message->content());


    spdlog::info(
      "Validating block #{} from peer {}",
      block.contrived_hash(),
      message->sender_identity()
    );

    auto validation_state = Validator::validate_block(blockchain, block);

    if (validation_state != ValidationState::Success) {

      spdlog::warn(
        "Failed to validation block #{}, {}",
        block.contrived_hash(),
        to_string(validation_state)
      );

      manager->send_and_forget(
        PeerMessageType::BlockRejected,
        manager->id(),
        message->sender_identity(),
        PeerMessage::create_tag(),
        block.header().to_json()
      );
      return;
    }

    spdlog::info(
      "Successfully Validated Block #{} from peer {}",
      block.contrived_hash(),
      message->sender_identity()
    );

    blockchain->put(block);

    spdlog::info(
      "Removing transactions in Block#{} from pool",
      block.contrived_hash()
    );

    for (const auto &transaction: block.transactions()) {

      if (transaction_pool->remove(transaction)) {
        spdlog::info("  Removed Tx#{} from pool", transaction.contrived_hash());
      }
    }

    manager->send_and_forget(
      PeerMessageType::BlockAccepted,
      manager->id(),
      message->sender_identity(),
      PeerMessage::create_tag(),
      block.header().to_json()
    );
  });

  event_loop->append_listener(InternalNotificationType::BlockMined, [=](Event e) {
    auto block = e.get<InternalNotification<Block>>()->content();
    auto validation_state = Validator::validate_block(blockchain, block);

    spdlog::info("Validating mined block #{}", block.contrived_hash());
    if (validation_state != ValidationState::Success) {

      spdlog::warn(
        "Failed to validate mined block #{}, {}",
        block.contrived_hash(),
        to_string(validation_state)
      );
      return;
    }

    spdlog::info(
      "Adding Mined Block#{} to blockchain",
      block.contrived_hash()
    );

    blockchain->put(block);

    spdlog::info(
      "Removing transactions within {} block from transaction pool",
      block.contrived_hash()
    );

    for (const auto &transaction: block.transactions()) {

      if (transaction_pool->remove(transaction)) {
        spdlog::info("    Removed Tx#{} from pool", transaction.contrived_hash());
      }
    }
  });

  spdlog::info("Starting initial block download");
  initial_block_download(
    {},
    runtime->thread_executor(),
    signaling_client,
    event_loop->event_queue(),
    manager,
    blockchain
  );

  manager->set_state(PeerState::Open);
  event_loop->wait();
}