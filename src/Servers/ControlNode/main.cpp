#include "EventLoop.h"
#include "EventQueue.h"
#include "Helpers.h"
#include "InternalMessage.h"
#include "PeerManager.h"
#include "PeerMessage.h"
#include "PeerType.h"
#include "SignalingClient.h"
#include "Transaction.h"
#include "eventpp/utilities/scopedremover.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "nlohmann/json_fwd.hpp"
#include "spdlog/spdlog.h"
#include <concurrencpp/executors/thread_executor.h>
#include <concurrencpp/executors/worker_thread_executor.h>
#include <concurrencpp/results/result_fwd_declarations.h>
#include <concurrencpp/runtime/runtime.h>
#include <concurrencpp/timers/timer_queue.h>
#include <cstdint>
#include <iostream>
#include <memory>
using namespace krapi;
using namespace std::chrono_literals;

using ThreadExecutor = std::shared_ptr<concurrencpp::thread_executor>;
using TimerQueue = std::shared_ptr<concurrencpp::timer_queue>;

struct ControlManager {
  PeerManagerPtr m_manager;
  EventQueuePtr m_event_queue;
  eventpp::ScopedRemover<EventQueueType> m_remover;
  TimerQueue m_timer_queue;
  ThreadExecutor m_executor;
  std::atomic<bool> m_control_state;
  std::atomic<uint64_t> m_last_control_begin{0};
  std::atomic<uint64_t> m_last_control_end{0};


  concurrencpp::result<void> start() {

    m_manager->broadcast_to_peers_of_type_and_forget(
      m_executor,
      {PeerType::Light, PeerType::Observer},
      PeerMessageType::ControlIsStartingPing
    );

    spdlog::info("Control Starting in 5 seconds...");
    co_await m_timer_queue->make_delay_object(5s, m_executor);

    m_last_control_begin.exchange(get_krapi_timestamp());

    m_control_state = true;

    m_manager->broadcast_to_peers_of_type_and_forget(
      m_executor,
      {PeerType::Light, PeerType::Observer},
      PeerMessageType::ControlStarted
    );
  }

  concurrencpp::result<void> stop() {

    m_manager->broadcast_to_peers_of_type_and_forget(
      m_executor,
      {PeerType::Light, PeerType::Observer},
      PeerMessageType::ControlIsEndingPing
    );

    spdlog::info("Control Stopping in 5 seconds...");
    co_await m_timer_queue->make_delay_object(5s, m_executor);

    m_last_control_end.exchange(get_krapi_timestamp());

    m_control_state = false;

    m_manager->broadcast_to_peers_of_type_and_forget(
      m_executor,
      {PeerType::Light, PeerType::Observer},
      PeerMessageType::ControlStopped
    );
  }

  void on_is_control_started(Event e) {
    auto peer_message = e.get<PeerMessage>();
    spdlog::info(
      "{} requested control state, replying with {}",
      peer_message->sender_identity(),
      m_control_state.load()
    );
    m_manager->send_and_forget(
      PeerMessageType::IsControlStartedResponse,
      peer_message->receiver_identity(),
      peer_message->sender_identity(),
      peer_message->tag(),
      nlohmann::json(m_control_state.load())
    );
  }

  void opearte() {
    auto json = nlohmann::json();
    json["begin"] = m_last_control_begin.load();
    json["end"] = m_last_control_end.load();

    m_manager->broadcast_to_peers_of_type_and_forget(
      m_executor,
      {PeerType::Full},
      PeerMessageType::ControlOperateBetween,
      json
    );
  }

  ControlManager(
    PeerManagerPtr manager,
    EventQueuePtr event_queue,
    TimerQueue timer_queue,
    ThreadExecutor executor
  )
      : m_manager(std::move(manager)),
        m_event_queue(event_queue),
        m_remover(event_queue->internal_queue()),
        m_executor(std::move(executor)),
        m_timer_queue(std::move(timer_queue)),
        m_control_state(false) {
    m_remover.appendListener(
      PeerMessageType::IsControlStartedRequest,
      std::bind_front(&ControlManager::on_is_control_started, this)
    );
  }
};

int main(int argc, char *argv[]) {
  auto runtime = std::make_shared<concurrencpp::runtime>();

  auto worker = runtime->make_worker_thread_executor();
  auto event_loop = EventLoop::create(worker);

  if (!std::filesystem::exists("control"))
    std::filesystem::create_directory("control");

  auto identity = "control";
  auto path = fmt::format("control");
  auto retry_handler_path = fmt::format("{}/retryhandler", path);

  auto signaling_client = SignalingClient::create(
    identity,
    event_loop->event_queue()
  );

  auto manager = PeerManager::create(
    worker,
    event_loop->event_queue(),
    RetryHandler::create(
      runtime->thread_executor(),
      retry_handler_path,
      event_loop->event_queue()
    ),
    signaling_client,
    PeerType::Control,
    PeerState::Open
  );

  event_loop->append_listener(
    InternalNotificationType::SignalingServerClosed,
    [=](Event) {
      spdlog::warn("Signaling Server Closed...");
      event_loop->end();
    }
  );

  auto control = ControlManager(
    manager,
    event_loop->event_queue(),
    runtime->timer_queue(),
    runtime->thread_executor()
  );

  signaling_client->initialize().wait();

  spdlog::info("Initialzed Signaling Client...");

  while (true) {

    fmt::print("1. Start\n");
    fmt::print("2. Stop\n");
    fmt::print("3. Operate\n");
    fmt::print("4. Exit\n");
    int choice;
    std::cin >> choice;
    if (choice == 1) {
      control.start().wait();
    } else if (choice == 2) {
      control.stop().wait();
    } else if (choice == 3) {
      control.opearte();
    } else {

      event_loop->end();
      break;
    }
  }

  event_loop->wait();
}
