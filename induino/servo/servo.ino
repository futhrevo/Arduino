/*  Induino R3 User Guide - Program 9.1 - Controlling a Servo Motor Using the LDR Value */


#include <Servo.h> 
 
Servo myservo;  // create servo object to control a servo 
 
int ldr_pin = 3;  // ldr is on Analog pin 3
int val;    // variable to read the value from the analog pin 
 
void setup() 
{ 
  myservo.attach(18);  // attaches the servo on pin 9 to the servo object 
} 
 
void loop() 
{ 
  val = analogRead(ldr_pin);      // reads the value of the LDR (value between 0 and 1023) 
  val = map(val, 0, 1023, 0, 179); // scale it to use it with the servo (value between 0 and 180) 
  myservo.write(val); // sets the servo position according to the scaled value 
  delay(15);   // waits for the servo to get there 
} 
