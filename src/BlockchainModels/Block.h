//
// Created by mythi on 12/11/22.
//

#ifndef NODE_BLOCKCHAIN_BLOCK_H
#define NODE_BLOCKCHAIN_BLOCK_H

#include <fstream>
#include <string>
#include <set>

#include "nlohmann/json.hpp"

#include "Transaction.h"
#include "BlockHeader.h"

namespace krapi {

    class Block {

    public:
        explicit Block(
                BlockHeader header = BlockHeader{},
                std::set<Transaction> transactions = {}
        );

        static Block from_json(const nlohmann::json &json);

        static std::optional<Block> from_disk(const std::filesystem::path &path);

        [[nodiscard]]
        std::string hash() const;

        [[nodiscard]]
        std::array<CryptoPP::byte, 32> hash_bytes() const;

        void to_disk(const std::filesystem::path &path) const;

        static void remove_from_disk(const std::filesystem::path &path, std::string hash);

        [[nodiscard]]
        nlohmann::json to_json() const;

        [[nodiscard]]
        BlockHeader header() const;

        [[nodiscard]]
        std::set<Transaction> transactions() const;

        bool operator==(const Block &) const;

    private:
        BlockHeader m_header;
        std::set<Transaction> m_transactions;
    };
} // krapi
namespace std {
    template<>
    struct hash<krapi::Block> {
        size_t operator()(const krapi::Block &block) const {

            auto byte_hash = block.hash_bytes();

            size_t x = 0;
            x |= static_cast<unsigned long>(byte_hash[10]) << 0;
            x |= static_cast<unsigned long>(byte_hash[11]) << 8;
            x |= static_cast<unsigned long>(byte_hash[12]) << 16;
            x |= static_cast<unsigned long>(byte_hash[13]) << 24;
            x |= static_cast<unsigned long>(byte_hash[14]) << 32;
            x |= static_cast<unsigned long>(byte_hash[15]) << 40;
            x |= static_cast<unsigned long>(byte_hash[16]) << 48;
            x |= static_cast<unsigned long>(byte_hash[17]) << 56;

            return x;
        }
    };
}
#endif //NODE_BLOCKCHAIN_BLOCK_H
