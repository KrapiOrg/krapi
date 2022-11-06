//
// Created by mythi on 02/11/22.
//

#ifndef SHARED_MODELS_BLOCKHEADER_H
#define SHARED_MODELS_BLOCKHEADER_H

#include <string>
#include <utility>

#include "nlohmann/json.hpp"

namespace krapi {
    class BlockHeader {
        friend class Block;
    public:
        explicit BlockHeader(
                std::string hash,
                std::string previous_hash,
                std::string timestamp,
                std::string merkle_root,
                int nonce
        ) : m_hash(std::move(hash)),
            m_previous_hash(std::move(previous_hash)),
            m_timestamp(std::move(timestamp)),
            m_merkle_root(std::move(merkle_root)),
            m_nonce(nonce) {

        }

        static BlockHeader from_json(nlohmann::json json) {

            return BlockHeader{
                    json["hash"].get<std::string>(),
                    json["previous_hash"].get<std::string>(),
                    json["timestamp"].get<std::string>(),
                    json["merkle_root"].get<std::string>(),
                    json["nonce"].get<int>()
            };
        }

        [[nodiscard]]
        nlohmann::json to_json() const {

            return {
                    {"hash", m_hash},
                    {"previous_hash", m_previous_hash},
                    {"timestamp", m_timestamp},
                    {"merkle_root", m_merkle_root},
                    {"nonce", m_nonce}
            };
        }

        [[nodiscard]]
        const std::string &previous_hash() const {

            return m_previous_hash;
        }

        [[nodiscard]]
        const std::string &timestamp() const {

            return m_timestamp;
        }

        [[nodiscard]]
        const std::string &merkle_root() const {

            return m_merkle_root;
        }

        [[nodiscard]]
        const std::string &hash() const {

            return m_hash;
        }

        [[nodiscard]]
        int nonce() const {

            return m_nonce;
        }

    private:
        std::string m_previous_hash;
        std::string m_timestamp;
        std::string m_merkle_root;
        std::string m_hash;
        int m_nonce;
    };
}

#endif //SHARED_MODELS_BLOCKHEADER_H
