#pragma once

#include "DBInterface.h"
#include "EventQueue.h"
#include "InternalMessage.h"
#include "InternalNotification.h"
#include "PeerMessage.h"
#include "eventpp/utilities/scopedremover.h"
#include "spdlog/spdlog.h"
#include <concurrencpp/executors/thread_executor.h>
#include <filesystem>
#include <memory>

namespace krapi {
  class RetryHandler : public DBInternface<InternalMessage<PeerMessage>> {

   public:
    static std::shared_ptr<RetryHandler> create(
      std::shared_ptr<concurrencpp::thread_executor> executor,
      std::string storage_path,
      EventQueuePtr event_queue
    ) {
      return std::shared_ptr<RetryHandler>(
        new RetryHandler(
          std::move(executor),
          std::move(storage_path),
          std::move(event_queue)
        )
      );
    }

    void retry(InternalMessage<PeerMessage> message);

    void on_datachannel_open(Event e);

   private:
    std::shared_ptr<concurrencpp::thread_executor> m_executor;
    EventQueuePtr m_event_queue;
    eventpp::ScopedRemover<EventQueueType> m_remover;

    RetryHandler(
      std::shared_ptr<concurrencpp::thread_executor> executor,
      std::string storage_path,
      EventQueuePtr event_queue
    );
  };
  using RetryHandlerPtr = std::shared_ptr<RetryHandler>;
}// namespace krapi