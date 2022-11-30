//
// Created by mythi on 12/11/22.
//

#include "Blockchain.h"

namespace krapi {

    std::shared_ptr<Blockchain> Blockchain::create() {

        return std::make_shared<Blockchain>();
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

        std::lock_guard l(m_blocks_mutex);
        return m_last_block;
    }

    Blockchain::~Blockchain() {

        spdlog::error("DROPPED BLOCKCHAIN !!!!");
    }

    bool Blockchain::contains(std::string hash) {

        std::lock_guard l(m_blocks_mutex);
        return m_blocks.contains(hash);
    }

    std::optional<Block> Blockchain::get_block(std::string hash) {

        std::lock_guard l(m_blocks_mutex);
        if (m_blocks.contains(hash)) {
            return m_blocks[hash];
        }
        return {};
    }

    std::vector<std::string> Blockchain::get_hashes() {

        std::lock_guard l(m_blocks_mutex);
        auto ans = std::vector<std::string>{};

        for (const auto &[hash, block]: m_blocks) {
            ans.push_back(hash);
        }
        return ans;
    }

    std::vector<BlockHeader> Blockchain::headers() {

        std::lock_guard l(m_blocks_mutex);
        auto ans = std::vector<BlockHeader>{};

        for (const auto &[hash, block]: m_blocks) {
            ans.push_back(block.header());
        }
        return ans;
    }

    std::vector<BlockHeader> Blockchain::get_all_after(const BlockHeader &header) {

        std::lock_guard l(m_blocks_mutex);
        std::vector<BlockHeader> ans;

        for (const auto &[hash, block]: m_blocks) {
            if (block.header().timestamp() > header.timestamp()) {

                ans.push_back(block.header());
            }
        }

        return ans;
    }

    void Blockchain::remove(std::string hash) {

        std::lock_guard l(m_blocks_mutex);

        if (m_blocks.contains(hash)) {

            m_blocks.erase(hash);
        }
    }
} // krapi