//
// Created by mythi on 12/11/22.
//

#include "Blockchain.h"

namespace krapi {
    Blockchain::Blockchain(std::list<Block> blocks)
            : m_blocks(std::move(blocks)) {

    }

    Blockchain Blockchain::from_disk(const std::filesystem::path &path) {

        namespace fs = std::filesystem;

        if (!fs::exists(path)) {
            spdlog::error("Could not find {}", path.string());
            exit(1);
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

        std::list<Block> blocks;


        for (const auto &entry: fs::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                if (auto block = Block::from_disk(entry.path())) {
                    blocks.push_back(block.value());
                }
            }

        }

        blocks.sort(
                [](const Block &a, const Block &b) {
                    return a.header().previous_hash() == b.hash();
                }
        );

        return Blockchain(blocks);
    }

    void Blockchain::add(Block block) {

        std::lock_guard l(m_blocks_mutex);
        m_blocks.push_back(block);
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

        for (const auto &block: m_blocks) {
            block.to_disk(path);
        }
    }

    void Blockchain::dump() {

        std::lock_guard l(m_blocks_mutex);
        for (const auto &block: m_blocks) {

            spdlog::info("Blockchain: Block {}", block.to_json().dump(4));
        }
    }

    Block Blockchain::last() {

        std::lock_guard l(m_blocks_mutex);
        return m_blocks.back();
    }

    Blockchain::~Blockchain() {

        spdlog::error("DROPPED BLOCKCHAIN !!!!");
    }
} // krapi