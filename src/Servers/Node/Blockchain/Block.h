//
// Created by mythi on 02/11/22.
//

#ifndef NODE_MODELS_BLOCK_H
#define NODE_MODELS_BLOCK_H

#include <fstream>
#include <string>
#include <unordered_set>
#include <utility>

#include "nlohmann/json.hpp"

#include "Transaction.h"
#include "BlockHeader.h"
#include "sha.h"
#include "filters.h"
#include "hex.h"
#include "spdlog/spdlog.h"

namespace krapi {

    class Block {

    public:
        explicit Block(
                BlockHeader header,
                std::unordered_set<Transaction> transactions
        ) :
                m_header(std::move(header)),
                m_transactions(std::move(transactions)) {
        }

        static Block from_json(const nlohmann::json &json) {

            std::unordered_set<Transaction> transactions;
            for (const auto &tx: json["transactions"]) {
                transactions.insert(Transaction::from_json(tx));
            }

            return Block{
                    BlockHeader::from_json(json["header"]),
                    transactions
            };
        }

        static std::optional<Block> from_disk(const std::filesystem::path &path) {

            if (path.has_filename()
                && path.has_extension()
                && !path.empty()
                && path.extension().string().ends_with(".json")) {

                std::fstream file(path);

                if (file.is_open() && file.good()) {
                    spdlog::info("Block: loaded from disk {}", path.string());

                    auto json = nlohmann::json::parse(file);
                    return Block::from_json(json);
                }

            }
            return {};
        }

        [[nodiscard]]
        std::string hash() const {

            return m_header.m_hash;
        }

        void to_disk(const std::filesystem::path &path) const {

            namespace fs = std::filesystem;

            std::ofstream file(path / (m_header.m_hash + ".json"));
            if (file.is_open() && file.good())
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

        [[nodiscard]] const std::unordered_set<Transaction> &transactions() const {

            return m_transactions;
        }

        void save_to_disk(std::string_view path) const {

        }

    private:
        BlockHeader m_header;
        std::unordered_set<Transaction> m_transactions;

    };
} // krapi
#endif //NODE_MODELS_BLOCK_H
