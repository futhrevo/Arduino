
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
#include "DHT.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <RTClib.h>
#include <Wire.h>
#include "IRremote.h"
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#define DHTPIN 16 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

unsigned int localPort = 8888;       // local port to listen for UDP packets

const char timeServer[] = "in.pool.ntp.org"; // time.nist.gov NTP server

const byte NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// pushing box
const char server[] = "api.pushingbox.com"; //YOUR SERVER
EthernetClient client;

// initialize the library with the numbers of the interface pins from the above pin mappings
LiquidCrystal lcd1(2, 14, 5, 6, 7, 8); //
LiquidCrystal lcd2(2, 3, 5, 6, 7, 8);
// https://www.carnetdumaker.net/articles/faire-une-barre-de-progression-avec-arduino-et-liquidcrystal/
// Constants for the size of the LCD screen
const byte LCD_NB_ROWS = 2 ;
const byte LCD_NB_COLUMNS = 16 ;
const int TDELAY = 25 * 60;
const byte receiver = 15;
const byte buzzer = 9;
const char daysOfTheWeek[7][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
const char tithi[16][4] = {"AMS", "PAD", "DWI", "TRI", "CHA", "PAN", "SHA", "SAP", "AST", "NAV", "DAS", "EKA", "DVA", "THR", "CTD", "PUN"};

DHT dht(DHTPIN, DHTTYPE);
RTC_DS1307 RTC;
IRrecv irrecv(receiver);           // create instance of 'irrecv'
decode_results results;
DateTime now;
long then;
bool running = false;
int bhour = -1;
int bminute = -1;
float h = 0.0;
float t = 99.0;
bool checkdht = true;
bool checkntp = true;
bool link = false;

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
  lcd1.createChar ( 0 , DIV_0_OF_5);
  lcd1.createChar ( 1 , DIV_1_OF_5);
  lcd1.createChar ( 2 , DIV_2_OF_5);
  lcd1.createChar ( 3 , DIV_3_OF_5);
  lcd1.createChar ( 4 , DIV_4_OF_5);
  lcd1.createChar ( 5 , DIV_5_OF_5);
  lcd1.createChar ( 6 , mySmiley);
}

//Function drawing the progress bar.
//
//@param percent The percentage to display.

void draw_progressbar () {

  //     Display the new numeric value on the first line
  lcd1.setCursor ( 0 , 0 );
  lcdOutput(now);
  regularAlert(now);
  getDHT(now);
  getNTP(now);
  // NB The two spaces at the end of the line allow to clear the figures of the percentage
  // previous when you change from a two or three digit value to a two or one digit value.

  byte nb_columns;
  lcd1.setCursor(15, 0);
  if (running) {
    lcd1.write(6);
    int left = then - now.unixtime();
    if (left < 0) {
      left = 0;
      running = false;
      buttonClick();
      delay(200);
      longbeep();
      send_to_google();
    }
    nb_columns = map (left, 0 , TDELAY , 0 , LCD_NB_COLUMNS * 5 );
  } else {
    lcd1.print(" ");
    // Map the range (0 ~ 100) to the range (0 ~ LCD_NB_COLUMNS * 5)
    nb_columns = map (now.second(), 0 , 59 , 0 , LCD_NB_COLUMNS * 5 );
  }

  // Move the cursor to the second line
  lcd1.setCursor ( 0 , 1 );
  // Draw each character of the line
  for (byte i = 0 ; i < LCD_NB_COLUMNS; ++ i) {

    // Depending on the number of columns remaining to display
    if (nb_columns == 0 ) { // empty box
      lcd1.write ((byte) 0 );

    } else if (nb_columns >= 5 ) { // Full case
      lcd1.write ( 5 );
      nb_columns -= 5 ;

    } else { // Last box not empty
      lcd1.write (nb_columns);
      nb_columns = 0 ;
    }
  }
}

void buttonClick() {
  tone(buzzer, 15000, 50);
}

