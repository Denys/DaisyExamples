#include "daisy_field.h"
#include "daisyhost/apps/MultiDelayCore.h"
#include "daisysp.h"

#include <cmath>
#include <cstdio>

using namespace daisy;
using namespace daisyhost;
using namespace daisyhost::apps;

namespace
{
DaisyField hw;
MultiDelayCore core("node0");
MultiDelayCore::DelayLineType DSY_SDRAM_BSS
    sdramDelays[MultiDelayCore::kDelayCount];

constexpr size_t kAudioBlockSize = 48;
constexpr float  kTouchEpsilon   = 0.0005f;
constexpr float  kLedDim         = 0.06f;

constexpr size_t kKnobLeds[8] = {
    DaisyField::LED_KNOB_1,
    DaisyField::LED_KNOB_2,
    DaisyField::LED_KNOB_3,
    DaisyField::LED_KNOB_4,
    DaisyField::LED_KNOB_5,
    DaisyField::LED_KNOB_6,
    DaisyField::LED_KNOB_7,
    DaisyField::LED_KNOB_8,
};

constexpr size_t kTopRowLeds[8] = {
    DaisyField::LED_KEY_A1,
    DaisyField::LED_KEY_A2,
    DaisyField::LED_KEY_A3,
    DaisyField::LED_KEY_A4,
    DaisyField::LED_KEY_A5,
    DaisyField::LED_KEY_A6,
    DaisyField::LED_KEY_A7,
    DaisyField::LED_KEY_A8,
};

constexpr size_t kBottomRowLeds[8] = {
    DaisyField::LED_KEY_B1,
    DaisyField::LED_KEY_B2,
    DaisyField::LED_KEY_B3,
    DaisyField::LED_KEY_B4,
    DaisyField::LED_KEY_B5,
    DaisyField::LED_KEY_B6,
    DaisyField::LED_KEY_B7,
    DaisyField::LED_KEY_B8,
};

const char* kKnobControlIds[4] = {
    "node0/control/ctrl_1",
    "node0/control/ctrl_2",
    "node0/control/ctrl_3",
    "node0/control/ctrl_4",
};
const char* k5ParameterId        = "node0/param/delay_tertiary";
const char* cv1PortId            = "node0/port/cv_in_1";
const char* fireImpulseMenuItemId = "node0/menu/input/fire_impulse";

std::array<float, 8> knobValues{};
float                cv1Value          = 0.0f;
float                lastK5Value       = -1.0f;
float                lastCv1Value      = -1.0f;
int                  oledPage          = 0;
int                  fireImpulsePulse  = 0;
bool                 sw2WasPressed     = false;

float Clamp01(float value)
{
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

bool HasMeaningfulChange(float value, float previous)
{
    return previous < -0.5f || fabsf(value - previous) > kTouchEpsilon;
}

uint16_t ToDacCode(float normalized)
{
    return static_cast<uint16_t>(Clamp01(normalized) * 4095.0f);
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    const float* inputPtrs[1]  = {in[0]};
    float*       outputPtrs[2] = {out[0], out[1]};

    core.Process({inputPtrs, 1},
                 {outputPtrs, 2},
                 size);
}

void WriteDisplayLine(int y, const char* text)
{
    hw.display.SetCursor(0, y);
    hw.display.WriteString(text, Font_6x8, true);
}

void DrawCoreDisplay()
{
    const DisplayModel& model = core.GetDisplayModel();

    for(const auto& text : model.texts)
    {
        hw.display.SetCursor(text.x, text.y);
        hw.display.WriteString(text.text.c_str(), Font_6x8, !text.inverted);
    }

    for(const auto& bar : model.bars)
    {
        const int x2 = bar.x + bar.width;
        const int y2 = bar.y + bar.height;
        hw.display.DrawRect(bar.x, bar.y, x2, y2, true, false);

        const int fillWidth = static_cast<int>(bar.width * bar.normalized);
        if(fillWidth > 0)
        {
            hw.display.DrawRect(
                bar.x, bar.y, bar.x + fillWidth, y2, true, true);
        }
    }
}

void DrawHardwareStatus()
{
    char line[32];
    WriteDisplayLine(0, "Field MultiDelay");

    snprintf(line,
             sizeof(line),
             "K1 %02d K2 %02d",
             static_cast<int>(knobValues[0] * 99.0f),
             static_cast<int>(knobValues[1] * 99.0f));
    WriteDisplayLine(10, line);

    snprintf(line,
             sizeof(line),
             "K3 %02d K4 %02d",
             static_cast<int>(knobValues[2] * 99.0f),
             static_cast<int>(knobValues[3] * 99.0f));
    WriteDisplayLine(20, line);

    snprintf(line,
             sizeof(line),
             "K5/CV1 D3 %02d/%02d",
             static_cast<int>(knobValues[4] * 99.0f),
             static_cast<int>(cv1Value * 99.0f));
    WriteDisplayLine(30, line);

    snprintf(line,
             sizeof(line),
             "CV OUT1 %1.2fV",
             Clamp01(knobValues[4]) * 5.0f);
    WriteDisplayLine(40, line);

    WriteDisplayLine(52, "SW1 Impulse SW2 Page");
}

void UpdateDisplay()
{
    hw.display.Fill(false);
    if(oledPage == 0)
    {
        DrawCoreDisplay();
    }
    else
    {
        DrawHardwareStatus();
    }
    hw.display.Update();
}

void UpdateLeds()
{
    for(size_t i = 0; i < 8; ++i)
    {
        hw.led_driver.SetLed(kKnobLeds[i], knobValues[i]);
    }

    for(size_t i = 0; i < 8; ++i)
    {
        const float value = i < 5 ? (0.15f + (0.70f * knobValues[i])) : 0.0f;
        hw.led_driver.SetLed(kTopRowLeds[i], value);
    }

    for(size_t i = 0; i < 8; ++i)
    {
        hw.led_driver.SetLed(kBottomRowLeds[i],
                             i == static_cast<size_t>(oledPage) ? 0.35f : 0.0f);
    }

    hw.led_driver.SetLed(DaisyField::LED_SW_1,
                         fireImpulsePulse > 0 ? 1.0f : kLedDim);
    hw.led_driver.SetLed(DaisyField::LED_SW_2,
                         oledPage == 1 ? 0.35f : kLedDim);
    hw.led_driver.SwapBuffersAndTransmit();
}

void ProcessControls()
{
    hw.ProcessAllControls();

    for(size_t i = 0; i < 8; ++i)
    {
        knobValues[i] = hw.GetKnobValue(i);
    }

    for(size_t i = 0; i < 4; ++i)
    {
        core.SetControl(kKnobControlIds[i], knobValues[i]);
    }

    if(HasMeaningfulChange(knobValues[4], lastK5Value))
    {
        lastK5Value = knobValues[4];
        core.SetParameterValue(k5ParameterId, knobValues[4]);
    }

    cv1Value = hw.GetCvValue(DaisyField::CV_1);
    if(HasMeaningfulChange(cv1Value, lastCv1Value))
    {
        lastCv1Value = cv1Value;
        PortValue value;
        value.type   = VirtualPortType::kCv;
        value.scalar = cv1Value;
        core.SetPortInput(cv1PortId, value);
    }

    if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
    {
        core.SetMenuItemValue(fireImpulseMenuItemId, 1.0f);
        fireImpulsePulse = 12;
    }

    const bool sw2Pressed = hw.GetSwitch(DaisyField::SW_2)->Pressed();
    if(sw2Pressed && !sw2WasPressed)
    {
        oledPage = (oledPage + 1) % 2;
    }
    sw2WasPressed = sw2Pressed;

    if(fireImpulsePulse > 0)
    {
        --fireImpulsePulse;
    }

    hw.SetCvOut1(ToDacCode(knobValues[4]));
    hw.SetCvOut2(0);
}
} // namespace

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(kAudioBlockSize);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    core.AttachDelayStorage(sdramDelays, MultiDelayCore::kDelayCount);
    core.Prepare(hw.AudioSampleRate(), hw.AudioBlockSize());

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(true)
    {
        ProcessControls();
        core.TickUi(0.0);
        UpdateDisplay();
        UpdateLeds();
        System::Delay(2);
    }
}
