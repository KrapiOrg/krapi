//
// Created by mythi on 24/11/22.
//

#pragma once

#include <concurrencpp/executors/thread_executor.h>
#include <concurrencpp/results/result.h>
#include <concurrencpp/results/result_fwd_declarations.h>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "concurrencpp/concurrencpp.h"
#include "eventpp/eventdispatcher.h"
#include "nlohmann/json_fwd.hpp"
#include "rtc/peerconnection.hpp"

#include "PeerConnection.h"
#include "PeerMessage.h"
#include "PeerState.h"
#include "PeerType.h"
#include "SignalingClient.h"
#include "SignalingMessage.h"

namespace krapi {

  class [[nodiscard]] NodeManager final {

   public:
    [[nodiscard]] static inline std::shared_ptr<NodeManager> create(
      std::shared_ptr<concurrencpp::worker_thread_executor> worker,
      EventQueuePtr event_queue,
      SignalingClientPtr signaling_client,
      PeerType pt,
      PeerState ps = PeerState::Closed
    ) {

      return std::shared_ptr<NodeManager>(new NodeManager(
        std::move(worker),
        std::move(event_queue),
        std::move(signaling_client),
        pt,
        ps
      ));
    }

    void set_state(PeerState state);

    [[nodiscard]] PeerState get_state() const;

    [[nodiscard]] std::string id() const;

    template<typename... Args>
    concurrencpp::shared_result<Event> send(Args &&...args) {

      return m_event_queue->submit<InternalMessage<PeerMessage>>(
        InternalMessageType::SendPeerMessage,
        PeerMessage::create(std::forward<Args>(args)...)
      );
    }

    template<typename... Args>
    void send_and_forget(Args &&...args) {

      m_event_queue->enqueue<InternalMessage<PeerMessage>>(
        InternalMessageType::SendPeerMessage,
        PeerMessage::create(std::forward<Args>(args)...)
      );
    }

    std::vector<concurrencpp::shared_result<Event>>
      broadcast(PeerMessageType, nlohmann::json = {});

    void broadcast_and_forget(PeerMessageType, nlohmann::json = {});

    void broadcast_to_peers_of_type_and_forget(
      std::shared_ptr<concurrencpp::thread_executor> executor,
      PeerType to_peer_type,
      PeerMessageType message_type,
      nlohmann::json content = {}
    );


    concurrencpp::result<std::vector<std::string>> wait_for_peers(
      std::shared_ptr<concurrencpp::thread_executor> executor,
      int count,
      std::set<PeerType> types,
      std::set<PeerState> states
    );

    concurrencpp::result<std::vector<std::string>> peers_of_type(PeerType type
    ) {
      auto ids = std::vector<std::string>{};
      for (const auto &[peer_id, connection]: m_connection_map) {
        if (co_await connection->type() == type) ids.push_back(peer_id);
      }
      co_return ids;
    }

    concurrencpp::shared_result<PeerState> state_of(std::string);

    concurrencpp::shared_result<PeerType> type_of(std::string);

   private:
    explicit NodeManager(
      std::shared_ptr<concurrencpp::worker_thread_executor>,
      EventQueuePtr,
      SignalingClientPtr,
      PeerType,
      PeerState
    );

    EventQueuePtr m_event_queue;
    SignalingClientPtr m_signaling_client;
    concurrencpp::async_lock m_lock;
    std::shared_ptr<concurrencpp::worker_thread_executor> m_worker;
    std::unordered_map<std::string, std::shared_ptr<PeerConnection>>
      m_connection_map;
    eventpp::ScopedRemover<EventQueueType> m_subscription_remover;

    PeerState m_peer_state;
    PeerType m_peer_type;

    void on_rtc_setup(Event);

    void on_rtc_candidate(Event);

    void on_peer_state_request(Event);

    void on_peer_type_request(Event);

    void on_send_peer_message(Event);

    void on_datachannel_opened(Event);

    void on_datachannel_closed(Event);

    void on_peer_closed(Event);

    void on_peer_available(Event);

    concurrencpp::null_result connect_to_peer(Event);
  };

  using NodeManagerPtr = std::shared_ptr<NodeManager>;
}// namespace krapi
