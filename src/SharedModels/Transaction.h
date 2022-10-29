//
// Created by mythi on 24/10/22.
//

#ifndef SHARED_MODELS_TRANSACTION_H
#define SHARED_MODELS_TRANSACTION_H

#include "nlohmann/json.hpp"

namespace krapi {

    enum class TransactionType {
        Send
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TransactionType, {
        { TransactionType::Send, "send_tx" },
    })

    struct Transaction {
        TransactionType type;
        int from;
        int to;

        bool operator==(const Transaction &) const = default;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Transaction, type, from, to)
    };

} // krapi

#endif //SHARED_MODELS_TRANSACTION_H
