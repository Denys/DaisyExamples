#include "../Field_delay_shared/FieldDelayFieldApp.h"

int main(void)
{
    return field_delay::RunFieldDelayProject(
        daisyhost::DaisyDelayFxSource::kReverbPlayground,
        "Field_delay_reverb");
}
