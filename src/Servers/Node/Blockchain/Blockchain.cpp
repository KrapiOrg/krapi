//
// Created by mythi on 12/11/22.
//

#include "Blockchain.h"
#include "BlockHeader.h"
#include "spdlog/spdlog.h"

#include <filesystem>
#include <utility>

namespace krapi {

  Block Blockchain::create_genesis_block() {

    CryptoPP::SHA256 sha_256;
    spdlog::info("Blockchain: Creating genesis block");
    auto previous_hash = std::string{"0"};
    auto merkle_root = std::string{"0"};
    auto timestamp = static_cast<uint64_t>(1668542625);
    auto nonce = (uint64_t) 0;
    auto block_hash = std::string{};

    StringSource s2(
      fmt::format("{}{}{}{}", previous_hash, merkle_root, timestamp, nonce),
      true,
      new HashFilter(sha_256, new HexEncoder(new StringSink(block_hash)))
    );

    return Block{
      BlockHeader{block_hash, previous_hash, merkle_root, timestamp, nonce},
      {}};
  }

  Blockchain::Blockchain(const std::string &path) {
    if (!initialize(path)) {
      spdlog::error("Failed to open blocks database");
      exit(1);
    }
    if (size() == 0) {
      auto block = create_genesis_block();
      spdlog::info("Creating gensis block...");
      put(block);
    }
  }

  std::vector<Block> Blockchain::remove_all_after(BlockHeader header) {

    auto removed_blocks = std::vector<Block>{};
    auto block_opt = get(header.hash());
    if (!block_opt.has_value()) {
      spdlog::warn(
        "Tried to remove after {} which does not exist",
        header.hash()
      );
      return {};
    }

    const auto &remove_after = block_opt.value();
    auto blocks = get_all();
    for (const auto &block: blocks) {

      if (block.timestamp() >= remove_after.timestamp()) {

        removed_blocks.push_back(block);
        remove(block);
      }
    }
    return removed_blocks;
  }

  std::vector<std::string> Blockchain::get_hashes() const {

    auto blocks = get_all();
    std::vector<std::string> hashes(blocks.size());
    for (const auto &block: blocks) { hashes.push_back(block.hash()); }
    return hashes;
  }

  std::vector<BlockHeader> Blockchain::get_headers() const {

    auto blocks = get_all();
    std::vector<BlockHeader> headers(blocks.size());
    for (const auto &block: blocks) { headers.push_back(block.header()); }
    return headers;
  }

  std::vector<BlockHeader> Blockchain::get_all_after(const BlockHeader &header
  ) const {

    auto blocks = get_all();
    auto headers = std::vector<BlockHeader>{};
    for (const auto &block: blocks) {
      if (block.hash().empty()) continue;
      if (block.header().timestamp() > header.timestamp()) {
        headers.push_back(block.header());
      }
    }
    return headers;
  }

  bool Blockchain::contains_transaction(std::string hash) const {

    auto blocks = get_all();
    for (const auto &block: blocks) {
      for (const auto &transaction: block.transactions()) {
        if (transaction.hash() == hash) return true;
      }
    }
    return false;
  }
}// namespace krapi