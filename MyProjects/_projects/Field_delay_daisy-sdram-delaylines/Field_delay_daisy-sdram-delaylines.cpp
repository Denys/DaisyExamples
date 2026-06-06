#include "../Field_delay_shared/FieldDelayFieldApp.h"

int main(void)
{
    return field_delay::RunFieldDelayProject(
        daisyhost::DaisyDelayFxSource::kSdramDelaylines,
        "Field_delay_SDRAM");
}
