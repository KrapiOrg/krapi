#include "RetryHandler.h"

namespace krapi {


  void RetryHandler::retry(InternalMessage<PeerMessage> message) {
    if (message.retry_count() >= 3) {

      remove(message);
      message.reset_retry_count();
      put(message);
    } else {
      message.increament_retry_count();
      m_event_queue->enqueue<InternalMessage<PeerMessage>>(message);
    }
  }
  void RetryHandler::on_datachannel_open(Event e) {
    auto pid = e.get<InternalNotification<std::string>>()->content();

    m_executor->enqueue([=, this]() {
      auto peer_id = pid;
      for (InternalMessage<PeerMessage> internal_message: data()) {

        auto content = internal_message.content();
        if (content->receiver_identity() == peer_id) {

          spdlog::info(
            "RetryHandler: Retrying to send {} to peer {}",
            to_string(content->type()),
            peer_id
          );
          internal_message.increament_retry_count();
          m_event_queue->enqueue<InternalMessage<PeerMessage>>(internal_message);
          remove(internal_message);
        }
      }
    });
  }
  RetryHandler::RetryHandler(
    std::shared_ptr<concurrencpp::thread_executor> executor,
    std::string storage_path,
    EventQueuePtr event_queue
  )
      : m_executor(std::move(executor)),
        m_event_queue(event_queue),
        m_remover(event_queue->internal_queue()) {

    if (!std::filesystem::exists(storage_path)) {
      std::filesystem::create_directory(storage_path);
    }

    if (!initialize(storage_path)) {
      spdlog::error("Failed to open storage path for retry handler");
      exit(1);
    }
    m_remover.appendListener(
      InternalNotificationType::DataChannelOpened,
      std::bind_front(&RetryHandler::on_datachannel_open, this)
    );
  }
}// namespace krapi