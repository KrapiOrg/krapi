//
// Created by mythi on 12/10/22.
//

#ifndef KRAPI_MODELS_BLOCKCHAIN_H
#define KRAPI_MODELS_BLOCKCHAIN_H

#include <vector>
#include "Box.h"
#include "Block.h"

namespace krapi {

    class BlockChain {
        std::vector<Box<Block>> m_chain;
    public:
        template<typename ...Args>
        void create_block(Args &&... args) {

            m_chain.push_back(make_box<Block>(std::forward<Args>(args)...));
        }

        [[nodiscard]]
        nlohmann::json to_json() const noexcept;
    };

} // krapi

#endif //KRAPI_MODELS_BLOCKCHAIN_H
