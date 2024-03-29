#include "PeerManager.h"
#include "EventQueue.h"
#include "InternalMessage.h"
#include "InternalNotification.h"
#include "PeerMessage.h"
#include "PeerState.h"
#include "RetryHandler.h"
#include "eventpp/utilities/scopedremover.h"
#include "nlohmann/json_fwd.hpp"
#include "rtc/description.hpp"
#include "spdlog/spdlog.h"
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "eventpp/utilities/conditionalremover.h"

using namespace std::chrono_literals;


namespace krapi {

  PeerManager::PeerManager(
    std::shared_ptr<concurrencpp::worker_thread_executor> worker,
    EventQueuePtr event_queue,
    RetryHandlerPtr retry_handler,
    SignalingClientPtr signaling_client,
    PeerType pt,
    PeerState ps
  )
      : m_worker(std::move(worker)),
        m_event_queue(event_queue),
        m_retry_handler(std::move(retry_handler)),
        m_subscription_remover(event_queue->internal_queue()),
        m_signaling_client(std::move(signaling_client)),
        m_peer_state(ps),
        m_peer_type(pt) {

    m_subscription_remover.appendListener(
      SignalingMessageType::RTCSetup,
      std::bind_front(&PeerManager::on_rtc_setup, this)
    );

    m_subscription_remover.appendListener(
      PeerMessageType::PeerTypeRequest,
      std::bind_front(&PeerManager::on_peer_type_request, this)
    );

    m_subscription_remover.appendListener(
      PeerMessageType::PeerStateRequest,
      std::bind_front(&PeerManager::on_peer_state_request, this)
    );

    m_subscription_remover.appendListener(
      InternalMessageType::SendPeerMessage,
      std::bind_front(&PeerManager::on_send_peer_message, this)
    );

    m_subscription_remover.appendListener(
      InternalNotificationType::DataChannelOpened,
      std::bind_front(&PeerManager::on_datachannel_opened, this)
    );

    m_subscription_remover.appendListener(
      InternalNotificationType::DataChannelClosed,
      std::bind_front(&PeerManager::on_datachannel_closed, this)
    );

    m_subscription_remover.appendListener(
      SignalingMessageType::PeerClosed,
      std::bind_front(&PeerManager::on_peer_closed, this)
    );

    m_subscription_remover.appendListener(
      SignalingMessageType::PeerAvailable,
      std::bind_front(&PeerManager::connect_to_peer, this)
    );
  }

  std::string PeerManager::id() const {
    return m_signaling_client->identity();
  }


  concurrencpp::null_result PeerManager::connect_to_peer(Event event) {

    auto peer = event.get<SignalingMessage>()->content().get<std::string>();
    spdlog::info("{} is available", peer);

    if (m_connection_map.contains(peer)) co_return;

    auto awaitable = m_event_queue->create_awaitable(peer);

    auto peer_connection = PeerConnection::create(
      m_event_queue,
      m_signaling_client,
      peer
    );

    co_await m_lock.lock(m_worker);
    m_connection_map.emplace(peer, std::move(peer_connection));

    co_await awaitable;
  }

  PeerState PeerManager::get_state() const {
    return m_peer_state;
  }

  void PeerManager::on_rtc_setup(Event event) {

    auto message = event.get<SignalingMessage>();
    auto description = message->content();

    auto sdp = description["sdp"].get<std::string>();
    auto type = description["type"].get<std::string>();

    if (m_connection_map.contains(message->sender_identity())) {


      m_connection_map[message->sender_identity()]->set_remote_description(
        sdp,
        type
      );
    } else {

      m_connection_map.emplace(
        message->sender_identity(),
        PeerConnection::create(
          m_event_queue,
          m_signaling_client,
          message->sender_identity(),
          sdp,
          type
        )
      );
    }
  }

  void PeerManager::on_peer_state_request(Event event) {

    auto message = event.get<PeerMessage>();

    m_event_queue->enqueue<InternalMessage<PeerMessage>>(
      InternalMessageType::SendPeerMessage,
      PeerMessageType::PeerStateResponse,
      m_signaling_client->identity(),
      message->sender_identity(),
      message->tag(),
      m_peer_state
    );
  }

