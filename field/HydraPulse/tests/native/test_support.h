#pragma once

#include <cmath>
#include <cstdlib>
#include <iostream>

#define HPF_CHECK(expr)                                                                                      \
    do                                                                                                       \
    {                                                                                                        \
        if (!(expr))                                                                                         \
        {                                                                                                    \
            std::cerr << __FILE__ << ':' << __LINE__ << " CHECK failed: " #expr << '\n';                     \
            return EXIT_FAILURE;                                                                             \
        }                                                                                                    \
    } while (false)

#define HPF_CHECK_EQ(lhs, rhs)                                                                               \
    do                                                                                                       \
    {                                                                                                        \
        const auto hpf_lhs = (lhs);                                                                          \
        const auto hpf_rhs = (rhs);                                                                          \
        if (!(hpf_lhs == hpf_rhs))                                                                           \
        {                                                                                                    \
            std::cerr << __FILE__ << ':' << __LINE__ << " CHECK_EQ failed: " #lhs " != " #rhs << " ("        \
                      << hpf_lhs << " vs " << hpf_rhs << ")\n";                                              \
            return EXIT_FAILURE;                                                                             \
        }                                                                                                    \
    } while (false)

#define HPF_CHECK_NEAR(lhs, rhs, tol)                                                                        \
    do                                                                                                       \
    {                                                                                                        \
        const auto hpf_lhs = static_cast<double>(lhs);                                                       \
        const auto hpf_rhs = static_cast<double>(rhs);                                                       \
        const auto hpf_tol = static_cast<double>(tol);                                                       \
        if (!std::isfinite(hpf_lhs) || !std::isfinite(hpf_rhs) || !std::isfinite(hpf_tol) || hpf_tol < 0 ||  \
            std::fabs(hpf_lhs - hpf_rhs) > hpf_tol)                                                          \
        {                                                                                                    \
            std::cerr << __FILE__ << ':' << __LINE__ << " CHECK_NEAR failed: " #lhs " vs " #rhs << " ("      \
                      << hpf_lhs << " vs " << hpf_rhs << ", tol=" << hpf_tol << ")\n";                       \
            return EXIT_FAILURE;                                                                             \
        }                                                                                                    \
    } while (false)
