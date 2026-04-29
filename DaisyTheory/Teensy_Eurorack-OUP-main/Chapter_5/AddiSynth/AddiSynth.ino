/*
  AddiSynth-simple
  Brent Edstrom, 2024

  Description: 
  Additive synthesizer for Teensy 4 with 
  Audio shield.
  Interface consist of 4 switches/rotary encoders and
  OLED display. 
    
   Project pin assignments (first pin = switch)
   Encoder 1: 1, 2, 3
   Encoder 2: 4, 5, 6
   Encoder 3: 9, 14, 15
   Encoder 4: 16, 17, 22

    OLED Pin      Teensy 4 Pin
    VCC           3.3V
    GND           GND
    SCL           19
    SDA           18

*/

#include <Encoder.h>
#include <SPI.h>
#include <Bounce.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=150.1999969482422,91
AudioSynthWaveformSine   sine2;          //xy=150.1999969482422,201
AudioSynthWaveform       waveform2;      //xy=150.1999969482422,250
AudioSynthWaveformSine   sine3;          //xy=150.1999969482422,322
AudioSynthWaveform       waveform3;      //xy=150.1999969482422,371
AudioSynthWaveform       waveform1;      //xy=151.1999969482422,139
AudioSynthWaveformSine   sine4;          //xy=150.1999969482422,442
AudioSynthWaveform       waveform4;      //xy=150.1999969482422,501
AudioEffectMultiply      multiply1;      //xy=314.1999969482422,104
AudioEffectMultiply      multiply4;      //xy=315.1999969482422,451
AudioEffectMultiply      multiply2;      //xy=319.1999969482422,210
AudioEffectMultiply      multiply3;      //xy=321.1999969482422,331
AudioMixer4              mixer1;         //xy=584.1999969482422,170
AudioEffectEnvelope      envelope1;      //xy=774.2000274658203,154.1999969482422
AudioOutputI2S           i2s1;           //xy=958.1999969482422,148
AudioConnection          patchCord1(sine1, 0, multiply1, 0);
AudioConnection          patchCord2(sine2, 0, multiply2, 0);
AudioConnection          patchCord3(waveform2, 0, multiply2, 1);
AudioConnection          patchCord4(sine3, 0, multiply3, 0);
AudioConnection          patchCord5(waveform3, 0, multiply3, 1);
AudioConnection          patchCord6(waveform1, 0, multiply1, 1);
AudioConnection          patchCord7(sine4, 0, multiply4, 0);
AudioConnection          patchCord8(waveform4, 0, multiply4, 1);
AudioConnection          patchCord9(multiply1, 0, mixer1, 0);
AudioConnection          patchCord10(multiply4, 0, mixer1, 3);
AudioConnection          patchCord11(multiply2, 0, mixer1, 1);
AudioConnection          patchCord12(multiply3, 0, mixer1, 2);
AudioConnection          patchCord13(mixer1, envelope1);
AudioConnection          patchCord14(envelope1, 0, i2s1, 0);
AudioConnection          patchCord15(envelope1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=772.1999969482422,319
// GUItool: end automatically generated code


//Set up OLED
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//Set up push buttons
Bounce multiplierBtn = Bounce(1, 15);
Bounce lfoModBtn = Bounce(4, 15);
Bounce amplitudeBtn = Bounce(9, 15);
Bounce lfoAmpBtn = Bounce(16, 22);

//Set up encoders
Encoder encoder1(2, 3);
Encoder encoder2(5, 6);
Encoder encoder3(14, 15);
Encoder encoder4(17, 22);

//Initialize an array to store encoder positions
long lastPosition[] = {0, 0, 0, 0};

float fundamental_frequency = 100;

//A structure to represent a harmonic
struct Harmonic
{
    float multiplier;
    float amplitude;
    float modRate;
    float lfoAmp;
};

const int num_harmonics = 4;
//Create an array of harmonics
Harmonic harmonic[num_harmonics];

//Menu options
enum{multiplier_mode, amplitude_mode, lfo_mod_mode, lfo_amp_mode};
int currentMode = multiplier_mode;

//Create arrays to point to AudioSynthWaveformSine 
//and AudioSynthWaveform objects
AudioSynthWaveformSine *lfo[num_harmonics];
AudioSynthWaveform *wave[num_harmonics];

void setup() 
{
   
  //Call helper functions to set up audio, harmonics, and pointers  
  setupAudioSystem();
  setAudioPointers();
  initializeHarmonics();
  updateHarmonics();

  //Set the envelope to default values
  envelope1.attack(10.5);
  envelope1.sustain(1.0);
  envelope1.release(300);

  //Set up internal pullup for switches
  pinMode(1, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP); 
  pinMode(9, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);

  //MIDI Callback setup
  usbMIDI.setHandleNoteOff(onNoteOff);
  usbMIDI.setHandleNoteOn(onNoteOn);

  //Configure the display and update the display
  setupDisplay();
  updateDisplay();
}

void loop() 
{
    //track MIDI input
    while (usbMIDI.read()) {
    }

    trackButtons();
    trackEncoders();
}

void trackButtons()
{
    //Track the buttons
    handleButton(multiplierBtn, multiplier_mode);
    handleButton(amplitudeBtn, amplitude_mode);
    handleButton(lfoModBtn, lfo_mod_mode);
    handleButton(lfoAmpBtn, lfo_amp_mode);
}

void handleButton(Bounce &rBtn, int mode)
{
    rBtn.update();
    if(rBtn.fallingEdge())
    {
        currentMode = mode;
        updateDisplay();
    }
}

void trackEncoders()
{
    //Track the encoders
    handleEncoder(encoder1, &harmonic[0], lastPosition[0], 0);
    handleEncoder(encoder2, &harmonic[1], lastPosition[1], 1);
    handleEncoder(encoder3, &harmonic[2], lastPosition[2], 2);
    handleEncoder(encoder4, &harmonic[3], lastPosition[3], 3);
}

void handleEncoder(Encoder &rEncoder, Harmonic *pHarmonic, long &last_position, int index)
{
    long pos = rEncoder.read();
    
    if(pos == last_position || pHarmonic == NULL)
    {
         //No need to continue
         return;
    }

    //Calculate the difference: 
    long difference = pos - last_position;
    last_position = pos;

    if(currentMode == multiplier_mode)
    {
        pHarmonic->multiplier += difference * 0.01;
        if(pHarmonic->multiplier <0)
          pHarmonic->multiplier = 0;
    }

    if(currentMode == amplitude_mode)
    {
        pHarmonic->amplitude += difference * 0.01;
        if(pHarmonic->amplitude < 0)
          pHarmonic->amplitude = 0;
         
    }

    if(currentMode == lfo_mod_mode)
    {
        pHarmonic->modRate += difference * 0.01;
        if(pHarmonic->modRate < 0)
          pHarmonic->modRate = 0;
    }
    
    if(currentMode == lfo_amp_mode)
    {
        pHarmonic->lfoAmp += difference * 0.01;
        if(pHarmonic->lfoAmp < 0)
          pHarmonic->lfoAmp = 0;
    }

    updateDisplay();
    updateHarmonics();
}


void updateDisplay()
{
    display.clearDisplay();
    switch(currentMode)
    {
        case multiplier_mode: drawFreqScreen();      break; 
        case amplitude_mode:  drawAmplitudeScreen(); break;
        case lfo_mod_mode:    drawLFOModScreen();    break;
        case lfo_amp_mode:    drawLFOAmpScreen();    break;
    }
    display.display();
}

void printValue(String label, float value, int x1, int y1, int x2, int y2)
{  
    display.setCursor(x1,y1);
    display.print(label);
    
    //Format the value as a string
    String val = String(value, 2);
    display.setCursor(x2, y2);
    display.print(val);
}

void printValue(String label, int value, int x1, int y1, int x2, int y2)
{   
    display.setCursor(x1,y1);
    display.print(label);
    
    //Format the value as a string
    String val = String(value, 3);
    display.setCursor(x2, y2);
    display.print(val);
}

void drawFreqScreen()
{
    display.setCursor(0,0);
    display.print("Frequency Multiplier");
    printValue("OS1: ", harmonic[0].multiplier, 0, 10, 25, 10);
    printValue("OS2: ", harmonic[1].multiplier, 58, 10, 83, 10);
    printValue("OS3: ", harmonic[2].multiplier, 0, 20, 25, 20);
    printValue("OS4: ", harmonic[3].multiplier, 58, 20, 83, 20);
  
}

void drawAmplitudeScreen()
{
    display.setCursor(0,0);
    display.print("OSC: Amplitude");
    printValue("OS1: ", harmonic[0].amplitude, 0, 10, 25, 10);
    printValue("OS2: ", harmonic[1].amplitude, 58, 10, 83, 10);
    printValue("OS3: ", harmonic[2].amplitude, 0, 20, 25, 20);
    printValue("OS4: ", harmonic[3].amplitude, 58, 20, 83, 20);
}
void drawLFOModScreen()
{
    display.setCursor(0,0);
    display.print("LFO: Mod. rate");
    printValue("OS1: ", harmonic[0].modRate, 0, 10, 25, 10);
    printValue("OS2: ", harmonic[1].modRate, 58, 10, 83, 10);
    printValue("OS3: ", harmonic[2].modRate, 0, 20, 25, 20);
    printValue("OS4: ", harmonic[3].modRate, 58, 20, 83, 20);
}

void drawLFOAmpScreen()
{
    display.setCursor(0,0);
    display.print("LFO: Amplitude");
    printValue("OS1: ", harmonic[0].lfoAmp, 0, 10, 25, 10);
    printValue("OS2: ", harmonic[1].lfoAmp, 58, 10, 83, 10);
    printValue("OS3: ", harmonic[2].lfoAmp, 0, 20, 25, 20);
    printValue("OS4: ", harmonic[3].lfoAmp, 58, 20, 83, 20);
}
 

void onNoteOn(byte channel, byte note, byte velocity) 
{
    if(velocity == 0)
    {
        onNoteOff(channel, note, velocity);
        return;
    }
    //Convert MIDI note to frequency
    fundamental_frequency = 440.0 * pow (2.0, (note-69) / 12.0);
    updateHarmonics();
    envelope1.noteOn();
}

void onNoteOff(byte channel, byte note, byte velocity) 
{
    envelope1.noteOff();
}

void setupAudioSystem()
{
  //Configure the audio system
  AudioMemory(20);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.7);
  
  //Set each oscillator to 25% gain
  mixer1.gain(0, 0.25);
  mixer1.gain(1, 0.25);
  mixer1.gain(2, 0.25);
  mixer1.gain(3, 0.25); 
}

