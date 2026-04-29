/*
 * EuroSynth (for use with Teensy4 and SMD CV/Gate input backpack)
 * Version: v1.8.2
 * Brent Edstrom, 2023
 *  
 */

#include <Encoder.h>
#include <Bounce.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MCP3202.h>
#include "FMVoice.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformModulated OP1C;           //xy=96.19999694824219,315
AudioSynthWaveformModulated OP1D;           //xy=100.19999694824219,457
AudioSynthWaveformModulated OP1B;           //xy=102.19999694824219,192
AudioSynthWaveformModulated OP1A;           //xy=108.19999694824219,77
AudioMixer4              OPMIX1A;        //xy=321.1999969482422,94
AudioMixer4              OPMIX1C;        //xy=322.1999969482422,322
AudioMixer4              OPMIX1B;        //xy=323.1999969482422,200
AudioMixer4              OPMIX1D;        //xy=322.1999969482422,451
AudioMixer4              VOICE1MIXER;    //xy=627.1999969482422,213
AudioEffectEnvelope      voice1Envelope; //xy=821.1999969482422,211
AudioOutputPT8211        pt8211_1;       //xy=1053.199966430664,214.20001220703125
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
AudioConnection          patchCord26(voice1Envelope, 0, pt8211_1, 0);
AudioConnection          patchCord27(voice1Envelope, 0, pt8211_1, 1);
// GUItool: end automatically generated code

//======= OLED =======
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

bool updateDisplay = true;

//======= ADC =======
const int chip_select = 10;
MCP3202 adc = MCP3202(chip_select);

//Set up push buttons
Bounce sw1 = Bounce(22, 40);      
Bounce sw2 = Bounce(23, 40);      
Bounce sw3 = Bounce(17, 40);      
Bounce sw4 = Bounce(6, 40);      

Encoder encoder1(1, 0);
Encoder encoder2(3, 2);     
Encoder encoder3(5, 4);
Encoder encoder4(9, 8);

//Menu modes
enum{algorithm_mode, coarse_tuning_mode, fine_tuning_mode,
     operator_level_mode, feedback_level_mode, utility_mode};

int currentMode = coarse_tuning_mode;
int currentAlgorithm = 0;
int numAlgorithms = 11;

FMVoice voice1;

bool note_on = false;
bool gate2_on = false;

const float circuit_tuning_constant = 1.0;

const int gatePin1 = 15;
const int gatePin2 = 16;
const int gate_threshold = 2150; //Trigger ~4V with scaling circuit

float cv1_volts = 0;
float cv2_volts = 0;

float basis_frequency = 10.0;   //10Hz

//Revision: Moved static last_position out of encoder handler
//Array to store encoder positions
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

    //Menu button setup
    pinMode(22, INPUT_PULLUP);
    pinMode(23, INPUT_PULLUP); 
    pinMode(17, INPUT_PULLUP);
    pinMode(6, INPUT_PULLUP);
    
    pinMode(4, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);


    //Gate pins (this version uses an analog threshold
    //instead of digital high/low
    //pinMode(gatePin1, INPUT);
    //pinMode(gatePin2, INPUT);
    
    //Start the external ADC (MCP3202)
    adc.begin();

    //Set analog inputs to 12-bit
    analogReadResolution(12);
    
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

    Serial.begin(9600);
}


void loop() 
{   
    //Handle incoming CV
    trackGate1();
    trackGate2();
    trackCV1();
    trackCV2();

    //Handle the menu buttons
    trackTuningBtn();
    trackAlgorithmBtn();
    trackLevelBtn();
    trackUtilityBtn();

    //Handle the four encoders
    handleEncoder(encoder1, &voice1.operators[0], lastPosition[0], 0);
    handleEncoder(encoder2, &voice1.operators[1], lastPosition[1], 1);
    handleEncoder(encoder3, &voice1.operators[2], lastPosition[2], 2);
    handleEncoder(encoder4, &voice1.operators[3], lastPosition[3], 3);

    if(updateDisplay == true)
    {
        drawDisplay();
        updateDisplay = false;
    }  
    
}


void trackTuningBtn()
{
   static int tuning_mode = coarse_tuning_mode;
   sw1.update();
   if(sw1.fallingEdge())
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
  sw2.update();
  if(sw2.fallingEdge())
  {
    currentMode = algorithm_mode;
    updateDisplay = true;
  }
}

