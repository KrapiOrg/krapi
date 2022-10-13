//
// Created by mythi on 13/10/22.
//
#include "MessageBase.h"

namespace krapi {
    MessageBase::MessageBase(std::string type) : m_type(std::move(type)) {

    }

    nlohmann::json MessageBase::to_json() const {

        return {
                {"type", m_type}
        };
    }

    std::string MessageBase::to_string() const {

        return to_json().dump();
    }

    std::string_view MessageBase::type() const {

        return m_type;
    }
}