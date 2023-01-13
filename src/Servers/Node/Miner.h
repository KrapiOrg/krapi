//
// Created by mythi on 12/11/22.
//

#pragma once

#include "Block.h"
#include "Blockchain.h"
#include "EventQueue.h"
#include "Helpers.h"
#include "InternalNotification.h"
#include "PeerManager.h"
#include "PeerMessage.h"
#include "PeerType.h"
#include "TransactionPool.h"
#include "ValidationState.h"
#include "fmt/core.h"
#include "merklecpp.h"
#include "sha.h"
#include "spdlog/spdlog.h"
#include <chrono>
#include <concurrencpp/executors/thread_executor.h>
#include <concurrencpp/results/make_result.h>
#include <concurrencpp/results/result.h>
#include <concurrencpp/results/result_fwd_declarations.h>
#include <concurrencpp/results/shared_result.h>
#include <concurrencpp/threads/async_lock.h>
#include <eventpp/utilities/scopedremover.h>
#include <memory>
#include <optional>
#include <unordered_set>
#include <variant>

using namespace std::chrono;


namespace krapi {
  inline void hash_function(
    const merkle::HashT<32> &l,
    const merkle::HashT<32> &r,
    merkle::HashT<32> &out
  ) {

    CryptoPP::SHA256 sha_256;
    sha_256.Update(l.bytes, 32);
    sha_256.Update(r.bytes, 32);
    sha_256.Final(out.bytes);
  }

  inline concurrencpp::result<Block> mining_function(
    concurrencpp::executor_tag,
    std::shared_ptr<concurrencpp::thread_executor>,
    std::string previous_hash,
    std::string identity,
    Transactions transactions
  ) {

    CryptoPP::SHA256 sha_256;

    auto get_merkle_root = [&]() {
      merkle::TreeT<32, hash_function> tree;
      for (const auto &tx: transactions) {
        tree.insert(tx.hash());
      }
      return tree.root().to_string(32, false);
    };

    auto merkle_root = get_merkle_root();
    auto timestamp = get_krapi_timestamp();

    uint64_t nonce = 0;
    while (true) {

      auto block_hash = std::string{};

      StringSource s2(
        fmt::format("{}{}{}{}", previous_hash, merkle_root, timestamp, nonce),
        true,
        new HashFilter(sha_256, new HexEncoder(new StringSink(block_hash)))
      );

      if (block_hash.starts_with("00000") && block_hash < previous_hash) {

        co_return Block(
          BlockHeader{
            block_hash,
            previous_hash,
            merkle_root,
            identity,
            timestamp,
            nonce},
          transactions
        );
      }
      nonce++;
    }
  }
  class Miner {
    std::shared_ptr<concurrencpp::thread_executor> m_executor;

    EventQueuePtr m_event_queue;
    TransactionPoolPtr m_transaction_pool;
    BlockchainPtr m_blockchain;
    PeerManagerPtr m_manager;

   public:
    Miner(
      std::shared_ptr<concurrencpp::thread_executor> executor,
      EventQueuePtr event_queue,
      TransactionPoolPtr transaction_pool,
      BlockchainPtr blockchain,
      PeerManagerPtr manager
    )
        : m_event_queue(event_queue), m_executor(std::move(executor)),
          m_blockchain(std::move(blockchain)),
          m_transaction_pool(std::move(transaction_pool)), m_manager(std::move(manager)) {

      m_executor->enqueue(std::bind_front(&Miner::task, this));
    }

    concurrencpp::null_result task() {

      while (true) {
        spdlog::info("Miner: Waiting for transactions...");

        auto transactions = co_await m_transaction_pool->wait(m_executor, 5);

        spdlog::info("Miner Acquired the following transaction...");
        for (const auto &transaction: transactions) {
          spdlog::info("  Transaction#{}", transaction.contrived_hash());
        }
        auto latest_hash = m_blockchain->last().hash();
        spdlog::info("Mining with {} as previous hash", latest_hash);

        auto block = co_await mining_function(
          {},
          m_executor,
          latest_hash,
          m_manager->id(),
          transactions
        );

        spdlog::info("Miner: Mined Block #{}, broadcasting to other miners", block.contrived_hash());

        m_manager->broadcast_to_peers_of_type_and_forget(
          m_executor,
          {PeerType::Full},
          PeerMessageType::AddBlock,
          block.to_json()
        );

        spdlog::info(
          "Miner: Waiting for Block #{} to be accepted or rejected",
          block.contrived_hash()
        );

        auto opt = co_await m_executor->submit([this, block]() {
          auto block_copy = block;

          auto result = m_event_queue->any_event_of_type<PeerMessage, PeerMessageType>(
            m_executor,
            {PeerMessageType::BlockAccepted, PeerMessageType::BlockRejected}
          );

          auto now = std::chrono::high_resolution_clock::now();
          auto result_status = result.wait_until(now + 2s);

          if (result_status == concurrencpp::result_status::value) {
            return std::make_optional<AnyEventResult<PeerMessage, PeerMessageType>>(
              result.get()
            );
          } else {
            return std::optional<AnyEventResult<PeerMessage, PeerMessageType>>(std::nullopt);
          }
        });

        if (opt) {

          auto [status, message] = opt.value();
          if (status == PeerMessageType::BlockAccepted) {
            spdlog::info(
              "Block #{} was accepted by {}",
              block.contrived_hash(),
              message->sender_identity()
            );

            m_event_queue->enqueue<InternalNotification<Block>>(
              InternalNotificationType::BlockMined,
              block
            );

            co_await m_event_queue->event_of_type(
              InternalNotificationType::MinedBlockValidated
            );
            spdlog::info("Miner: Ran Validator on #{}", block.contrived_hash());
          } else {

            spdlog::info(
              "Block #{} was rejected",
              block.contrived_hash(),
              message->sender_identity()
            );
          }
        } else {

          spdlog::info(
            "Miner: Network timedout when trying to verify Block #{}",
            block.contrived_hash()
          );
          m_event_queue->enqueue<InternalNotification<Block>>(
            InternalNotificationType::BlockMined,
            block
          );
          co_await m_event_queue->event_of_type(
            InternalNotificationType::MinedBlockValidated
          );
          spdlog::info("Miner: Ran Validator on #{}", block.contrived_hash());
        }
      }
    }
  };

}// namespace krapi
