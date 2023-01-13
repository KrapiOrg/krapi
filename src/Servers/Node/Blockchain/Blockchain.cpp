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
    auto mined_by = std::string{"0"};
    auto timestamp = static_cast<uint64_t>(1668542625);
    auto nonce = (uint64_t) 0;
    auto block_hash = std::string{};

    StringSource s2(
      fmt::format("{}{}{}{}", previous_hash, merkle_root, timestamp, nonce),
      true,
      new HashFilter(sha_256, new HexEncoder(new StringSink(block_hash)))
    );

    return Block{
      BlockHeader{
        block_hash,
        previous_hash,
        merkle_root,
        mined_by,
        timestamp,
        nonce},
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

    for (auto block: data()) {

      if (block.timestamp() >= remove_after.timestamp()) {

        removed_blocks.push_back(block);
        remove(block);
      }
    }
    return removed_blocks;
  }

  std::vector<std::string> Blockchain::get_hashes() const {

    std::vector<std::string> hashes;
    for (auto block: data()) {
      hashes.push_back(block.hash());
    }
    return hashes;
  }

  std::vector<BlockHeader> Blockchain::get_headers() const {

    std::vector<BlockHeader> headers;
    for (auto block: data()) {
      headers.push_back(block.header());
    }
    return headers;
  }

  std::vector<BlockHeader> Blockchain::get_all_after(const BlockHeader &header
  ) const {

    std::vector<BlockHeader> headers;
    for (auto block: data()) {
      if (block.hash().empty()) continue;
      if (block.header().timestamp() > header.timestamp()) {
        headers.push_back(block.header());
      }
    }
    return headers;
  }
}// namespace krapi