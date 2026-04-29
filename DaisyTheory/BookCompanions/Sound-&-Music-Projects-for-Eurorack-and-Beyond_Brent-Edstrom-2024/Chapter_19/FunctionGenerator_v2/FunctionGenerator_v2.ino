/*
    Function Generator v1.2
    A test tool for use with Teensy4,  SMD CV/Gate input backpack, 
    and Eurorack interface.
    By Brent Edstrom 8/21/2023

    Usage:
    SW 1: Waveform selection
    SW 2: Frequency selection:
          Encoder 1: ± 1
          Encoder 2: ± 10
          Encoder 3: ± 100
          Encoder 4: ± 1000
    SW 3: Sweep toggle/rate
    SW 4: Operating mode selection: Internal, External CV, Read CV

*/

#include <Encoder.h>
#include <Bounce.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MCP3202.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       vco;      //xy=127,269
AudioSynthWaveformModulated waveform1;   //xy=262,136
AudioOutputPT8211        pt8211_1;       //xy=439,144
AudioConnection          patchCord1(vco, 0, waveform1, 0);
AudioConnection          patchCord2(waveform1, 0, pt8211_1, 0);
AudioConnection          patchCord3(waveform1, 0, pt8211_1, 1);
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

//Waveforms
enum{sine, sawtooth, reverse_sawtooth, square, triangle};

//Operating modes
enum{internal, external_cv, read_cv};

//Edit modes
enum{waveform_selection, frequency_selection, sweep_selection, operating_mode_selection};

//Globals
int waveform = sine;
float amplitude = 0.8;
float frequency = 1000;
int operating_mode = internal;
int edit_mode = waveform_selection;
bool sweep = false;
float lfoRate = 1.0;
int freq_inc_amount = 1;
float input_voltage = 0.0;
float input_frequency = 0.0;
int gate_threshold = 2150;  //This will trigger high ~4V with scaling circuit
const int gatePin1 = 15;
const int gatePin2 = 16;
bool gate1_on = false;
bool gate2_on = false;

float cv1_volts = 0;
float cv2_volts = 0;

float basis_frequency = 10.0;   //10Hz
const float circuit_tuning_constant = 1.0;

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
    AudioMemory(15);
    waveform1.frequency(frequency);
    waveform1.amplitude(amplitude);
    setWaveform(waveform);

    turnLfoOff();   

    //Menu button setup
    pinMode(22, INPUT_PULLUP);
    pinMode(23, INPUT_PULLUP); 
    pinMode(17, INPUT_PULLUP);
    pinMode(6, INPUT_PULLUP);
    
    //To do: why does encoder 3 need pullup?
    pinMode(4, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);

    adc.begin();

    //Set analog inputs to 12-bit
    analogReadResolution(12);

}

void loop() 
{
    trackWaveBtn();
    trackFrequencyBtn();
    trackSweepBtn();
    trackOperatingModeBtn();
    trackGates();

    handleEncoder(encoder1, lastPosition[0], 0);
    handleEncoder(encoder2, lastPosition[1], 1);
    handleEncoder(encoder3, lastPosition[2], 2);
    handleEncoder(encoder4, lastPosition[3], 3);

    trackCV1();
    trackCV2();

    if(updateDisplay == true)
    {
        drawDisplay();
        updateDisplay = false;
    } 

}

void trackWaveBtn()
{
  sw1.update();
  if(sw1.fallingEdge())
  {
      edit_mode = waveform_selection;
      updateDisplay = true;
  }
}

void trackFrequencyBtn()
{
  sw2.update();
  if(sw2.fallingEdge())
  {
      edit_mode = frequency_selection;
      updateDisplay = true;
  }
}

void trackSweepBtn()
{
  sw3.update();
  if(sw3.fallingEdge())
  {
      if(edit_mode == sweep_selection)
      {
          //Toggle sweep 
          if(sweep == true)
          {
            sweep = false;
            turnLfoOff();
          }else{
            sweep = true;
            turnLfoOn();
          }
      }else{
        edit_mode = sweep_selection;
      }
      updateDisplay = true;
  }
}

void trackOperatingModeBtn()
{
  sw4.update();
  if(sw4.fallingEdge())
  {
      if(edit_mode == operating_mode_selection)
      {
          operating_mode++;
          if(operating_mode > read_cv)
          {
              operating_mode = internal;
              turnLfoOff();
          }
      }else
      {
        edit_mode = operating_mode_selection;
      }
      updateDisplay = true;
  }
  
}

