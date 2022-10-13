//
// Created by mythi on 13/10/22.
//
#include "TransactionBase.h"

namespace krapi {

    TransactionBase::TransactionBase(std::string type) : m_type(std::move(type)) {
    }

    std::string_view TransactionBase::type() const {

        return m_type;
    }

    nlohmann::json TransactionBase::to_json() const {

        return {{"type", m_type}};
    }

    std::string TransactionBase::to_string() const {

        std::stringstream ss;
        ss << std::setw(4) << to_json();
        return ss.str();
    }

}