void longbeep() {
  tone(buzzer, 15550, 150);
}

void regularAlert(DateTime now) {
  if (now.hour() != bhour) {
    bhour = now.hour();
    bminute = 30;
    tone(buzzer, 14750, 250);
    return;
  }
  if (now.minute() == bminute) {
    bminute = 0;
    tone(buzzer, 14750, 100);
  }
}

void getDHT(DateTime now) {
  // every 5 seconds
  if (now.second() % 5 == 0) {
    if (checkdht) {
      checkdht = false;
      // Reading temperature or humidity takes about 250 milliseconds!
      h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      t = dht.readTemperature();

      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t)) {
        Serial.println(F("DHT FAIL"));
        return;
      }

      // Compute heat index in Fahrenheit (the default)
      float hic = dht.computeHeatIndex(t, h, false);
      lcd2.setCursor(0, 1);
      lcd2.print("RH:");
      lcd2.print((int)h);
      lcd2.print("% T:");
      lcd2.print((int)t);
      lcd2.print("/");
      lcd2.print((int)hic);
      lcd2.print("\337C");
      return;
    }
  } else {
    checkdht = true;
  }
}

void getNTP(DateTime now) {
  // every 45 MINUTES
  if (now.minute() - 45 == 0) {
    if (checkntp) {
      checkntp = false;
      sendNTPpacket(timeServer); // send an NTP packet to a time server
    }
  } else {
    checkntp = true;
  }
}

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("DHCP FAIL"));
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println(F("Et FAIL"));
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println(F("CAB FAIL"));
    }
    link = false;
  }
  link = true;
  Udp.begin(localPort);
  // set up the LCD's number of characters per line and lines:
  // Initialize the LCD screen
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  lcd1.begin (LCD_NB_COLUMNS, LCD_NB_ROWS);
  lcd2.begin (LCD_NB_COLUMNS, LCD_NB_ROWS);
  setup_progressbar ();
  lcd1.clear ();
  lcd2.clear();
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println(F("RTC START!"));
    //    following line sets the RTC to the date &amp; time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  pinMode (buzzer, OUTPUT);
  dht.begin();
  sendNTPpacket(timeServer);
  getPhase();
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
  draw_progressbar();
  if (link && Udp.parsePacket()) {
    parseUDP();
  }
  // Small waiting time
  delay ( 100 );

}

void printDigits(int digits, LiquidCrystal lcd)  //this void function is really useful; it adds a "0" to the beginning of the number, so that 5 minutes is displayed as "00:05:00", rather than "00:5 :00"
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
  lcd1.setCursor(0, 0);
  printDigits(now.hour(), lcd1);
  lcd1.print(":");
  printDigits(now.minute(), lcd1);
  lcd1.print(":");
  printDigits(now.second(), lcd1);
  lcd2.setCursor(0, 0);
  printDigits(now.day(), lcd2);
  lcd2.print("/");
  printDigits(now.month(), lcd2);
  lcd2.print("/");
  printDigits(now.year()%100, lcd2);
  lcd2.print(" ");
  lcd2.print(daysOfTheWeek[now.dayOfTheWeek()]);
  
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
      if(running) {
        running = false;
        send_to_google();
      }
      break;
    //      case 0xFF4AB5: Serial.println(" 0");    break;
    //      case 0xFF52AD: Serial.println(" #");    break;
    //  case 0xFFFFFFFF: Serial.println(" REPEAT");break;

    default:
      Serial.println(" other button   ");

  }// End Case

} //END translateIR

