////////////////////////
//  Binary Conversion Module MARK 7
//  Easy Mode
//  Created by Rory Le Grand
//  Connected with master module.
//  Issue with GPIO 1,2,12,13 on ESP32, so not using them.
//  To use Serial Monitor, change RGB3_BLUE to pin GPIO23.
////////////////////////

#include <Adafruit_MCP23X17.h>

// Define the DIP switch input pins in order of LSB to MSB
const int dip0 = 0;
const int dip1 = 1;
const int dip2 = 2;
const int dip3 = 3;

// Define the output pins to CD4511 decoder for the first display (2^0 in the right side)
const int decoderPinsFirstDisplay[4] = {7,6,5,4};
// Define the output pins to CD4511 decoder for the second display (2^1 in the right side)
const int decoderPinsSecondDisplay[4] = {12,13,14,15};
// Define the output pins to CD4511 decoder for the third display (2^2 in the right side)
const int decoderPinsThirdDisplay[4] = {8,9,10,11};
// Define the output pins to the fourth display (2^0 in the left side)
const int decoderPinsFourthDisplay[7] = {7,6,5,4,3,2,1};
// Define the output pins to the fifth display (2^1 in the left side)
const int decoderPinsFifthDisplay[7] = {8,9,10,11,12,13,14};

// Define other components
const int SUBMIT_BUTTON = 0;    //mcp2.GPIOA0
const int SELECT_BUTTON = 15;   //mcp2.GPIOB7
const int GREEN_LED = 15;
const int first_led = 4;
const int second_led = 16;
const int third_led = 17;
const int RGB1_RED = 5;         //the right side of the equation
const int RGB1_GREEN = 27;
const int RGB1_BLUE = 14;
const int RGB2_RED = 33;        //the left side of the equation
const int RGB2_BLUE = 32; 
const int RGB2_GREEN = 18; 
const int RGB3_RED = 19;        //the operator on the left side of the equation
const int RGB3_GREEN = 23; 
const int RGB3_BLUE = 1;        //change to 23 or 1

// Define I2C extenders
#define addr1 0x20
Adafruit_MCP23X17 mcp1; 
#define addr2 0x21
Adafruit_MCP23X17 mcp2;

// Define variables for software logic
int selectedDisplay;       	//keeps track of what display is selected
int firstDisplayValue;
int secondDisplayValue;
int thirdDisplayValue;
int fourthDisplayValue;
int fifthDisplayValue;
int targetValue;           	//the randomly generated answer
bool Solved = false;       	//to lock the controls
String prompt;             	//prompt on the screen

// Define master communication
#define SLAVE_ADDR 4
const int Master_input = 26;
const int Master_output = 25;
TwoWire MasterComm = TwoWire(1);
int difficulty = 3;
bool modulePass = false;    	//change these to true in your code when you would send 
bool moduleFail = false;    	//a pass or fail message, the slave code will automatically reset it

// Define debouncing for buttons
int buttonStateSELECT;
int buttonStateSUBMIT;
int lastButtonStateSUBMIT = LOW;
int lastButtonStateSELECT = LOW;
unsigned long lastDebounceTimeSUBMIT = 0;
unsigned long lastDebounceTimeSELECT = 0;
int reading;
unsigned long debounceDelay = 50;    //the debounce time; increase if the output flickers

