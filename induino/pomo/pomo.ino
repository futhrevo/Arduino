
/*  Induino R3 User Guide - Program 10.0 - Interfacing a Character LCD using the LCD Shield to display LDR value on the LCD */
/* Pin Mappings as per the Simple Labs' LCD shield
   LCD RS pin to digital pin 8
   LCD Enable pin to digital pin 9
   LCD D4 pin to digital pin 10
   LCD D5 pin to digital pin 11
   LCD D6 pin to digital pin 12
   LCD D7 pin to digital pin 13
   LCD R/W pin to ground
*/

// include the library code:
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <RTClib.h>
#include <Wire.h>
#include "IRremote.h"

// initialize the library with the numbers of the interface pins from the above pin mappings
LiquidCrystal lcd(8, 9, 10, 11, 12, 13); //
// https://www.carnetdumaker.net/articles/faire-une-barre-de-progression-avec-arduino-et-liquidcrystal/
// Constants for the size of the LCD screen
const int LCD_NB_ROWS = 2 ;
const int LCD_NB_COLUMNS = 16 ;
const int TDELAY = 25 * 60;
const int receiver = 15;
const int buzzer = 3;


RTC_DS1307 RTC;
IRrecv irrecv(receiver);           // create instance of 'irrecv'
decode_results results;
DateTime now;
long then;
int setRTC = 0;
bool running = false;
int bhour = -1;
int bminute = -1;

//  Custom characters
byte DIV_0_OF_5 [ 8 ] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};  // 0/5

byte DIV_1_OF_5 [ 8 ] = {
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000
};  // 1/5

byte DIV_2_OF_5 [ 8 ] = {
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000
};  // 2/5

byte DIV_3_OF_5 [ 8 ] = {
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100
};  // 3/5

byte DIV_4_OF_5 [ 8 ] = {
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110
};  // 4/5

byte DIV_5_OF_5 [ 8 ] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};  // 5/5

byte mySmiley[8] = {
  B00000,
  B01010,
  B01010,
  B01010,
  B00000,
  B10001,
  B01110,
  B00000
};
//
//  LCD screen setup function for the progress bar.
//  Uses custom characters from 0 to 6 (7 remain available).
//
void setup_progressbar () {

  //     Save custom characters in the LCD screen memory
  lcd.createChar ( 0 , DIV_0_OF_5);
  lcd.createChar ( 1 , DIV_1_OF_5);
  lcd.createChar ( 2 , DIV_2_OF_5);
  lcd.createChar ( 3 , DIV_3_OF_5);
  lcd.createChar ( 4 , DIV_4_OF_5);
  lcd.createChar ( 5 , DIV_5_OF_5);
  lcd.createChar ( 6 , mySmiley);
}

//Function drawing the progress bar.
//
//@param percent The percentage to display.

void draw_progressbar () {

  //     Display the new numeric value on the first line
  lcd.setCursor ( 0 , 0 );
  lcdOutput(now);
  regularAlert(now);
  // NB The two spaces at the end of the line allow to clear the figures of the percentage
  // previous when you change from a two or three digit value to a two or one digit value.

  byte nb_columns;
  lcd.setCursor(15,0);
  if (running) {
    lcd.write(6);
    int left = then - now.unixtime();
    if (left < 0) {
      left = 0;
      running = false;
      buttonClick();
      delay(200);
      longbeep();
    }
    nb_columns = map (left, 0 , TDELAY , 0 , LCD_NB_COLUMNS * 5 );
  } else {
    lcd.print(" ");
    // Map the range (0 ~ 100) to the range (0 ~ LCD_NB_COLUMNS * 5)
    nb_columns = map (now.second(), 0 , 59 , 0 , LCD_NB_COLUMNS * 5 );
  }

  // Move the cursor to the second line
  lcd.setCursor ( 0 , 1 );
  // Draw each character of the line
  for (byte i = 0 ; i < LCD_NB_COLUMNS; ++ i) {

    // Depending on the number of columns remaining to display
    if (nb_columns == 0 ) { // empty box
      lcd.write ((byte) 0 );

    } else if (nb_columns >= 5 ) { // Full case
      lcd.write ( 5 );
      nb_columns -= 5 ;

    } else { // Last box not empty
      lcd.write (nb_columns);
      nb_columns = 0 ;
    }
  }
}

