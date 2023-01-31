#pragma once

#include "DBInterface.h"
#include "EventQueue.h"
#include "InternalMessage.h"
#include "PeerMessage.h"
#include "Transaction.h"
#include "eventpp/utilities/scopedremover.h"
#include "spdlog/spdlog.h"
#include <memory>
namespace krapi {
  class SpentTransactionsStore : public DBInternface<Transaction> {

   public:
    static std::shared_ptr<SpentTransactionsStore> create(
      std::string path,
      EventQueuePtr event_queue
    ) {
      return std::shared_ptr<SpentTransactionsStore>(
        new SpentTransactionsStore(std::move(path), std::move(event_queue))
      );
    }

    void on_transactions_in_request(Event e) {
      auto peer_message = e.get<PeerMessage>();
      auto identity = peer_message->content().get<std::string>();

      auto counter = 0;
      for (auto transaction: data()) {
        if(transaction.to() == identity)
          counter++;
      }
      m_event_queue->enqueue<InternalMessage<PeerMessage>>(
        InternalMessageType::SendPeerMessage,
        PeerMessageType::TransactionsInResponse,
        peer_message->receiver_identity(),
        peer_message->sender_identity(),
        peer_message->tag(),
        nlohmann::json(counter)
      );
    }

    void on_transactions_out_request(Event e) {
      auto peer_message = e.get<PeerMessage>();
      auto identity = peer_message->content().get<std::string>();

      auto counter = 0;
      for (auto transaction: data()) {
        if(transaction.from() == identity)
          counter++;
      }
      m_event_queue->enqueue<InternalMessage<PeerMessage>>(
        InternalMessageType::SendPeerMessage,
        PeerMessageType::TransactionsOutResponse,
        peer_message->receiver_identity(),
        peer_message->sender_identity(),
        peer_message->tag(),
        nlohmann::json(counter)
      );
    }

   private:
    SpentTransactionsStore(
      std::string path,
      EventQueuePtr event_queue
    )
        : m_event_queue(event_queue),
          m_remover(event_queue->internal_queue()) {
      if (!initialize(path)) {
        spdlog::error("Failed to open spent transactions database");
        exit(1);
      }
      m_remover.appendListener(
        PeerMessageType::TransactionsInRequest,
        std::bind_front(&SpentTransactionsStore::on_transactions_in_request, this)
      );
      m_remover.appendListener(
        PeerMessageType::TransactionsOutRequest,
        std::bind_front(&SpentTransactionsStore::on_transactions_out_request, this)
      );
    }
    EventQueuePtr m_event_queue;
    eventpp::ScopedRemover<EventQueueType> m_remover;
  };

  using SpentTransactionsStorePtr = std::shared_ptr<SpentTransactionsStore>;
}// namespace krapi