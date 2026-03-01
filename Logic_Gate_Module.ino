////////////////////////
//  Logic Gates Module MARK 10
//  Easy Mode
//  Created by Rory Le Grand
//  Connected with master module.
//  PCB code.
//  To use Serial Monitor, change LED_RED2 to 32 instead of 1.
////////////////////////

#include <Adafruit_MCP23X17.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Define leds in order of the ESP32 layout
#define LED_GREEN 26
#define RGB1_RED 13
#define RGB1_BLUE 14
#define RGB1_GREEN 27
#define LED_RED1 23
#define LED_RED2 32  //change back to 1 for not Serial Monitor
#define LED_RED3 3
#define LED_RED4 19
#define RGB2_BLUE 18
#define RGB2_GREEN 5
#define RGB2_RED 17
#define RGB3_BLUE 16
#define RGB3_GREEN 4
#define RGB3_RED 15

// Pins for the buttons for the I2C
#define BUTTON_RED1 7
#define BUTTON_RED2 6
#define BUTTON_RGB1 5
#define BUTTON_RED3 4
#define BUTTON_RED4 3
#define BUTTON_RGB2 2
#define BUTTON_RGB3 1
#define BUTTON_SUBMIT 0

// Set up the I2C
#define addr1 0x20
Adafruit_MCP23X17 mcp1;

// Set up the small screen
LiquidCrystal_I2C lcdLG(0x27,20,4);  //set the lcdLG address to 0x27 for a 16 char and 2 line display

//For communication with the master
TwoWire MasterComm = TwoWire(1);
#define Master_input 25
#define Master_output 26
#define SLAVE_ADDR 6
bool modulePass = false;    	//change these to true in your code when you would send 
bool moduleFail = false;    	//a pass or fail message, the slave code will automatically reset it


// Define variables for software logic
int States[7] =    {0,0,0,0,0,0,0};  //array to hold led states
int AnswerKey[7] = {1,1,1,1,1,1,1};  //array that holds the answer key
                  //red1,red2,red3,red4,rgb1,rgb2,rgb3 
bool buttonPressed = false;          //used for when a button is pressed
bool Solved = true;		     //to lock the controls
int difficulty;                      //from the master input

// Define debouncing for buttons
int buttonStateRED1;
int buttonStateRED2;
int buttonStateRED3;
int buttonStateRED4;
int buttonStateRGB1;
int buttonStateRGB2;
int buttonStateRGB3;
int buttonStateSUBMIT;
int lastButtonStateRED1 = LOW;
int lastButtonStateRED2 = LOW;
int lastButtonStateRED3 = LOW;
int lastButtonStateRED4 = LOW;
int lastButtonStateRGB1 = LOW;
int lastButtonStateRGB2 = LOW;
int lastButtonStateRGB3 = LOW;
int lastButtonStateSUBMIT = LOW;
unsigned long lastDebounceTimeRED1 = 0;
unsigned long lastDebounceTimeRED2 = 0;
unsigned long lastDebounceTimeRED3 = 0;
unsigned long lastDebounceTimeRED4 = 0;
unsigned long lastDebounceTimeRGB1 = 0;
unsigned long lastDebounceTimeRGB2 = 0;
unsigned long lastDebounceTimeRGB3 = 0;
unsigned long lastDebounceTimeSUBMIT = 0;
int reading;
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// Set up hardware components
void setup() {

  // Initialize I2C and lcdLG screen
  lcdLG.init();
  lcdLG.backlight();
  mcp1.begin_I2C(addr1);  

  // Pin setup for the I2C
  mcp1.pinMode(BUTTON_RED1, INPUT_PULLUP);
  mcp1.pinMode(BUTTON_RED2, INPUT_PULLUP);
  mcp1.pinMode(BUTTON_RED3, INPUT_PULLUP);
  mcp1.pinMode(BUTTON_RED4, INPUT_PULLUP);
  mcp1.pinMode(BUTTON_RGB1, INPUT_PULLUP);
  mcp1.pinMode(BUTTON_RGB2, INPUT_PULLUP);
  mcp1.pinMode(BUTTON_RGB3, INPUT_PULLUP);
  mcp1.pinMode(BUTTON_SUBMIT, INPUT_PULLUP);

  // Setup communication with master
  MasterComm.begin(SLAVE_ADDR, Master_input, Master_output, 100000);
  MasterComm.onReceive(onReceive);

  MasterComm.onRequest(onRequest);
  // Call methods to setup the module
  //Serial.begin(9600);
  poweringUp();
  generateAnswer();
  //Serial.println("setup complete");
}

