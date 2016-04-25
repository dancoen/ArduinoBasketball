/* Group 14 Systems Programming Final Project
Dan Coen, Max Friedman, Joseph Haaga
*/

#include <SoftwareSerial.h>  
#include <EEPROM.h>
#include <Wire.h>
#include <Arduino.h>
#include <LiquidCrystal.h>
#define DS3231_I2C_ADDRESS 0x68

// LCD setup
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Game Variables
int gameStartTime = 0;
int gameScore = 0;
int gameInProgress = 0;

//a4 is serial clock
// a5 is data


float distanceSensorValue = 0.0;


int bluetoothTx = 6;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 7;  // RX-I pin of bluetooth mate, Arduino D3

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);
 
int fsrAnalogPin = 0; // FSR is connected to analog 0
//int LEDpin = 13;      // connect Red LED to pin 11 (PWM pin)
int fsrReading;      // the analog reading from the FSR resistor divider
//int LEDbrightness;



///////////////////////////////////////////////////////////////////////////////

//////// LCD CODE /////////

void lcdDisplayText(String text)
{
  lcd.print(text);
}

void clear_lcd(){
  lcd.print("        ");
}

//////// RTC CODE /////////

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}

void readDS3231time(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

int getSeconds()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);
  return second;
}
 
void setup(void) {
   Serial.begin(9600);  // Begin the serial monitor at 9600bps

  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  bluetooth.print("$");  // Print three times individually
  bluetooth.print("$");
  bluetooth.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  bluetooth.begin(9600);  // Start bluetooth serial at 9600
//     EEPROM.write(0,30);
//     EEPROM.write(1,25);
//     EEPROM.write(2,20);
//     EEPROM.write(3,15);
//     EEPROM.write(4,10);
    //}
  //pinMode(LEDpin, OUTPUT);
  lcd.begin(8, 1);
  Wire.begin();
  setDS3231time(00,0,0,7,27,12,14);
}
int seconds;

 
void loop(void) {
  //bluetooth.println("hey");
  seconds = (int) getSeconds();
  fsrReading = analogRead(fsrAnalogPin);
  if (fsrReading > 10){
    distanceSensorValue = analogRead(A2) * 5 /1023.0;
    //Serial.print("Analog reading = ");
    //Serial.println(fsrReading);
    if(gameInProgress==1){
      if(distanceSensorValue<0.5){
        Serial.print("sweet shot\n");
        bluetooth.println("sweet shot\n");
        gameScore+=2;
        bluetooth.print("score is ");
        bluetooth.println(gameScore);
      }else{
        Serial.print("$wi$h");
        bluetooth.println("3 pointer!!");
        gameScore+=3;
        bluetooth.print("score is ");
        bluetooth.println(gameScore);
      }
      Serial.print("\nSensor Value:");
      Serial.print(distanceSensorValue);
      Serial.print("\n");
    }else{
      Serial.print("starting new game...");
      bluetooth.println("starting new game...");
      gameInProgress=1;
      gameStartTime=30;
      setDS3231time(00,0,0,7,27,12,14);
      seconds = (int) getSeconds();
    }
    //Serial.print("$$$");
    
    
    delay(500);
    /*Serial.println(EEPROM.read(0));
    Serial.println(EEPROM.read(1));
    Serial.println(EEPROM.read(2));
    Serial.println(EEPROM.read(3));
    Serial.println(EEPROM.read(4));
    //Serial.println(EEPROM.read(1),DEC);
    */
    
  }
  if(gameStartTime-seconds <= 1 && gameInProgress){
    lcdDisplayText("1");
    delay(1000);
    clear_lcd();
    //lcdDisplayText("gameover");
   // game has run out of time
   gameInProgress=0;
   bluetooth.print("your score was ");
   bluetooth.println(gameScore);
   if(gameScore > EEPROM.read(0)){
    EEPROM.write(4,EEPROM.read(3));
    EEPROM.write(3,EEPROM.read(2));
    EEPROM.write(2,EEPROM.read(1));
    EEPROM.write(1,EEPROM.read(0));
    EEPROM.write(0,gameScore);
    bluetooth.println("you made the highscore list!");
    bluetooth.print("your score is number ");
    bluetooth.println("1!!!!!! HYPE MATRON-3000!");
    }
   else if(gameScore > EEPROM.read(1)){
    EEPROM.write(4,EEPROM.read(3));
    EEPROM.write(3,EEPROM.read(2));
    EEPROM.write(2,EEPROM.read(1));
    EEPROM.write(1,gameScore);
    bluetooth.println("you made the highscore list!");
    bluetooth.print("your score is number ");
    bluetooth.println("2. NICE SHOOTING HYPE MONSTER");
    }
   else if(gameScore > EEPROM.read(2)){
    EEPROM.write(4,EEPROM.read(3));
    EEPROM.write(3,EEPROM.read(2));
    EEPROM.write(2,gameScore);
    bluetooth.println("you made the highscore list!");
    bluetooth.print("your score is number ");
    bluetooth.println("3. HYPE. I guess");
    }
   else if(gameScore > EEPROM.read(3)){
    EEPROM.write(4,EEPROM.read(3));
    EEPROM.write(3,gameScore);
    bluetooth.println("you made the highscore list!");
    bluetooth.print("your score is number ");
    bluetooth.println("4. good job. I guess.");
    }
   else if(gameScore > EEPROM.read(4)){
    EEPROM.write(4,gameScore);
    bluetooth.println("you made the highscore list!");
    bluetooth.print("your score is number ");
    bluetooth.println("5. Decent I guess.");
    }
    bluetooth.println("Highscores:");
    delay(100);
    bluetooth.print("1. "); bluetooth.println(String(EEPROM.read(0)));
    delay(100);
    bluetooth.print("2. "); bluetooth.println(String(EEPROM.read(1)));
    delay(100);
    bluetooth.print("3. "); bluetooth.println(String(EEPROM.read(2)));
    delay(100);
    bluetooth.print("4. "); bluetooth.println(String(EEPROM.read(3)));
    delay(100);
    bluetooth.print("5. "); bluetooth.println(String(EEPROM.read(4)));
    
   gameScore = 0;
   bluetooth.println("Game over. Press pad to restart");
   
   // write score to leaderboard
  }
  if(gameInProgress == 1){
    if (gameStartTime - seconds == 9){
        lcdDisplayText("9 ");
      }
    else{lcdDisplayText(String(gameStartTime - seconds));}
    }
  
  lcd.setCursor(0,0);
  /*if (gameStartTime > 30) {
    Serial.println(60 - seconds, DEC);
    lcdDisplayText(String(60 - seconds));
  } else {
    Serial.println(30 - seconds, DEC);
    lcdDisplayText(String(30 - seconds));
  }
  */
  //delay(1000);
}

