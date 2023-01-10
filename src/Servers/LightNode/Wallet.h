//
// Created by mythi on 18/11/22.
//

#pragma once

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <unordered_map>

#include "Content/SetTransactionStatusContent.h"
#include "DBInterface.h"
#include "EventQueue.h"
#include "PeerMessage.h"
#include "cryptopp/sha.h"

#include "Transaction.h"
#include "eventpp/utilities/scopedremover.h"
#include "spdlog/spdlog.h"

namespace krapi {

  class Wallet : DBInternface<Transaction> {

   public:
    void on_add_transaction(Event e) {

      auto message = e.get<PeerMessage>();
      auto transaction = Transaction::from_json(message->content());

      if (put(transaction)) {

        spdlog::info(
          "Wallet: {} Sent transaction #{}",
          message->sender_identity(),
          transaction.contrived_hash()
        );
      } else {
        spdlog::error(
          "Wallet: Failed to add Tx#{} from peer {}",
          transaction.contrived_hash(),
          message->sender_identity()
        );
      }
    }

    void on_transactions_status_changed(Event e) {
      auto message = e.get<PeerMessage>();
      auto content = SetTransactionStatusContent::from_json(message->content());


      if (auto transaction = get(content.hash())) {

        remove(transaction->hash());
        put(
          Transaction(
            transaction->type(),
            content.status(),
            content.hash(),
            transaction->timestamp(),
            transaction->from(),
            transaction->to()
          )
        );
        spdlog::info(
          "Wallet: {} Set status of transaction #{} to {}",
          message->sender_identity(),
          transaction->contrived_hash(),
          to_string(content.status())
        );
      } else {
        spdlog::error(
          "Wallet: Failed to set status of Tx#{} from peer {}",
          content.hash().substr(0, 10),
          message->sender_identity()
        );
      }
    }

    static std::shared_ptr<Wallet> create(
      EventQueuePtr event_queue
    ) {

      return std::shared_ptr<Wallet>(
        new Wallet(
          std::move(event_queue)
        )
      );
    }

    Transaction create_transaction(std::string my_id, std::string receiver_id);

   private:
    eventpp::ScopedRemover<EventQueueType> m_remover;

    Wallet(EventQueuePtr event_queue)
        : m_remover(event_queue->internal_queue()) {
      auto path = PeerMessage::create_tag();

      std::filesystem::create_directory(path);

      if (!initialize(path)) {
        spdlog::error("Faield to open wallet database!");
        exit(1);
      }

      m_remover.appendListener(
        PeerMessageType::AddTransaction,
        std::bind_front(&Wallet::on_add_transaction, this)
      );

      m_remover.appendListener(
        PeerMessageType::SetTransactionStatus,
        std::bind_front(&Wallet::on_transactions_status_changed, this)
      );
    }
  };
  using WalletPtr = std::shared_ptr<Wallet>;
}// namespace krapi
