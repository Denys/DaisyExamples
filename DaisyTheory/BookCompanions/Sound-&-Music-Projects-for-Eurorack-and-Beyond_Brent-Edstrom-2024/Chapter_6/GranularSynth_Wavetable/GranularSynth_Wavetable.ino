/*
 *  Simple Granular Synthesizer
 *  Brent Edstrom, 2020
 */

#include <Bounce.h>
#include <Encoder.h>
#include <Adafruit_GFX.h>
//Use Tools...Manage Library option to install Adafruit SD1306 library
#include <Adafruit_SSD1306.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWavetable      wavetable;      //xy=119,125
AudioEffectGranular      granular1;      //xy=312,125
AudioMixer4              mixer;          //xy=528,146
AudioOutputI2S           i2s1;           //xy=737,141
AudioConnection          patchCord1(wavetable, granular1);
AudioConnection          patchCord2(granular1, 0, mixer, 0);
AudioConnection          patchCord3(mixer, 0, i2s1, 0);
AudioConnection          patchCord4(mixer, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=733,229
// GUItool: end automatically generated code

//Note: Only room for one sample set on Teensy 3.2
//#include "strings_samples.h"
#include "loop_samples.h"


#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

//Set up push buttons
Bounce beginFreezeBtn = Bounce(1, 15);
Bounce beginPitchShiftBtn = Bounce(4, 15);

//Set up encoders
Encoder lengthEncoder(2, 3);
Encoder speedEncoder(5, 6);

bool updateDisplay = true;
byte last_note = 0;

//Memory for granular processing
#define GRANULAR_MEMORY_SIZE 12800  // enough for 290 ms at 44.1 kHz
int16_t granularMemory[GRANULAR_MEMORY_SIZE];

//Global granular variables 
float grainSpeed = 1.0;   //Default to normal speed
long  grainLength = 250;  //1/4 second loop
bool  freezing = false;
bool  speeding = false;

void setup() 
{
    
  AudioMemory(20);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);
  mixer.gain(0, 0.7);

  //Let circutry stabilize and initialize OLED display
  delay(500);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  display.setTextSize(1);
  //This color mode overwrites text
  display.setTextColor(WHITE, BLACK);

   pinMode(1, INPUT_PULLUP);
   pinMode(4, INPUT_PULLUP); 

   wavetable.setInstrument(my_loop);
  
  //The Granular effect requires memory to operate
  granular1.begin(granularMemory, GRANULAR_MEMORY_SIZE);
 
  //MIDI Callback setup
  usbMIDI.setHandleNoteOff(onNoteOff);
  usbMIDI.setHandleNoteOn(onNoteOn);

}

void loop() 
{
    while (usbMIDI.read()) {
    }

    handleLengthEncoder();
    handleSpeedEncoder();

    handleFreezeBtn();
    handlePitchShiftBtn();

    //Update the display if necessary
    if(updateDisplay)
    {
        drawDisplay();
    }
}

void handleFreezeBtn()
{
    //Track the freeze button
    beginFreezeBtn.update();
    if (beginFreezeBtn.fallingEdge()) 
    {
        if(!freezing)
        {
            granular1.beginFreeze(grainLength);
            freezing = true;
        }else{
            granular1.stop();
            freezing = false; 
        }
        updateDisplay = true;
    }
}

void handlePitchShiftBtn()
{
    //Track the speed button
    beginPitchShiftBtn.update();
    if (beginPitchShiftBtn.fallingEdge()) 
    {
        if(!speeding)
        {
            float temp_grain_length = grainLength;
            if(temp_grain_length > (GRANULAR_MEMORY_SIZE -1) /3)
            {
                 temp_grain_length = (GRANULAR_MEMORY_SIZE -1) / 3;
            }
            granular1.beginPitchShift(temp_grain_length);
            speeding = true;
        }else{
            granular1.stop();
            speeding = false;
        }
        updateDisplay = true;
    }
}


void handleSpeedEncoder()
{
    static int last_position = 0;
    
    int pos = speedEncoder.read();
    if(pos == last_position)
    {
        return; //no need to continue
    }

    int difference = pos - last_position;
    last_position = pos;

    grainSpeed += difference * 0.02;
    if(grainSpeed > 8.0)
    {
        grainSpeed = 8.0;
    }
    if(grainSpeed < 0.125)
    {
        grainSpeed = 0.125;
    }

    granular1.setSpeed(grainSpeed);

    //Value changed...update the display
    updateDisplay = true;
}

void handleLengthEncoder()
{
    static int last_position = 0;
    
    int pos = lengthEncoder.read();
    if(pos == last_position)
    {
        return; //no need to continue
    }

    int difference = pos - last_position;
    last_position = pos;

    //increment by 1/100th of a second
    grainLength += difference * 5; 

    if(grainLength > 290 /* cap at 290 ms as per array size*/)
    {
        grainLength = 290;
    }
    if(grainLength < 1)
    {
        grainLength = 1;
    }

    //Comment this section to prohibit a re-start of freezing and shifting 
    //as length changes
    if(freezing)
    {
        granular1.beginFreeze(grainLength);
    }
    if(speeding)
    {
        granular1.beginPitchShift(grainLength);
    }

    //Value changed...update the display
    updateDisplay = true;
}

void drawDisplay()
{
    display.clearDisplay();

    display.setCursor(0,0);
    display.print("Granular Synth");
    printValue("GRN: ", grainLength, 0, 10, 25, 10);
    printValue("SPD: ", grainSpeed, 58, 10, 83, 10);
    printValue("FR?: ", freezing, 0, 20, 25, 20);
    printValue("SP?: ", speeding, 58, 20, 83, 20);
    
    
    display.display();
    updateDisplay = false; 
}

void printValue(String label, float value, int x1, int y1, int x2, int y2)
{
    static char buffer[10];
    
    display.setCursor(x1,y1);
    display.print(label);
    
    sprintf(buffer, "%.2f", value);
    display.setCursor(x2, y2);
    display.print(buffer);
}

void printValue(String label, long value, int x1, int y1, int x2, int y2)
{
    static char buffer[10];
    
    display.setCursor(x1,y1);
    display.print(label);
    
    sprintf(buffer, "%.3ld", value);
    display.setCursor(x2, y2);
    display.print(buffer);
}

void printValue(String label, bool value, int x1, int y1, int x2, int y2)
{   
    display.setCursor(x1,y1);
    display.print(label);
 
    display.setCursor(x2, y2);
    if(value)
    {
      display.print("yes");
    }else{
      display.print("no");
    }
}

void onNoteOn(byte channel, byte note, byte velocity) 
{
    wavetable.playNote(note, velocity);
    last_note = note;
}

void onNoteOff(byte channel, byte note, byte velocity) 
{
    wavetable.stop();
}
