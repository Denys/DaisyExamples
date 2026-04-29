/*
  OLED and Encoder test
  Brent Edstrom, 2023

  OLED Pin      Teensy 4 Pin
  VCC           3.3V
  GND           GND
  SCL           19
  SDA           18

  Project pin assignments (first pin = switch)
  Encoder 1: 1, 2, 3
  Encoder 2: 4, 5, 6

*/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>
#include <Bounce.h>

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

//Create two rotary encoders
Encoder encoder1(2, 3);
Encoder encoder2(5, 6);

//Create two Bounce objects to track pushbuttons
Bounce btn1 = Bounce(1, 15);  
Bounce btn2 = Bounce(4, 15);  

//Variables to store encoder positions
long encoder1_pos = 0;
long encoder2_pos = 0;

void setup() 
{
   //OLED setup
    //Let circutry stabilize and initialize OLED display
    delay(100);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
    display.setTextSize(1);
    //This color mode overwrites text
    display.setTextColor(WHITE, BLACK);

    //Pullup for switches
     pinMode(1, INPUT_PULLUP);
     pinMode(4, INPUT_PULLUP);

    drawMessage("OLED Test...");
}

void loop() 
{
    trackButtons();
    trackEncoders();
}

void trackButtons()
{
    btn1.update();
    if(btn1.fallingEdge())
    {
        drawMessage("Button 1 pressed");
    }

    btn2.update();
    if(btn2.fallingEdge())
    {
        drawMessage("Button 2 pressed");
    }
}

void trackEncoders()
{
    //Store the last position of each encoder
    static long pos1 = 0; 
    static long pos2 = 0; 
 
    //Update the position if it changes
    long position = encoder1.read();
    if(position != pos1)
    {
        encoder1_pos = position - pos1;
        pos1 = position;
        drawMessage("Encoder 1: ", pos1);
    }

    position = encoder2.read();
    if(position != pos2)
    {
        encoder2_pos = position - pos2;
        pos2 = position;
        drawMessage("Encoder 2: ", pos2);
    }
}

//Draw a string message at the top left corner of the screen
void drawMessage(String msg)
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(msg);
  display.display();
}

//Draw a string message on the first row and value on the 2nd row
void drawMessage(String msg, long value)
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(msg);
  display.setCursor(0, 10);
  display.print(value);
  display.display();
}
