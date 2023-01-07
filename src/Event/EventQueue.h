//
// Created by mythi on 24/12/22.
//

#pragma once

#include "concurrencpp/concurrencpp.h"
#include "eventpp/eventqueue.h"
#include "spdlog/spdlog.h"

#include "PromiseStore.h"

#include "Block.h"
#include "Event.h"
#include "NotNull.h"
#include "Overload.h"
#include "Transaction.h"
#include <concurrencpp/executors/executor.h>
#include <concurrencpp/results/shared_result.h>
#include <concurrencpp/results/when_result.h>
#include <functional>
#include <memory>

namespace krapi {
  using EventPromise = concurrencpp::result_promise<Event>;
  using EventQueueType = eventpp::EventQueue<EventType, void(Event)>;


  template<typename T>
  concept has_create = requires(T) { T::create(); };

  class EventQueue : public std::enable_shared_from_this<EventQueue> {

    EventQueueType m_event_queue;
    PromiseStore<Event> m_promise_store;

   public:
    [[nodiscard]] static inline std::shared_ptr<EventQueue> create() {

      return std::make_shared<EventQueue>();
    }

    void enqueue(Event event) {

      auto event_type =
        std::visit([](auto &&ev) -> EventType { return ev->type(); }, event);
      m_event_queue.enqueue(event_type, std::move(event));
    }

    template<has_create T, typename... U>
    void enqueue(U &&...params) {

      Event event = T::create(std::forward<U>(params)...);
      auto event_type =
        std::visit([](auto &&ev) -> EventType { return ev->type(); }, event);
      m_event_queue.enqueue(event_type, std::move(event));
    }

    concurrencpp::shared_result<Event> create_awaitable(std::string tag) {

      return m_promise_store.add(tag);
    }

    template<has_create T, typename... U>
    concurrencpp::shared_result<Event> submit(U &&...params) {

      Event event = T::create(std::forward<U>(params)...);
      auto tag = std::visit([](auto &&e) { return e->tag(); }, event);
      auto event_type =
        std::visit([](auto &&ev) -> EventType { return ev->type(); }, event);

      auto awaitable = m_promise_store.add(tag);
      m_event_queue.enqueue(event_type, std::move(event));
      return awaitable;
    }

    bool process_one() {

      EventQueueType::QueuedEvent queue_event;
      if (m_event_queue.takeEvent(&queue_event)) {

        Event event = queue_event.getArgument<0>();

        std::visit(
          Overload{
            [=, this]<typename T>(Box<InternalMessage<T>> ev) {
              m_event_queue.dispatch(queue_event);
            },
            [=, this](Box<InternalNotification<std::string>> notif) {
              auto tag = notif->content();
              m_promise_store.set_result(tag, notif);
              m_event_queue.dispatch(queue_event);
            },
            [=, this](auto &&ev) {
              auto tag = ev->tag();

              m_promise_store.set_result(tag, ev);
              m_event_queue.dispatch(queue_event);
            }},
          event
        );
        return true;
      }

      return false;
    }

    void append_listener(EventType type, std::function<void(Event)> callback) {

      m_event_queue.appendListener(type, callback);
    }

    concurrencpp::result<Event> event_of_type(
      EventType type,
      int count = 1,
      std::optional<std::function<bool(Event)>> predicate = {}
    ) {
      using Handle = EventQueueType::Handle;
      auto promise = std::make_shared<concurrencpp::result_promise<Event>>();
      auto handle = std::make_shared<Handle>();
      auto count_ptr = std::make_shared<int>(count);
      auto predicate_ptr =
        std::make_shared<std::optional<std::function<bool(Event)>>>(predicate);

      *handle = m_event_queue.appendListener(
        type,
        [=, self = weak_from_this()](Event e) {
          auto type_copy = type;
          auto promise_copy = promise;
          auto handle_copy = handle;
          auto count_copy = count_ptr;
          auto predicate_copy = predicate_ptr;

          if (*predicate_copy) {
            if (std::invoke(predicate_copy->value(), e)) {
              *count_copy = *count_copy - 1;
            }
          } else {
            *count_copy = *count_copy - 1;
          }

          if (*count_copy <= 0) {
            promise_copy->set_result(e);
            if (auto instance = self.lock()) {
              instance->m_event_queue.removeListener(type_copy, *handle_copy);
            }
          }
        }
      );

      return promise->get_result();
    }

    template<typename Message, typename MessageType>
    concurrencpp::result<std::pair<MessageType, Box<Message>>>
    any_event_of_type(
      std::shared_ptr<concurrencpp::executor> executor,
      std::vector<MessageType> types
    ) {

      auto promises = std::vector<concurrencpp::result<Event>>();

      for (auto type: types) { promises.push_back(event_of_type(type)); }

      auto [index, events] = co_await concurrencpp::when_any(
        executor,
        promises.begin(),
        promises.end()
      );
      auto event = co_await events[index];
      auto message = event.template get<Message>();
      auto type = message->type();
      co_return std::pair<MessageType, Box<Message>>{type, std::move(message)};
    }

    void close() { m_event_queue.clearEvents(); }

    void wait() { m_event_queue.wait(); }

    EventQueueType &internal_queue() { return m_event_queue; }
  };

  using EventQueuePtr = std::shared_ptr<EventQueue>;
}// namespace krapi