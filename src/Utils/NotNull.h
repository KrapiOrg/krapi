#pragma once

#include <type_traits>
#include <utility>
#include "Assert.h"

namespace krapi {
    namespace details {
        template<typename T, typename = void>
        struct is_comparable_to_nullptr : std::false_type {
        };

        template<typename T>
        struct is_comparable_to_nullptr<
                T,
                std::enable_if_t<std::is_convertible<decltype(std::declval<T>() != nullptr), bool>::value>>
                : std::true_type {
        };

        template<typename T>
        using value_or_reference_return_t = std::conditional_t<
                sizeof(T) < 2 * sizeof(void *) && std::is_trivially_copy_constructible<T>::value,
                const T,
                const T &>;
    }

    template<class T>
    class NotNull {
    public:
        static_assert(details::is_comparable_to_nullptr<T>::value, "T cannot be compared to nullptr.");

        template<typename U, typename = std::enable_if_t<std::is_convertible<U, T>::value>>
        constexpr NotNull(U &&u) : ptr_(std::forward<U>(u)) {

            Expects(ptr_ != nullptr);
        }

        template<typename = std::enable_if_t<!std::is_same<std::nullptr_t, T>::value>>
        constexpr NotNull(T u) : ptr_(std::move(u)) {

            Expects(ptr_ != nullptr);
        }

        template<typename U, typename = std::enable_if_t<std::is_convertible<U, T>::value>>
        constexpr NotNull(const NotNull<U> &other) : NotNull(other.get()) {}

        NotNull(const NotNull &other) = default;

        NotNull &operator=(const NotNull &other) = default;

        constexpr details::value_or_reference_return_t<T> get() const {

            return ptr_;
        }

        constexpr operator T() const { return get(); }

        constexpr decltype(auto) operator->() const { return get(); }

        constexpr decltype(auto) operator*() const { return *get(); }

        // prevents compilation when someone attempts to assign a null pointer constant
        NotNull(std::nullptr_t) = delete;

        NotNull &operator=(std::nullptr_t) = delete;

        // unwanted operators...pointers only point to single objects!
        NotNull &operator++() = delete;

        NotNull &operator--() = delete;

        NotNull operator++(int) = delete;

        NotNull operator--(int) = delete;

        NotNull &operator+=(std::ptrdiff_t) = delete;

        NotNull &operator-=(std::ptrdiff_t) = delete;

        void operator[](std::ptrdiff_t) const = delete;

    private:
        T ptr_;
    };

    template<class T>
    auto make_not_null(T &&t) noexcept {

        return NotNull<std::remove_cv_t<std::remove_reference_t<T>>>{std::forward<T>(t)};
    }

}