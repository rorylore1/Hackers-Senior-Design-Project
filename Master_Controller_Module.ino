////////////////////////
//  Master Module
//  Created by Jacob Du
//  Using I2C connection with Wire.h library to communicate to the modules.
////////////////////////

#include <LiquidCrystal_I2C.h>
#include <Adafruit_MCP23X17.h>
#include <Wire.h>

//define pins for your I2Cs
#define led1_1 8  //GPB0-3
#define led1_2 9
#define led1_3 10
#define led1_4 11
#define led2_1 7  //GPA7-4
#define led2_2 6
#define led2_3 5
#define led2_4 4
#define led3_1 12 //GPB4-7
#define led3_2 13
#define led3_3 14
#define led3_4 15
#define led4_1 3  //GPA3-0
#define led4_2 2
#define led4_3 1
#define led4_4 0
//define hardwired address for each I2C
#define addr1 0x20	//(all pins went to ground)
Adafruit_MCP23X17 mcp1;
int timer;
int minutes, seconds;
int dis1, dis2, dis3, dis4;
int buttonPin = 13;   //to start game
int sensorPin = A0;
int sensorVal = 0;
int selectDiff = 0;
int prevSelectDiff;
int life1 = 27;
int life2 = 26;
int life3 = 25;
bool startFlag = false;
int lives = 3;
int ModulesInput = 12;   //input from the modules
int ModulesOutput = 17;  //output to the modules
bool buttonPressed = false;    //used for when a button is pressed
LiquidCrystal_I2C lcd_1(0x27, 16, 2);
//for button debouncing
unsigned long lastDebounceTime = 0;  // The last time the button state was checked
unsigned long debounceDelay = 50;    // The debounce time (in milliseconds)
int buttonState = 0;         // Current button state
int lastButtonState = 0;     // Previous button state
int numCompletedModules = 0;
TwoWire MasterComm = TwoWire(1);   //for the module communication
  //slave addresses
  //1 = equations
  //2 = hidden maze
  //3 = oscilloscope
  //4 = base conversions
  //5 = resistor
  //6 = logic gates

void setup()  {
  pinMode(buttonPin, INPUT);
  pinMode(sensorPin, INPUT);
  pinMode(life1, OUTPUT);
  pinMode(life2, OUTPUT);
  pinMode(life3, OUTPUT);
  pinMode(2, OUTPUT);
  lcd_1.init();
  lcd_1.setBacklight(1);
  mcp1.begin_I2C(addr1);
  mcp1.pinMode(led1_1, OUTPUT);
  mcp1.pinMode(led1_2, OUTPUT);
  mcp1.pinMode(led1_3, OUTPUT);
  mcp1.pinMode(led1_4, OUTPUT);
  mcp1.pinMode(led2_1, OUTPUT);
  mcp1.pinMode(led2_2, OUTPUT);
  mcp1.pinMode(led2_3, OUTPUT);
  mcp1.pinMode(led2_4, OUTPUT);
  mcp1.pinMode(led3_1, OUTPUT);
  mcp1.pinMode(led3_2, OUTPUT);
  mcp1.pinMode(led3_3, OUTPUT);
  mcp1.pinMode(led3_4, OUTPUT);
  mcp1.pinMode(led4_1, OUTPUT);
  mcp1.pinMode(led4_2, OUTPUT);
  mcp1.pinMode(led4_3, OUTPUT);
  mcp1.pinMode(led4_4, OUTPUT);
  // Start the serial monitor for debugging
  Serial.begin(9600);  
  // Start I2C communication as master
  MasterComm.begin(12, 17, 100000); 
  //set the 7-seg displays to off
  writeDigit(mcp1, 0, led1_1, led1_2, led1_3, led1_4);  
  writeDigit(mcp1, 0, led2_1, led2_2, led2_3, led2_4);  
  writeDigit(mcp1, 0, led3_1, led3_2, led3_3, led3_4);  
  writeDigit(mcp1, 0, led4_1, led4_2, led4_3, led4_4);
}