// Set up hardware components
void setup() {
  // Initialize MCP23X17 I2C communication
  if (!mcp1.begin_I2C(addr1)) {
    //Serial.println("I2C number 1 initialization failed!");
    while (1); 			     //stop the program here if initialization fails
  }
  // Initialize MCP23X17 I2C communication
  if (!mcp2.begin_I2C(addr2)) {
    //Serial.println("I2C number 2 initialization failed!");
    while (1); 			     //stop the program here if initialization fails
  }

  // Initialize the random seed based on an unconnected analog pin reading for better randomness
  // Assuming A1 is unconnected and can provide floating noise for randomness
  randomSeed(analogRead(27));
  
  // Set DIP switch pins as inputs with internal pull-up resistors
  mcp1.pinMode(dip0, INPUT_PULLUP);
  mcp1.pinMode(dip1, INPUT_PULLUP);
  mcp1.pinMode(dip2, INPUT_PULLUP);
  mcp1.pinMode(dip3, INPUT_PULLUP);
  
  // Set decoder output pins for the first display as outputs
  for (int i = 0; i < 4; i++) {
    mcp1.pinMode(decoderPinsFirstDisplay[i], OUTPUT);
  }
  // Set decoder output pins for the second display as outputs
  for (int i = 0; i < 4; i++) {
    mcp1.pinMode(decoderPinsSecondDisplay[i], OUTPUT);
  }
  // Set decoder output pins for the third display as outputs
  for (int i = 0; i < 4; i++) {
    mcp1.pinMode(decoderPinsThirdDisplay[i], OUTPUT);
  }
  // Set decoder output pins for the fourth display as outputs
  for (int i = 0; i < 7; i++) {
    mcp2.pinMode(decoderPinsFourthDisplay[i], OUTPUT);
  }
  // Set decoder output pins for the fifth display as outputs
  for (int i = 0; i < 7; i++) {
    mcp2.pinMode(decoderPinsFifthDisplay[i], OUTPUT);
  }  
  mcp2.pinMode(SUBMIT_BUTTON, INPUT_PULLUP);
  mcp2.pinMode(SELECT_BUTTON, INPUT_PULLUP);  

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
// As long as the module is not solved, check if the select display button or the submit button has been pressed.
// Use debouncing to ensure logic is only executed once with each click.
void loop() {
  if(!Solved) {
    reading = mcp2.digitalRead(SELECT_BUTTON);
    if (reading != lastButtonStateSELECT) {
      lastDebounceTimeSELECT = millis();
    }
    if ((millis() - lastDebounceTimeSELECT) > debounceDelay) {
      if(reading != buttonStateSELECT) {
        buttonStateSELECT = reading;
        if(buttonStateSELECT == HIGH)   {
          selectButtonPressed();
          //Serial.print("select button pressed: selected display is ");
          //Serial.println(selectedDisplay);      
        }
      } 
    }
    lastButtonStateSELECT = reading;
    reading = mcp2.digitalRead(SUBMIT_BUTTON);
    if (reading != lastButtonStateSUBMIT) {
      lastDebounceTimeSUBMIT = millis();
    }
    if ((millis() - lastDebounceTimeSUBMIT) > debounceDelay) {
      if(reading != buttonStateSUBMIT) {
        buttonStateSUBMIT = reading;
        if(buttonStateSUBMIT == HIGH)   {    
          checkAnswer();
          //Serial.println("submit button pressed");
        }
      }
    } 
    lastButtonStateSUBMIT = reading;   
  }
}

// Given the difficulty level from the master, set up the game.
void generateAnswer() {
  switch (difficulty) {
    //easy difficulty, converting hex to decimal
    case 1:
      //choose a target value in decimal
      targetValue = random(1,255);
      //convert to hex
      prompt = String(targetValue, HEX);
      //set right side to base 10 (yellow color)
      setRGB(1,150,255,150);
      //set left side to base 16 (magenta color)
      setRGB(2,150,150,255);
      //turn off operand
      setRGB(3,255,255,255);
      //setup components
      WriteDisplay(decoderPinsFourthDisplay, prompt.charAt(0));
      WriteDisplay(decoderPinsFifthDisplay, prompt.charAt(1));
      analogWrite(first_led, 100);
      selectedDisplay = 1;
      break;
    //medium difficulty, setting the operand
    //NOTE: this difficulty level is incomplete due to deadlines on the Hackers Project
    case 2:
      int ran = 1;
      switch(ran) {
        case 1:   //addition = red
          setRGB(3,150,255,255);
          break;
        case 2:   //subtraction = green
          setRGB(3,255,150,255);
          break;
        case 3:   //multiplication = blue
          setRGB(3,255,255,150);
          break;
        case 4:   //division = yellow
          setRGB(3,150,150,255);
          break;
        case 5:   //2's compliment = purple
          setRGB(3,150,255,150);
          break;
        case 6:   //??? = cyan
          setRGB(3,255,150,150);
          break;                                
      }
   
  }
  Solved = false;   //unlock the user controls
  //Serial.print("Chosen Hex Value: ");
  //Serial.println(prompt);
}

//when check answer button is pressed, check the answer
void checkAnswer()  {
  Solved = true;          //freeze the controls until we check the answer  
  int userAnswer;
  switch(difficulty)  {   //compute the user answer inputted based on the  difficulty level
    case 1:
      userAnswer =  firstDisplayValue + secondDisplayValue*10 + thirdDisplayValue*100;  //convert the number to base 10
      break;
  }
  if(userAnswer == targetValue) {
    //Serial.println("module solved!"); //print message 
    analogWrite(GREEN_LED, 100); //turn on green led 
    modulePass = true;  //send success signal to master module
  }  
  else  {
    Solved = false;               	//unlock the user controls
    //Serial.println("incorrect!");
    moduleFail = true;  		//send fail signal to master module
    if(difficulty == 2) {   		//medium difficulty
      //reset SSDs and red leds to first display
      sendToDisplay(decoderPinsFirstDisplay, 11);
      sendToDisplay(decoderPinsSecondDisplay, 11);
      sendToDisplay(decoderPinsThirdDisplay, 11);
      analogWrite(second_led, 0);
      analogWrite(third_led, 0);
      analogWrite(first_led, 100);
      selectedDisplay = 1;      
    }
    if(difficulty == 3) {   		//hard difficulty
      //generate new problem
      generateAnswer();
      //reset SSDs
      sendToDisplay(decoderPinsFirstDisplay, 11);
      sendToDisplay(decoderPinsSecondDisplay, 11);
      sendToDisplay(decoderPinsThirdDisplay, 11);   
      analogWrite(second_led, 0);
      analogWrite(third_led, 0);
      analogWrite(first_led, 100);
      selectedDisplay = 1;          
    }
  }
}

// Turns the corresponding led on and increments the selectedDisplay counter
void selectButtonPressed()  {
  //turn the led off and the next one on
  //get the value from the dip switches and send to displays
  switch(selectedDisplay) {
    case 1:
      analogWrite(first_led, 0);
      analogWrite(second_led, 100);
      firstDisplayValue = readDIPSwitch();
      sendToDisplay(decoderPinsFirstDisplay, firstDisplayValue);
      if (firstDisplayValue > 9)  { firstDisplayValue = 0;  }
      //Serial.print("First Display (Ones) Value: ");
      //Serial.println(firstDisplayValue);        
      break;
    case 2:
      analogWrite(second_led, 0);
      analogWrite(third_led, 100);
      secondDisplayValue = readDIPSwitch();
      sendToDisplay(decoderPinsSecondDisplay, secondDisplayValue);
      if (secondDisplayValue > 9)  { secondDisplayValue = 0;  }
      //Serial.print("Second Display (Tens) Value: ");
      //Serial.println(secondDisplayValue);
      break;
    case 3:
      analogWrite(third_led, 0);
      analogWrite(first_led, 100);
      thirdDisplayValue = readDIPSwitch();
      sendToDisplay(decoderPinsThirdDisplay, thirdDisplayValue);
      if (thirdDisplayValue > 9)  { thirdDisplayValue = 0;  }
      //Serial.print("Third Display (100s) Value: ");
      //Serial.println(thirdDisplayValue);
      break;
  }
  //increase counter
  selectedDisplay += 1;
  if(selectedDisplay == 4)  {
    selectedDisplay = 1;
  }
}

// Function to read DIP switch values
int readDIPSwitch() {
  int number = 0;

  //read DIP switch states and calculate the binary number
  if (mcp1.digitalRead(dip0) == HIGH) number |= 0x01;  //LSB
  if (mcp1.digitalRead(dip1) == HIGH) number |= 0x02;
  if (mcp1.digitalRead(dip2) == HIGH) number |= 0x04;
  if (mcp1.digitalRead(dip3) == HIGH) number |= 0x08; //MSB

  return number;
}

// Function to send the binary number to the 7-segment display via CD4511 decoder (right side of equation)
void sendToDisplay(const int displayPins[], int value) {
  for (int i = 0; i < 4; i++) {
    mcp1.digitalWrite(displayPins[i], (value >> i) & 0x01);  //send each bit to the display
  }
}

// Function to send the binary number to the 7-segment display (left side of equation)
// NOTE: this only takes lowercase letters
void WriteDisplay(const int displayPins[], char value) {
  switch (value) {
    case '0':
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      mcp2.digitalWrite(displayPins[6], LOW);
      break;
    case '1':
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], LOW);
      }    
      mcp2.digitalWrite(displayPins[1], HIGH);
      mcp2.digitalWrite(displayPins[2], HIGH);
      break;
    case '2':
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      mcp2.digitalWrite(displayPins[2], LOW);
      mcp2.digitalWrite(displayPins[5], LOW);
      break;      
    case '3':
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      mcp2.digitalWrite(displayPins[4], LOW);
      mcp2.digitalWrite(displayPins[5], LOW);
      break;
    case '4':
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      mcp2.digitalWrite(displayPins[0], LOW);
      mcp2.digitalWrite(displayPins[3], LOW);
      mcp2.digitalWrite(displayPins[4], LOW);
      break;
    case '5':
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      mcp2.digitalWrite(displayPins[1], LOW);
      mcp2.digitalWrite(displayPins[4], LOW);
      break;
    case '6':
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      mcp2.digitalWrite(displayPins[1], LOW);
      break;      
    case '7':
      for(int i = 0; i < 3; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      for(int i = 3; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], LOW);
      }      
      break;      
    case '8':
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      break; 
    case '9':
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      mcp2.digitalWrite(displayPins[3], LOW);
      mcp2.digitalWrite(displayPins[4], LOW);      
      break;
    case 'a':
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      mcp2.digitalWrite(displayPins[3], LOW);
      break;           
    case 'b':   // b
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }   
      mcp2.digitalWrite(displayPins[0], LOW);
      mcp2.digitalWrite(displayPins[1], LOW);
      break;             
    case 'c':
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      mcp2.digitalWrite(displayPins[1], LOW);
      mcp2.digitalWrite(displayPins[2], LOW);      
      mcp2.digitalWrite(displayPins[6], LOW);
      break;
    case 'd':   // d
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      mcp2.digitalWrite(displayPins[0], LOW);
      mcp2.digitalWrite(displayPins[5], LOW);
      break;        
    case 'e':
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      mcp2.digitalWrite(displayPins[1], LOW);
      mcp2.digitalWrite(displayPins[2], LOW);
      break;        
    case 'f':
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], HIGH);
      }
      mcp2.digitalWrite(displayPins[1], LOW);
      mcp2.digitalWrite(displayPins[2], LOW);
      mcp2.digitalWrite(displayPins[3], LOW);
      break;        
    default:
      for(int i = 0; i < 7; i++)  {
        mcp2.digitalWrite(displayPins[i], LOW);
      }    
  }
}

