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
        m_last_block = block;
    }

    void Blockchain::dump() {

        for (const auto &[hash, block]: m_blocks) {

            spdlog::info("== Block: {}", hash.substr(0, 10));
        }
    }

    Block Blockchain::last() {

        return m_last_block;
    }

    Blockchain::~Blockchain() {

        spdlog::error("DROPPED BLOCKCHAIN !!!!");
    }

    bool Blockchain::contains(std::string hash) {

        return m_blocks.contains(hash);
    }

    Block Blockchain::get_block(std::string hash) {

        return m_blocks.find(hash)->second;
    }

    std::vector<std::string> Blockchain::get_hashes() {

        auto ans = std::vector<std::string>{};

        for (const auto &[hash, block]: m_blocks) {
            ans.push_back(hash);
        }
        return ans;
    }

    std::vector<BlockHeader> Blockchain::headers() {

        auto ans = std::vector<BlockHeader>{};

        for (const auto &[hash, block]: m_blocks) {
            ans.push_back(block.header());
        }
        return ans;
    }

    std::vector<BlockHeader> Blockchain::get_all_after(const BlockHeader &header) {

        std::vector<BlockHeader> ans;

        for (const auto &[hash, block]: m_blocks) {
            if (block.header().timestamp() > header.timestamp()) {

                ans.push_back(block.header());
            }
        }

        return ans;
    }
} // krapi