// Main method
// As long as the module is not solved, check if any buttons have been pressed.
// Use debouncing to ensure logic is only executed once with each click.
void loop() { 
  if(!Solved)  {

    // Check buttons for the red leds
    reading = mcp1.digitalRead(BUTTON_RED1);
    if (reading != lastButtonStateRED1) {
      lastDebounceTimeRED1 = millis();
    }
    if ((millis() - lastDebounceTimeRED1) > debounceDelay) {
      if (reading != buttonStateRED1) {
        buttonStateRED1 = reading;
        if (buttonStateRED1 == HIGH) {
          States[0] = !States[0];
          analogWrite(LED_RED1, States[0] ? 100 : 0);
          //Serial.print("States[0] = ");
          //Serial.println(States[0]);
        }
      }
    }
    lastButtonStateRED1 = reading;
    reading = mcp1.digitalRead(BUTTON_RED2);
    if (reading != lastButtonStateRED2) {
      lastDebounceTimeRED2 = millis();
    }
    if ((millis() - lastDebounceTimeRED2) > debounceDelay) {
      if (reading != buttonStateRED2) {
        buttonStateRED2 = reading;
        if (buttonStateRED2 == HIGH) {
          States[1] = !States[1];
          analogWrite(LED_RED2, States[1] ? 100 : 0);
          //Serial.print("States[1] = ");
          //Serial.println(States[1]);
        }
      }      
    } 
    lastButtonStateRED2 = reading;
    reading = mcp1.digitalRead(BUTTON_RED3);
    if (reading != lastButtonStateRED3) {
      lastDebounceTimeRED3 = millis();
    }
    if ((millis() - lastDebounceTimeRED3) > debounceDelay) {
      if (reading != buttonStateRED3) {
        buttonStateRED3 = reading;
        if (buttonStateRED3 == HIGH) {
          States[2] = !States[2];
          analogWrite(LED_RED3, States[2] ? 100 : 0);
          //Serial.print("States[2] = ");
          //Serial.println(States[2]);
        }
      }  
    } 
    lastButtonStateRED3 = reading;
    reading = mcp1.digitalRead(BUTTON_RED4);
    if (reading != lastButtonStateRED4) {
      lastDebounceTimeRED4 = millis();
    }
    if ((millis() - lastDebounceTimeRED4) > debounceDelay) {
      if (reading != buttonStateRED4) {
        buttonStateRED4 = reading;
        if (buttonStateRED4 == HIGH) {
          States[3] = !States[3];
          analogWrite(LED_RED4, States[3] ? 100 : 0);
          //Serial.print("States[3] = ");
          //Serial.println(States[3]);
        }
      }  
    }     
    lastButtonStateRED4 = reading;

    // Check buttons for the RGB leds
    reading = mcp1.digitalRead(BUTTON_RGB1);
    if (reading != lastButtonStateRGB1) {
      lastDebounceTimeRGB1 = millis();
    }
    if ((millis() - lastDebounceTimeRGB1) > debounceDelay) {
      if (reading != buttonStateRGB1) {
        buttonStateRGB1 = reading;
        if (buttonStateRGB1 == HIGH) {
          States[4] += 1;
          if(States[4] == 3 && difficulty == 1)  {
            States[4] = 0;
          }
          if(States[4] == 5 && difficulty == 2)  {
            States[4] = 0;
          }
          if(States[4] == 7 && difficulty == 3)  {
            States[4] = 0;
          }    
          switch(States[4]) {
            case 0:   //off
              setRGB(1,255,255,255);
              break;
            case 1:   //AND = red
              setRGB(1,150,255,255);
              break;
            case 2:   //OR = green
              setRGB(1,255,150,255);
              break;
            case 3:   //NAND = blue
              setRGB(1,255,255,150);
              break;
            case 4:   //NOR = yellow
              setRGB(1,150,150,255);
              break;
            case 5:   //XOR = purple
              setRGB(1,150,255,150);
              break;
            case 6:   //XNOR = cyan
              setRGB(1,255,150,150);
              break;                                
          }
          //Serial.print("States[4] = ");
          //Serial.println(States[4]); 
        }
      }  
    } 
    lastButtonStateRGB1 = reading;
    reading = mcp1.digitalRead(BUTTON_RGB2);
    if (reading != lastButtonStateRGB2) {
      lastDebounceTimeRGB2 = millis();
    }
    if ((millis() - lastDebounceTimeRGB2) > debounceDelay) {
      if (reading != buttonStateRGB2) {
        buttonStateRGB2 = reading;
        if (buttonStateRGB2 == HIGH) {
          States[5] += 1;
          if(States[5] == 3 && difficulty == 1)  {
            States[5] = 0;
          }
          if(States[5] == 5 && difficulty == 2)  {
            States[5] = 0;
          }
          if(States[5] == 7 && difficulty == 3)  {
            States[5] = 0;
          }  
          switch(States[5]) {
            case 0:   //off
              setRGB(2,255,255,255);
              break;
            case 1:   //AND = red
              setRGB(2,150,255,255);
              break;
            case 2:   //OR = green
              setRGB(2,255,150,255);
              break;
            case 3:   //NAND = blue
              setRGB(2,255,255,150);
              break;
            case 4:   //NOR = yellow
              setRGB(2,150,150,255);
              break;
            case 5:   //XOR = purple
              setRGB(2,150,255,150);
              break;
            case 6:   //XNOR = cyan
              setRGB(2,255,150,150);
              break;                                
          }
          //Serial.print("States[5] = ");
          //Serial.println(States[5]);
        }
      }  
    }              
    lastButtonStateRGB2 = reading;
    reading = mcp1.digitalRead(BUTTON_RGB3);
    if (reading != lastButtonStateRGB3) {
      lastDebounceTimeRGB3 = millis();
    }
    if ((millis() - lastDebounceTimeRGB3) > debounceDelay) {
      if (reading != buttonStateRGB3) {
        buttonStateRGB3 = reading;
        if (buttonStateRGB3 == HIGH) {
          States[6] += 1;
          if(States[6] == 3 && difficulty == 1)  {
            States[6] = 0;
          }
          if(States[6] == 5 && difficulty == 2)  {
            States[6] = 0;
          }
          if(States[6] == 7 && difficulty == 3)  {
            States[6] = 0;
          }  
          switch(States[6]) {
            case 0:   //off
              setRGB(3,255,255,255);
              break;
            case 1:   //AND = red
              setRGB(3,150,255,255);
              break;
            case 2:   //OR = green
              setRGB(3,255,150,255);
              break;
            case 3:   //NAND = blue
              setRGB(3,255,255,150);
              break;
            case 4:   //NOR = yellow
              setRGB(3,150,150,255);
              break;
            case 5:   //XOR = purple
              setRGB(3,150,255,150);
              break;
            case 6:   //XNOR = cyan
              setRGB(3,255,150,150);
              break;                                
          }
          //Serial.print("States[6] = ");
          //Serial.println(States[6]);
        }
      }  
    }     
    lastButtonStateRGB3 = reading;

    // Check the submit button
    reading = mcp1.digitalRead(BUTTON_SUBMIT);
    if (reading != lastButtonStateSUBMIT) {
      lastDebounceTimeSUBMIT = millis();
    }
    if ((millis() - lastDebounceTimeSUBMIT) > debounceDelay) {
      if (reading != buttonStateSUBMIT) {
        buttonStateSUBMIT = reading;
        if (buttonStateSUBMIT == HIGH) {
          checkAnswer();
          //Serial.println("Checking Answer");
        }
      }
    }
    lastButtonStateSUBMIT = reading;    
  }
}