void buttonClick() {
    tone(buzzer,15000, 50);  
}

void longbeep() {
  tone(buzzer, 15550, 150);
}

void regularAlert(DateTime now) {
  if(now.hour()!= bhour) {
    bhour = now.hour();
    bminute = 30;
    tone(buzzer, 14750, 250);
    return;
  }
  if(now.minute() == bminute){
    bminute = 0;
    tone(buzzer, 14750, 100);
  }
}

void setup() {
  // set up the LCD's number of characters per line and lines:
  // Initialize the LCD screen
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  lcd.begin (LCD_NB_COLUMNS, LCD_NB_ROWS);
  setup_progressbar ();
  lcd.clear ();
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning() || setRTC == 1) {
    Serial.println("RTC is NOT running!");
    //    following line sets the RTC to the date &amp; time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  pinMode (buzzer, OUTPUT);
}

void loop() {
  now = RTC.now();
  if (irrecv.decode(&results)) // have we received an IR signal?
  {
    //    Serial.println(results.value, HEX);  // UN Comment to see raw values
    translateIR();
    irrecv.resume(); // receive the next value
  }
  // Displays the value
  draw_progressbar ();
  // Small waiting time
  delay ( 100 );

}

void printDigits(int digits)  //this void function is really useful; it adds a "0" to the beginning of the number, so that 5 minutes is displayed as "00:05:00", rather than "00:5 :00"
{
  if (digits < 10)
  {
    lcd.print("0");
    lcd.print(digits);
  }
  else
  {
    lcd.print(digits);
  }
}

void lcdOutput(DateTime now)  //this is just used to display the timer on the LCD
{
  lcd.setCursor(0, 0);
  printDigits(now.hour());
  lcd.print(":");
  printDigits(now.minute());
  lcd.print(":");
  printDigits(now.second());
  delay(100);
}

/*-----( Declare User-written Functions )-----*/
void translateIR() // takes action based on IR code received

// describing KEYES Remote IR codes

{
  switch (results.value)

  {

    //  case 0xFF629D: Serial.println(" FORWARD"); break;
    //  case 0xFF22DD: Serial.println(" LEFT");    break;
    //  case 0xFF02FD: Serial.println(" -OK-");    break;
    //  case 0xFFC23D: Serial.println(" RIGHT");   break;
    //  case 0xFFA857: Serial.println(" REVERSE"); break;
    //  case 0xFF6897: Serial.println(" 1");    break;
    //  case 0xFF9867: Serial.println(" 2");    break;
    //  case 0xFFB04F: Serial.println(" 3");    break;
    //  case 0xFF30CF: Serial.println(" 4");    break;
    case 0xFF18E7: // SEL Button
      buttonClick();
      if (!running) {
        then = now.unixtime() + TDELAY;
        running = true;
      }
      break;
    //      case 0xFF7A85: Serial.println(" 6");    break;
    //      case 0xFF10EF: Serial.println(" 7");    break;
    //      case 0xFF38C7: Serial.println(" 8");    break;
    //      case 0xFF5AA5: Serial.println(" 9");    break;
    case 0xFF42BD:
      buttonClick();
      running = false;
      break;
    //      case 0xFF4AB5: Serial.println(" 0");    break;
    //      case 0xFF52AD: Serial.println(" #");    break;
    //  case 0xFFFFFFFF: Serial.println(" REPEAT");break;

    default:
      Serial.println(" other button   ");

  }// End Case

} //END translateIR
