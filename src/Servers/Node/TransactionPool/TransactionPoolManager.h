#pragma once
#include "Event.h"
#include "EventLoop.h"
#include "NodeManager.h"
#include "PeerMessage.h"
#include "SpentTransactionsStore.h"
#include "TransactionPool.h"
#include "ValidationState.h"
#include "Validator.h"
#include "spdlog/spdlog.h"
#include <memory>

namespace krapi {

  class TransactionPoolManager {
   public:
    static std::shared_ptr<TransactionPoolManager> create(
      EventLoopPtr event_loop,
      NodeManagerPtr node_manager,
      TransactionPoolPtr transaction_pool,
      SpentTransactionsStorePtr spent_transactions_store
    ) {

      return std::shared_ptr<TransactionPoolManager>(
        new TransactionPoolManager(
          std::move(event_loop),
          std::move(node_manager),
          std::move(transaction_pool),
          std::move(spent_transactions_store)
        )
      );
    }

   private:
    void on_add_transaction(Event e);
    void on_sync_pool_request(Event e);

    TransactionPoolManager(
      EventLoopPtr event_loop,
      NodeManagerPtr node_manager,
      TransactionPoolPtr transaction_pool,
      SpentTransactionsStorePtr spent_transactions_store
    );

    EventLoopPtr m_event_loop;
    NodeManagerPtr m_node_manager;
    TransactionPoolPtr m_transaction_pool;
    SpentTransactionsStorePtr m_spent_transactions_store;
  };

}// namespace krapi