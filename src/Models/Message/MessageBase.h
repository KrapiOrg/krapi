//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_MESSAGEBASE_H
#define KRAPI_MODELS_MESSAGEBASE_H

#include "nlohmann/json.hpp"

namespace krapi {
    class MessageBase {

    protected:
        std::string m_type;
    public:
        explicit MessageBase(std::string type);

        [[nodiscard]]
        std::string_view type() const;

        [[nodiscard]]
        virtual nlohmann::json to_json() const = 0;

        [[nodiscard]]
        std::string to_string() const;
    };
}

#endif //KRAPI_MODELS_MESSAGEBASE_H