  void PeerManager::on_peer_type_request(Event event) {

    auto message = event.get<PeerMessage>();
    m_event_queue->enqueue<InternalMessage<PeerMessage>>(
      InternalMessageType::SendPeerMessage,
      PeerMessage::create(
        PeerMessageType::PeerTypeResponse,
        m_signaling_client->identity(),
        message->sender_identity(),
        message->tag(),
        m_peer_type
      )
    );
  }

  void PeerManager::on_send_peer_message(Event event) {

    auto internal_message = event.get<InternalMessage<PeerMessage>>();
    auto message = internal_message->content();

    auto retry = [=, this]() {
      spdlog::error(
        "PeerManager: Failed to send {} to {}, adding to retry handler...",
        to_string(message->type()),
        message->receiver_identity()
      );
      m_retry_handler->retry(*internal_message);
    };
    {
      m_lock.lock(m_worker).run().wait();
      if (!m_connection_map.contains(message->receiver_identity())) {

        retry();
        return;
      }
      if (!m_connection_map[message->receiver_identity()]->send_and_forget(message)) {

        retry();
      }
    }
  }

  std::vector<concurrencpp::shared_result<Event>>
  PeerManager::broadcast(PeerMessageType type, nlohmann::json content) {

    std::vector<concurrencpp::shared_result<Event>> results;

    for (const auto &[peer, ignore]: m_connection_map) {

      results.push_back(send(
        type,
        m_signaling_client->identity(),
        peer,
        PeerMessage::create_tag(),
        content
      ));
    }

    return results;
  }

  void PeerManager::broadcast_and_forget(
    PeerMessageType type,
    nlohmann::json content
  ) {

    for (const auto &[peer, _]: m_connection_map) {
      send_and_forget(
        type,
        m_signaling_client->identity(),
        peer,
        PeerMessage::create_tag(),
        content
      );
    }
  }

  void PeerManager::on_datachannel_opened(Event event) {

    auto id = event.get<InternalNotification<std::string>>()->content();
    auto connection = m_connection_map[id];
    spdlog::info("Peer {} connected", id);
  }

  void PeerManager::on_datachannel_closed(Event event) {

    auto id = event.get<InternalNotification<std::string>>()->content();
    spdlog::warn("DataChannel with {} closed", id);
  }

  void PeerManager::on_peer_closed(Event event) {

    auto id = event.get<SignalingMessage>()->content().get<std::string>();
    spdlog::info("PeerConnection with {} closed", id);
    m_connection_map.erase(id);
  }

  void PeerManager::set_state(PeerState state) {

    m_peer_state = state;
    broadcast_and_forget(PeerMessageType::PeerStateUpdate, state);
  }

  concurrencpp::shared_result<PeerState>
  PeerManager::state_of(std::string peer_id) {

    if (m_connection_map.contains(peer_id))
      return m_connection_map.find(peer_id)->second->state();

    return concurrencpp::make_ready_result<PeerState>(PeerState::Unknown);
  }

