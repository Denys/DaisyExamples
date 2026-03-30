#include <Arduino.h>
#line 1 "C:\\Users\\denko\\Gemini\\Antigravity\\DVPE_Daisy-Visual-Programming-Environment\\DaisyExamples\\MyProjects\\_projects\\Seed_MIDI_Grainlet\\teensy_controller\\Seed_MIDI_Grainlet_Teensy40\\Seed_MIDI_Grainlet_Teensy40.ino"
constexpr uint8_t kMidiChannel  = 1;
constexpr uint8_t kNoteNumber   = 60;
constexpr uint8_t kNoteVelocity = 100;
constexpr uint8_t kButtonPin    = 2;
constexpr uint8_t kPotCount     = 3;
constexpr uint8_t kPotPins[kPotCount]   = {A0, A1, A2};
constexpr uint8_t kCcNumbers[kPotCount] = {14, 15, 16};
constexpr uint8_t kCcThreshold          = 1;
constexpr unsigned long kDebounceMs     = 15;

int lastCcValues[kPotCount] = {-1, -1, -1};

bool          buttonStableState = HIGH;
bool          buttonLastReading = HIGH;
unsigned long buttonLastChangeMs = 0;

#line 17 "C:\\Users\\denko\\Gemini\\Antigravity\\DVPE_Daisy-Visual-Programming-Environment\\DaisyExamples\\MyProjects\\_projects\\Seed_MIDI_Grainlet\\teensy_controller\\Seed_MIDI_Grainlet_Teensy40\\Seed_MIDI_Grainlet_Teensy40.ino"
uint8_t toMidi7Bit(int rawValue);
#line 23 "C:\\Users\\denko\\Gemini\\Antigravity\\DVPE_Daisy-Visual-Programming-Environment\\DaisyExamples\\MyProjects\\_projects\\Seed_MIDI_Grainlet\\teensy_controller\\Seed_MIDI_Grainlet_Teensy40\\Seed_MIDI_Grainlet_Teensy40.ino"
void sendMidi3(uint8_t status, uint8_t data1, uint8_t data2);
#line 30 "C:\\Users\\denko\\Gemini\\Antigravity\\DVPE_Daisy-Visual-Programming-Environment\\DaisyExamples\\MyProjects\\_projects\\Seed_MIDI_Grainlet\\teensy_controller\\Seed_MIDI_Grainlet_Teensy40\\Seed_MIDI_Grainlet_Teensy40.ino"
void sendControlChange(uint8_t control, uint8_t value);
#line 37 "C:\\Users\\denko\\Gemini\\Antigravity\\DVPE_Daisy-Visual-Programming-Environment\\DaisyExamples\\MyProjects\\_projects\\Seed_MIDI_Grainlet\\teensy_controller\\Seed_MIDI_Grainlet_Teensy40\\Seed_MIDI_Grainlet_Teensy40.ino"
void sendNoteOn(uint8_t note, uint8_t velocity);
#line 44 "C:\\Users\\denko\\Gemini\\Antigravity\\DVPE_Daisy-Visual-Programming-Environment\\DaisyExamples\\MyProjects\\_projects\\Seed_MIDI_Grainlet\\teensy_controller\\Seed_MIDI_Grainlet_Teensy40\\Seed_MIDI_Grainlet_Teensy40.ino"
void sendNoteOff(uint8_t note);
#line 51 "C:\\Users\\denko\\Gemini\\Antigravity\\DVPE_Daisy-Visual-Programming-Environment\\DaisyExamples\\MyProjects\\_projects\\Seed_MIDI_Grainlet\\teensy_controller\\Seed_MIDI_Grainlet_Teensy40\\Seed_MIDI_Grainlet_Teensy40.ino"
void updatePots(bool forceSend);
#line 65 "C:\\Users\\denko\\Gemini\\Antigravity\\DVPE_Daisy-Visual-Programming-Environment\\DaisyExamples\\MyProjects\\_projects\\Seed_MIDI_Grainlet\\teensy_controller\\Seed_MIDI_Grainlet_Teensy40\\Seed_MIDI_Grainlet_Teensy40.ino"
void updateButton();
#line 89 "C:\\Users\\denko\\Gemini\\Antigravity\\DVPE_Daisy-Visual-Programming-Environment\\DaisyExamples\\MyProjects\\_projects\\Seed_MIDI_Grainlet\\teensy_controller\\Seed_MIDI_Grainlet_Teensy40\\Seed_MIDI_Grainlet_Teensy40.ino"
void setup();
#line 106 "C:\\Users\\denko\\Gemini\\Antigravity\\DVPE_Daisy-Visual-Programming-Environment\\DaisyExamples\\MyProjects\\_projects\\Seed_MIDI_Grainlet\\teensy_controller\\Seed_MIDI_Grainlet_Teensy40\\Seed_MIDI_Grainlet_Teensy40.ino"
void loop();
#line 17 "C:\\Users\\denko\\Gemini\\Antigravity\\DVPE_Daisy-Visual-Programming-Environment\\DaisyExamples\\MyProjects\\_projects\\Seed_MIDI_Grainlet\\teensy_controller\\Seed_MIDI_Grainlet_Teensy40\\Seed_MIDI_Grainlet_Teensy40.ino"
uint8_t toMidi7Bit(int rawValue)
{
  rawValue = constrain(rawValue, 0, 1023);
  return static_cast<uint8_t>(rawValue >> 3);
}

void sendMidi3(uint8_t status, uint8_t data1, uint8_t data2)
{
  Serial1.write(status);
  Serial1.write(data1);
  Serial1.write(data2);
}

void sendControlChange(uint8_t control, uint8_t value)
{
  sendMidi3(static_cast<uint8_t>(0xB0 | ((kMidiChannel - 1) & 0x0F)),
            control,
            value);
}

void sendNoteOn(uint8_t note, uint8_t velocity)
{
  sendMidi3(static_cast<uint8_t>(0x90 | ((kMidiChannel - 1) & 0x0F)),
            note,
            velocity);
}

void sendNoteOff(uint8_t note)
{
  sendMidi3(static_cast<uint8_t>(0x80 | ((kMidiChannel - 1) & 0x0F)),
            note,
            0);
}

void updatePots(bool forceSend)
{
  for(uint8_t i = 0; i < kPotCount; ++i)
  {
    const int     rawValue  = analogRead(kPotPins[i]);
    const uint8_t midiValue = toMidi7Bit(rawValue);
    if(forceSend || abs(midiValue - lastCcValues[i]) > kCcThreshold)
    {
      sendControlChange(kCcNumbers[i], midiValue);
      lastCcValues[i] = midiValue;
    }
  }
}

void updateButton()
{
  const bool reading = digitalRead(kButtonPin);
  if(reading != buttonLastReading)
  {
    buttonLastChangeMs = millis();
    buttonLastReading  = reading;
  }

  if((millis() - buttonLastChangeMs) > kDebounceMs
     && reading != buttonStableState)
  {
    buttonStableState = reading;
    if(buttonStableState == LOW)
    {
      sendNoteOn(kNoteNumber, kNoteVelocity);
    }
    else
    {
      sendNoteOff(kNoteNumber);
    }
  }
}

void setup()
{
  analogReadResolution(10);

  for(uint8_t i = 0; i < kPotCount; ++i)
  {
    pinMode(kPotPins[i], INPUT);
  }

  pinMode(kButtonPin, INPUT_PULLUP);

  Serial1.begin(31250);
  delay(50);

  updatePots(true);
}

void loop()
{
  updatePots(false);
  updateButton();
  delay(2);
}