// Called when master requests a status on if module passed or failed
void onRequest() {
  if(modulePass)  {       //if module passed
    MasterComm.print(9);
    modulePass = false;
  }
  else if(moduleFail)  {  //if module failed
    MasterComm.print(8);
    moduleFail = false;
  }
  else  {
    MasterComm.print(SLAVE_ADDR);
  }
  //Serial.println("onRequest method called");
}

// Called when master sends start and end game notifications
void onReceive(int len) {
  unsigned masterValue = MasterComm.read();
  //Serial.println(masterValue);
  if(masterValue == '9')  {   //game over
    // Call method to reset leds and values to off, also freeze the module controls
    poweringOff();
  }
  else  {  
    // To start the module
    switch (masterValue)  {
      case(1):    //easy
        difficulty = 1;
        break;
      case(2):    //medium
        difficulty = 2;
        break;
      case(3):    //hard
        difficulty = 3;
        break;
    }
    // Generate the new answer for the game
    generateAnswer();
    Solved = false;
  }
  //Serial.println("onReceived method called");
}

// Sends analogWrite() to the RGB leds
// NOTE: 255 is off, 0 is on
void setRGB(int num, int redValue, int blueValue,  int greenValue) {
  if(num == 1)  {
    analogWrite(RGB1_RED, redValue);
    analogWrite(RGB1_BLUE, blueValue);
    analogWrite(RGB1_GREEN, greenValue);
  }
  if(num == 2)  {
    analogWrite(RGB2_RED, redValue);
    analogWrite(RGB2_BLUE, blueValue);
    analogWrite(RGB2_GREEN, greenValue);
  }
  if(num == 3)  {
    analogWrite(RGB3_RED, redValue);
    analogWrite(RGB3_BLUE, blueValue);
    analogWrite(RGB3_GREEN, greenValue);
  }
}

