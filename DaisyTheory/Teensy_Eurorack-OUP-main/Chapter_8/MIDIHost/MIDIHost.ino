/*
  Chapter 8: MIDI Host Example
*/

#include <USBHost_t36.h>

USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
MIDIDevice midi1(myusb);

void setup()
{
    Serial.begin(115200);
    //Delay before turning on host
    delay(1500);
    Serial.println("MIDI USB Host input test");
    delay(10);
    myusb.begin();

    //Set up callbacks
    midi1.setHandleNoteOn(myNoteOn);
    midi1.setHandleNoteOff(myNoteOff);

}

void loop()
{
    myusb.Task();
    midi1.read();
}

void myNoteOn(byte channel, byte note, byte velocity)
{
  printMessage("Note On: ", channel, note, velocity);
}

void myNoteOff(byte channel, byte note, byte velocity)
{
  printMessage("Note Off: ", channel, note, velocity);
}

void printMessage(String msg, byte channel, byte note, byte velocity)
{
    Serial.print(msg);
    Serial.print(" Channel: ");    Serial.print(channel);
    Serial.print(" Note: ");      Serial.println(note);
    Serial.print(" Velocity: ");  Serial.println(velocity);
}
