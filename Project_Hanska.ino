//libraries

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


//definitions 

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

//  Variables
int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 13;                // pin to blink led at each beat
int fadePin = 9;                  // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin

// width x height = 32,32
static const uint8_t glove[] PROGMEM = {
  B00000000, B00000000, B00000000, B00000000, 
  B00000000, B00000000, B00000000, B00000000, 
  B00000000, B00000000, B00000000, B00000000, 
  B00000000, B00000011, B11111110, B00000000, 
  B00000000, B00000100, B00000001, B00000000, 
  B00000000, B00001000, B00000000, B11000000, 
  B00000000, B00000000, B00000000, B11110000, 
  B00000000, B00100000, B00000000, B11110000, 
  B00000000, B01000000, B00000001, B00001000, 
  B00011111, B11000001, B11111110, B00000110, 
  B01100011, B00000000, B00000000, B00000001, 
  B00000011, B00000000, B00000000, B00000001, 
  B10000011, B00000000, B00000000, B00000001, 
  B10000011, B00000000, B00000000, B00000001, 
  B10000011, B00000000, B00000000, B00000001, 
  B10000011, B00000000, B00000000, B00000001, 
  B10000011, B00000000, B00000000, B00000001, 
  B10000011, B00000000, B00000000, B00000001, 
  B10000000, B10000000, B00000000, B00000001, 
  B10000000, B10000000, B00000000, B00000001, 
  B10000000, B10000000, B00000000, B00000000, 
  B10000000, B10000000, B00000000, B00000110, 
  B10000000, B10000000, B00000000, B00000110, 
  B01111111, B01100000, B00000000, B00001000, 
  B00000000, B00001111, B00000000, B00110000, 
  B00000000, B00001111, B00000000, B00110000, 
  B00000000, B00000000, B01111111, B11000000, 
  B00000000, B00000000, B00000000, B00000000, 
  B00000000, B00000000, B00000000, B00000000, 
  B00000000, B00000000, B00000000, B00000000, 
  B00000000, B00000000, B00000000, B00000000, 
  B00000000, B00000000, B00000000, B00000000, 
  
};

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded! 
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat". 
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

// Regards Serial OutPut  -- Set This Up to your needs
static boolean serialVisual = true;   // Set to 'false' by Default.  Re-set to 'true' to see Arduino Serial Monitor ASCII Visual Pulse 


void setup(){

  //--------keypad initialisation---------
  DDRD = B11111111; //  D port as output
  PORTD = B01111101; //  pullup resistors
  //--------------------------------------
  
  pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
  pinMode(fadePin,OUTPUT);          // pin that will fade to your heartbeat!
  Serial.begin(9600);             // we agree to talk fast!
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
   // IF YOU ARE POWERING The Pulse Sensor AT VOLTAGE LESS THAN THE BOARD VOLTAGE, 
   // UN-COMMENT THE NEXT LINE AND APPLY THAT VOLTAGE TO THE A-REF PIN
   analogReference(EXTERNAL);   
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)

   display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.clearDisplay();
  display.println("Project Hanska");
  display.println("versio 0.12\nbeta");
  display.drawBitmap(90, 0, glove, 32, 32, WHITE);
  display.display();
  display.clearDisplay();
  delay(2000);
  
}

// Global variables
int option = 1;
bool pressed = false;
int pressedKey = 2;  // global variable for memoring the currently pressed key
int pulseBar = 0;
int pulX = 0;
int choice = 0;

//  Where the Magic Happens
void loop(){

  //----Check pressed key-----
  if ((PIND & B01111101) != B01111101 && pressed != true){
  pressed = true;
      if ((PIND & B01111101) == B01111100) pressedKey = 1; 
      else if ((PIND & B01111101) == B01111001) pressedKey = 2; 
      else if ((PIND & B01111101) == B01110101) pressedKey = 3; 
      else if ((PIND & B01111101) == B01101101) pressedKey = 4; 
      else if ((PIND & B01111101) == B01011101) pressedKey = 5; 
      else if ((PIND & B01111101) == B00111101) pressedKey = 6;
  }
  Serial.println(pressedKey);
  Serial.println(PIND, BIN);
  Serial.println(option);
  //---------------------------
  
 // serialOutput();       
    
  if (QS == true){     // A Heartbeat Was Found
  pulseBar = 7;
  pulX = 0;
                       // BPM and IBI have been Determined
                       // Quantified Self "QS" true when arduino finds a heartbeat
        fadeRate = 255;         // Makes the LED Fade Effect Happen
                                // Set 'fadeRate' Variable to 255 to fade LED with pulse
        //serialOutputWhenBeatHappens();   // A Beat Happened, Output that to serial.     
        QS = false;                      // reset the Quantified Self flag for next time    
  }
     
  ledFadeToBeat();                      // Makes the LED Fade Effect Happen 
  delay(20);                             //  take a break

  if(pressedKey == 4 && pressed == true){ //RIGHT
      option++;
      if(option > 4){ option = 1; }
      pressedKey = 2;
  }
  if(pressedKey == 3 && pressed == true){ //RIGHT
      option--;
      if(option < 1){ option = 4; }
      pressedKey = 2;
  }
  if(pressedKey == 2){  //MAIN MENU
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      if(option == 1){
        display.print("Option1<-");
        if ((PIND & B01111101) == B01011101) { choice = 1; } 
      }
      else{ display.print("Option1"); }
      display.setCursor(64,0);
      if(option == 2){
        display.print("Option2<-");
      }
      else{ display.print("Option2"); }
      display.print("\n");
      display.setCursor(0,15);
      if(option == 3){
        display.print("Option3<-");
      }
      else{ display.print("Option3"); }
      display.setCursor(64,15);
      if(option == 4){
        display.print("Option4<-");
      }
      else{ display.print("Option4"); }
      display.display();
      display.clearDisplay();
  }

  else if (choice == 1){
      //lcd
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.print("Heart rate:  ");
      if (pulseBar == 7){ display.print("IIIIIII"); }
          if(pulseBar == 6){ display.print(" IIIIII"); }
          if(pulseBar == 5){ display.print("  IIIII"); }
          if(pulseBar == 4){ display.print("   IIII"); }
          if(pulseBar == 3){ display.print("    III"); }
          if(pulseBar == 2){ display.print("     II"); }
          if(pulseBar == 1){ display.print("      I"); }
      if (pulX % 2 == 0)
      {
          pulseBar--;
      }
      pulX++;
      display.print("\n\n");
      display.setTextSize(2);
      display.print("    ");
      display.print(BPM);
      display.display();
      display.clearDisplay();
  }

  if ((PIND & B01111101) == B01111101){pressed = false;} //to let the main loop flow freely until next keypress
}

void ledFadeToBeat(){
    fadeRate -= 15;                         //  set LED fade value
    fadeRate = constrain(fadeRate,0,255);   //  keep LED fade value from going into negative numbers!
    analogWrite(fadePin,fadeRate);          //  fade LED
}