// Generates a random expr1 and the answer based on the difficulty level
void generateAnswer()  {
    //generate not gate states
    for(int i=0; i<=3; i++) {
      AnswerKey[i] = random(2);
    }
    //generate the rgb gates based on difficulty selected
    switch(difficulty)  {
      case 1: //easy
        AnswerKey[4] = random(2);
        break;
      case 2: //medium
        AnswerKey[5] = random(4);
        break;
      case 3: //hard
        AnswerKey[6] = random(6);
        break;
    }
    //Serial.print("Answer Key is ");
    for(int i=0; i<7; i++) {
      //Serial.print(AnswerKey[i]);
    }
    //Serial.println();
    generateExpression();
}

// Generate expr1 based on AnswerKey[] and output the expr1
void generateExpression() {
  String expr1 = "";
  String expr2 = "";
  if(AnswerKey[0] == 0) {
    expr1 += "A ";
  }
  else  expr1 += "-A ";
  switch(AnswerKey[4])  {
    case 0:
      expr1 += "AND ";
      break;
    case 1:
      expr1 += "OR ";
      break;
    case 2:
      expr1 += "NAND ";
      break;
    case 3:
      expr1 += "NOR ";
      break;
    case 4:
      expr1 += "XOR ";
      break;
    case 5:
      expr1 += "XNOR ";
      break;                              
  }
  if(AnswerKey[1] == 0) {
    expr1 += "B ";
  }
  else  expr1 += "-B ";
  switch(AnswerKey[6])  {
    case 0:
      expr1 += "AND ";
      break;
    case 1:
      expr1 += "OR ";
      break;
    case 2:
      expr1 += "NAND ";
      break;
    case 3:
      expr1 += "NOR ";
      break;
    case 4:
      expr1 += "XOR ";
      break;
    case 5:
      expr1 += "XNOR ";
      break;                              
  }  
  if(AnswerKey[2] == 0) {
    expr2 += "C ";
  }
  else  expr2 += "-C ";
  switch(AnswerKey[5])  {
    case 0:
      expr2 += "AND ";
      break;
    case 1:
      expr2 += "OR ";
      break;
    case 2:
      expr2 += "NAND ";
      break;
    case 3:
      expr2 += "NOR ";
      break;
    case 4:
      expr2 += "XOR ";
      break;
    case 5:
      expr2 += "XNOR ";
      break;                              
  }  
  if(AnswerKey[3] == 0) {
    expr2 += "D ";
  }
  else  expr2 += "-D ";

  //Serial.print("expr1 is ");
  //Serial.println(expr1);

  // Print expr1 to screen
  lcdLG.clear();
  lcdLG.setCursor(0,0);
  // Prints the answer key
  /*for(int i=0; i<=6; i++) {
    lcdLG.setCursor(i,0);
    lcdLG.print(AnswerKey[i]);
  } */ 
  lcdLG.print(expr1);
  lcdLG.setCursor(0,1);
  lcdLG.print(expr2);
  Solved = false;   //unlock the user controls
}

