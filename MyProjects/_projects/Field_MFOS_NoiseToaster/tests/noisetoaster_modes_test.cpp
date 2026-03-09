#include "noisetoaster_modes.h"

using namespace noisetoaster;

static_assert(KeyALedId(0) != KeyALedId(1));
static_assert(KeyALedId(0) == 15);
static_assert(KeyALedId(1) == 14);
static_assert(KeyALedId(7) == 8);

static_assert(AdvanceVcoWave(VcoWave::Saw) == VcoWave::Square);
static_assert(AdvanceVcoWave(VcoWave::Square) == VcoWave::Triangle);
static_assert(AdvanceVcoWave(VcoWave::Triangle) == VcoWave::Saw);

static_assert(AdvanceLfoWave(LfoWave::Sine) == LfoWave::Square);
static_assert(AdvanceLfoWave(LfoWave::Square) == LfoWave::Triangle);
static_assert(AdvanceLfoWave(LfoWave::Triangle) == LfoWave::Sine);

static_assert(AdvanceVcfModSource(VcfModSource::Lfo) == VcfModSource::Areg);
static_assert(AdvanceVcfModSource(VcfModSource::Areg) == VcfModSource::Off);
static_assert(AdvanceVcfModSource(VcfModSource::Off) == VcfModSource::Lfo);

static_assert(LedBrightnessForThreeState(ThreeStateLed::On, true) == 1.0f);
static_assert(LedBrightnessForThreeState(ThreeStateLed::Blink, true) == 1.0f);
static_assert(LedBrightnessForThreeState(ThreeStateLed::Blink, false) == 0.0f);
static_assert(LedBrightnessForThreeState(ThreeStateLed::Off, true) == 0.0f);

static_assert(ThreeStateForVcfModSource(VcfModSource::Lfo) == ThreeStateLed::On);
static_assert(ThreeStateForVcfModSource(VcfModSource::Areg) == ThreeStateLed::Blink);
static_assert(ThreeStateForVcfModSource(VcfModSource::Off) == ThreeStateLed::Off);

static_assert(ShouldQueueRepeatTrigger(true, true, false, false));
static_assert(!ShouldQueueRepeatTrigger(false, true, false, false));
static_assert(!ShouldQueueRepeatTrigger(true, false, false, false));
static_assert(!ShouldQueueRepeatTrigger(true, true, true, false));
static_assert(!ShouldQueueRepeatTrigger(true, true, false, true));

int main() { return 0; }
