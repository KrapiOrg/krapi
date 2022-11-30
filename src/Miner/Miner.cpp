//
// Created by mythi on 12/11/22.
//

#include "Miner.h"

#include <utility>

#include "spdlog/spdlog.h"

namespace krapi {


    std::optional<std::future<Block>> Miner::mine(std::string previous_hash, std::set<Transaction> batch) {

        if (!was_used(previous_hash)) {
            m_used.insert(previous_hash);

            return m_queue.submit(
                    MiningJob{
                            previous_hash,
                            std::move(batch)
                    }
            );
        }

        return {};
    }

    Miner::Miner() = default;

    bool Miner::was_used(std::string hash) {

        return m_used.contains(hash);
    }
} // krapi