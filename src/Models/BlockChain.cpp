//
// Created by mythi on 12/10/22.
//

#include "BlockChain.h"

namespace krapi {
    nlohmann::json BlockChain::to_json() const noexcept {

        auto arr = nlohmann::json::array();
        for (const auto &blk: m_chain) {
            arr.push_back(blk->to_json());
        }
        return arr;
    }
} // krapi