// Flashing hardware components when power is first sent to the module (to make it look cool)
void poweringUp() {
  //turn everything off
  poweringOff();
  //blink the red leds in order
  analogWrite(first_led, 100);
  delay(550);
  analogWrite(second_led, 100);
  delay(550);
  analogWrite(third_led, 100);
  delay(550);
  analogWrite(GREEN_LED, 100);
  //gradually increase rgb brightness
  for(int i=255; i>=0; i-=5) {
    analogWrite(RGB1_RED, i);
    analogWrite(RGB2_RED, i);
    analogWrite(RGB3_RED, i);
    delay(10);
  }
  analogWrite(RGB1_RED, 255);
  analogWrite(RGB2_RED, 255);
  analogWrite(RGB3_RED, 255);   
  for(int i=255; i>=0; i-=5) {
    analogWrite(RGB1_GREEN, i);
    analogWrite(RGB2_GREEN, i);
    analogWrite(RGB3_GREEN, i);
    delay(10);
  }
  analogWrite(RGB1_GREEN, 255);
  analogWrite(RGB2_GREEN, 255);
  analogWrite(RGB3_GREEN, 255);  
  for(int i=255; i>=0; i-=5) {
    analogWrite(RGB1_BLUE, i);
    analogWrite(RGB2_BLUE, i);
    analogWrite(RGB3_BLUE, i);
    delay(10);
  }    
  //increment SSDs
  for(int a=0; a<10; a++) {
    sendToDisplay(decoderPinsFirstDisplay, a);
    sendToDisplay(decoderPinsSecondDisplay, a);
    sendToDisplay(decoderPinsThirdDisplay, a);
    WriteDisplay(decoderPinsFourthDisplay, '0' + a);
    WriteDisplay(decoderPinsFifthDisplay, '0' + a);
    delay(250);
  }
  WriteDisplay(decoderPinsFourthDisplay, 'a');
  WriteDisplay(decoderPinsFifthDisplay, 'a');
  delay(250);
  WriteDisplay(decoderPinsFourthDisplay, 'b');
  WriteDisplay(decoderPinsFifthDisplay, 'b'); 
  delay(250);
  WriteDisplay(decoderPinsFourthDisplay, 'c');
  WriteDisplay(decoderPinsFifthDisplay, 'c'); 
  delay(250);
  WriteDisplay(decoderPinsFourthDisplay, 'd');
  WriteDisplay(decoderPinsFifthDisplay, 'd'); 
  delay(250);
  WriteDisplay(decoderPinsFourthDisplay, 'e');
  WriteDisplay(decoderPinsFifthDisplay, 'e'); 
  delay(250);
  WriteDisplay(decoderPinsFourthDisplay, 'f');
  WriteDisplay(decoderPinsFifthDisplay, 'f');
  delay(250);
  //turn everything off
  poweringOff();
}

