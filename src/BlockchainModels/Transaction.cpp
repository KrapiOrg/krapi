//
// Created by mythi on 16/11/22.
//
#include "Transaction.h"

namespace krapi {

    Transaction::Transaction(
            TransactionType type,
            TransactionStatus status,
            std::string hash,
            uint64_t timestamp,
            int from,
            int to
    ) :
            m_type(type),
            m_status(status),
            m_hash(std::move(hash)),
            m_timestamp(timestamp),
            m_from(from),
            m_to(to) {

        CryptoPP::StringSource s(m_hash, true,
                                 new CryptoPP::HexDecoder(new CryptoPP::ArraySink(m_byte_hash.data(), 32)));
    }

    Transaction Transaction::from_json(const nlohmann::json &json) {


        return Transaction{
                json["type"].get<TransactionType>(),
                json["status"].get<TransactionStatus>(),
                json["hash"].get<std::string>(),
                json["timestamp"].get<uint64_t>(),
                json["from"].get<int>(),
                json["to"].get<int>()
        };
    }

    nlohmann::json Transaction::to_json() const {

        return {
                {"type",      nlohmann::json(m_type)},
                {"status",    m_status},
                {"hash",      m_hash},
                {"timestamp", m_timestamp},
                {"from",      m_from},
                {"to",        m_to}
        };
    }

    bool Transaction::operator==(const Transaction &) const = default;

    TransactionType Transaction::type() const {

        return m_type;
    }

    int Transaction::from() const {

        return m_from;
    }

    int Transaction::to() const {

        return m_to;
    }

    std::string Transaction::hash() const {

        return m_hash;
    }

    std::array<CryptoPP::byte, 32> Transaction::byte_hash() const {

        return m_byte_hash;
    }

    TransactionStatus Transaction::status() const {

        return m_status;
    }

    uint64_t Transaction::timestamp() const {

        return m_timestamp;
    }

    bool Transaction::set_status(TransactionStatus status) const {

        if (m_status == TransactionStatus::Verified) {
            return false;
        }

        m_status = status;
        return true;
    }

    bool Transaction::operator<(const Transaction &other) const {

        return m_timestamp < other.m_timestamp;
    }
}