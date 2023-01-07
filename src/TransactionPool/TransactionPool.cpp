//
// Created by mythi on 12/11/22.
//

#include "TransactionPool.h"
#include "EventQueue.h"
#include "InternalNotification.h"
#include "Transaction.h"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <concurrencpp/results/make_result.h>
#include <concurrencpp/results/shared_result.h>
#include <filesystem>
#include <iterator>
#include <memory>
#include <string>


namespace krapi {

  TransactionPool::TransactionPool(std::string path, EventQueuePtr event_queue)
      : m_event_queue(std::move(event_queue)) {

    namespace fs = std::filesystem;

    if (!fs::exists(path)) {

      spdlog::info("Creating txpool directory");
      fs::create_directory(path);
    }
    if (!initialize(path)) {
      spdlog::error("Failed to open transaction pool database");
      exit(1);
    }
  }

  bool TransactionPool::add(Transaction transaction) {

    auto added = TransactionPool::put(transaction);

    if (added) {

      m_transaction_queue.push(transaction);

      m_event_queue->enqueue<InternalNotification<void>>(
        InternalNotificationType::TransactionAddedToPool
      );
    }


    return added;
  }


  TransactionsResult TransactionPool::wait_for(int n) const {
    auto promise = std::make_shared<TransactionsPromise>();

    using Handle = EventQueueType::Handle;
    using HandlePtr = std::shared_ptr<Handle>;

    auto handle = std::make_shared<Handle>();
    spdlog::info("Before {}", m_transaction_queue.size());
    auto transactions = std::make_shared<Transactions>(take_from_queue(n));

    if (std::ssize(*transactions) == n) {
      return concurrencpp::make_ready_result<Transactions>(*transactions);
    }

    *handle = m_event_queue->internal_queue().appendListener(
      InternalNotificationType::TransactionAddedToPool,
      [=, self = weak_from_this()](Event) {
        
        auto promise_copy = promise;
        auto handle_copy = handle;
        auto transactions_copy = transactions;
        auto n_copy = n;

        auto instance = self.lock();
        
        if (!instance) return;
                
        auto tx = instance->take_from_queue(1);
        
        transactions_copy->push_back(tx[0]);

        if (std::ssize(*transactions_copy) >= n_copy) {
          
          promise_copy->set_result(*transactions_copy);
          instance->m_event_queue->internal_queue().removeListener(
            InternalNotificationType::TransactionAddedToPool,
            *handle_copy
          );
        }
      }
    );

    return promise->get_result();
  }
  Transactions TransactionPool::take_from_queue(int n) const {
    std::vector<Transaction> taken;
    if (std::ssize(m_transaction_queue) >= n) {
      while (n--) {
        auto front = m_transaction_queue.front();
        m_transaction_queue.pop();
        taken.push_back(front);
      }
    } else {
      while (!m_transaction_queue.empty()) {
        auto front = m_transaction_queue.front();
        m_transaction_queue.pop();
        taken.push_back(front);
      }
    }

    return taken;
  }
}// namespace krapi