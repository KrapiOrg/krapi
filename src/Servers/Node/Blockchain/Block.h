//
// Created by mythi on 12/11/22.
//

#ifndef NODE_BLOCKCHAIN_BLOCK_H
#define NODE_BLOCKCHAIN_BLOCK_H

#include <fstream>
#include <string>
#include <unordered_set>

#include "nlohmann/json.hpp"

#include "Transaction.h"
#include "BlockHeader.h"
#include "spdlog/spdlog.h"

namespace krapi {

    class Block {

    public:
        explicit Block(
                BlockHeader header = BlockHeader{},
                std::unordered_set<Transaction> transactions = {}
        );

        static Block from_json(const nlohmann::json &json);

        static std::optional<Block> from_disk(const std::filesystem::path &path);

        [[nodiscard]]
        std::string hash() const;

        [[nodiscard]]
        std::array<CryptoPP::byte, 32> hash_bytes() const;

        void to_disk(const std::filesystem::path &path) const;

        [[nodiscard]]
        nlohmann::json to_json() const;

        [[nodiscard]]
        BlockHeader header() const;

        [[nodiscard]]
        std::unordered_set<Transaction> transactions() const;

    private:
        BlockHeader m_header;
        std::unordered_set<Transaction> m_transactions;
    };
} // krapi
namespace std {
    template<>
    struct hash<krapi::Block> {
        size_t operator()(const krapi::Block &block) const {

            auto byte_hash = block.hash_bytes();

            size_t x = 0;
            x |= static_cast<unsigned long>(byte_hash[0]) << 0;
            x |= static_cast<unsigned long>(byte_hash[1]) << 8;
            x |= static_cast<unsigned long>(byte_hash[2]) << 16;
            x |= static_cast<unsigned long>(byte_hash[3]) << 24;
            x |= static_cast<unsigned long>(byte_hash[4]) << 32;
            x |= static_cast<unsigned long>(byte_hash[5]) << 40;
            x |= static_cast<unsigned long>(byte_hash[6]) << 48;
            x |= static_cast<unsigned long>(byte_hash[7]) << 56;

            return x;
        }
    };
}
#endif //NODE_BLOCKCHAIN_BLOCK_H
