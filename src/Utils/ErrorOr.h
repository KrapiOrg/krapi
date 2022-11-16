//
// Created by mythi on 11/10/22.
//

#pragma once

#include <string_view>

using namespace std::string_view_literals;

#include <variant>
#include <string>
#include <optional>

#define TRY(expression)                                                        \
  ({                                                                           \
    auto _temporary_result = (expression);                                     \
    if (_temporary_result.is_error()) [[unlikely]]                             \
      return _temporary_result.release_error();                                \
    _temporary_result.release_value();                                         \
  })

namespace krapi {

    enum class KrapiCode {
        WARN,
        INFO,
        CRITICAL,
        ERROR
    };

    struct KrapiErr {

        std::string_view err_str;
        KrapiCode code;

        explicit constexpr KrapiErr(
                std::string_view err_str,
                KrapiCode code = KrapiCode::ERROR
        ) :
                err_str(err_str),
                code(code) {

        }


    };

    template<typename T>
    class [[nodiscard]] ErrorOr final : public std::variant<T, KrapiErr> {
        using std::variant<T, KrapiErr>::variant;

    public:
        template<typename U>
        explicit ErrorOr(U &&value)requires (!std::is_same<std::remove_reference<U>, ErrorOr<T>>::value)
                : std::variant<T, KrapiErr>(std::forward<U>(value)) {}

        T &value() { return std::get<T>(*this); }

        T const &value() const { return std::get<T>(*this); }

        KrapiErr &error() { return std::get<KrapiErr>(*this); }

        [[nodiscard]] KrapiErr const &error() const {

            return std::get<KrapiErr>(*this);
        }

        [[nodiscard]] bool is_error() const {

            return std::holds_alternative<KrapiErr>(*this);
        }

        T release_value() { return std::move(value()); }

        KrapiErr release_error() { return std::move(error()); }
    };

    template<>
    class [[nodiscard]] ErrorOr<void> {
    public:
        ErrorOr(KrapiErr error) : m_error(std::move(error)) {}

        ErrorOr() = default;

        ErrorOr(ErrorOr &&other) noexcept = default;

        ErrorOr(ErrorOr const &other) = default;

        ~ErrorOr() = default;

        ErrorOr &operator=(ErrorOr &&other) = default;

        ErrorOr &operator=(ErrorOr const &other) = default;

        KrapiErr &error() { return m_error.value(); }

        [[nodiscard]] bool is_error() const { return m_error.has_value(); }

        KrapiErr release_error() { return std::move(m_error.value()); }

        void release_value() {}

    private:
        std::optional<KrapiErr> m_error;
    };
} // hasha