void handleEncoder(Encoder &rEncoder, long &last_position, int index)
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

    if(edit_mode == waveform_selection)
    {
        waveform += difference;
        if(waveform > triangle)
          waveform = sine;
        if(waveform < sine)
          waveform = triangle;
        
        setWaveform(waveform);
        updateDisplay = true;
    }

    if(edit_mode == frequency_selection)
    {
        if(index == 0)
          frequency += difference;
        if(index == 1)
          frequency += difference * 10;
        if(index == 2)
          frequency += difference * 100;  
        if(index == 3)
          frequency += difference * 1000;

        if(frequency > 20000)
          frequency = 20000;
        if(frequency < 1)
          frequency = 1;
        
        waveform1.frequency(frequency);
        updateDisplay = true;
    }

    if(edit_mode == sweep_selection && index == 2)
    {
        lfoRate += difference *0.1;
        if(lfoRate < 0)
          lfoRate = 0;
        if(sweep)
          turnLfoOn(); //update LFO value
        updateDisplay = true;
    }
}

void drawDisplay()
{
  display.clearDisplay();

  if(edit_mode == waveform_selection)
    drawWaveSelectionScreen();
  if(edit_mode == frequency_selection)
    drawFrequencySelectionScreen();
  if(edit_mode == sweep_selection)
    drawSweepSelectionScreen();
  if(edit_mode == operating_mode_selection)
    drawOperatingModeSelectionScreen();
  
  display.display();
}

void drawFrequencySelectionScreen()
{
    display.setCursor(0,0);
    display.print("FREQUENCY");
    display.setCursor(0,10);
    display.print(frequency);
} 

void drawSweepSelectionScreen()
{
  display.setCursor(0,0);
  display.print("SWEEP SELECTION");
  display.setCursor(0,10);
  if(sweep)
    display.print("ON (press)");
  else
    display.print("OFF (press)");
  display.setCursor(0,20);  
  display.print(lfoRate);
}

void drawOperatingModeSelectionScreen()
{
    display.setCursor(0,0);
    if(operating_mode == internal)
    {
        display.print("MODE:");
        display.setCursor(0, 10);
        display.print("Internal (press)");
   }else if(operating_mode == external_cv){
        display.print("MODE:");
        display.setCursor(0, 10);
        display.print("External (press)");
   }else if (operating_mode == read_cv)
   {
      display.print("MODE:Read CV (press)");
      drawInputValues();
   }
}

void drawInputValues()
{
    printValue("CV1: ", cv1_volts, 1, 10, 25, 10);
    printValue("GT1: ", gate1_on, 58, 10, 83, 10);
    printValue("CV2: ", cv2_volts, 1, 20, 25, 20);
    printValue("GT2: ", gate2_on, 58, 20, 83, 20);
}

void drawWaveSelectionScreen()
{
    display.setCursor(0,0);
    display.print("OUTPUT WAVEFORM");
    String description;
    if(waveform == sine)
      description = "sine";
    if(waveform == sawtooth)
      description = "sawtooth";
    if(waveform == reverse_sawtooth)
      description = "reverse sawtooth";
    if(waveform == square)
      description = "square";
    if(waveform == triangle)
      description = "triangle";

    display.setCursor(0,10);
    display.print(description);
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

void setWaveform(int wave)
{
    int new_wave = WAVEFORM_SINE;
    if(wave == sawtooth)
      new_wave = WAVEFORM_SAWTOOTH;
    if(wave == reverse_sawtooth)
      new_wave = WAVEFORM_SAWTOOTH_REVERSE;
    if(wave == square)
      new_wave = WAVEFORM_SQUARE;
    if(wave == triangle)
      new_wave = WAVEFORM_TRIANGLE;

    waveform1.begin(new_wave);
}

void turnLfoOff()
{
  //Turn LFO off
  vco.begin(0.0, 0.0, WAVEFORM_SINE);
  //Reset waveform frequency
  waveform1.frequency(frequency);

}

void turnLfoOn()
{
   vco.begin(0.5, lfoRate, WAVEFORM_SINE); 
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
        
        if(operating_mode == external_cv)
        {
            waveform1.frequency(freq);
        }

        cv1_volts = volts10V;
        if(edit_mode == operating_mode_selection && operating_mode == read_cv)
        {
            updateDisplay = true;
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
        cv2_volts = volts10V;
        if(edit_mode == operating_mode_selection && operating_mode == external_cv)
        {
           updateDisplay = true;
        }
    }
}

void trackGates()
{
  //Note: inverting op-amp configuration so gate is inverted
   bool last_gate1_status = gate1_on;
   bool last_gate2_status = gate2_on;
   int gate_1 = analogRead(gatePin1);

   //Reading from analog pins with 12-bit resolution
   //Gate will trigger ~4V 0.33 scaling circuit and threshold value
   if(gate_1 >= gate_threshold)
   {
      gate1_on = false;
   }else{
      gate1_on = true;
   }
   if(gate1_on != last_gate1_status)
   {
      last_gate1_status = gate1_on;
      if(operating_mode == read_cv)
        updateDisplay = true;
   }

  int gate_2 = analogRead(gatePin2);
   if(gate_2 >=gate_threshold)
   {
      gate2_on = false;
   }else{
      gate2_on = true;
   }
   if(gate2_on != last_gate2_status)
   {
      last_gate2_status = gate2_on;
      if(operating_mode == read_cv)
        updateDisplay = true;
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
