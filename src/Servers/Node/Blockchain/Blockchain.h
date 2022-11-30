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
        Block m_last_block;
        std::mutex m_blocks_mutex;

    public:

        static std::shared_ptr<Blockchain> create();

        void remove(std::string hash);

        void add(Block block);

        void dump();

        Block last();

        std::optional<Block> get_block(std::string);

        std::vector<std::string> get_hashes();

        std::vector<BlockHeader> headers();

        bool contains(std::string hash);

        std::vector<BlockHeader> get_all_after(const BlockHeader &header);

        ~Blockchain();
    };

} // krapi

#endif //NODE_BLOCKCHAIN_BLOCKCHAIN_H