void loop() { 
  /*
  MasterComm.requestFrom(6, 1);
  char moduleValue = MasterComm.read();
  Serial.print("recieving value from slave: ");
  Serial.println(moduleValue);
  delay(1000);
  */
  //when game is in select difficulty mode
  if (!startFlag)  {
    sensorVal = analogRead(sensorPin);
    if (sensorVal < 200)
      selectDiff = 0;
    else if (sensorVal < 2000)    //easy mode
      selectDiff = 1;
    else if(sensorVal < 3300)     //medium mode
      selectDiff = 2;
    else                          //hard mode
      selectDiff = 3;
    //to prevent the screen from constantly refreshing
    if (selectDiff != prevSelectDiff) {
      prevSelectDiff = selectDiff;
      lcd_1.setCursor(0, 1);
      switch(selectDiff)  {
        case 0:
            lcd_1.clear();
            lcd_1.print("Select");
            break;
        case 1:
            lcd_1.clear();
            lcd_1.print("Easy Mode");
            break;
        case 2:
            lcd_1.clear();
            lcd_1.print("Medium Mode");
            break;
        case 3:
            lcd_1.clear();
            lcd_1.print("Hard Mode");
            break;
      }
    }
    //if the button state has changed, reset the debounce timer
    int reading = digitalRead(buttonPin);
    if (reading != lastButtonState) {
      lastDebounceTime = millis();
    }
    //if button has been in a stable state for longer than debounceDelay, register it
    if ((millis() - lastDebounceTime) > debounceDelay) {
      //if the button state has changed and it's a press (LOW means pressed)
      if (reading != buttonState) {
        buttonState = reading;
        //starting the game
        if (buttonState == HIGH && selectDiff != 0) {   
          Serial.println("slave addresses: 1=equations, 2=hidden maze, 3=oscilloscope, 4=base conversions, 5=resistor, 6=logic gates");
          // Start transmission to modules
          for(int a=1; a<7; a++)  {
            MasterComm.beginTransmission(a);
            MasterComm.write(selectDiff);
            MasterComm.endTransmission();
            Serial.print("Sent value: ");
            Serial.println(selectDiff);
          }
          startFlag = true;
          lcd_1.clear();
          lcd_1.print("Good Luck!");          
          lives = 3;
          livesUpdate(lives, life1, life2, life3);
          switch(selectDiff)  {
            case 1:
            timer = 1200;
            break;
            case 2:
            timer = 600;
            break;
            case 3:
            timer = 300;
            break;
          }
        }
      }
    }   
    lastButtonState = reading;  //save the current reading as the last state 
  }
  //when the game has started
  if (startFlag) {
    //when the game is going until you run out of lives or the timer runs out
    while(lives != 0 && timer >= 0) {
    //if button pressed while game running, reset to main menu
      int reading = digitalRead(buttonPin);
      if (reading != lastButtonState) {
        lastDebounceTime = millis();
      }
      if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
          buttonState = reading;
          //starting the game
          if (buttonState == HIGH) {     
            gameOver();
          }
        }
      }
      lastButtonState = reading;  //save the current reading as the last state
    ///////
      for(int a=1; a<7; a++)  {
        MasterComm.requestFrom(a, 1);
        unsigned moduleValue = MasterComm.read();
        Serial.print("recieving value from slave: ");
        Serial.println(moduleValue);
        if(moduleValue == 8)  {     //if module failed
          lives--;
          livesUpdate(lives, life1, life2, life3);
          if(lives == 0)  {
            lcd_1.clear();
            lcd_1.print("WOMP WOMP");
            gameOver();
            break;
          }
        }
        if(moduleValue == 9)  {     //if module passed
          numCompletedModules += 1;
          if(numCompletedModules == 6)  {
            lcd_1.clear();
            lcd_1.setCursor(0, 1);
            lcd_1.print("Vault Breached!");
            lcd_1.print("You win!!");
            gameOver();
          }
        } 
      }
      minutes = timer / 60;
      seconds = timer % 60;
      dis1 = minutes / 10;
      dis2 = minutes % 10;
      dis3 = seconds / 10;
      dis4 = seconds % 10;
      writeDigit(mcp1, dis1, led1_1, led1_2, led1_3, led1_4);  
      writeDigit(mcp1, dis2, led2_1, led2_2, led2_3, led2_4);  
      writeDigit(mcp1, dis3, led3_1, led3_2, led3_3, led3_4);  
      writeDigit(mcp1, dis4, led4_1, led4_2, led4_3, led4_4);
      timer--;
      if(timer == 0)  {
        lcd_1.clear();
        lcd_1.print("WOMP WOMP");
        delay(3000);
        lives = 0;
        livesUpdate(lives, life1, life2, life3);
        gameOver();
        break;
      }
      delay(900);
    }
    gameOver();
  }    
}

