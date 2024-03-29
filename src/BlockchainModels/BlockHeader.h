#pragma once
#include "cryptopp/config_int.h"
#include <string>

#include "filters.h"
#include "hex.h"
#include "nlohmann/json.hpp"

using namespace CryptoPP;

namespace krapi {
  class BlockHeader {
    friend class Block;

   public:
    explicit BlockHeader(
      std::string hash = {},
      std::string previous_hash = {},
      std::string merkle_root = {},
      std::string mined_by = {},
      uint64_t timestamp = {},
      uint64_t nonce = {}
    )
        : m_hash(std::move(hash)),
          m_previous_hash(std::move(previous_hash)),
          m_merkle_root(std::move(merkle_root)),
          m_mined_by(std::move(mined_by)),
          m_timestamp(timestamp),
          m_nonce(nonce) {

      StringSource s(
        m_hash,
        true,
        new HexDecoder(new ArraySink(m_hash_bytes.data(), 32))
      );
    }

    static BlockHeader from_json(nlohmann::json json) {

      return BlockHeader{
        json["hash"].get<std::string>(),
        json["previous_hash"].get<std::string>(),
        json["merkle_root"].get<std::string>(),
        json["mined_by"].get<std::string>(),
        json["timestamp"].get<uint64_t>(),
        json["nonce"].get<uint64_t>()};
    }

    [[nodiscard]] nlohmann::json to_json() const {

      return {
        {"hash", m_hash},
        {"previous_hash", m_previous_hash},
        {"merkle_root", m_merkle_root},
        {"mined_by", m_mined_by},
        {"timestamp", m_timestamp},
        {"nonce", m_nonce}};
    }

    [[nodiscard]] std::string hash() const {
      return m_hash;
    }

    [[nodiscard]] std::string previous_hash() const {
      return m_previous_hash;
    }

    [[nodiscard]] std::string merkle_root() const {
      return m_merkle_root;
    }

    [[nodiscard]] uint64_t timestamp() const {
      return m_timestamp;
    }

    [[nodiscard]] uint64_t nonce() const {
      return m_nonce;
    }

    [[nodiscard]] std::string contrived_hash() const {

      return m_hash.substr(0, 10);
    }

    [[nodiscard]] std::string mined_by() const {

      return m_mined_by;
    }

   private:
    std::string m_hash;
    std::string m_previous_hash;
    std::string m_merkle_root;
    std::string m_mined_by;
    uint64_t m_timestamp;
    uint64_t m_nonce;

    std::array<CryptoPP::byte, 32> m_hash_bytes{};
  };
}// namespace krapi