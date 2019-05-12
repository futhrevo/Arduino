/* Induino R3 User Guide - Program 5.2 - Simple LDR Value using Internal Pull-Up */

int ldr_val = 0; // variable to store the LDR value

void setup()
{
  pinMode(15,INPUT_PULLUP);  // enabling the Internal Pull-Up on Pin 15 (A2)
  Serial.begin(9600); // Initialise Serial Communication
}

void loop()
{
  ldr_val = analogRead(2); // Read the ldr value and store it in the variable
  Serial.print("Current LDR Value : ");
  Serial.println(ldr_val); // print the ldr value to serial monitor
  delay(1000); // a delay - do not remove this delay (you can change it) as otherwise the serial monitor will get flooded with data and might crash
}
