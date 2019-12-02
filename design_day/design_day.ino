#include <SoftwareSerial.h>
#include <SPI.h>
#include <mcp_can_dfs.h>
#include <mcp_can.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

int demo = 5;
int contr = 4;
int ext = 6;
int potPin = A0;
int spiCSPin = 10;
int val = 0;
unsigned char pos = 0;
const int btn = 2;
const int tuning = 3;
int selector = 0;
boolean isPressed = false;
unsigned char buf[8];

SoftwareSerial serial(10,11);
LiquidCrystal_I2C lcd(0x27,16,2);                           //I2C address of 0x27
MCP_CAN CAN(spiCSPin);                                      //Initializes the CAN pin to be spiCSPin = pin 10

void setup() {
  lcd.init();                                               //Initialize the library to use the LCD
  lcd.backlight();                                          //Turns on the backlight on the screen (optional)
  lcd.home();                                               //Sets the cursor home location
  Serial.begin(38400);                                      //Sets the baud rate of the serial communication
  pinMode(btn, INPUT_PULLUP);                               //Sets the button pin to be an input with a pull up
  pinMode(tuning, OUTPUT);                                  //Sets the tuning pin as an output
  pinMode(ext, OUTPUT);                                     //Sets the ext pin as an output
  pinMode(contr, OUTPUT);                                   //Sets the contr pin as an output
  pinMode(demo, OUTPUT);                                    //Sets the demo pin as an output
  pinMode(potPin, INPUT);                                   //Sets the potentiometer pin as an input
  while (CAN_OK != CAN.begin(CAN_1000KBPS)) {               //Sets the baud rate for the CAN bus to be 1Mbps
    lcd.print("CAN BUS Init Failed");                       //Provides feedback to the LCD to print
    delay(100);                                             //Slows down the print speed so it is readable 
  }
  lcd.print("CAN BUS Init OK!");                            //Feedbacks that the initialization is set
}

void loop ()
{
  if (digitalRead(btn) == LOW && isPressed == false )       //Button is pressed AND this is the first digitalRead() that the button is pressed
  {
    isPressed = true;                                       //Set to true, so this code will not run again until button released
    doSwitchStatement();                                    //A call to a separate function that performs the switch statement and subsequent evoked code
    selector++;                                             //This is done after the doSwitchStatement(), so case 0 will be executed on the first button press
    if (selector > 3) {                                     //Resets the selector to go back to 0 after it gets to 3
      selector = 0;
    }
    // selector = (selector+1) % 4;                         // does the same, without if-statement (Alternative suggested method)
  } else if (digitalRead(btn) == HIGH)                      //If the button is not pressed, it will read high
  {
    isPressed = false;                                      //button is released, variable reset
  }
}

void doSwitchStatement() {                                  //Sets up the switch statments to select between the cases (Settings on the device)
  switch(selector) {                                        
  case 0:                                                   //Runs the RPM Tuning Mode
    digitalWrite(contr, LOW);                               //All the subsiquent LOW settings are to turn off LED's for each mode. Without it, all LED's will slowly turn on as modes are toggled
    digitalWrite(demo, LOW);
    digitalWrite(ext, LOW);
    digitalWrite(tuning, HIGH);                             //Sets the tuning pin HIGH
    lcd.clear();                                            //Clears the LCD
    delay(250);
    doRun();
    break;
  case 1:                                                    //Fully Extends the actuator
    digitalWrite(contr, LOW);
    digitalWrite(demo, LOW);
    digitalWrite(ext, HIGH);
    digitalWrite(tuning, LOW);
    lcd.clear();
    lcd.print("Fully Extended");
    delay(250);
    doExtend();
    break;
  case 2:                                                     //Fully Contracts the actuator
    digitalWrite(demo, LOW);
    digitalWrite(ext, LOW);
    digitalWrite(tuning, LOW);
    digitalWrite(contr, HIGH);
    lcd.clear();
    lcd.print("Fully Contracted");
    delay(250);
    doContract();
    break;
  case 3:                                                      //Runs the boundary demo of the actuator
    digitalWrite(ext, LOW);
    digitalWrite(tuning, LOW);
    digitalWrite(contr, LOW);
    digitalWrite(demo, HIGH);
    lcd.clear();
    lcd.print("Boundary Demo");
    delay(250);
    doDemo();
    break;
  }
}

void doRun(){
      do {                                                      //Runs the RPM loop 10,000 times, unless the button is pressed
          val = analogRead(potPin);                             
          
          pos = map(val, 0, 1023, 0, 255);                      //
          unsigned char buf[8] = {pos,0,0,0,0,0,0,0};
          CAN.sendMsgBuf(0x520, 0, 8, buf);
          
          lcd.setCursor(0,0);
          lcd.print("RPM: ");
          float  RPM_Test = map(pos, 0, 255, 15350, 0);
          lcd.print(RPM_Test);
          
          lcd.setCursor(0,3);
          float len = map(val, 0, 1023, 0.000, 5.000);
          lcd.print("Length: ");
          lcd.print(len);
      } while (digitalRead(btn) == HIGH);
  return;
}

void doExtend(){
  do{
  unsigned char buf[8] = {255,0,0,0,0,0,0,0};
  Serial.print(buf[0]);
  CAN.sendMsgBuf(0x520, 0, 8, buf);
  lcd.setCursor(0,3);
  lcd.print("Length: 5in    ");
  } while (digitalRead(btn) == HIGH);
  return;
}

void doContract(){
  do{
  unsigned char buf[8] = {0,0,0,0,0,0,0,0};
  CAN.sendMsgBuf(0x520, 0, 8, buf);
  lcd.print(buf[0]);
  lcd.setCursor(0,3);
  lcd.print("Length: 0in    ");
  } while (digitalRead(btn) == HIGH);
  return;
}

void doDemo(){
  do{
    unsigned char buf[8] = {255,0,0,0,0,0,0,0};
    Serial.print(buf[0]);
    CAN.sendMsgBuf(0x520, 0, 8, buf);
    lcd.setCursor(0,3);
    lcd.print("Length: 5in    ");
    if(digitalRead(btn) == LOW){
      return;
    }
    delay(3000);
    unsigned char buf_2[8] = {0,0,0,0,0,0,0,0};
    CAN.sendMsgBuf(0x520, 0, 8, buf_2);
    lcd.print(buf[0]);
    lcd.setCursor(0,3);
    lcd.print("Length: 0in    ");
    if(digitalRead(btn) == LOW){
      return;
    }
    delay(3000);
  }while(digitalRead(btn) == HIGH);
  home();
  return;
}

void home(){
  do{
  unsigned char buf[8] = {0,0,0,0,0,0,0,0};
  CAN.sendMsgBuf(0x520, 0, 8, buf);
  lcd.setCursor(0,3);
  lcd.print("Homing Actuator    ");
  }while(digitalRead(btn) == HIGH);
  return;
}