//resets the game to the start menu
void gameOver() {
  delay(1000);    //give 1 sec before we reset
  //reset modules     
  for(int a=1; a<7; a++)  {
    MasterComm.beginTransmission(a);
    MasterComm.write(9);
    MasterComm.endTransmission();
    Serial.println("Sent value: 9");
  }
  //reset master controls
  timer = 0;
  lcd_1.clear();
  writeDigit(mcp1, 0, led1_1, led1_2, led1_3, led1_4);  
  writeDigit(mcp1, 0, led2_1, led2_2, led2_3, led2_4);  
  writeDigit(mcp1, 0, led3_1, led3_2, led3_3, led3_4);  
  writeDigit(mcp1, 0, led4_1, led4_2, led4_3, led4_4);
  digitalWrite(life1, LOW);
  digitalWrite(life2, LOW);
  digitalWrite(life3, LOW);  
  startFlag = false;
  prevSelectDiff = -1;
}

void livesUpdate(int currLives, int l1, int l2, int l3)
{
  switch (currLives)
  {
    case 0:
      digitalWrite(l1, LOW);
      digitalWrite(l2, LOW);
      digitalWrite(13, LOW);
      break;
    case 1:
      digitalWrite(11, HIGH);
      digitalWrite(l2, LOW);
      digitalWrite(13, LOW);
      break;
    case 2:
      digitalWrite(l1, HIGH);
      digitalWrite(l2, HIGH);
      digitalWrite(l3, LOW);
      break;
    case 3:
      digitalWrite(l1, HIGH);
      digitalWrite(l2, HIGH);
      digitalWrite(l3, HIGH);
      break;
    default:
      digitalWrite(l1, LOW);
      digitalWrite(l2, LOW);
      digitalWrite(13, LOW);
      break;
  }
}

void writeDigit(Adafruit_MCP23X17 mcp1, int digit, int bit1, int bit2, int bit3, int bit4)
{
  switch(digit)
  {
    case 0:
    mcp1.digitalWrite(bit1, LOW);
    mcp1.digitalWrite(bit2, LOW);
    mcp1.digitalWrite(bit3, LOW);
    mcp1.digitalWrite(bit4, LOW);
    break;
    case 1:
    mcp1.digitalWrite(bit1, HIGH);
    mcp1.digitalWrite(bit2, LOW);
    mcp1.digitalWrite(bit3, LOW);
    mcp1.digitalWrite(bit4, LOW);
    break;
    case 2:
    mcp1.digitalWrite(bit1, LOW);
    mcp1.digitalWrite(bit2, HIGH);
    mcp1.digitalWrite(bit3, LOW);
    mcp1.digitalWrite(bit4, LOW);
    break;
    case 3:
    mcp1.digitalWrite(bit1, HIGH);
    mcp1.digitalWrite(bit2, HIGH);
    mcp1.digitalWrite(bit3, LOW);
    mcp1.digitalWrite(bit4, LOW);
    break;
    case 4:
    mcp1.digitalWrite(bit1, LOW);
    mcp1.digitalWrite(bit2, LOW);
    mcp1.digitalWrite(bit3, HIGH);
    mcp1.digitalWrite(bit4, LOW);
    break;
    case 5:
    mcp1.digitalWrite(bit1, HIGH);
    mcp1.digitalWrite(bit2, LOW);
    mcp1.digitalWrite(bit3, HIGH);
    mcp1.digitalWrite(bit4, LOW);
    break;
    case 6:
    mcp1.digitalWrite(bit1, LOW);
    mcp1.digitalWrite(bit2, HIGH);
    mcp1.digitalWrite(bit3, HIGH);
    mcp1.digitalWrite(bit4, LOW);
    break;
    case 7:
    mcp1.digitalWrite(bit1, HIGH);
    mcp1.digitalWrite(bit2, HIGH);
    mcp1.digitalWrite(bit3, HIGH);
    mcp1.digitalWrite(bit4, LOW);
    break;
    case 8:
    mcp1.digitalWrite(bit1, LOW);
    mcp1.digitalWrite(bit2, LOW);
    mcp1.digitalWrite(bit3, LOW);
    mcp1.digitalWrite(bit4, HIGH);
    break;
    case 9:
    mcp1.digitalWrite(bit1, HIGH);
    mcp1.digitalWrite(bit2, LOW);
    mcp1.digitalWrite(bit3, LOW);
    mcp1.digitalWrite(bit4, HIGH);
    break;
  }
}