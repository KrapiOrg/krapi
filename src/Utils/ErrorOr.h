//
// Created by mythi on 11/10/22.
//

#pragma once

#include "tl/expected.hpp"

#define TRY(expression)                                                       \
  ({                                                                           \
    auto _temporary_result = (expression);                                      \
    if (!_temporary_result.has_value()) [[unlikely]]                             \
      return tl::make_unexpected(_temporary_result.error());                      \
    _temporary_result.value();                                                     \
  })

namespace krapi {

    enum class KrapiCode {
        WARN,
        INFO,
        CRITICAL,
        ERROR
    };

    inline std::string to_string(KrapiCode code) {

        if (code == KrapiCode::WARN)
            return "Warning";
        if (code == KrapiCode::INFO)
            return "Info";
        if (code == KrapiCode::CRITICAL)
            return "Critical";

        return "Error";
    }

    struct KrapiErr {

        std::string err_str;
        KrapiCode code;

        explicit KrapiErr(
                std::string err_str,
                KrapiCode code = KrapiCode::ERROR
        ) :
                err_str(std::move(err_str)),
                code(code) {

        }


    };

    template<typename T>
    using ErrorOr = tl::expected<T, KrapiErr>;

} // hasha