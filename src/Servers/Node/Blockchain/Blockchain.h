//
// Created by mythi on 02/11/22.
//

#ifndef NODE_BLOCKCHAIN_H
#define NODE_BLOCKCHAIN_H

#include <list>
#include <filesystem>
#include <fstream>


#include "Block.h"
#include "spdlog/spdlog.h"

namespace krapi {

    class Blockchain {
        std::list<Block> m_blocks;
        std::mutex m_blocks_mutex;

    public:
        explicit Blockchain(std::list<Block> blocks)
                : m_blocks(std::move(blocks)) {

        }

        static std::shared_ptr<Blockchain> from_disk(const std::filesystem::path &path) {

            namespace fs = std::filesystem;

            std::list<Block> blocks;

            if (!fs::exists(path)) {
                spdlog::error("Could not find {}", path.string());
                exit(1);
            }

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


            auto blk = std::make_shared<Blockchain>(blocks);
            blk->dump();
            return blk;
        }

        void add(const Block &block) {

            std::lock_guard l(m_blocks_mutex);
            m_blocks.push_back(block);
        }

        void save_to_disk(const std::filesystem::path &path) {

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

        void dump() {

            std::lock_guard l(m_blocks_mutex);
            for (const auto &block: m_blocks) {

                spdlog::info("Blockchain: Block {}", block.to_json().dump());
            }
        }

        [[nodiscard]]
        const Block &last() {

            std::lock_guard l(m_blocks_mutex);
            return m_blocks.back();
        }

        ~Blockchain() {

            spdlog::error("DROPPED BLOCKCHAIN !!!!");
        }
    };

} // krapi

#endif //NODE_BLOCKCHAIN_H
