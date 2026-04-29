/*
 * FM Synthesizer
 * Version: FMSynth5
 * Brent Edstrom, 2020
 * 
 * Notes: 
 * Code is complete except for optional features including mode (MIDI input channel,
 * store/retrieve, and (possibly) CV/MIDI input selection.
 * Menu selection and place-holder drawing functions have been added,
 * so code for MIDI channel selection and storage/load are all that 
 * remains to be done.
 * 
 * Screen re-draws are still a tad slow and some slight MIDI timing delay  
 * can be heard when adjusting parameters. See if more optimization of
 * screen drawing can be done. Timing is tight without any screen re-draws
 * but "overwrite" screen redraws are an improvement. 
 * 
 * Optional: 
 * Consider waveform selection option (via algorithm button 3)
 * Add polyphony using an array of FMVoice objects (see polysynth example)
 * Consider changing MIDI input from USB to 5-pin DIN
 * Consider an envelope for output (or each operator)
 * 
 */


#include <Encoder.h>
#include <Bounce.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "FMVoice.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformModulated OP1C;           //xy=246.8235321044922,385
AudioSynthWaveformModulated OP1D;           //xy=250.8235321044922,527
AudioSynthWaveformModulated OP1B;           //xy=252.8235321044922,262
AudioSynthWaveformModulated OP1A;           //xy=258.8235321044922,147
AudioMixer4              OPMIX1A;        //xy=471.8235321044922,164
AudioMixer4              OPMIX1C;        //xy=472.8235321044922,392
AudioMixer4              OPMIX1B;        //xy=473.8235321044922,270
AudioMixer4              OPMIX1D;        //xy=472.8235321044922,521
AudioMixer4              VOICE1MIXER;    //xy=777.8235321044922,283
AudioEffectEnvelope      voice1Envelope;      //xy=971.8235626220703,281.82359313964844
AudioOutputI2S           i2s1;           //xy=1182.8234405517578,281.0000762939453
AudioConnection          patchCord1(OP1C, 0, OPMIX1A, 2);
AudioConnection          patchCord2(OP1C, 0, OPMIX1B, 2);
AudioConnection          patchCord3(OP1C, 0, OPMIX1C, 2);
AudioConnection          patchCord4(OP1C, 0, OPMIX1D, 2);
AudioConnection          patchCord5(OP1C, 0, VOICE1MIXER, 2);
AudioConnection          patchCord6(OP1D, 0, OPMIX1A, 3);
AudioConnection          patchCord7(OP1D, 0, OPMIX1B, 3);
AudioConnection          patchCord8(OP1D, 0, OPMIX1C, 3);
AudioConnection          patchCord9(OP1D, 0, OPMIX1D, 3);
AudioConnection          patchCord10(OP1D, 0, VOICE1MIXER, 3);
AudioConnection          patchCord11(OP1B, 0, OPMIX1A, 1);
AudioConnection          patchCord12(OP1B, 0, OPMIX1B, 1);
AudioConnection          patchCord13(OP1B, 0, OPMIX1C, 1);
AudioConnection          patchCord14(OP1B, 0, OPMIX1D, 1);
AudioConnection          patchCord15(OP1B, 0, VOICE1MIXER, 1);
AudioConnection          patchCord16(OP1A, 0, OPMIX1A, 0);
AudioConnection          patchCord17(OP1A, 0, OPMIX1B, 0);
AudioConnection          patchCord18(OP1A, 0, OPMIX1C, 0);
AudioConnection          patchCord19(OP1A, 0, OPMIX1D, 0);
AudioConnection          patchCord20(OP1A, 0, VOICE1MIXER, 0);
AudioConnection          patchCord21(OPMIX1A, 0, OP1A, 0);
AudioConnection          patchCord22(OPMIX1C, 0, OP1C, 0);
AudioConnection          patchCord23(OPMIX1B, 0, OP1B, 0);
AudioConnection          patchCord24(OPMIX1D, 0, OP1D, 0);
AudioConnection          patchCord25(VOICE1MIXER, voice1Envelope);
AudioConnection          patchCord26(voice1Envelope, 0, i2s1, 0);
AudioConnection          patchCord27(voice1Envelope, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=1177.8234405517578,360.0000457763672
// GUItool: end automatically generated code


#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

bool updateDisplay = true;

/*  Teensy 4: Available pins with Audio Board
 *  0, 1, 2, 3, 4, 5, 6 (SPI memory), 9, 10 (SPI SD card)
 *  14, 15, 16, 17, 22
 *  
 *  Project pin assignments (first pin = switch)
 *  Encoder 1: 1, 2, 3
 *  Encoder 2: 4, 5, 6
 *  Encoder 3: 9, 14, 15
 *  Encoder 4: 16, 17, 22
 */

//PIN ASSIGNMENTS FOR TEENSY 4 W/AUDIO SHIELD
//Set up push buttons
Bounce btn1 = Bounce(1, 15);  //Tuning
Bounce btn2 = Bounce(4, 15);  //Algorithm
Bounce btn3 = Bounce(9, 15);  //Levels 
Bounce btn4 = Bounce(16, 15); //Utility


//Set up encoders
Encoder encoder1(2, 3);
Encoder encoder2(5, 6);
Encoder encoder3(14, 15);
Encoder encoder4(17, 22);

//Menu modes
enum{algorithm_mode, coarse_tuning_mode, fine_tuning_mode,
     operator_level_mode, feedback_level_mode, utility_mode};

int currentMode = algorithm_mode;
int currentAlgorithm = 0;
int numAlgorithms = 11;

FMVoice voice1;

/*
 *  Revised 6/22: 
 *  pass last position as a reference
 *  instead of using static function variables
 */

//Encoder positions
long lastPosition[4];

void setup() 
{
    //OLED setup
    //Let circutry stabilize and initialize OLED display
    delay(100);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
    display.setTextSize(1);
    //This color mode overwrites text
    display.setTextColor(WHITE, BLACK);

    //Teensy audio setup
    AudioMemory(40);
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.7);
    delay(1000);

    //Menu button setup
    pinMode(1, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP); 
    pinMode(9, INPUT_PULLUP);
    pinMode(16, INPUT_PULLUP);

      
    //MIDI Callback setup
    usbMIDI.setHandleNoteOff(onNoteOff);
    usbMIDI.setHandleNoteOn(onNoteOn);

    //Attach objects to FMVoice object
    voice1.attachSines(&OP1A, &OP1B, &OP1C, &OP1D);
    voice1.attachMixers(&OPMIX1A, &OPMIX1B, &OPMIX1C, &OPMIX1D);
    voice1.attachVoiceMixer(&VOICE1MIXER);
    voice1.attachEnvelope(&voice1Envelope);

    //Configure the FMVoice
    voice1.setADSR(20, 10, 0.8, 100);
    voice1.updateAlgorithm(currentAlgorithm);
    voice1.updateOperatorOutputLevel();
    voice1.updateFeedbackLevel();
    voice1.updateTuning();

    //TEST
    onNoteOn(0, 20, 120);
}

