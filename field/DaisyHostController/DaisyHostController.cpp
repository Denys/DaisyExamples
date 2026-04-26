#include "daisy_field.h"
#include "hid/midi.h"

#include <cmath>
#include <cstdio>
#include <cstdint>

using namespace daisy;

namespace
{
DaisyField     hw;
MidiUsbHandler usbMidi;

constexpr uint8_t kMidiChannel = 1;
constexpr uint8_t kKnobCcBase = 20;
constexpr uint8_t kCvCcBase = 28;
constexpr uint8_t kSwitchCcBase = 80;
constexpr uint8_t kKeyNoteBase = 60;
constexpr float kLedIdle = 0.04f;
constexpr uint32_t kDisplayMs = 80;
constexpr uint32_t kLedMs = 16;

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

constexpr size_t kKeyLeds[16] = {
    DaisyField::LED_KEY_A1,
    DaisyField::LED_KEY_A2,
    DaisyField::LED_KEY_A3,
    DaisyField::LED_KEY_A4,
    DaisyField::LED_KEY_A5,
    DaisyField::LED_KEY_A6,
    DaisyField::LED_KEY_A7,
    DaisyField::LED_KEY_A8,
    DaisyField::LED_KEY_B1,
    DaisyField::LED_KEY_B2,
    DaisyField::LED_KEY_B3,
    DaisyField::LED_KEY_B4,
    DaisyField::LED_KEY_B5,
    DaisyField::LED_KEY_B6,
    DaisyField::LED_KEY_B7,
    DaisyField::LED_KEY_B8,
};

uint8_t knobValues[8] = {};
uint8_t cvValues[4]   = {};
bool    analogInitialized = false;
char    lastEvent[32] = "Ready";
uint32_t lastDisplayUpdate = 0;
uint32_t lastLedUpdate     = 0;

float Clamp01(float value)
{
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

uint8_t ToMidiValue(float normalized)
{
    return static_cast<uint8_t>(Clamp01(normalized) * 127.0f + 0.5f);
}

uint8_t BipolarToMidiValue(float value)
{
    return ToMidiValue(value * 0.5f + 0.5f);
}

uint8_t StatusByte(uint8_t type)
{
    return static_cast<uint8_t>(type | ((kMidiChannel - 1) & 0x0F));
}

void SendBytes(uint8_t status, uint8_t data1, uint8_t data2)
{
    uint8_t message[3] = {status, data1, data2};
    usbMidi.SendMessage(message, 3);
}

void SendControlChange(uint8_t cc, uint8_t value)
{
    SendBytes(StatusByte(0xB0), cc, value);
    std::snprintf(lastEvent, sizeof(lastEvent), "CC %u = %u", cc, value);
}

void SendNote(uint8_t note, bool pressed)
{
    SendBytes(StatusByte(pressed ? 0x90 : 0x80), note, pressed ? 100 : 0);
    std::snprintf(lastEvent,
                  sizeof(lastEvent),
                  "%s %u",
                  pressed ? "Note on" : "Note off",
                  note);
}

void ProcessAnalogControllers()
{
    for(size_t i = 0; i < 8; ++i)
    {
        const uint8_t value = ToMidiValue(hw.GetKnobValue(i));
        if(analogInitialized && value != knobValues[i])
        {
            SendControlChange(static_cast<uint8_t>(kKnobCcBase + i), value);
        }
        knobValues[i] = value;
    }

    for(size_t i = 0; i < 4; ++i)
    {
        const uint8_t value = BipolarToMidiValue(hw.GetCvValue(i));
        if(analogInitialized && value != cvValues[i])
        {
            SendControlChange(static_cast<uint8_t>(kCvCcBase + i), value);
        }
        cvValues[i] = value;
    }

    analogInitialized = true;
}

void ProcessKeys()
{
    for(size_t i = 0; i < 16; ++i)
    {
        const uint8_t note = static_cast<uint8_t>(kKeyNoteBase + i);
        if(hw.KeyboardRisingEdge(i))
        {
            SendNote(note, true);
        }
        if(hw.KeyboardFallingEdge(i))
        {
            SendNote(note, false);
        }
    }
}

void ProcessSwitches()
{
    for(size_t i = 0; i < DaisyField::SW_LAST; ++i)
    {
        Switch* sw = hw.GetSwitch(i);
        const uint8_t cc = static_cast<uint8_t>(kSwitchCcBase + i);
        if(sw->RisingEdge())
        {
            SendControlChange(cc, 127);
        }
        if(sw->FallingEdge())
        {
            SendControlChange(cc, 0);
        }
    }
}

void UpdateDisplay()
{
    const uint32_t now = System::GetNow();
    if(now - lastDisplayUpdate < kDisplayMs)
    {
        return;
    }
    lastDisplayUpdate = now;

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    char title[] = "DaisyHost USB MIDI";
    hw.display.WriteString(title, Font_6x8, true);
    hw.display.SetCursor(0, 14);
    char channel[24];
    std::snprintf(channel, sizeof(channel), "Ch %u  K20-27", kMidiChannel);
    hw.display.WriteString(channel, Font_6x8, true);
    hw.display.SetCursor(0, 28);
    char cvLine[] = "CV28-31  SW80-81";
    hw.display.WriteString(cvLine, Font_6x8, true);
    hw.display.SetCursor(0, 44);
    hw.display.WriteString(lastEvent, Font_6x8, true);
    hw.display.Update();
}

void UpdateLeds()
{
    const uint32_t now = System::GetNow();
    if(now - lastLedUpdate < kLedMs)
    {
        return;
    }
    lastLedUpdate = now;

    for(size_t i = 0; i < 8; ++i)
    {
        hw.led_driver.SetLed(kKnobLeds[i],
                             static_cast<float>(knobValues[i]) / 127.0f);
    }
    for(size_t i = 0; i < 16; ++i)
    {
        hw.led_driver.SetLed(kKeyLeds[i],
                             hw.KeyboardState(i) ? 0.6f : kLedIdle);
    }
    hw.led_driver.SetLed(DaisyField::LED_SW_1,
                         hw.GetSwitch(DaisyField::SW_1)->Pressed() ? 0.8f
                                                                    : kLedIdle);
    hw.led_driver.SetLed(DaisyField::LED_SW_2,
                         hw.GetSwitch(DaisyField::SW_2)->Pressed() ? 0.8f
                                                                    : kLedIdle);
    hw.led_driver.SwapBuffersAndTransmit();
}
} // namespace

int main(void)
{
    hw.Init();
    hw.StartAdc();

    MidiUsbHandler::Config usbConfig;
    usbConfig.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
    usbMidi.Init(usbConfig);
    usbMidi.StartReceive();

    for(;;)
    {
        hw.ProcessAllControls();
        usbMidi.Listen();
        ProcessAnalogControllers();
        ProcessKeys();
        ProcessSwitches();
        UpdateDisplay();
        UpdateLeds();
        System::Delay(2);
    }
}
