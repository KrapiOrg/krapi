#pragma once

#include "Block.h"
#include "Blockchain.h"
#include "Transaction.h"
#include "ValidationState.h"

#include <string>
namespace krapi {

  struct Validator {


    static ValidationState validate_header(BlockHeader last_block_header, Block block) {

      auto check_timestamp = [&]() {
        return block.header().timestamp() > last_block_header.timestamp();
      };
      auto check_proof_of_work = [&]() {
        return block.header().hash().starts_with("00000");
      };
      auto check_previous_hash = [&]() {
        return last_block_header.hash() == block.header().previous_hash();
      };

      if (!check_timestamp()) {
        return ValidationState::WrongTimestamp;
      }
      if (!check_proof_of_work()) {
        return ValidationState::IncompeleteProofOfWork;
      }
      if (!check_previous_hash()) {
        return ValidationState::WrongPreviousHash;
      }

      return ValidationState::Success;
    }

    static ValidationState validate_transaction(BlockchainPtr blockchain, Transaction transaction) {
      for (Block block: blockchain->data()) {

        if (block.contains_transaction(transaction)) {
          return ValidationState::TransactionAlreadySpent;
        }
      }
      return ValidationState::Success;
    }

    static ValidationState validate_block(BlockchainPtr blockchain, Block block) {

      auto header_state = validate_header(blockchain->last().header(), block);

      if (header_state != ValidationState::Success) {
        return header_state;
      }

      for (auto transaction: block.transactions()) {

        auto transaction_state = validate_transaction(blockchain, transaction);

        if (transaction_state != ValidationState::Success) {
          return transaction_state;
        }
      }

      return ValidationState::Success;
    }
  };
}// namespace krapi