long last_time = 0;

void loop() 
{
    //Note: This method can also read specific channels
    while (usbMIDI.read()) {
    }
    
    trackTuningBtn();
    trackAlgorithmBtn();
    trackLevelBtn();
    trackUtilityBtn();

    handleEncoder(&encoder1, &voice1.operators[0], lastPosition[0]);
    handleEncoder(&encoder2, &voice1.operators[1], lastPosition[1]);
    handleEncoder(&encoder3, &voice1.operators[2], lastPosition[2]);
    handleEncoder(&encoder4, &voice1.operators[3], lastPosition[3]);

    if(updateDisplay == true)
    {
        drawDisplay();
        updateDisplay = false;
        last_time = millis();
    }
    
}


void trackTuningBtn()
{
   static int tuning_mode = coarse_tuning_mode;
   btn1.update();
   if(btn1.fallingEdge())
   {
      //Only toggle if we are already in a tuning mode
      if(currentMode == coarse_tuning_mode || 
         currentMode == fine_tuning_mode)
      {
          //Toggle between modes
          if(tuning_mode == coarse_tuning_mode)
          {
             tuning_mode = fine_tuning_mode;
          }else if(tuning_mode == fine_tuning_mode)
          {
             tuning_mode = coarse_tuning_mode;
          }
      }
      currentMode = tuning_mode;
      updateDisplay = true;
   }
}

void trackAlgorithmBtn()
{
  btn2.update();
  if(btn2.fallingEdge())
  {
    currentMode = algorithm_mode;
    updateDisplay = true;
  }
}

