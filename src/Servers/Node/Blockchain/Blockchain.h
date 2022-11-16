#ifndef NODE_BLOCKCHAIN_BLOCKCHAIN_H
#define NODE_BLOCKCHAIN_BLOCKCHAIN_H

#include <list>
#include <filesystem>
#include <fstream>


#include "Block.h"
#include "spdlog/spdlog.h"
#include "sha.h"

namespace krapi {

    class Blockchain {
        std::list<Block> m_blocks;
        std::mutex m_blocks_mutex;

    public:
        explicit Blockchain(std::list<Block> blocks);

        static Blockchain from_disk(const std::filesystem::path &path);

        void add(Block block);

        void save_to_disk(const std::filesystem::path &path);

        void dump();

        Block last();

        ~Blockchain();
    };

} // krapi

#endif //NODE_BLOCKCHAIN_BLOCKCHAIN_H
