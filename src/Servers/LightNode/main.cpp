#include "EventLoop.h"
#include "PeerManager.h"
#include "PeerMessage.h"
#include "PeerType.h"
#include "SignalingClient.h"
#include "Transaction.h"
#include "Wallet.h"
#include "fmt/format.h"
#include "spdlog/spdlog.h"
#include <concurrencpp/executors/thread_executor.h>
#include <concurrencpp/executors/worker_thread_executor.h>
#include <concurrencpp/results/result_fwd_declarations.h>
#include <concurrencpp/runtime/runtime.h>
#include <concurrencpp/timers/timer_queue.h>
#include <iostream>
#include <memory>
using namespace krapi;
using namespace std::chrono_literals;

using ThreadExecutor = std::shared_ptr<concurrencpp::thread_executor>;
using TimerQueue = std::shared_ptr<concurrencpp::timer_queue>;

concurrencpp::null_result initialize(
  concurrencpp::executor_tag,
  ThreadExecutor executor,
  TimerQueue timer_queue,
  SignalingClientPtr signaling_client,
  PeerManagerPtr manager,
  WalletPtr wallet
) {
  co_await signaling_client->initialize();

  spdlog::info(
    "Waiting for 2 Full peers to connect and 1 Light peer to connect..."
  );
  auto peer_ids = co_await manager->wait_for_peers(
    executor,
    3,
    {PeerType::Full, PeerType::Light},
    {PeerState::Open}
  );

  spdlog::info("Connected to: [{}]", fmt::join(peer_ids, ", "));

  while (true) {
    std::cin.get();
    auto light_peer_ids = co_await manager->peers_of_type(PeerType::Light);
    for (const auto &peer_id: light_peer_ids) {
      auto transaction = wallet->create_transaction(manager->id(), peer_id);
      spdlog::info(
        "Broadcasting TX# {} sent to {} to miners",
        transaction.contrived_hash(),
        peer_id
      );
      manager->broadcast_to_peers_of_type_and_forget(
        executor,
        PeerType::Full,
        PeerMessageType::AddTransaction,
        transaction.to_json()
      );
    }
  }
}

int main() {
  auto runtime = std::make_shared<concurrencpp::runtime>();

  auto worker = runtime->make_worker_thread_executor();
  auto event_loop = EventLoop::create(worker);
  auto signaling_client = SignalingClient::create(event_loop->event_queue());
  event_loop->append_listener(
    InternalNotificationType::SignalingServerClosed,
    [=](Event) {
      spdlog::warn("Signaling Server Closed...");
      event_loop->end();
    }
  );
  auto manager = PeerManager::create(
    worker,
    event_loop->event_queue(),
    signaling_client,
    PeerType::Light,
    PeerState::Open
  );
  auto wallet = Wallet::create();

  initialize(
    {},
    runtime->thread_executor(),
    runtime->timer_queue(),
    signaling_client,
    manager,
    wallet
  );

  event_loop->wait();
}