/* Teensy 4.0 with Audio Shield 
   FM-SimpleTest
   Brent Edstrom, 2023

   A simple test to demonstrate the interaction between 
   modulator and carrier. Connect two rotary encoders to
   pins 2, 3 and 5, 6 of a Teensy 4. The demo
   assumes audio output via the PJRC audio shield. 
 */

#include <Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Added by Audio System Design Tool
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformModulated modulatorOsc;           //xy=128.8235626220703,245.00006103515625
AudioSynthWaveformModulated carrierOsc;           //xy=339.82359313964844,251.00003051757812
AudioMixer4              mainMixer;      //xy=543.8235626220703,271.0000457763672
AudioOutputI2S           i2s1;           //xy=763.8235626220703,275.0000457763672
AudioConnection          patchCord1(modulatorOsc, 0, carrierOsc, 0);
AudioConnection          patchCord2(carrierOsc, 0, mainMixer, 0);
AudioConnection          patchCord3(mainMixer, 0, i2s1, 0);
AudioConnection          patchCord4(mainMixer, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=774.8235626220703,365.0000762939453
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
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.5);

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
