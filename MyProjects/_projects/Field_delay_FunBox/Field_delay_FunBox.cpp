#include "../Field_delay_shared/FieldDelayFieldApp.h"

int main(void)
{
    return field_delay::RunFieldDelayProject(
        daisyhost::DaisyDelayFxSource::kFunBox,
        "Field_delay_FunBox");
}
