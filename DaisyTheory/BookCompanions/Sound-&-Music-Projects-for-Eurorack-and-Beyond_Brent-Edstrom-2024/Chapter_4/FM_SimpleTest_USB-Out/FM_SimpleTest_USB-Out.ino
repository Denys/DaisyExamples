/* Teensy 4.0 with USB Audio output (no audio adapter required) 
   FM-SimpleTest
   Brent Edstrom, 2023

   A simple test to demonstrate the interaction between 
   modulator and carrier. Connect two rotary encoders to
   pins 2, 3 and 5, 6 of a Teensy 4. 
 */

#include <Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformModulated modulatorOsc;   //xy=178.1999969482422,212
AudioSynthWaveformModulated carrierOsc;     //xy=389.1999969482422,218
AudioMixer4              mainMixer;      //xy=593.1999969482422,238
AudioOutputUSB           usb1;           //xy=774.2000274658203,245.20000839233398
AudioConnection          patchCord1(modulatorOsc, 0, carrierOsc, 0);
AudioConnection          patchCord2(carrierOsc, 0, mainMixer, 0);
AudioConnection          patchCord3(mainMixer, 0, usb1, 0);
// GUItool: end automatically generated code



#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

Encoder encoder1(2, 3);
Encoder encoder2(5, 6);

float mod_factor = .5;
float mod_amp = 0.3;
float frequency = 100;
float amplitude = 0.3;

void setup() 
{
    AudioMemory(12);
   
    mainMixer.gain(0, 0.5);

    //OLED setup
    //Let circutry stabilize and initialize OLED display
    delay(100);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
    display.setTextSize(1);
    //This color mode overwrites text
    display.setTextColor(WHITE, BLACK);

    updateScreen();

    carrierOsc.begin(amplitude, frequency, WAVEFORM_SINE);
    modulatorOsc.begin(mod_amp, mod_factor * frequency, WAVEFORM_SINE);

    //MIDI Callback setup
    usbMIDI.setHandleNoteOff(onNoteOff);
    usbMIDI.setHandleNoteOn(onNoteOn);

    //pinMode(2, INPUT_PULLUP);
    //pinMode(3, INPUT_PULLUP);
}

void loop() 
{
   //Note: This method can also read specific channels
    while (usbMIDI.read()) {
    }

   trackFreqEncoder();
   trackAmpEncoder();
   
}

void trackFreqEncoder()
{
    static int last_position = 0;
    int pos = encoder1.read();
    
    if(pos == last_position)
    {
         //No need to continue
         return;
    }

    int difference = pos - last_position;
    last_position = pos;

    mod_factor += difference * 0.01;
    if(mod_factor < 0)
    {
      mod_factor = 0;
    }
    modulatorOsc.frequency(mod_factor * frequency);
    updateScreen();
}

void trackAmpEncoder()
{
    static int last_position = 0;
    int pos = encoder2.read();
    if(pos == last_position)
    {
         //No need to continue
         return;
    }

    int difference = pos - last_position;
    last_position = pos;

    mod_amp += difference * 0.01;
    if(mod_amp < 0.0)
    {
        mod_amp = 0.0;
    } 
    else if(mod_amp > 1.0)
    {
        mod_amp = 1.0;
    }
    modulatorOsc.amplitude(mod_amp);
    updateScreen();
}

void onNoteOn(byte channel, byte note, byte velocity) 
{
  
  //Convert MIDI note to frequency
  frequency = 440.0* pow (2.0, (note-69) / 12.0);
  modulatorOsc.frequency(frequency * mod_factor);
  carrierOsc.frequency(frequency);
  amplitude = (float) velocity / 127.0;
  carrierOsc.amplitude(amplitude);
  
}

void onNoteOff(byte channel, byte note, byte velocity) 
{
  carrierOsc.amplitude(0.0);
}

void updateScreen()
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Mod factor: ");
  display.print(mod_factor);
  display.setCursor(0, 10);
  display.print("Mod amp: ");
  display.print(mod_amp);
  display.display();
}
