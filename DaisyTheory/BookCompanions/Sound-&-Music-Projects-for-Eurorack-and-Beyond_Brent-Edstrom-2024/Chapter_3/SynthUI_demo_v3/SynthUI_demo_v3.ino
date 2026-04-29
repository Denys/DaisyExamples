/*  Synth UI-2024-v3
 *  Description: Interface with 4 switches/rotary encoders and
 *               OLED display. 
 *   
 *  Project pin assignments (first pin = switch)
 *  Encoder 1: 1, 2, 3
 *  Encoder 2: 4, 5, 6
 *  Encoder 3: 9, 14, 15
 *  Encoder 4: 16, 17, 22

    OLED Pin      Teensy 4 Pin
      VCC           3.3V
      GND           GND
      SCL           19
      SDA           18
 */

#include <Encoder.h>
#include <Bounce.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

//Create four Bounce objects to track pushbuttons
Bounce btn1 = Bounce(1, 15);  
Bounce btn2 = Bounce(4, 15); 
Bounce btn3 = Bounce(9, 15);  
Bounce btn4 = Bounce(16, 15); 

//Rotary encoders
Encoder encoder1(2, 3);
Encoder encoder2(5, 6);
Encoder encoder3(14, 15);
Encoder encoder4(17, 22);

//Arrays to store values for each mode
float mode1Array[4];
float mode2Array[4];
float mode3Array[4];
float mode4Array[4];

//Array to store encoder positions
long lastPosition[4];

//Menu mode enumeration
enum{mode1, mode2, mode3, mode4};
int current_mode = mode1;

bool updateDisplay = true;


void setup() 
{
    //OLED setup
    //Let circutry stabilize and initialize OLED display
    delay(100);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
    display.setTextSize(1);
    //This color mode overwrites text
    display.setTextColor(WHITE, BLACK);

    //Menu button setup
    pinMode(1, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP); 
    pinMode(9, INPUT_PULLUP);
    pinMode(16, INPUT_PULLUP);
}

void loop() 
{
    trackButtons();
    trackEncoders();
    
    if(updateDisplay == true)
    {
        redrawDisplay();
    }
}

void trackButton(Bounce &rBounce, int mode)
{
    rBounce.update();
    if(rBounce.fallingEdge())
    {
        current_mode = mode;
        updateDisplay = true;
    }
}

void trackButtons()
{
   trackButton(btn1, mode1);
   trackButton(btn2, mode2);
   trackButton(btn3, mode3);
   trackButton(btn4, mode4);
}



void trackEncoders()
{
    handleEncoder(encoder1, lastPosition[0], 0);
    handleEncoder(encoder2, lastPosition[1], 1);
    handleEncoder(encoder3, lastPosition[2], 2);
    handleEncoder(encoder4, lastPosition[3], 3);
}

void handleEncoder(Encoder &rEncoder, long &last_position, int index)
{
    long pos = rEncoder.read();
    
    if(pos == last_position)
    {
         //No need to continue: difference = 0
         return;
    }

    //Calculate the difference: 
    long difference = pos - last_position;
    last_position = pos;

    //Update values (demonstrate fractional values)
    switch(current_mode)
    {
        case mode1:  mode1Array[index] += difference * 0.01; break;
        case mode2:  mode2Array[index] += difference * 0.1;  break;
        case mode3:  mode3Array[index] += difference * 1.0;  break;
        case mode4:  mode4Array[index] += difference * 10.0; break;
    }

    updateDisplay = true;
}


//======================= OLED CODE =======================
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

/*  NOTE: 
 *  clearDisplay() is slow, so time-sensitive applications
 *  should overwrite individual values when possible
*/

void redrawDisplay()
{
  //Clear the display
  display.clearDisplay();

  //Print the current mode
  String mode = "MODE: " + String (current_mode +1);
  display.setCursor(0, 0);
  display.print(mode);

  drawEncoderValues();

  display.display();
  updateDisplay = false;
}

void drawEncoderValues()
{
   switch(current_mode)
   {
      case mode1: drawModeScreen(mode1Array); break;
      case mode2: drawModeScreen(mode2Array); break;
      case mode3: drawModeScreen(mode3Array); break;
      case mode4: drawModeScreen(mode4Array); break;
   }
   
}

void drawModeScreen(float valueArray[])
{
   printValue("E1: ", valueArray[0], 0, 10, 25, 10);
   printValue("E2: ", valueArray[1], 58, 10, 83, 10);
   printValue("E3: ", valueArray[2], 0, 20, 25, 20);
   printValue("E4: ", valueArray[3], 58, 20, 83, 20);
}

//======================= END OLED CODE =======================
