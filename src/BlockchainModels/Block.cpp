#include "Block.h"

namespace krapi {
  Block::Block(BlockHeader header, Transactions transactions)
      : m_header(std::move(header)),
        m_transactions(std::move(transactions)) {
  }

  Block Block::from_json(const nlohmann::json &json) {

    Transactions transactions;
    for (const auto &tx: json["transactions"]) {
      transactions.push_back(Transaction::from_json(tx));
    }

    return Block{BlockHeader::from_json(json["header"]), transactions};
  }

  std::string Block::hash() const {
    return m_header.m_hash;
  }

  std::array<CryptoPP::byte, 32> Block::hash_bytes() const {

    return m_header.m_hash_bytes;
  }

  nlohmann::json Block::to_json() const {

    auto transactions = nlohmann::json::array();
    for (const auto &tx: m_transactions) {
      transactions.push_back(tx.to_json());
    }

    return {{"header", m_header.to_json()}, {"transactions", transactions}};
  }

  BlockHeader Block::header() const {
    return m_header;
  }

  Transactions Block::transactions() const {
    return m_transactions;
  }

  bool Block::operator==(const Block &other) const {

    return m_header.m_hash == other.m_header.m_hash;
  }

  bool Block::operator<(const Block &other) const {

    return m_header.m_timestamp < other.m_header.m_timestamp;
  }

  std::string Block::contrived_hash() const {

    return m_header.m_hash.substr(0, 10);
  }
  uint64_t Block::timestamp() const {
    return m_header.m_timestamp;
  }
  std::string Block::previous_hash() const {
    return m_header.m_previous_hash;
  }
}// namespace krapi