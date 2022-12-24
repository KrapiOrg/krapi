#pragma once

#include <exception>

#if defined(__clang__) || defined(__GNUC__)
#define K_LIKELY(x) __builtin_expect(!!(x), 1)
#define K_UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif

#define K_CONTRACT_CHECK(type, cond)                                                             \
    (K_LIKELY(cond) ? static_cast<void>(0) : std::terminate())

#define Expects(cond) K_CONTRACT_CHECK("Precondition", cond)