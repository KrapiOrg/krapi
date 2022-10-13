//
// Created by mythi on 13/10/22.
//
#include "ResponseBase.h"

namespace krapi {
    ResponseBase::ResponseBase(std::string type) : m_type(std::move(type)) {

    }

    nlohmann::json ResponseBase::to_json() const {

        return {
                {"type", m_type}
        };
    }

    std::string ResponseBase::to_string() const {

        return to_json().dump();

    }

    std::string_view ResponseBase::type() const {

        return m_type;
    }
}