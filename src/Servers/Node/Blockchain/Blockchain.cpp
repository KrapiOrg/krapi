//
// Created by mythi on 12/11/22.
//

#include "Blockchain.h"

#include <utility>

namespace krapi {


    inline Block Blockchain::block_from_slice(const leveldb::Slice &slice) {

        return Block::from_json(nlohmann::json::parse(slice.ToString()));
    }

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
                BlockHeader{
                        block_hash,
                        previous_hash,
                        merkle_root,
                        timestamp,
                        nonce
                },
                {}
        };
    }

    Blockchain::Blockchain(const std::string &path) {

        m_db_options.create_if_missing = true;
        m_write_options.sync = true;

        leveldb::DB *db;
        auto status = leveldb::DB::Open(m_db_options, path, &db);
        if (status.ok()) {

            m_db = std::unique_ptr<leveldb::DB>(db);
            if (length() == 0) {

                spdlog::info("Creating Genesis Block...");
                put(create_genesis_block());
            }
        } else {

            spdlog::error("Failed to open blockchain database");
        }
    }

    int Blockchain::length() const {

        int counter = 0;

        auto itr_begin = std::unique_ptr<leveldb::Iterator>(m_db->NewIterator(m_read_options));
        itr_begin->SeekToFirst();

        for (; itr_begin->Valid(); itr_begin->Next()) {
            counter++;
        }

        return counter;
    }

    std::optional<Block> Blockchain::get(std::string hash) const {

        auto block_str = std::string{};
        auto status = m_db->Get(m_read_options, hash.data(), &block_str);

        if (status.ok() && !status.IsNotFound()) {
            auto block_json = nlohmann::json::parse(block_str);
            return Block::from_json(block_json);
        }
        return {};
    }

    bool Blockchain::remove(std::string hash) {

        return m_db->Delete(m_write_options, hash.data()).IsNotFound();
    }

    std::vector<Block> Blockchain::remove_all_after(std::string hash) {

        auto removed_blocks = std::vector<Block>{};
        auto block_opt = get(std::move(hash));
        if (!block_opt.has_value())
            return {};

        const auto &remove_after = block_opt.value();
        auto blocks = get_blocks();
        for (const auto &block: blocks) {

            if (block.header().timestamp() >= remove_after.header().timestamp()) {

                removed_blocks.push_back(block);
                remove(block.hash());
            }
        }
        return removed_blocks;
    }

    bool Blockchain::contains(std::string hash) const {

        auto block_str = std::string{};
        auto status = m_db->Get(m_read_options, hash.data(), &block_str);

        return !status.IsNotFound();
    }

    bool Blockchain::put(Block block) {

        if (contains(block.hash())) {
            return false;
        }
        return m_db->Put(m_write_options, block.hash(), block.to_json().dump()).ok();
    }

    Block Blockchain::last() const {

        auto blocks = get_blocks();

        return *blocks.rbegin();
    }

    std::set<Block> Blockchain::get_blocks() const {

        std::set<Block> blocks;
        auto itr = std::unique_ptr<leveldb::Iterator>(m_db->NewIterator(m_read_options));
        itr->SeekToFirst();
        for (; itr->Valid(); itr->Next()) {

            blocks.insert(block_from_slice(itr->value()));
        }
        return blocks;
    }

    std::vector<std::string> Blockchain::get_hashes() const {

        auto blocks = get_blocks();
        std::vector<std::string> hashes(blocks.size());
        for (const auto &block: blocks) {
            hashes.push_back(block.hash());
        }
        return hashes;
    }

    std::vector<BlockHeader> Blockchain::get_headers() const {

        auto blocks = get_blocks();
        std::vector<BlockHeader> headers(blocks.size());
        for (const auto &block: blocks) {
            headers.push_back(block.header());
        }
        return headers;
    }

    std::vector<BlockHeader> Blockchain::get_all_after(const BlockHeader &header) const {

        auto blocks = get_blocks();
        auto headers = std::vector<BlockHeader>{};
        for (const auto &block: blocks) {
            if (block.hash().empty())
                continue;
            if (block.header().timestamp() > header.timestamp()) {
                headers.push_back(block.header());
            }
        }
        return headers;
    }

    bool Blockchain::contains_transaction(std::string hash) const {

        auto blocks = get_blocks();
        for (const auto &block: blocks) {
            for (const auto &transaction: block.transactions()) {
                if(transaction.hash() == hash)
                    return true;
            }
        }
        return false;
    }
} // krapi