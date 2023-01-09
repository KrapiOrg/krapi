#include "TransactionPoolManager.h"

namespace krapi {

  void TransactionPoolManager::on_add_transaction(Event e) {

    auto peer_message = e.get<PeerMessage>();
    auto transaction = Transaction::from_json(peer_message->content());

    spdlog::info(
      "Validating transaction #{} from peer {}",
      transaction.contrived_hash(),
      peer_message->sender_identity()
    );

    auto validation_state = Validator::validate_transaction(
      m_spent_transactions_store,
      transaction
    );

    if (validation_state != ValidationState::Success) {

      spdlog::warn(
        "Validaiton failed for transaction {}, {}",
        transaction.contrived_hash(),
        to_string(validation_state)
      );
      return;
    }

    if (m_transaction_pool->add(transaction)) {

      spdlog::info(
        "Added transaction #{} from {}",
        transaction.contrived_hash(),
        peer_message->sender_identity()
      );
    } else {

      spdlog::info(
        "Did not Add transaction #{} from {} because it is already in the pool",
        transaction.contrived_hash(),
        peer_message->sender_identity()
      );
    }
  }
  void TransactionPoolManager::on_sync_pool_request(Event e) {

    auto peer_message = e.get<PeerMessage>();

    spdlog::info(
      "{} Requested to sync transaction pools",
      peer_message->sender_identity()
    );

    for (auto transaction: m_transaction_pool->data()) {

      spdlog::info(
        "Sending pool transaction #{} to {}",
        transaction.contrived_hash(),
        peer_message->sender_identity()
      );

      m_peer_manager->send_and_forget(
        PeerMessageType::AddTransaction,
        m_peer_manager->id(),
        peer_message->sender_identity(),
        peer_message->tag(),
        transaction.to_json()
      );
    }
  }
  TransactionPoolManager::TransactionPoolManager(
    EventLoopPtr event_loop,
    PeerManagerPtr peer_manager,
    TransactionPoolPtr transaction_pool,
    SpentTransactionsStorePtr spent_transactions_store
  )
      : m_event_loop(std::move(event_loop)),
        m_peer_manager(std::move(peer_manager)),
        m_transaction_pool(std::move(transaction_pool)),
        m_spent_transactions_store(std::move(spent_transactions_store)) {

    m_event_loop->append_listener(
      PeerMessageType::AddTransaction,
      std::bind_front(&TransactionPoolManager::on_add_transaction, this)
    );
    m_event_loop->append_listener(
      PeerMessageType::SyncPoolRequest,
      std::bind_front(&TransactionPoolManager::on_sync_pool_request, this)
    );
  }
}// namespace krapi