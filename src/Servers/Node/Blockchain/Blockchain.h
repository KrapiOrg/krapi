#pragma once

#include <list>
#include <vector>
#include <filesystem>
#include <fstream>

#include "leveldb/db.h"
#include "Block.h"
#include "spdlog/spdlog.h"
#include "sha.h"

namespace krapi {

    class Blockchain {

        std::unique_ptr<leveldb::DB> m_db;
        leveldb::Options m_db_options;
        leveldb::ReadOptions m_read_options;
        leveldb::WriteOptions m_write_options;

        [[nodiscard]]
        static Block block_from_slice(const leveldb::Slice &slice) ;

        static Block create_genesis_block();

    public:

        explicit Blockchain(const std::string &path);

        [[nodiscard]]
        int length() const;

        [[nodiscard]]
        std::optional<Block> get(std::string hash) const;

        bool remove(std::string hash);

        [[nodiscard]]
        bool contains(std::string hash) const;

        bool put(Block block);

        [[nodiscard]]
        Block last() const;

        [[nodiscard]]
        std::set<Block> get_blocks() const;

        [[nodiscard]]
        std::vector<std::string> get_hashes() const;

        [[nodiscard]]
        std::vector<BlockHeader> get_headers() const;

        [[nodiscard]]
        std::vector<BlockHeader> get_all_after(const BlockHeader &header) const;

        std::vector<Block> remove_all_after(std::string hash);

        bool contains_transaction(std::string transaction_hash) const;
    };

} // krapi