// Compare States[] with AnswerKey[]
void checkAnswer()  {
  Solved = true;          //freeze the user controls until we check the answer
  //temp: printing the states to the screen
  /*for(int i=0; i<=6; i++) {
    lcdLG.setCursor(i,1);
    lcdLG.print(States[i]);
  } */ 
  delay(3000);
  for(int i=0; i<=6; i++) {
    if(States[i] != AnswerKey[i]) {
      Solved = false;               //unlock the user controls
      moduleFail = true;            //send fail signal to master module
      if(difficulty == 2) {   //medium
          // Reset leds
          analogWrite(LED_RED1, 0);
          analogWrite(LED_RED2, 0);
          analogWrite(LED_RED3, 0);
          analogWrite(LED_RED4, 0);
          setRGB(1,255,255,255);
          setRGB(2,255,255,255);
          setRGB(3,255,255,255);
          for(int i=0; i<=6; i++) {
            States[i] = 0;        }
      }
      if(difficulty == 3) {   //hard
          // Reset leds and generate new answer & expr1
          analogWrite(LED_RED1, 0);
          analogWrite(LED_RED2, 0);
          analogWrite(LED_RED3, 0);
          analogWrite(LED_RED4, 0);
          setRGB(1,255,255,255);
          setRGB(2,255,255,255);
          setRGB(3,255,255,255);
          for(int i=0; i<=6; i++) {
            States[i] = 0;        }
          generateAnswer();
      }
      break;
    }
  }
  // If answer is correct
  if(Solved) {
    //Serial.println("module solved!");
    analogWrite(LED_GREEN, 100); //turn on green led 
    modulePass = true;  	 //send success signal to master module
  }
}

// Flashing hardware components when power is first sent to the module (to make it look cool)
void poweringUp() {

  // Print a message to the screen
  lcdLG.setCursor(0,0);
  lcdLG.print("Hello world!!");
  lcdLG.setCursor(0,1);
  lcdLG.print("Hackers for Life!");

  // Set all leds to off
  poweringOff();
  
  // Blink the red leds in order
  analogWrite(LED_RED1, 100);
  delay(550);
  analogWrite(LED_RED2, 100);
  delay(550);
  analogWrite(LED_RED3, 100);
  delay(550);
  analogWrite(LED_RED4, 100);    
  delay(550);
  analogWrite(LED_GREEN, 100);   
  delay(550);  
  
  // Print a message to the screen
  lcdLG.clear();
  lcdLG.setCursor(0,0);
  lcdLG.print("Inspired by Danny");
  lcdLG.setCursor(0,1);
  lcdLG.print("Created by Rory");
  
  // Gradually increase rgb brightness
  for(int i=255; i>=0; i-=5) {
    setRGB(1,i,255,255);
    setRGB(2,255,i,255);
    setRGB(3,255,255,i);
    delay(30);
  }  
  
  // Print a message to the screen
  lcdLG.clear();
  lcdLG.setCursor(0,0);
  lcdLG.print("Logic Gates Puzzle");
  lcdLG.setCursor(0,1);
  lcdLG.print("Can you use logic on gates?");  
  delay(1000);

  // Set everything off
  poweringOff();
}

// Set everything to off
void poweringOff() {
  analogWrite(LED_RED1, 0);
  analogWrite(LED_RED2, 0);
  analogWrite(LED_RED3, 0);
  analogWrite(LED_RED4, 0);
  analogWrite(LED_GREEN, 0);
  setRGB(1,255,255,255);
  setRGB(2,255,255,255);
  setRGB(3,255,255,255);
  lcdLG.clear();
  difficulty = 0;
  int States[7] = {0,0,0,0,0,0,0};
  int AnswerKey[7] = {1,1,1,1,1,1,1};
}