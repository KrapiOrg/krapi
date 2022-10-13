//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_TRANSACTIONBASE_H
#define KRAPI_MODELS_TRANSACTIONBASE_H

#include <string>
#include "nlohmann/json.hpp"
namespace krapi {

    class TransactionBase {
    protected:
        std::string m_type;
    public:
        explicit TransactionBase(std::string type);

        [[nodiscard]]
        std::string_view type() const;

        [[nodiscard]]
        virtual nlohmann::json to_json() const = 0;

        [[nodiscard]]
        std::string to_string() const;
    };

} // krapi

#endif //KRAPI_MODELS_TRANSACTIONBASE_H
