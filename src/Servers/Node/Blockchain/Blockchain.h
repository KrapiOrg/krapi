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

        explicit Blockchain(std::list<Block> blocks)
                : m_blocks(std::move(blocks)) {

        }

    public:


        static Blockchain from_disk(const std::filesystem::path &path) {

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

            return Blockchain{blocks};
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

            for (const auto &block: m_blocks) {
                block.to_disk(path);
            }
        }
    };

} // krapi

#endif //NODE_BLOCKCHAIN_H
