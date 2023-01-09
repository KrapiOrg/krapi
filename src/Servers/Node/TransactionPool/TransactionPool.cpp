//
// Created by mythi on 12/11/22.
//

#include "TransactionPool.h"

#include "Overload.h"
#include "spdlog/spdlog.h"
#include <concurrencpp/results/result.h>
#include <concurrencpp/results/shared_result.h>
#include <memory>

using namespace std::chrono_literals;


namespace krapi {

  TransactionPool::TransactionPool(std::string path)
      : m_queue(std::make_shared<Queue>()) {
    if (!initialize(path)) {

      spdlog::error("Failed to open pool database");
      exit(1);
    }
  }

  bool TransactionPool::add(Transaction transaction) {
    auto pooled = put(transaction);
    if (pooled) {

      auto enqueued = m_queue->try_enqueue(transaction);
      return true;
    }
    return false;
  }
  concurrencpp::result<Transactions>
  TransactionPool::wait(std::shared_ptr<concurrencpp::thread_executor> executor, int n) {

    auto promise = std::make_shared<concurrencpp::result_promise<Transactions>>();

    executor->enqueue([=, self = weak_from_this()]() {
      int n_copy = n;
      auto promise_copy = promise;

      std::variant<Transaction, PoolEndReason> item;
      Transactions transactions(n_copy);
      size_t index = 0;

      while (index != n_copy) {

        if (auto instance = self.lock()) {

          instance->m_queue->wait_dequeue(item);

          auto should_end = std::visit(
            Overload{
              [](Transaction tx) {
                return false;
              },
              [](PoolEndReason) {
                spdlog::warn("Ending transaction pool...");
                return true;
              }},
            item
          );

          if (should_end) {
            //promise->set_result(Transactions{});
            return;
          }

          auto transaction = std::get<Transaction>(item);

          if (instance->get(transaction.hash())) {
            transactions[index++] = transaction;
          }
        }
      }
      promise_copy->set_result(transactions);
    });

    return promise->get_result();
  }

}// namespace krapi