#include <SoftwareSerial.h>
#include <SPI.h>
#include <mcp_can_dfs.h>
#include <mcp_can.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

int potPin = A0;
int spiCSPin = 10;
int val = 0;
unsigned char pos = 0;
const int btn = 2;
const int led = 3;
int selector = 0;
boolean isPressed = false;
unsigned char buf[8];

SoftwareSerial serial(10,11);
LiquidCrystal_I2C lcd(0x27,16,2);  //I have used I2C scan program, so address is correct.
MCP_CAN CAN(spiCSPin);

void setup() {
  lcd.init(); //Initialize the library to use the LCD
  lcd.backlight(); //Turns on the backlight on the screen (optional)
  lcd.home(); //Sets the cursor home location
  Serial.begin(38400);
  pinMode(btn, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(potPin, INPUT);
  while (CAN_OK != CAN.begin(CAN_1000KBPS)) {
    lcd.print("CAN BUS Init Failed");
    delay(100);
  }
  lcd.print("CAN BUS Init OK!");
}

void loop ()
{
  if (digitalRead(btn) == LOW && isPressed == false ) //button is pressed AND this is the first digitalRead() that the button is pressed
  {
    isPressed = true;  //set to true, so this code will not run again until button released
    doSwitchStatement(); // a call to a separate function that performs the switch statement and subsequent evoked code

    selector++; // this is done after the doSwitchStatement(), so case 0 will be executed on the first button press
    if (selector > 3) {
      selector = 0;
    }
    // selector = (selector+1) % 4;  // does the same, without if-statement
  } else if (digitalRead(btn) == HIGH)
  {
    isPressed = false; //button is released, variable reset
  }
}

void doSwitchStatement() {
  switch(selector) {
  case 0:
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(led, HIGH);
    lcd.clear();
    //lcd.print("RPM Tuning");
    delay(250);
    doRun();
    break;
  case 1:
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, HIGH);
    digitalWrite(led, LOW);
    lcd.clear();
    lcd.print("Fully Extended");
    delay(250);
    doExtend();
    break;
  case 2:
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(led, LOW);
    digitalWrite(4, HIGH);
    lcd.clear();
    lcd.print("Fully Contracted");
    delay(250);
    doContract();
    break;
  case 3:
    digitalWrite(6, LOW);
    digitalWrite(led, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, HIGH);
    lcd.clear();
    lcd.print("Boundary Demo");
    delay(250);
    doDemo();
    break;
  }
}

void doRun(){
  lcd.print("RPM: ");
    for(int i = 0; i<=10000; i++){
      lcd.clear();
      val = analogRead(potPin);
      pos = map(val, 0, 1023, 0, 255);
      unsigned char buf[8] = {pos,0,0,0,0,0,0,0};
      CAN.sendMsgBuf(0x520, 0, 8, buf);
      lcd.print("RPM: ");
      float  RPM_Test = map(pos, 0, 255, 15350, 0);
      lcd.print(RPM_Test);
      lcd.setCursor(0,3);
      float len = map(pos, 0, 255, 0, 8);
      lcd.print("Length: ");
      lcd.print(len);
      if(digitalRead(btn) == LOW){
         return;
      }
  }
  return;
}

void doExtend(){
  pos = 127;
  unsigned char buf[8] = {pos,0,0,0,0,0,0,0};
  CAN.sendMsgBuf(0x520, 0, 8, buf);
  lcd.setCursor(0,3);
  lcd.print("Length: 8in  ");
  return;
}

void doContract(){
  pos = 0;
  unsigned char buf[8] = {pos,0,0,0,0,0,0,0};
  CAN.sendMsgBuf(0x520, 0, 8, buf);
  lcd.print(buf[0]);
  lcd.setCursor(0,3);
  lcd.print("Length: 0in  ");
  return;
}

void doDemo(){
  for (int i = 0; i<=4; i++){
    doExtend();
    if(digitalRead(btn) == LOW){
      return;
    }
    delay(10000);
    doContract();
    if(digitalRead(btn) == LOW){
      return;
    }
    delay(10000);
  }
  return;
}
