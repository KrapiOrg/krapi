#pragma once

#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <type_traits>
#include <vector>

#include "Block.h"
#include "BlockHeader.h"
#include "DBInterface.h"
#include "leveldb/db.h"
#include "nlohmann/json_fwd.hpp"
#include "sha.h"
#include "spdlog/spdlog.h"

namespace krapi {


  class Blockchain : public DBInternface<Block> {

    std::unique_ptr<leveldb::DB> m_db;
    leveldb::Options m_db_options;
    leveldb::ReadOptions m_read_options;
    leveldb::WriteOptions m_write_options;

    static Block create_genesis_block();

    explicit Blockchain(const std::string &path);

   public:
    [[nodiscard]] static inline std::shared_ptr<Blockchain>
    from_path(const std::string &path) {

      return std::shared_ptr<Blockchain>(new Blockchain(path));
    }

    [[nodiscard]] std::vector<std::string> get_hashes() const;

    [[nodiscard]] std::vector<BlockHeader> get_headers() const;

    [[nodiscard]] std::vector<BlockHeader>
    get_all_after(const BlockHeader &header) const;

    std::vector<Block> remove_all_after(BlockHeader);

    bool contains_transaction(std::string transaction_hash) const;
  };
  using BlockchainPtr = std::shared_ptr<Blockchain>;
}// namespace krapi
