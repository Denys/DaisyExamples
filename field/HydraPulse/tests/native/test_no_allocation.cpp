#include "src/app/Controller.h"
#include "tests/native/test_support.h"
#include <cstdlib>
#include <new>
static bool monitoring = false;
static unsigned allocations = 0;
void *operator new(std::size_t n)
{
    if (monitoring)
        ++allocations;
    if (void *p = std::malloc(n ? n : 1))
        return p;
    throw std::bad_alloc();
}
void operator delete(void *p) noexcept
{
    std::free(p);
}
void operator delete(void *p, std::size_t) noexcept
{
    std::free(p);
}
void *operator new[](std::size_t n)
{
    return ::operator new(n);
}
void operator delete[](void *p) noexcept
{
    ::operator delete(p);
}
void operator delete[](void *p, std::size_t) noexcept
{
    ::operator delete(p);
}
int main()
{
    hydrapulse::Engine e;
    e.Init();
    hydrapulse::Controller c;
    hydrapulse::InputFrame f{};
    c.Init(e, f);
    monitoring = true;
    e.RequestPreset(7);
    for (int n = 0; n < 1000; ++n)
        (void)e.Process();
    e.Start();
    for (unsigned n = 0; n < 48000 * 2; ++n)
    {
        if (n % 48u == 0)
            c.Tick(e, f);
        if (n % 1201u == 0)
            e.Trigger(n % 4u);
        (void)e.Process();
    }
    e.Panic();
    monitoring = false;
    HPF_CHECK_EQ(allocations, 0u);
    return EXIT_SUCCESS;
}
