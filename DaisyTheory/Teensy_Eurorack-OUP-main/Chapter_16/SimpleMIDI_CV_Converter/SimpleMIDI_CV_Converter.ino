/* Simple UART MIDI to CV-Converter
 *  
 */

#include <SPI.h>
#include <MIDI.h>

//Chip select and gate pins
const int cs_pin = 1;
const int gate_out_pin = 2;

//Volt and semitone using 12-bit DAC and 5x amplification
float volt = 2000.0 /5; 
float semitone = volt / 12.0;

//Tuning offset
float tuning_offset = 0.0;

//Store last_note to prevent extraneous offs for legato playing
int last_note = 0;

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

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
}

void loop() 
{
    MIDI.read();
}

void noteOnHandler(byte channel, byte note, byte velocity) 
{ 
  float n = note;
  float voltage = (n * semitone) + tuning_offset;
  if(velocity != 0)
  {
    updateDAC(cs_pin, voltage);
    gateOn(gate_out_pin);
    last_note = note;
  }else{
     //Note-On with velocity of zero = Note-Off
     noteOffHandler(channel, note, velocity);
  }
}

void noteOffHandler(byte channel, byte note, byte velocity) 
{ 
  if(note == last_note)
  {
    gateOff(gate_out_pin);
  }
}

void gateOn(int gate_pin)
{
    //Logic is reversed for transistor gate
    digitalWrite(gate_pin, LOW);
}

void gateOff(int gate_pin)
{
    //Logic is reversed for transistor gate
    digitalWrite(gate_pin, HIGH);
}

void updateDAC(int cs, uint16_t value)
{
  uint16_t out = (0 << 15) | (0 << 14) | (1 << 13) | (1 << 12) | ( value );

  //Set chip select LOW and transfer 16-bit value as two bytes
  digitalWrite(cs, LOW);
  SPI.transfer(out>>8);      
  SPI.transfer(out & 0xFF);
  //End transmission by setting chip select HIGH
  digitalWrite(cs, HIGH);
}

//Two-channel output (channel = 0 or 1)
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
