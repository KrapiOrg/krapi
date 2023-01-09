#pragma once

#include "Blockchain.h"
#include "Content/BlockHeadersResponseContent.h"
#include "Event.h"
#include "EventLoop.h"
#include "EventQueue.h"
#include "NodeManager.h"
#include "TransactionPool.h"
#include "ValidationState.h"
#include "Validator.h"
#include "spdlog/spdlog.h"

namespace krapi {
  class BlockchainManager {

   public:
    static std::shared_ptr<BlockchainManager> create(
      EventLoopPtr event_loop,
      NodeManagerPtr node_manager,
      TransactionPoolPtr transaction_pool,
      BlockchainPtr blockchain,
      SpentTransactionsStorePtr spent_transactions_store
    ) {
      return std::shared_ptr<BlockchainManager>(new BlockchainManager(
        std::move(event_loop),
        std::move(node_manager),
        std::move(transaction_pool),
        std::move(blockchain),
        std::move(spent_transactions_store)
      ));
    }

   private:

    void on_block_headers_request(Event);
    void on_block_request(Event);
    void on_add_block(Event);
    void on_block_mined(Event);

    EventLoopPtr m_event_loop;
    NodeManagerPtr m_node_manager;
    TransactionPoolPtr m_transaction_pool;
    BlockchainPtr m_blockchain;
    SpentTransactionsStorePtr m_spent_transactions_store;

    BlockchainManager(
      EventLoopPtr event_loop,
      NodeManagerPtr node_manager,
      TransactionPoolPtr transaction_pool,
      BlockchainPtr blockchain,
      SpentTransactionsStorePtr spent_transactions_store
    );
  };
  using BlockchainManagerPtr = std::shared_ptr<BlockchainManager>;
}// namespace krapi