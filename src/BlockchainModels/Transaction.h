//
// Created by mythi on 24/10/22.
//

#ifndef NODE_BLOCKCHAIN_TRANSACTION_H
#define NODE_BLOCKCHAIN_TRANSACTION_H

#include "nlohmann/json.hpp"
#include "cryptopp/hex.h"

namespace krapi {

    enum class TransactionType {
        Send
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TransactionType, {
        { TransactionType::Send, "send_tx" }
    })

    enum class TransactionStatus {
        Pending,
        Verified,
        Rejected
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TransactionStatus, {
        { TransactionStatus::Pending, "tx_status_pending" },
        { TransactionStatus::Verified, "tx_status_verified" },
        { TransactionStatus::Rejected, "tx_status_rejected" }
    })

    class Transaction {
    public:
        explicit Transaction(
                TransactionType type,
                TransactionStatus status,
                std::string hash,
                uint64_t timestamp,
                int from,
                int to
        );

        static Transaction from_json(const nlohmann::json &json);

        [[nodiscard]]
        nlohmann::json to_json() const;

        bool operator==(const Transaction &) const;

        [[nodiscard]]
        TransactionType type() const;

        [[nodiscard]]
        TransactionStatus status() const;

        [[nodiscard]]
        uint64_t timestamp() const;

        [[nodiscard]]
        int from() const;

        [[nodiscard]]
        int to() const;

        [[nodiscard]]
        std::string hash() const;


        [[nodiscard]]
        std::array<CryptoPP::byte, 32> byte_hash() const;

    private:
        TransactionType m_type;
        TransactionStatus m_status;
        std::string m_hash;
        uint64_t m_timestamp;
        int m_from;
        int m_to;

        std::array<CryptoPP::byte, 32> m_byte_hash{};
    };

} // krapi

namespace std {
    template<>
    struct hash<krapi::Transaction> {
        size_t operator()(const krapi::Transaction &tx) const {

            auto byte_hash = tx.byte_hash();

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

#endif //NODE_BLOCKCHAIN_TRANSACTION_H