void trackLevelBtn()
{
   static int level_mode = operator_level_mode;
   btn3.update();
   if(btn3.fallingEdge())
   {
      //Only toggle if we are already in a level adjustment mode
      if(currentMode == feedback_level_mode || 
         currentMode == operator_level_mode)
      {
          //Toggle between modes
          if(level_mode == feedback_level_mode)
          {
             level_mode = operator_level_mode;
          }else if(currentMode == operator_level_mode)
          {
             level_mode = feedback_level_mode;
          }
    
      }
      currentMode = level_mode;
      updateDisplay = true;
   }
}


void trackUtilityBtn()
{
   btn4.update();
   if(btn4.fallingEdge())
   {
      //Only toggle if we are in one of the utility modes
      if(currentMode == utility_mode)
      {
          //Note: function "stub:" add utility code here
      }else{
          currentMode = utility_mode;
      }
      updateDisplay = true;
   }
}

void handleEncoder(Encoder *pEncoder, FMOperator *pOperator, long &last_position)
{
   
    long pos = pEncoder->read();
    
    if(pos == last_position)
    {
         //No need to continue
         return;
    }

    //Calculate the difference: 
    long difference = pos - last_position;
    last_position = pos;

    //Update the course tuning parameter pointed to by pOperator
    if(currentMode == coarse_tuning_mode)
    {
        
        pOperator->coarseTuning += difference; 
        if(pOperator->coarseTuning < 0)
        {
            pOperator->coarseTuning = 0;
        }
        updateDisplay = true;
        voice1.updateTuning();
    }

    if(currentMode == fine_tuning_mode)
    {
        //Update fine tuning
        pOperator->fineTuning += (float)difference * 0.005; 
        if(pOperator->fineTuning < 0)
        {
            pOperator->fineTuning = 0;
        }
        if(pOperator->fineTuning > 2.0)
        {
            pOperator->fineTuning = 2.0;
        }
        updateDisplay = true;
        voice1.updateTuning();
    }
    
    if(currentMode == operator_level_mode)
    {
        //Update operator level
        pOperator->outputLevel += difference * 0.01; 
        if(pOperator->outputLevel < 0.0)
        {
            pOperator->outputLevel = 0.0;
        }
        if(pOperator->outputLevel > 1.0)
        {
            pOperator->outputLevel = 1.0;
        }
        updateDisplay = true;
        voice1.updateOperatorOutputLevel();
    }

    if(currentMode == feedback_level_mode)
    {
        //Update operator level
        pOperator->feedback += difference * 0.01; 
        if(pOperator->feedback < 0.0)
        {
            pOperator->feedback = 0.0;
        }
        if(pOperator->feedback > 1.0)
        {
            pOperator->feedback = 1.0;
        }
        updateDisplay = true;
        voice1.updateFeedbackLevel();
    }

    if(currentMode == algorithm_mode)
    {
        currentAlgorithm += difference;// * 0.5; 
        if(currentAlgorithm < 0)
        {
            currentAlgorithm = 0;
        }
        if(currentAlgorithm >= numAlgorithms)
        {
            currentAlgorithm = numAlgorithms -1;
        }
        updateDisplay = true;
        voice1.updateAlgorithm(currentAlgorithm);
    }
}

