//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_RESPONSEBASE_H
#define KRAPI_MODELS_RESPONSEBASE_H

#include "nlohmann/json.hpp"

namespace krapi {
    class ResponseBase {

    protected:
        std::string m_type;
    public:
        explicit ResponseBase(std::string type);

        [[nodiscard]]
        std::string_view type() const;

        [[nodiscard]]
        virtual nlohmann::json to_json() const;

        [[nodiscard]]
        std::string to_string() const;

        virtual ~ResponseBase();
    };
}

#endif //KRAPI_MODELS_RESPONSEBASE_H
