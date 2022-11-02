//
// Created by mythi on 24/10/22.
//

#ifndef SHARED_MODELS_TRANSACTION_H
#define SHARED_MODELS_TRANSACTION_H

#include "nlohmann/json.hpp"
#include "magic_enum.hpp"

namespace krapi {

    enum class TransactionType {
        Send
    };

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

        static Transaction from_json(const nlohmann::json& json) {


            return Transaction{
                    magic_enum::enum_cast<TransactionType>(json["type"].get<std::string>()).value(),
                    json["from"].get<int>(),
                    json["to"].get<int>()
            };
        }

        [[nodiscard]]
        nlohmann::json to_json() const {

            return {
                    {"type", magic_enum::enum_name(m_type)},
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

    private:
        TransactionType m_type;
        int m_from;
        int m_to;
    };

} // krapi

#endif //SHARED_MODELS_TRANSACTION_H
