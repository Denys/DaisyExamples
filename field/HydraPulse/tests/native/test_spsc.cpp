#include "src/core/SpscQueue.h"
#include "tests/native/test_support.h"
#include <atomic>
#include <cstdint>
#include <thread>
int main()
{
    using hydrapulse::core::SpscQueue;
    SpscQueue<std::uint32_t, 4> q;
    HPF_CHECK(q.Push(1));
    HPF_CHECK(q.Push(2));
    HPF_CHECK(q.Push(3));
    HPF_CHECK(!q.Push(4));
    std::uint32_t x = 0;
    for (unsigned i = 1; i <= 3; ++i)
    {
        HPF_CHECK(q.Pop(x));
        HPF_CHECK_EQ(x, i);
    }
    HPF_CHECK(!q.Pop(x));
    SpscQueue<std::uint32_t, 64> threaded;
    std::atomic<bool> bad{false};
    constexpr std::uint32_t count = 200000;
    std::thread producer([&]() {
        for (std::uint32_t i = 0; i < count; ++i)
            while (!threaded.Push(i))
                std::this_thread::yield();
    });
    for (std::uint32_t i = 0; i < count; ++i)
    {
        while (!threaded.Pop(x))
            std::this_thread::yield();
        if (x != i)
            bad.store(true);
    }
    producer.join();
    HPF_CHECK(!bad.load());
    return EXIT_SUCCESS;
}
