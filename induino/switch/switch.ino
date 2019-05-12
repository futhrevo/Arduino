/*  Induino R3 User Guide - Program 3.0 - Button controlled Binary Counter with Serial Print
This sketch increases a 3 bit number every time a button is pressed by the user and shows the output on 3 LEDs   
 */  

 int i = 0;  
 char txt[]="The Current Value is : "; // A string stored in a character array
 void setup()  
 {  
  pinMode(11,OUTPUT);   // declare LED pins as output pins  
  pinMode(12,OUTPUT);  
  pinMode(13,OUTPUT);  
  pinMode(7,INPUT_PULLUP);// declare the Button as INPUT pin with internal Pull up enabled
  Serial.begin(9600); // initialize Serial Communication
  Serial.println("Starting the Program");// This will be printed only once
 }  
 void loop()  
 {  
  if(digitalRead(7)==0)  // if the button is pressed  
  {  
   if(i<7)        // if counter value is less than 7 or 3 bits  
    i++;        // increment counter value  
   else           
    i=0;        // reset counter to 0  
   Serial.print(txt); // Print Descriptive test from the character array
   Serial.println(i); // print the current value
   int a=i%2;      // calculate LSB   
   int b=i/2 %2;     // calculate middle bit  
   int c=i/4 %2;     // calculate MSB   
   digitalWrite(11,a);  // write LSB 
   digitalWrite(12,b);  // write middle bit  
   digitalWrite(13,c);  // write MSB  
   while(digitalRead(7)==0);  // wait till button is released to avoid incrementing the counter again  
   delay(100);         // small delay to avoid debounce  
  }  
 }
