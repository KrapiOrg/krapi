#include "BlockchainManager.h"
#include "EventLoop.h"
#include "InternalNotification.h"
#include "Miner.h"
#include "PeerManager.h"
#include "PeerMessage.h"
#include "PeerType.h"
#include "SignalingClient.h"
#include "SpentTransactionsStore.h"
#include "TransactionPoolManager.h"
#include "fmt/core.h"
#include "spdlog/spdlog.h"
#include <concurrencpp/runtime/runtime.h>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

using namespace krapi;
using namespace std::chrono_literals;

concurrencpp::result<std::pair<std::vector<BlockHeader>, std::string>> request_headers(
  PeerManagerPtr manager,
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
  PeerManagerPtr manager,
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
  PeerManagerPtr manager,
  BlockchainPtr blockchain,
  TransactionPoolPtr transaction_pool
) {

  co_await signaling_client->initialize();

  auto last_known_header = blockchain->last().header();

  spdlog::info("Waiting for a full node to connect");

  auto peers =
    co_await manager->wait_for_peers(
      executor,
      1,
      {PeerType::Full},
      {PeerState::Open}
    );

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

  transaction_pool->enqueue_stored();

  spdlog::info("Broadcasting SyncPoolRequest");
  manager->broadcast_to_peers_of_type_and_forget(
    executor,
    {PeerType::Full},
    PeerMessageType::SyncPoolRequest
  );
  spdlog::info("Broadcasted SyncPoolRequest");

  spdlog::info("Initial Block download compeleted");
}

int main(int argc, char *argv[]) {

  auto runtime = std::make_shared<concurrencpp::runtime>();

  auto event_loop = EventLoop::create(runtime->make_worker_thread_executor());

  if (!std::filesystem::exists("miners"))
    std::filesystem::create_directory("miners");

  auto identity = uuids::to_string(uuids::uuid_system_generator{}());

  auto path = fmt::format("miners/{}", identity);

  auto pool_path = fmt::format("{}/txpool", path);
  auto store_path = fmt::format("{}/spenttxs", path);
  auto retry_handler_path = fmt::format("{}/retryhandler", path);

  auto transaction_pool = TransactionPool::create(pool_path);
  auto spent_transactions_store = SpentTransactionsStore::create(store_path);
  auto blockchain = Blockchain::from_path(path);

  auto signaling_client = SignalingClient::create(
    identity,
    event_loop->event_queue()
  );

  auto manager = PeerManager::create(
    runtime->make_worker_thread_executor(),
    event_loop->event_queue(),
    RetryHandler::create(
      runtime->thread_executor(),
      retry_handler_path,
      event_loop->event_queue()
    ),
    signaling_client,
    PeerType::Full
  );

  auto blockchain_manager = BlockchainManager::create(
    runtime->thread_executor(),
    event_loop,
    manager,
    transaction_pool,
    blockchain,
    spent_transactions_store
  );

  auto transaction_pool_manager = TransactionPoolManager::create(
    event_loop,
    manager,
    transaction_pool,
    spent_transactions_store
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

  event_loop->append_listener(
    PeerMessageType::ControlOperateBetween,
    [&](Event e) {
      auto peer_message = e.get<PeerMessage>();
      auto begin = peer_message->content()["begin"].get<uint64_t>();
      auto end = peer_message->content()["end"].get<uint64_t>();
      spdlog::info("Control Requested to operate Between {} and {}", begin, end);
      std::unordered_map<std::string, int> results;
      for (auto block: blockchain->data()) {
        auto transactions = block.transactions();
        for (auto transaction: transactions) {
          if (transaction.timestamp() >= begin && transaction.timestamp() <= end) {
            spdlog::info("TX#{}, {}", transaction.contrived_hash(), transaction.timestamp());
            results[transaction.to()]++;
          }
        }
      }

      manager->broadcast_to_peers_of_type_and_forget(
        runtime->thread_executor(),
        {PeerType::Light},
        PeerMessageType::ControlResult,
        nlohmann::json(results)
      );
    }
  );

  event_loop->append_listener(
    PeerMessageType::TransactionsInRequest,
    [=](Event e) {
      auto peer_message = e.get<PeerMessage>();
      auto identity = peer_message->content().get<std::string>();

      auto counter = 0;

      for (auto transaction: transaction_pool->data()) {
        if (transaction.to() == identity)
          counter++;
      }
      for (auto transaction: spent_transactions_store->data()) {
        if (transaction.to() == identity)
          counter++;
      }

      event_loop->event_queue()->enqueue<InternalMessage<PeerMessage>>(
        InternalMessageType::SendPeerMessage,
        PeerMessageType::TransactionsInResponse,
        peer_message->receiver_identity(),
        peer_message->sender_identity(),
        peer_message->tag(),
        nlohmann::json(counter)
      );
    }
  );

  event_loop->append_listener(
    PeerMessageType::TransactionsOutRequest,
    [=](Event e) {
      auto peer_message = e.get<PeerMessage>();
      spdlog::info(
        "SpentTransactionStore: {} transactions spent request",
        peer_message->sender_identity()
      );
      auto identity = peer_message->content().get<std::string>();

      auto counter = 0;
      for (auto transaction: transaction_pool->data()) {
        if (transaction.from() == identity)
          counter++;
      }
      spdlog::info(
        "Replying With {}",
        counter
      );
      event_loop->event_queue()->enqueue<InternalMessage<PeerMessage>>(
        InternalMessageType::SendPeerMessage,
        PeerMessageType::TransactionsOutResponse,
        peer_message->receiver_identity(),
        peer_message->sender_identity(),
        peer_message->tag(),
        nlohmann::json(counter)
      );
    }
  );


  spdlog::info("Starting initial block download");
  initial_block_download(
    {},
    runtime->thread_executor(),
    signaling_client,
    event_loop->event_queue(),
    manager,
    blockchain,
    transaction_pool
  );

  manager->set_state(PeerState::Open);
  event_loop->wait();
}