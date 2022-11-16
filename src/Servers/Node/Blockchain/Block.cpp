//
// Created by mythi on 12/11/22.
//

#include "Block.h"

namespace krapi {
    Block::Block(BlockHeader header, std::unordered_set<Transaction> transactions) :
            m_header(std::move(header)),
            m_transactions(std::move(transactions)) {
    }

    Block Block::from_json(const nlohmann::json &json) {

        std::unordered_set<Transaction> transactions;
        for (const auto &tx: json["transactions"]) {
            transactions.insert(Transaction::from_json(tx));
        }

        return Block{
                BlockHeader::from_json(json["header"]),
                transactions
        };
    }

    std::optional<Block> Block::from_disk(const std::filesystem::path &path) {

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

    std::string Block::hash() const {

        return m_header.m_hash;
    }

    std::array<CryptoPP::byte, 32> Block::hash_bytes() const {

        return m_header.m_hash_bytes;
    }

    void Block::to_disk(const std::filesystem::path &path) const {

        namespace fs = std::filesystem;

        std::ofstream file(path / (m_header.m_hash + ".json"));
        if (file.is_open() && file.good()) {

            file << to_json().dump(4);
        } else {
            spdlog::error("Failed to load block from {}");
            exit(1);
        }
    }

    nlohmann::json Block::to_json() const {

        auto transactions = nlohmann::json::array();
        for (const auto &tx: m_transactions) {
            transactions.push_back(tx.to_json());
        }

        return {
                {"header",       m_header.to_json()},
                {"transactions", transactions}
        };
    }

    BlockHeader Block::header() const {

        return m_header;
    }

    std::unordered_set<Transaction> Block::transactions() const {

        return m_transactions;
    }
} // krapi