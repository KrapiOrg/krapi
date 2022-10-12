//
// Created by mythi on 12/10/22.
//

#ifndef NODE_BLOCK_H
#define NODE_BLOCK_H

#include <string>
#include "nlohmann/json.hpp"

namespace krapi {

    class Block {
        std::string m_previous_hash;
        int m_nonce;
        int m_difficulty;
        int m_index;
    public:

        explicit Block(
                std::string previous_hash,
                int nonce,
                int difficulty,
                int index
        ) noexcept;

        [[nodiscard]]
        std::string_view get_previous_hash() const noexcept;

        [[nodiscard]]
        int get_nonce() const noexcept;

        [[nodiscard]]
        int get_difficulty() const noexcept;

        [[nodiscard]]
        int get_index() const noexcept;

        [[nodiscard]]
        nlohmann::json to_json() const noexcept;
    };

} // krapi

#endif //NODE_BLOCK_H
