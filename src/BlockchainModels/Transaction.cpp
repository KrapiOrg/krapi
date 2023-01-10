//
// Created by mythi on 16/11/22.
//
#include "Transaction.h"

namespace krapi {

  Transaction::Transaction(
    TransactionType type,
    TransactionStatus status,
    std::string hash,
    uint64_t timestamp,
    std::string from,
    std::string to
  )
      : m_type(type), m_status(status), m_hash(std::move(hash)),
        m_timestamp(timestamp), m_from(std::move(from)), m_to(std::move(to)) {

    CryptoPP::StringSource s(
      m_hash,
      true,
      new CryptoPP::HexDecoder(new CryptoPP::ArraySink(m_byte_hash.data(), 32))
    );
  }

  Transaction Transaction::from_json(const nlohmann::json &json) {


    return Transaction{
      json["type"].get<TransactionType>(),
      json["status"].get<TransactionStatus>(),
      json["hash"].get<std::string>(),
      json["timestamp"].get<uint64_t>(),
      json["from"].get<std::string>(),
      json["to"].get<std::string>()};
  }

  nlohmann::json Transaction::to_json() const {

    return {
      {"type", nlohmann::json(m_type)},
      {"status", m_status},
      {"hash", m_hash},
      {"timestamp", m_timestamp},
      {"from", m_from},
      {"to", m_to}};
  }

  bool Transaction::operator==(const Transaction &) const = default;

  TransactionType Transaction::type() const { return m_type; }

  std::string Transaction::from() const { return m_from; }

  std::string Transaction::to() const { return m_to; }

  std::string Transaction::hash() const { return m_hash; }

  std::array<CryptoPP::byte, 32> Transaction::byte_hash() const {

    return m_byte_hash;
  }

  TransactionStatus Transaction::status() const { return m_status; }

  uint64_t Transaction::timestamp() const { return m_timestamp; }

  bool Transaction::operator<(const Transaction &other) const {

    return m_timestamp < other.m_timestamp;
  }

  std::string Transaction::contrived_hash() const {

    return m_hash.substr(0, 10);
  }
}// namespace krapi