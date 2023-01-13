#pragma once

#include "EventQueue.h"
#include "eventpp/utilities/scopedremover.h"

namespace krapi {
  class EventLoop {
    std::shared_ptr<concurrencpp::worker_thread_executor> m_worker;
    std::shared_ptr<std::atomic_bool> m_end;
    EventQueuePtr m_event_queue;
    eventpp::ScopedRemover<EventQueueType> m_remover;

    EventLoop(std::shared_ptr<concurrencpp::worker_thread_executor> worker)
        : m_worker(std::move(worker)),
          m_end(std::make_shared<std::atomic<bool>>(false)),
          m_event_queue(EventQueue::create()),
          m_remover(m_event_queue->internal_queue()) {

      m_worker->enqueue([this]() {
        while (!m_end->load()) {

          m_event_queue->wait();
          m_event_queue->process_one();
        }
      });
    }

   public:
    static inline std::shared_ptr<EventLoop>
    create(std::shared_ptr<concurrencpp::worker_thread_executor> worker) {

      return std::shared_ptr<EventLoop>(new EventLoop(std::move(worker)));
    }

    void
    append_listener(EventType event_type, std::function<void(Event)> callback) {

      m_remover.appendListener(event_type, callback);
    }

    void end() {

      m_end->exchange(true);
      m_end->notify_all();
    }

    EventQueuePtr event_queue() {
      return m_event_queue;
    }

    void wait() {
      m_end->wait(false);
    }

    ~EventLoop() {
      wait();
    }
  };
  using EventLoopPtr = std::shared_ptr<EventLoop>;
}// namespace krapi