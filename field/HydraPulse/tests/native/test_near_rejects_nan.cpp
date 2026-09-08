#include "tests/native/test_support.h"
#include <limits>
int main()
{
    HPF_CHECK_NEAR(std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.1f);
    return EXIT_SUCCESS;
}
