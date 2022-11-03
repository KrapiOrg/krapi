//
// Created by mythi on 24/10/22.
//

#ifndef SHARED_MODELS_TRANSACTION_H
#define SHARED_MODELS_TRANSACTION_H

#include "nlohmann/json.hpp"
#include "magic_enum.hpp"
#include "cryptopp/sha.h"
#include "filters.h"
#include "hex.h"

namespace krapi {

    enum class TransactionType {
        Send
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TransactionType, {
        { TransactionType::Send, "send_tx" }
    })

    class Transaction {
    public:
        explicit Transaction(
                TransactionType type = {},
                int from = {},
                int to = {}
        ) :
                m_type(type),
                m_from(from),
                m_to(to) {}

        static Transaction from_json(const nlohmann::json &json) {


            return Transaction{
                    json["type"].get<TransactionType>(),
                    json["from"].get<int>(),
                    json["to"].get<int>()
            };
        }

        [[nodiscard]]
        nlohmann::json to_json() const {

            return {
                    {"type", nlohmann::json(m_type)},
                    {"from", m_from},
                    {"to",   m_to}
            };
        }

        bool operator==(const Transaction &) const = default;

        [[nodiscard]]
        TransactionType type() const {

            return m_type;
        }

        [[nodiscard]]
        int from() const {

            return m_from;
        }

        [[nodiscard]]
        int to() const {

            return m_to;
        }

        [[nodiscard]]
        std::string hashcode() const {

            using namespace CryptoPP;
            auto str = to_json().dump();
            auto digest = std::string();
            auto hash = SHA256();
            StringSource s1(str, true,
                            new HashFilter(hash, new HexEncoder(new StringSink(digest))));

            return digest;
        }

    private:
        TransactionType m_type;
        int m_from;
        int m_to;
    };

} // krapi

namespace std {
    template<>
    struct hash<krapi::Transaction> {
        size_t operator()(const krapi::Transaction &transaction) const {

            return hash<std::string>()(transaction.hashcode());
        }
    };
}

#endif //SHARED_MODELS_TRANSACTION_H
