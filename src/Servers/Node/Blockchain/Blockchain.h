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
        std::unordered_map<std::string, Block> m_blocks;
        std::mutex m_blocks_mutex;

    public:
        explicit Blockchain(std::unordered_map<std::string,Block> blocks);

        static Blockchain from_disk(const std::filesystem::path &path);

        void add(Block block);

        void save_to_disk(const std::filesystem::path &path);

        std::list<Block> get_after(std::string hash);

        void dump();

        Block last();

        ~Blockchain();

        bool append_to_end(std::list<Block> blocks);
    };

} // krapi

#endif //NODE_BLOCKCHAIN_BLOCKCHAIN_H
