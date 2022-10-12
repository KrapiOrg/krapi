//
// Created by mythi on 12/10/22.
//

#include "Block.h"

#include <utility>

namespace krapi {

    nlohmann::json Block::to_json() const noexcept {

        return {
                {"previous_hash", m_previous_hash},
                {"nonce",         m_nonce},
                {"difficulty",    m_difficulty},
                {"index",         m_index}
        };
    }

    Block::Block(
            std::string previous_hash,
            int nonce,
            int difficulty,
            int index
    ) noexcept:
            m_previous_hash(std::move(previous_hash)),
            m_nonce(nonce),
            m_difficulty(difficulty),
            m_index(index) {}

    std::string_view Block::get_previous_hash() const noexcept {

        return m_previous_hash;
    }

    int Block::get_nonce() const noexcept {

        return m_nonce;
    }

    int Block::get_difficulty() const noexcept {

        return m_difficulty;
    }

    int Block::get_index() const noexcept {

        return m_index;
    }
} // krapi