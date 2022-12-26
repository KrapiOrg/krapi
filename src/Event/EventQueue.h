//
// Created by mythi on 24/12/22.
//

#pragma once

#include "Event.h"
#include "eventpp/eventqueue.h"
#include "eventpp/utilities/counterremover.h"

#include "concurrencpp/concurrencpp.h"
#include "Overload.h"

namespace krapi {
    using EventPromise = concurrencpp::result_promise<Event>;
    using EventQueueType = eventpp::EventQueue<EventType, void(Event)>;

    template<typename T>
    concept has_create = requires(T){
        T::create();
    };

    class EventQueue {

        EventQueueType m_event_queue;

        std::shared_ptr<concurrencpp::worker_thread_executor> m_worker;
        std::unordered_map<std::string, EventPromise> m_promises;
        std::atomic<bool> m_closed;


    public:

        explicit EventQueue() :
                m_closed(false) {

        }

        static std::unique_ptr<EventQueue> create() {

            return std::make_unique<EventQueue>();
        }

        void enqueue(Event event) {

            auto event_type = std::visit([](auto &&ev) -> EventType { return ev->type(); }, event);
            m_event_queue.enqueue(event_type, std::move(event));
        }

        template<has_create T, typename ...U>
        void enqueue(U &&...params) {

            enqueue(T::create(std::forward<U>(params)...));
        }

        concurrencpp::shared_result<Event> create_awaitable(std::string tag) {

            auto promise = EventPromise();
            auto result = promise.get_result();

            m_promises.emplace(tag, std::move(promise));
            return result;
        }

        template<has_create T, typename ...U>
        concurrencpp::shared_result<Event> submit(U &&...params) {

            Event event = T::create(std::forward<U>(params)...);
            auto awaitable = create_awaitable(std::visit(
                    Overload{
                            [](Box<InternalMessage> e) {
                                return e->content()["tag"].template get<std::string>();
                            },
                            [](auto &&e) {
                                return e->tag();
                            }
                    }, event
            ));
            enqueue(std::move(event));
            return awaitable;
        }

        bool process_one() {

            if (m_closed)
                return false;
            EventQueueType::QueuedEvent queue_event;
            if (m_event_queue.takeEvent(&queue_event)) {
                auto event_type = queue_event.getEvent();
                auto event = queue_event.getArgument<0>();

                return std::visit(
                        Overload{
                                [=, this](Box<InternalMessage>) {
                                    m_event_queue.dispatch(queue_event);
                                    return true;
                                },
                                [=, this](auto &&ev) {
                                    auto tag = ev->tag();

                                    if (m_promises.contains(tag)) {
                                        m_promises.find(tag)->second.set_result(ev);
                                        m_promises.erase(tag);
                                        return true;
                                    }

                                    m_event_queue.dispatch(queue_event);
                                    return true;
                                }
                        },
                        event
                );
            }

            return false;
        }

        void append_listener(EventType event_type, std::function<void(Event)> callback) {

            m_event_queue.appendListener(event_type, callback);
        }

        void close() {

            m_closed = true;
            m_event_queue.clearEvents();
        }

        void wait() {

            m_event_queue.wait();
        }

        EventQueueType &internal_queue() {

            return m_event_queue;
        }
    };

    using EventQueuePtr = std::unique_ptr<EventQueue>;
}