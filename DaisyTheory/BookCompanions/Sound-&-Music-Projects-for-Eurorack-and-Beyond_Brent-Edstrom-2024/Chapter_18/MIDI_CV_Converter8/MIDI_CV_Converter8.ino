/*  MIDI-CV-Converter v8: 
 *  For use with (SMD) MIDI->CV Backpack
 *  Brent Edstrom, 4/29/2024
 *  
 *  Tested with TeensyLC 
 *  Eurorack Module:
 *  +12V: 30mA
 *  -12V: 12mA 
 *
 *  Pin connections:
    MCP 4821 pin    Teensy LC pin assignment
    1-VDD           3.3V
    2-CS            chip select: 10
    3-SCK           13
    4-SDI           11
    5-LDAC          GND (for rising edge of CS)
    6-SHDN          HIGH (3.3V) unless shutdown mechanism needed
    7-Vss           GND
    8-Vout          voltage out from MCP4821
 */

#include <SPI.h>
#include <MIDI.h>

/*  MODE1: 
 *     Output CV1 pitch, CV2 velocity, and Gate 1 for any MIDI input
 *     Gate 2 = MIDI clock
 *     
 *  MODE2:
 *    Output CV1 pitch and Gate 1 for MIDI ch 1 input
 *    Output CV2 pitch and Gate 2 for MIDI ch 2 input
 * 
 */
enum {MODE1, MODE2};
int active_mode = MODE1;

//Chip select and gate pins
const int cs_pin = 10;
const int gate_out_pin1 = 2;
const int gate_out_pin2 = 3;

//Volt and semitone using 12-bit DAC and 5X amplification
float volt = 2000.0 /5; 
float semitone = volt / 12.0;

//Tuning offset
float tuning_offset = 0.0;

//Default to CV octave
bool CV_octave = true;

//Store last notes to prevent extraneous offs for legato playing
int last_ch1_note = 0;
int last_ch2_note = 0;
int last_note = 0;

const int CLOCK = 0xF8;
unsigned long last_clock_time = 0;
bool clock_received = false;

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

static int toggle_pin1 = 4;
static int toggle_pin2 = 5;
static int led_pin = 6;

static int last_led_on_time = 0;

static int clock_duration = 1;  //Default to 1ms clock

void setup() 
{
  //Set up SPI for the DAC
  pinMode(cs_pin, OUTPUT);
  digitalWrite(cs_pin, HIGH);

  //Start SPI communication
  delay(100);
  SPI.begin();

  //Start UART MIDI input and set MIDI handlers
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(noteOnHandler);
  MIDI.setHandleNoteOff(noteOffHandler);
  MIDI.setHandleClock(handleClock);

  //Set up toggle switches and LED
  pinMode(toggle_pin1, INPUT_PULLUP);
  pinMode(toggle_pin2, INPUT_PULLUP);
  pinMode(led_pin, OUTPUT);

  //Set up gate pins
  pinMode(gate_out_pin1, OUTPUT);
  pinMode(gate_out_pin2, OUTPUT);
  
}

void loop() 
{
    MIDI.read();

    trackModeToggleSwitch();
    trackCVToggleSwitch();

    //See if clock output or LED needs to be turned off
    checkGateOff();
    checkLED();
}

//Selection switch for MODE 1 & 2
void trackModeToggleSwitch()
{
    if(digitalRead(toggle_pin1))
    {
        active_mode = MODE2;
    }else{
        active_mode = MODE1;
    }
}

//Selection switch for V/oct and Hz/V
void trackCVToggleSwitch()
{
    if(digitalRead(toggle_pin2))
    {
        CV_octave = false;
    }else{
        CV_octave = true;
    }
}

void noteOnHandler(byte channel, byte note, byte velocity) 
{ 
  //Show MIDI activity
  lightLED();
  
  if(velocity != 0)
  {
      if(active_mode == MODE1)
      {
          mode1NoteOnHandler(channel, note, velocity);
      }else{
          mode2NoteOnHandler(channel, note, velocity);
      }
  }else{
     //Note-On with velocity of zero = Note-Off
     noteOffHandler(channel, note, velocity);
  }
}

/*
  Convert MIDI to V/oct:
  Semitone is a global constant calculated on 
  5X amplification circuitry and 12-bit DAC
*/
float midiToCVOctaveScaled(byte note)
{
    return (note * semitone) + tuning_offset;
}

