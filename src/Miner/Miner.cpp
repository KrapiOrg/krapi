//
// Created by mythi on 12/11/22.
//

#include "Miner.h"

#include <utility>

#include "spdlog/spdlog.h"

namespace krapi {


    std::optional<std::shared_future<std::pair<bool, Block>>> Miner::mine(
            std::string previous_hash,
            std::set<Transaction> batch
    ) {

        bool added;

        {
            std::lock_guard l(m_used_mutex);
            added = m_used.insert(previous_hash).second;
        }

        if (added) {
            return m_queue.submit(
                    MiningJob{
                            previous_hash,
                            std::move(batch),
                            m_cancellation_token
                    }
            );
        }

        return {};
    }

    Miner::Miner() :
            m_cancellation_token(std::make_shared<std::atomic<bool>>(false)) {

    };

    bool Miner::was_used(std::string hash) {

        std::lock_guard l(m_used_mutex);
        return m_used.contains(hash);
    }

    void Miner::cancel() {

        m_cancellation_token->exchange(true);
        m_queue.wait_for_tasks();
        m_cancellation_token->exchange(false);
    }

    bool Miner::remove_hash(std::string hash) {

        std::lock_guard l(m_used_mutex);
        return m_used.erase(hash) > 0;
    }
} // krapi