void drawDisplay()
{
    static int lastMode = -1;
    bool redraw = false;

    if(currentMode != lastMode || currentMode == algorithm_mode)
    {
        display.clearDisplay();
        lastMode = currentMode;
        redraw = true; 
    }
    
    switch(currentMode)
    {
       //Program mode
       case coarse_tuning_mode: drawCoarseTuningScreen(); break;
       case fine_tuning_mode: drawFineTuningScreen(redraw); break;
       case operator_level_mode: drawOutputLevelScreen(); break;
       case feedback_level_mode: drawFeedbackLevelScreen(); break;
       case algorithm_mode: drawAlgorithmScreen(); break;
       case utility_mode: drawUtilityScreen(); break;
    }
           
    display.display();
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

void printValue(String label, int value, int x1, int y1, int x2, int y2)
{
    static char buffer[10];
    
    display.setCursor(x1,y1);
    display.print(label);
    
    sprintf(buffer, "%.3i", value);
    display.setCursor(x2, y2);
    display.print(buffer);
}

void drawCoarseTuningScreen()
{
    display.setCursor(0,0);
    display.print("Coarse Tuning");
    printValue("OP1: ", voice1.operators[0].coarseTuning, 0, 10, 25, 10);
    printValue("OP2: ", voice1.operators[1].coarseTuning, 58, 10, 83, 10);
    printValue("OP3: ", voice1.operators[2].coarseTuning, 0, 20, 25, 20);
    printValue("OP4: ", voice1.operators[3].coarseTuning, 58, 20, 83, 20);
}

void drawFineTuningScreen(bool redraw)
{
    display.setCursor(0,0);
    display.print("Fine Tuning");
    printValue("OP1: ", voice1.operators[0].fineTuning, 0, 10, 25, 10);
    printValue("OP2: ", voice1.operators[1].fineTuning, 58, 10, 83, 10);
    printValue("OP3: ", voice1.operators[2].fineTuning, 0, 20, 25, 20);
    printValue("OP4: ", voice1.operators[3].fineTuning, 58, 20, 83, 20);
}

void drawOutputLevelScreen()
{
    display.setCursor(0,0);
    display.print("Output Level");
    printValue("OP1: ", voice1.operators[0].outputLevel, 0, 10, 25, 10);
    printValue("OP2: ", voice1.operators[1].outputLevel, 58, 10, 83, 10);
    printValue("OP3: ", voice1.operators[2].outputLevel, 0, 20, 25, 20);
    printValue("OP4: ", voice1.operators[3].outputLevel, 58, 20, 83, 20);
}

void drawFeedbackLevelScreen()
{
    display.setCursor(0,0);
    display.print("Feedback Level");
    printValue("OP1: ", voice1.operators[0].feedback, 0, 10, 25, 10);
    printValue("OP2: ", voice1.operators[1].feedback, 58, 10, 83, 10);
    printValue("OP3: ", voice1.operators[2].feedback, 0, 20, 25, 20);
    printValue("OP4: ", voice1.operators[3].feedback, 58, 20, 83, 20);
}


void drawUtilityScreen()
{
  display.setCursor(0,0);
  display.print("Utility Mode");
  display.setCursor(0,10);
  display.print("Customize code");
}



/*
 * To Do: I can clean this up by using a 4x3 grid instead
 * of a bunch of magic numbers...
 */

void drawAlgorithmScreen()
{
    display.setCursor(0,0);
    display.print("Al: ");
    display.setCursor(16, 0);
    display.print(currentAlgorithm);
    //TO DO: Consider drawing graphics of the algorithms instead of just a number
    
    int xp = 50;
    int h = 5;
    int w = 15;
    switch(currentAlgorithm)
    {
        case 0: drawAlgorithm(w, h, xp, 0, xp, 8, xp, 16, xp, 24); break;
        case 1: drawAlgorithm(w, h, xp-10, 0, xp+10, 0, xp, 8, xp, 16); break;
        case 2: drawAlgorithm(w, h, xp, 0, xp, 8, xp +20, 8, xp, 16); break;
        case 3: drawAlgorithm(w, h, xp, 0, xp-10, 8, xp+10, 8, xp, 16); break;
        case 4: drawAlgorithm(w, h, xp, 0, xp, 8, xp-10, 16, xp+10, 16); break;
        case 5: drawAlgorithm(w, h, xp+20, 0, xp+20, 8, xp+20, 16, xp, 16); break;
        case 6: drawAlgorithm(w, h, xp-20, 0, xp, 0, xp+20, 0, xp, 8); break;
        case 7: drawAlgorithm(w, h, xp-10, 0, xp+10, 0, xp-10, 8, xp+10, 8); break;
        case 8: drawAlgorithm(w, h, xp, 0, xp-20, 8, xp, 8, xp+20, 8); break;
        case 9: drawAlgorithm(w, h, xp+20, 0, xp-20, 8, xp, 8, xp+20, 8); break;
        case 10: drawAlgorithm(w, h, xp-20, 8, xp, 8, xp+20, 8, xp + 40, 8); break;
    }
}

void drawAlgorithm(int w, int h, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
   
    display.drawRect(x1, y1, w, h, WHITE);
    display.drawRect(x2, y2, w, h, WHITE);
    display.drawRect(x3, y3, w, h, WHITE);
    display.drawRect(x4, y4, w, h, WHITE);
}


//byte lastNoteOnMsg = 0;
void onNoteOn(byte channel, byte note, byte velocity) 
{
  //lastNoteOnMsg = note;
  voice1.lastNoteOn = note;
  voice1.midiNoteOn(note, velocity);
  
}

void onNoteOff(byte channel, byte note, byte velocity) 
{
  if(note != voice1.lastNoteOn)
   {
      return;
   }
   
   voice1.midiNoteOff(note);
}
