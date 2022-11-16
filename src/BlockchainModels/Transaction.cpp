//
// Created by mythi on 16/11/22.
//
#include "Transaction.h"

namespace krapi {

    Transaction::Transaction(TransactionType type, std::string hash, int from, int to) :
            m_type(type),
            m_hash(std::move(hash)),
            m_from(from),
            m_to(to) {

        CryptoPP::StringSource s(m_hash, true,
                                 new CryptoPP::HexDecoder(new CryptoPP::ArraySink(m_byte_hash.data(), 32)));
    }

    Transaction Transaction::from_json(const nlohmann::json &json) {


        return Transaction{
                json["type"].get<TransactionType>(),
                json["hash"].get<std::string>(),
                json["from"].get<int>(),
                json["to"].get<int>()
        };
    }

    nlohmann::json Transaction::to_json() const {

        return {
                {"type", nlohmann::json(m_type)},
                {"hash", m_hash},
                {"from", m_from},
                {"to",   m_to}
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
}