/*
  Convert MIDI to Hz/Volt:
  Volt is a global constant calculated on 
  5X amplification circuitry and 12-bit DAC
*/
float midiToHertzPerVoltScaled(byte note)
{
    float frequency = 440 * pow(2.0, (note - 69.0)/12.0);
    float hertzPerVolt = frequency / 55;
    //Scale for 5X amplification at 12 bits
    return (hertzPerVolt * volt) + tuning_offset;
}

void mode1NoteOnHandler(byte channel, byte note, byte velocity)
{
    //Output CV/Oct on DAC channel 0
    float voltage = 0;
   
    if(CV_octave == true)
    {
        voltage = midiToCVOctaveScaled(note);
    }else{
        voltage = midiToHertzPerVoltScaled(note);      
    }
    updateDAC(cs_pin, 0, voltage);

    //Output velocity on DAC channel 1
    //Velocity as a percentage of MIDI velocity (0-127)
    float vel = (float) velocity / 127.0;
    //Apply percentage to DAC voltage range 0-2000
    vel = vel * 2000;
    updateDAC(cs_pin, 1, vel);
    
    gateOn(gate_out_pin1);
    last_note = note;
}

void mode2NoteOnHandler(byte channel, byte note, byte velocity)
{
    if(channel == 1)
    {
        //Output CV/Oct on DAC channel 0
        float voltage = 0;
   
        if(CV_octave == true)
        {
            voltage = midiToCVOctaveScaled(note);
        }else{
            voltage = midiToHertzPerVoltScaled(note);      
        }
        updateDAC(cs_pin, 0, voltage);
        gateOn(gate_out_pin1);
        last_ch1_note = note;
    }
    if(channel == 2)
    {
        //Output CV/Oct on DAC channel 1
        float voltage = 0;
        if(CV_octave == true)
        {
            voltage = midiToCVOctaveScaled(note);
        }else{
            voltage = midiToHertzPerVoltScaled(note);      
        }
        updateDAC(cs_pin, 1, voltage);
        gateOn(gate_out_pin2);
        last_ch2_note = note;
    }
}


void noteOffHandler(byte channel, byte note, byte velocity) 
{ 
  //Show MIDI activity for note-On and note-Off messages
  lightLED();
  if(active_mode == MODE1)
  {
      mode1NoteOffHandler(channel, note, velocity);
  }else{
      mode2NoteOffHandler(channel, note, velocity);
  }
}

void mode1NoteOffHandler(byte channel, byte note, byte velocity)
{
    if(note == last_note)
    {
       gateOff(gate_out_pin1);
    }
}

void mode2NoteOffHandler(byte channel, byte note, byte velocity)
{
    if(channel == 1 && last_ch1_note == note)
    {
        gateOff(gate_out_pin1);
    }

    if(channel == 2 && last_ch2_note == note)
    {
        gateOff(gate_out_pin2);
    }   
}

void handleClock()
{
    if(active_mode == MODE1)
    {
      lightLED();
      //Send clock messages from gate 2
      gateOn(gate_out_pin2);
      last_clock_time = millis();
      clock_received = true;
    }
}

void gateOn(int gate_pin)
{
    digitalWrite(gate_pin, HIGH);
}

void gateOff(int gate_pin)
{
    digitalWrite(gate_pin, LOW);
}

void checkGateOff()
{
    if(clock_received)
    {
        if(millis() - last_clock_time > clock_duration)
        {
            gateOff(gate_out_pin2);
            clock_received = false;
        }
    }
}

void lightLED()
{
    digitalWrite(led_pin, HIGH);
    last_led_on_time = millis();
}

void checkLED()
{
    if(millis() - last_led_on_time >= 50)
    {
        digitalWrite(led_pin, LOW);
    }
}

/*
  DAC output: 
  The DAC outputs on two channels (channel = 0 or 1) 
  and is configured for 2V output at 12-bits. The output
  is amplified 5X.
*/
void updateDAC(int cs, int channel, uint16_t value)
{
  uint16_t out = (channel << 15) | (0 << 14) | (1 << 13) | (1 << 12) | ( value );

  //Set chip select LOW and transfer 16-bit value as two bytes
  digitalWrite(cs, LOW);
  SPI.transfer(out>>8);      
  SPI.transfer(out & 0xFF);
  //End transmission by setting chip select HIGH
  digitalWrite(cs, HIGH);
}
