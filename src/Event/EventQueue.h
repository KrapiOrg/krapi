//
// Created by mythi on 24/12/22.
//

#pragma once
#include "Event.h"
#include "eventpp/eventqueue.h"

namespace krapi {
    using _EventQueue = eventpp::EventQueue<EventType, void(Event)>;

    class EventQueue {
        _EventQueue m_event_queue;

        std::mutex m_mutex;
        std::unordered_map<std::string, EventPromise> m_promises;

    public:

        static std::unique_ptr<EventQueue> create() {

            return std::make_unique<EventQueue>();
        }

        void enqueue(EventType event_type, Event event) {

            m_event_queue.enqueue(event_type, event);
        }

        concurrencpp::result<Event> create_awaitable(std::string tag) {

            auto promise = EventPromise();
            auto result = promise.get_result();
            {
                std::lock_guard l(m_mutex);
                m_promises.emplace(tag, std::move(promise));
            }
            return result;
        }

        bool process_one() {

            _EventQueue::QueuedEvent queue_event;
            if (m_event_queue.takeEvent(&queue_event)) {
                auto event_type = queue_event.getEvent();
                auto event = queue_event.getArgument<0>();
                std::string tag = std::visit([](auto &&ev) { return ev->tag(); }, event);

                {
                    std::lock_guard l(m_mutex);
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
    };

    using EventQueuePtr = std::unique_ptr<EventQueue>;
}