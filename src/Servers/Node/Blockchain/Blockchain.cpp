//
// Created by mythi on 12/11/22.
//

#include "Blockchain.h"

namespace krapi {
    Blockchain::Blockchain(std::unordered_map<std::string, Block> blocks) : m_blocks(std::move(blocks)) {


    }

    Blockchain Blockchain::from_disk(const std::filesystem::path &path) {

        namespace fs = std::filesystem;

        if (!fs::exists(path)) {
            spdlog::error("Could not find {}, creating...", path.string());
            fs::create_directory(path);
        }

        if (!fs::exists(path / "genesis.json")) {

            spdlog::info("Blockchain: Creating genesis block");
            auto previous_hash = std::string{"0"};
            auto merkle_root = std::string{"0"};
            auto timestamp = static_cast<uint64_t>(1668542625);
            auto nonce = (uint64_t) 0;
            auto block_hash = std::string{};
            auto sha_256 = CryptoPP::SHA256{};

            StringSource s2(
                    fmt::format("{}{}{}{}", previous_hash, merkle_root, timestamp, nonce),
                    true,
                    new HashFilter(sha_256, new HexEncoder(new StringSink(block_hash)))
            );
            auto genesis = Block{
                    BlockHeader{
                            block_hash,
                            previous_hash,
                            merkle_root,
                            timestamp,
                            nonce
                    },
                    {}
            };
            genesis.to_disk(path);
        }

        std::unordered_map<std::string, Block> blocks;

        for (const auto &entry: fs::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                if (auto block = Block::from_disk(entry.path())) {

                    blocks.insert({block.value().hash(), block.value()});
                }
            }

        }

        return Blockchain(blocks);
    }

    void Blockchain::add(Block block) {

        std::lock_guard l(m_blocks_mutex);
        m_blocks.insert({block.hash(), block});
    }

    void Blockchain::save_to_disk(const std::filesystem::path &path) {

        namespace fs = std::filesystem;

        if (!fs::exists(path)) {
            spdlog::error("Could not find {}", path.string());
            exit(1);
        }
        if (!fs::is_directory(path)) {
            spdlog::error("{} is not a directory", path.string());
            exit(1);
        }

        std::lock_guard l(m_blocks_mutex);

        for (const auto &[hash, block]: m_blocks) {
            block.to_disk(path);
        }
    }

    void Blockchain::dump() {

        std::lock_guard l(m_blocks_mutex);
        for (const auto &[hash, block]: m_blocks) {

            spdlog::info("== Block: {}", hash.substr(0, 10));
        }
    }

    Block Blockchain::last() {

        std::lock_guard l(m_blocks_mutex);

        return std::max_element(
                m_blocks.begin(), m_blocks.end(),
                [](const std::pair<std::string, Block> &a, const std::pair<std::string, Block> &b) {
                    return a.second.header().timestamp() < b.second.header().timestamp();
                }
        )->second;
    }

    Blockchain::~Blockchain() {

        spdlog::error("DROPPED BLOCKCHAIN !!!!");
    }

    std::list<Block> Blockchain::get_after(std::string hash) {

        std::lock_guard l(m_blocks_mutex);
        auto blocks = std::list<Block>{};
        auto blk = m_blocks[hash];
        for (const auto &[hash, block]: m_blocks) {
            if (block.header().timestamp() > blk.header().timestamp()) {
                blocks.push_back(block);
            }
        }

        return blocks;
    }

    bool Blockchain::append_to_end(std::list<Block> blocks) {

        std::lock_guard l(m_blocks_mutex);

        auto last_block = std::max_element(
                m_blocks.begin(), m_blocks.end(),
                [](const std::pair<std::string, Block> &a, const std::pair<std::string, Block> &b) {
                    return a.second.header().timestamp() < b.second.header().timestamp();
                }
        )->second;

        auto block_with_last_block_as_prev = std::find_if(
                blocks.begin(),
                blocks.end(),
                [&](const Block &block) {
                    return block.header().previous_hash() == last_block.hash();
                }
        );

        if (block_with_last_block_as_prev == blocks.end())
            return false;

        for (const auto &block: blocks) {
            m_blocks.insert({block.hash(), block});
        }

        return true;
    }
} // krapi