void trackLevelBtn()
{
   static int level_mode = operator_level_mode;
   sw3.update();
   if(sw3.fallingEdge())
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
   sw4.update();
   if(sw4.fallingEdge())
   {
      currentMode = utility_mode;
      updateDisplay = true;
      //TO DO: add other utility options e.g., master tune, patch save, etc.
   }
   
}

void handleEncoder(Encoder &rEncoder, FMOperator *pOperator, long &last_position, int index)
{
    long pos = rEncoder.read();
    
    if(pos == last_position)
    {
         //No need to continue
         return;
    }

    //Calculate the difference: 
    long difference = pos - last_position;
    last_position = pos;

    //Update the coarse tuning parameter pointed to by pOperator
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

    if(currentMode == algorithm_mode && index == 1)
    {
        currentAlgorithm += difference; 
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
  display.print("CV/GATE Status");
  printValue("CV1: ", cv1_volts, 1, 10, 25, 10);
  printValue("GT1: ", note_on, 58, 10, 83, 10);
  printValue("CV2: ", cv2_volts, 1, 20, 25, 20);
  printValue("GT2: ", gate2_on, 58, 20, 83, 20);
}



/*
 * To Do: Clean this up by using a 4x3 grid instead
 * of "magic numbers?""
 */

void drawAlgorithmScreen()
{
    display.setCursor(0,0);
    display.print("Al: ");
    display.setCursor(16, 0);
    display.print(currentAlgorithm);
    
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

/*
    Note: 
    Gates can be read with digitalRead() or analogRead().
    digitalRead() will trigger at about 5.1V applied to the scaling 
    circuit but analogRead() can be used to provide a programmable 
    threshold. A threshold of 2150 (12-bit resolution) will trigger 
    at about 4V. The voltage-scaling circuit uses an inverting op-amp, 
    so logic is reversed: <= threshold == on, > threshold == off

*/
void trackGate1()
{
   //=====Track gate 1
   int gate_1 = analogRead(gatePin1);
   if(gate_1 <= gate_threshold)
   {
      if(note_on == false)
      {
          note_on = true;
          voice1Envelope.noteOn();
          if(currentMode == utility_mode)
          {
              updateDisplay = true;
          }
      }
   } else if (gate_1 > gate_threshold)
   {
      if(note_on == true)
      {
          note_on = false;
          //Note: optionally set releaseNoteOn() to reduce clicks or pops.
          voice1Envelope.noteOff();
          if(currentMode == utility_mode)
          {
              updateDisplay = true;
          }
      }
   }
}

void trackGate2()
{
   
  //TO DO: Add code for algorithm selection, second voice, etc.
  
  //=====Track gate 2
   int gate_2 = analogRead(gatePin2);
   if(gate_2 <= gate_threshold)
   {
      if(gate2_on == false)
      {
          gate2_on = true;
          if(currentMode == utility_mode)
          {
              updateDisplay = true;
          }
      }
   } else if (gate_2 > gate_threshold)
   {
      if(gate2_on == true)
      {
          gate2_on = false;
          if(currentMode == utility_mode)
          {
              updateDisplay = true;
          }
      }
   }
   
}

void trackCV1()
{
    static unsigned long last_input;

    //Read CV1
    unsigned long val = adc.readChannel(0);
    int window = 3;
    if(val < last_input - window || val > last_input + window)
    {
        last_input = val;
        float volts3V = convert12BitTo3V((float)val);
        float volts10V = convert3Vto10V(volts3V);
        float freq = basis_frequency * pow(2.0, volts10V);
        voice1.setFundamentalFrequency(freq);

        if(currentMode == utility_mode)
        {
            updateDisplay = true;
            cv1_volts = volts10V;
        }
    }
}

void trackCV2()
{
    static unsigned long last_input;

    //Read CV1
    unsigned long val = adc.readChannel(1);
    int window = 3;
    if(val < last_input - window || val > last_input + window)
    {
        last_input = val;
        float volts3V = convert12BitTo3V((float)val);
        float volts10V = convert3Vto10V(volts3V);
        //TO DO: Add code for second voice, velocity scaling, etc.

        if(currentMode == utility_mode)
        {
            updateDisplay = true;
            cv2_volts = volts10V;
        }
    }
}

float convert12BitTo3V(float value)
{
    return (value / 4096.0) * 3.3;
}

float convert3Vto10V(float v_out_3v)
{
    /* Using 33k feedback resistor, 100k input resistor, 
     *  and 140k bias resistor with -10V reference
     */
    return ((2.357 -  v_out_3v) / 0.33) * circuit_tuning_constant;  
}