// Powers displays off and resets variables to 0
void poweringOff()  {
  //turn components off
  analogWrite(first_led, 0);
  analogWrite(second_led, 0);
  analogWrite(third_led, 0);
  analogWrite(GREEN_LED, 0);
  setRGB(1,255,255,255);
  setRGB(2,255,255,255);
  setRGB(3,255,255,255);
  sendToDisplay(decoderPinsFirstDisplay, 11);
  sendToDisplay(decoderPinsSecondDisplay, 11);
  sendToDisplay(decoderPinsThirdDisplay, 11);   
  WriteDisplay(decoderPinsFourthDisplay, 'Q');  
  WriteDisplay(decoderPinsFifthDisplay, 'Q'); 
  //reset vars
  selectedDisplay = 1;
  firstDisplayValue = 0;
  secondDisplayValue = 0;
  thirdDisplayValue = 0;
  fourthDisplayValue = 0;
  fifthDisplayValue = 0;
  targetValue = 0;
  String prompt = "";
  //freeze the controls
  Solved = true;
}

// Sends analogWrite() to the rgb leds
//NOTE: 255 is off, 0 is on
void setRGB(int componentNumber, int redValue, int blueValue,  int greenValue) {
  if(componentNumber == 1)  {
    analogWrite(RGB1_RED, redValue);
    analogWrite(RGB1_BLUE, blueValue);
    analogWrite(RGB1_GREEN, greenValue);
  }
  if(componentNumber == 2)  {
    analogWrite(RGB2_RED, redValue);
    analogWrite(RGB2_BLUE, blueValue);
    analogWrite(RGB2_GREEN, greenValue);
  }
  if(componentNumber == 3)  {
    analogWrite(RGB3_RED, redValue);
    analogWrite(RGB3_BLUE, blueValue);
    analogWrite(RGB3_GREEN, greenValue);
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
