#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace hydrapulse::core
{
// Exactly ONE producer and ONE consumer. No overwrite, no spin, no blocking.
// Usable capacity is N-1. Do not Reset while either context is active.
template <typename T, std::uint32_t N> class SpscQueue
{
    static_assert(N >= 2 && (N & (N - 1u)) == 0, "capacity must be a power of two");
    static_assert(std::is_trivially_copyable<T>::value, "queue payload must be trivially copyable");
    static_assert(std::atomic<std::uint32_t>::is_always_lock_free, "32-bit atomics must be lock-free");

  public:
    bool Push(const T &value) noexcept
    {
        const auto w = write_.load(std::memory_order_relaxed);
        const auto next = (w + 1u) & (N - 1u);
        if (next == read_.load(std::memory_order_acquire))
            return false;
        slots_[w] = value;
        write_.store(next, std::memory_order_release);
        return true;
    }
    bool Pop(T &value) noexcept
    {
        const auto r = read_.load(std::memory_order_relaxed);
        if (r == write_.load(std::memory_order_acquire))
            return false;
        value = slots_[r];
        read_.store((r + 1u) & (N - 1u), std::memory_order_release);
        return true;
    }

  private:
    std::array<T, N> slots_{};
    std::atomic<std::uint32_t> write_{0}, read_{0};
};
} // namespace hydrapulse::core
