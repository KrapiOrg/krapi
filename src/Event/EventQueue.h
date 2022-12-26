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

        void enqueue(EventType event_type, Event event) {

            m_event_queue.enqueue(event_type, event);
        }

        concurrencpp::shared_result <Event> create_awaitable(std::string tag) {

            auto promise = EventPromise();
            auto result = promise.get_result();

            m_promises.emplace(tag, std::move(promise));
            return result;
        }

        bool process_one() {

            if (m_closed)
                return false;
            EventQueueType::QueuedEvent queue_event;
            if (m_event_queue.takeEvent(&queue_event)) {
                auto event_type = queue_event.getEvent();
                auto event = queue_event.getArgument<0>();

                std::string tag = std::visit([](auto &&ev) { return ev->tag(); }, event);

                {
                    if (m_promises.contains(tag)) {
                        m_promises.find(tag)->second.set_result(event);
                        m_promises.erase(tag);
                        return true;
                    }
                }

                m_event_queue.dispatch(queue_event);
                return true;
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