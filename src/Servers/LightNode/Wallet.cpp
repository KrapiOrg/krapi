//
// Created by mythi on 18/11/22.
//

#include "Helpers.h"
#include "fmt/format.h"
#include "spdlog/spdlog.h"
#include <chrono>

#include "Wallet.h"

using namespace CryptoPP;
using namespace std::chrono;

namespace krapi {

  Transaction
  Wallet::create_transaction(std::string my_id, std::string receiver_id) {
    CryptoPP::SHA256 sha_256;
    auto timestamp = get_krapi_timestamp();

    std::string tx_hash;
    StringSource s(
      fmt::format("{}{}{}{}", "send_tx", timestamp, my_id, receiver_id),
      true,
      new HashFilter(sha_256, new HexEncoder(new StringSink(tx_hash)))
    );
    auto tx = Transaction{
      TransactionType::Send,
      TransactionStatus::Pending,
      tx_hash,
      timestamp,
      my_id,
      receiver_id};

    put(tx);

    return tx;
  }
}// namespace krapi