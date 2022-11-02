//
// Created by mythi on 02/11/22.
//

#ifndef NODE_MODELS_BLOCK_H
#define NODE_MODELS_BLOCK_H

#include <string>
#include <vector>
#include <utility>

#include "nlohmann/json.hpp"

#include "Transaction.h"
#include "BlockHeader.h"
#include "sha.h"
#include "filters.h"
#include "hex.h"

namespace krapi {

    class Block {
    public:
        explicit Block(
                BlockHeader header,
                std::vector<Transaction> transactions
        ) :
                m_header(std::move(header)),
                m_transactions(std::move(transactions)) {
        }

        static Block from_json(const nlohmann::json &json) {

            std::vector<Transaction> transactions;
            for (const auto &tx: json["transactions"]) {
                transactions.push_back(Transaction::from_json(tx));
            }

            return Block{
                    BlockHeader::from_json(json["header"]),
                    transactions
            };
        }

        static std::optional<Block> from_disk(const std::filesystem::path &path) {

            if (!path.has_filename()
                && !path.has_extension()
                && !path.empty()
                && !path.extension().string().ends_with(".json")) {

                std::fstream file(path);

                if (file.is_open() && file.good()) {

                    auto json = nlohmann::json::parse(file);
                    return Block::from_json(json);
                }

            }
            return {};
        }

        [[nodiscard]]
        std::string hashcode() const {

            using namespace CryptoPP;
            auto str = to_json().dump();
            auto digest = std::string();
            auto hash = SHA256();
            StringSource s1(str, true,
                            new HashFilter(hash, new HashFilter(hash, new HexEncoder(new StringSink(digest)))));

            return digest;
        }

        void to_disk(const std::filesystem::path &path) const {
            namespace fs = std::filesystem;

            std::ofstream file(path / (hashcode() + ".json"));
            if(file.is_open() && file.good())
                file << to_json().dump(4);
        }

        [[nodiscard]]
        nlohmann::json to_json() const {

            auto transactions = nlohmann::json::array();
            for (const auto &tx: m_transactions) {
                transactions.push_back(tx.to_json());
            }

            return {
                    {"header",       m_header.to_json()},
                    {"transactions", transactions}
            };
        }

        [[nodiscard]] const BlockHeader &header() const {

            return m_header;
        }

        [[nodiscard]] const std::vector<Transaction> &transactions() const {

            return m_transactions;
        }

        void save_to_disk(std::string_view path) const {

        }

    private:
        BlockHeader m_header;
        std::vector<Transaction> m_transactions;

    };

} // krapi

#endif //NODE_MODELS_BLOCK_H