// send an NTP request to the time server at the given address
void sendNTPpacket(const char * address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void parseUDP() {
  // We've received a packet, read the data from it
  Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

  // the timestamp starts at byte 40 of the received packet and is four bytes,
  // or two words, long. First, extract the two words:

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  //    Serial.print("Seconds since Jan 1 1900 = ");
  //    Serial.println(secsSince1900);

  // now convert NTP time into everyday time:
  //    Serial.print("Unix time = ");
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years and add GMT5:30 offset
  unsigned long epoch = (secsSince1900 - seventyYears) + 19800L;
  // print Unix time:
//  Serial.println(epoch);
  RTC.adjust(DateTime(epoch));
  getPhase();

  // print the hour, minute and second:
  //    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
  //    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
  //    Serial.print(':');
  //    if (((epoch % 3600) / 60) < 10) {
  //      // In the first 10 minutes of each hour, we'll want a leading '0'
  //      Serial.print('0');
  //    }
  //    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
  //    Serial.print(':');
  //    if ((epoch % 60) < 10) {
  //      // In the first 10 seconds of each minute, we'll want a leading '0'
  //      Serial.print('0');
  //    }
  //    Serial.println(epoch % 60); // print the second
  Ethernet.maintain();
}

// https://create.arduino.cc/projecthub/embedotronics-technologies/attendance-system-based-on-arduino-and-google-spreadsheet-105621
// https://github.com/Embedotronics/Attendance-System-with-storing-Data-on-Google-Spreadsheet-using-RFID-and-Arduino-Ethernet-Shield/blob/master/rfid_data_to_google_spreadsheet.ino
// http://api.pushingbox.com/pushingbox?devid=v8DF280187E9B882&R_H=30&T_C=37&task_c=1

void send_to_google() {
  if (client.connect(server, 80)) {
    Serial.println(F("G CONN"));
    // Make a HTTP request:
    client.print("GET /pushingbox?devid=v8DF280187E9B882&R_H="); //YOUR URL
    client.print(h);
    client.print("&T_C=");
    client.print(t);
    client.print("&task_c=");
    client.print('1');
    client.print("&dur=");
    client.print(now.unixtime() - then + TDELAY);
    client.print(" ");      //SPACE BEFORE HTTP/1.1
    client.print("HTTP/1.1");
    client.println();
    client.println("Host: api.pushingbox.com");
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println(F("G FAIL"));
  }
}

void getPhase() {  // calculate the current phase of the moon
  now = RTC.now();
  int Y = now.year();
  int M = now.month(); 
  int D = now.day();
  double AG, IP;                      // based on the current date
  byte phase;                         // algorithm adapted from Stephen R. Schmitt's
                                      // Lunar Phase Computation program, originally
  long YY, MM, K1, K2, K3, JD;        // written in the Zeno programming language
                                      // http://home.att.net/~srschmitt/lunarphasecalc.html
  // calculate julian date
  YY = Y - floor((12 - M) / 10);
  MM = M + 9;
  if(MM >= 12)
    MM = MM - 12;
  
  K1 = floor(365.25 * (YY + 4712));
  K2 = floor(30.6 * MM + 0.5);
  K3 = floor(floor((YY / 100) + 49) * 0.75) - 38;

  JD = K1 + K2 + D + 59;
/* Day 1 of the Gregorian calendar was Oct. 15, 1582, whose JD is
  JD = 2299161,
  so the JD of the last day of the Julian calendar was
  JD = 2299160
*/
  if(JD > 2299160)
    JD = JD -K3;
/* 
  magic Julian date 2451550.1.
  It's 14:24 6 January 2000 and I suspect this is a day of a new moon cycle on the moment of algorithm development.
*/
  IP = normalize((JD - 2451550.1) / 29.530588853);
  AG = (IP*29.53) + 0.23;
  byte i = round(AG);
  lcd2.setCursor(12, 0);
  if(i < 16) {  // waxing cycle
    lcd2.print("\76");
    lcd2.print(tithi[i]);
  } else {  // waning cycle
    lcd2.print("\74");
    lcd2.print(tithi[i-15]);
  }
  
  
}

double normalize(double v) {           // normalize moon calculation between 0-1
    v = v - floor(v);
    if (v < 0)
        v = v + 1;
    return v;
}
