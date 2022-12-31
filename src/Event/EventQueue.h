//
// Created by mythi on 24/12/22.
//

#pragma once

#include "eventpp/eventqueue.h"
#include "concurrencpp/concurrencpp.h"
#include "spdlog/spdlog.h"

#include "PromiseStore.h"

#include "Event.h"
#include "Overload.h"
#include "NotNull.h"

namespace krapi {
    using EventPromise = concurrencpp::result_promise<Event>;
    using EventQueueType = eventpp::EventQueue<EventType, void(Event)>;


    template<typename T>
    concept has_create = requires(T){
        T::create();
    };

    class EventQueue {

        EventQueueType m_event_queue;
        PromiseStore<Event> m_promise_store;

    public:

        [[nodiscard]]
        static inline std::shared_ptr<EventQueue> create() {

            return std::make_shared<EventQueue>();
        }

        void enqueue(Event event) {

            auto event_type = std::visit([](auto &&ev) -> EventType { return ev->type(); }, event);
            m_event_queue.enqueue(event_type, std::move(event));
        }

        template<has_create T, typename ...U>
        void enqueue(U &&...params) {

            Event event = T::create(std::forward<U>(params)...);
            auto event_type = std::visit([](auto &&ev) -> EventType { return ev->type(); }, event);
            m_event_queue.enqueue(event_type, std::move(event));
        }

        concurrencpp::shared_result <Event> create_awaitable(std::string tag) {

            return m_promise_store.add(tag);
        }

        template<has_create T, typename ...U>
        concurrencpp::shared_result <Event> submit(U &&...params) {

            Event event = T::create(std::forward<U>(params)...);
            auto tag = std::visit([](auto &&e) { return e->tag(); }, event);
            auto event_type = std::visit([](auto &&ev) -> EventType { return ev->type(); }, event);

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
                                    if (m_promise_store.set_result(tag, notif))
                                        m_event_queue.dispatch(queue_event);
                                },
                                [=, this](auto &&ev) {
                                    auto tag = ev->tag();

                                    if (m_promise_store.set_result(tag, ev))
                                        m_event_queue.dispatch(queue_event);
                                }
                        },
                        event
                );
                return true;
            }

            return false;
        }

        void append_listener(EventType type, std::function<void(Event)> callback) {

            m_event_queue.appendListener(type, callback);
        }

        void close() {

            m_event_queue.clearEvents();
        }

        void wait() {

            m_event_queue.wait();
        }

        EventQueueType &internal_queue() {

            return m_event_queue;
        }
    };

    using EventQueuePtr = std::shared_ptr<EventQueue>;
}