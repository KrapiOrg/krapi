#pragma once

#include "Blockchain.h"
#include "Content/BlockHeadersResponseContent.h"
#include "Event.h"
#include "EventLoop.h"
#include "EventQueue.h"
#include "PeerManager.h"
#include "TransactionPool.h"
#include "ValidationState.h"
#include "Validator.h"
#include "spdlog/spdlog.h"
#include <concurrencpp/executors/thread_executor.h>
#include <memory>

namespace krapi {
  class BlockchainManager {

   public:
    static std::shared_ptr<BlockchainManager> create(
      std::shared_ptr<concurrencpp::thread_executor> executor,
      EventLoopPtr event_loop,
      PeerManagerPtr peer_manager,
      TransactionPoolPtr transaction_pool,
      BlockchainPtr blockchain,
      SpentTransactionsStorePtr spent_transactions_store
    ) {
      return std::shared_ptr<BlockchainManager>(
        new BlockchainManager(
          std::move(executor),
          std::move(event_loop),
          std::move(peer_manager),
          std::move(transaction_pool),
          std::move(blockchain),
          std::move(spent_transactions_store)
        )
      );
    }

   private:
    void on_block_headers_request(Event);
    void on_block_request(Event);
    void on_add_block(Event);
    void on_block_mined(Event);
    void on_set_transaction_status(Event);

    std::shared_ptr<concurrencpp::thread_executor> m_executor;
    EventLoopPtr m_event_loop;
    PeerManagerPtr m_peer_manager;
    TransactionPoolPtr m_transaction_pool;
    BlockchainPtr m_blockchain;
    SpentTransactionsStorePtr m_spent_transactions_store;

    BlockchainManager(
      std::shared_ptr<concurrencpp::thread_executor> executor,
      EventLoopPtr event_loop,
      PeerManagerPtr peer_manager,
      TransactionPoolPtr transaction_pool,
      BlockchainPtr blockchain,
      SpentTransactionsStorePtr spent_transactions_store
    );
  };
  using BlockchainManagerPtr = std::shared_ptr<BlockchainManager>;
}// namespace krapi