void setAudioPointers()
{
  //Point to sine objects
  lfo[0] = &sine1;
  lfo[1] = &sine2;
  lfo[2] = &sine3;
  lfo[3] = &sine4;

  //Point to waveform object
  wave[0] = &waveform1;
  wave[1] = &waveform2;
  wave[2] = &waveform3;
  wave[3] = &waveform4;
}

void setupDisplay()
{
     //Let circutry stabilize and initialize OLED display
    delay(500);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
    display.setTextSize(1);
    //This color mode overwrites text
    display.setTextColor(WHITE, BLACK);
}

void updateHarmonics()
{
    for(int i = 0; i < num_harmonics; i++)
    {
      lfo[i]->amplitude(harmonic[i].lfoAmp);
      lfo[i]->frequency(harmonic[i].modRate);
      wave[i]->amplitude(harmonic[i].amplitude);
      wave[i]->frequency(harmonic[i].multiplier * fundamental_frequency);
    }
}

void initializeHarmonics()
{
  harmonic[0].multiplier = 1.0;
  harmonic[0].amplitude = 0.7;
  harmonic[0].modRate = 0.03;
  harmonic[0].lfoAmp = 0.5;

  harmonic[1].multiplier = 2.0;
  harmonic[1].amplitude = 0.5;
  harmonic[1].modRate = 0.01;
  harmonic[1].lfoAmp = 0.3;

  harmonic[2].multiplier = 2.5;
  harmonic[2].amplitude = 0.3;
  harmonic[2].modRate = 0.04;
  harmonic[2].lfoAmp = 0.4;

  harmonic[3].multiplier = 3;
  harmonic[3].amplitude = 0.2;
  harmonic[3].modRate = 0.09;
  harmonic[3].lfoAmp = 0.2;
}