  concurrencpp::shared_result<PeerType> PeerManager::type_of(std::string peer_id
  ) {

    if (m_connection_map.contains(peer_id))
      return m_connection_map.find(peer_id)->second->type();
    return concurrencpp::make_ready_result<PeerType>(PeerType::Unknown);
  }
  concurrencpp::result<std::vector<std::string>> PeerManager::wait_for_peers(
    std::shared_ptr<concurrencpp::thread_executor> executor,
    int count,
    std::set<PeerType> types,
    std::set<PeerState> states
  ) {
    using Promise = concurrencpp::result_promise<std::vector<std::string>>;
    using Identities = std::vector<std::string>;
    using Types = std::set<PeerType>;
    using States = std::set<PeerState>;

    using PromisePtr = std::shared_ptr<Promise>;
    using IdentitiesPtr = std::shared_ptr<Identities>;
    using Handle = EventQueueType::Handle;
    using HandlePtr = std::shared_ptr<Handle>;
    using Null = concurrencpp::null_result;
    using GetterResult = concurrencpp::result<std::pair<PeerType, PeerState>>;

    auto promise = std::make_shared<Promise>();
    auto identities = std::make_shared<Identities>();

    auto details_getter = [this](std::string peer) -> GetterResult {
      co_return std::make_pair(co_await type_of(peer), co_await state_of(peer));
    };
    for (const auto &[peer_id, ignore]: m_connection_map) {
      auto [type, state] = co_await details_getter(peer_id);

      if (types.contains(type) && states.contains(state)) {

        identities->push_back(peer_id);
      }
    }

    if (identities->size() >= count) co_return *identities;


    HandlePtr handle1 = std::make_shared<Handle>();
    HandlePtr handle2 = std::make_shared<Handle>();

    auto on_datachannel_task = [=, this](
                                 Event e,
                                 PromisePtr promise,
                                 int count,
                                 IdentitiesPtr identities,
                                 Types types,
                                 States states
                               ) -> Null {
      auto h1 = handle1;
      auto h2 = handle2;
      auto peer_id = e.get<InternalNotification<std::string>>()->content();
      auto [type, state] = co_await details_getter(peer_id);

      if (types.contains(type) && states.contains(state)) {

        identities->push_back(peer_id);
      }
      if (identities->size() >= count) {
        promise->set_result(*identities);
        m_event_queue->internal_queue().removeListener(
          InternalNotificationType::DataChannelOpened,
          *h1
        );
        m_event_queue->internal_queue().removeListener(
          PeerMessageType::PeerStateUpdate,
          *h2
        );
      }
    };


    auto on_peer_state_update_task = [=, this](
                                       Event e,
                                       PromisePtr promise,
                                       int count,
                                       IdentitiesPtr identities,
                                       Types types,
                                       States states
                                     ) -> Null {
      auto h1 = handle1;
      auto h2 = handle2;
      auto peer_message = e.get<PeerMessage>();
      auto peer_id = peer_message->sender_identity();
      auto [type, state] = co_await details_getter(peer_id);

      if (types.contains(type) && states.contains(state)) {
        identities->push_back(peer_id);
      }
      if (identities->size() >= count) {
        promise->set_result(*identities);
        m_event_queue->internal_queue().removeListener(
          InternalNotificationType::DataChannelOpened,
          *h1
        );
        m_event_queue->internal_queue().removeListener(
          PeerMessageType::PeerStateUpdate,
          *h2
        );
      }
    };

    *handle1 = m_event_queue->internal_queue().appendListener(
      InternalNotificationType::DataChannelOpened,
      [=](Event e) {
        auto fn = std::bind(
          on_datachannel_task,
          e,
          promise,
          count,
          identities,
          types,
          states
        );
        executor->post(std::move(fn));
      }
    );
    *handle2 = m_event_queue->internal_queue().appendListener(
      PeerMessageType::PeerStateUpdate,
      [=](Event e) {
        auto fn = std::bind(
          on_datachannel_task,
          e,
          promise,
          count,
          identities,
          types,
          states
        );
        executor->post(std::move(fn));
      }
    );

    co_return co_await promise->get_result();
  }
  void PeerManager::broadcast_to_peers_of_type_and_forget(
    std::shared_ptr<concurrencpp::thread_executor> executor,
    std::set<PeerType> peer_types,
    PeerMessageType message_type,
    nlohmann::json content,
    std::set<std::string> except
  ) {
    auto my_id = m_signaling_client->identity();

    for (auto [peer_id, connection]: m_connection_map) {
      if (except.contains(peer_id))
        continue;

      auto sender = [](
                      std::string sender_identity,
                      std::set<PeerType> peer_types,
                      std::string receiver_identity,
                      PeerConnectionPtr connection,
                      PeerMessageType message_type,
                      nlohmann::json content
                    ) -> concurrencpp::null_result {
        auto type = co_await connection->type();
        if (peer_types.contains(type)) {
          (void) connection->send_and_forget(
            PeerMessage::create(
              message_type,
              sender_identity,
              receiver_identity,
              PeerMessage::create_tag(),
              content
            )
          );
        }
      };
      auto fn = std::bind(
        sender,
        my_id,
        peer_types,
        peer_id,
        connection,
        message_type,
        content
      );
      executor->post(std::move(fn));
    }
  }
}// namespace krapi