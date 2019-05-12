/*  Induino R3 User Guide - Program 7.1 -  Remote Controlled Binary Counter
 
 This sketch increases a 3 bit number every time the channel up key on a remote is pressed
 and decreases a 3 bit number every time the channel down key on a remote is pressed
 and shows the output on 3 LEDs   
 */

int i = 0; // the countervalue for the binary counter
void setup()  
{  
  pinMode(11,OUTPUT);   // declare LED pins as output pins  
  pinMode(12,OUTPUT);  
  pinMode(13,OUTPUT);  
  pinMode(15,INPUT); // declare the tsop pin as Input
  Serial.begin(9600); 
}  
void loop()  
{  
  int remote_val = remote();  // Call the remote function to get the value of the key pressed
  Serial.println(remote_val);  
  if(remote_val>0)  // check if the value is greater than 0. A 0 means that no signal was received by the TSOP
  {
    if(remote_val == 144) // 144 is the Channel UP Button Value on the Sony Remote provided as part of the Induino R3 Learners Kit
    {
      if(i<7)        // if counter value is less than 7 or 3 bits  
        i++;        // increment counter value  
      else           
        i=0;        // reset counter to 0 
    }
    if(remote_val == 145) // 145 is the Channel UP Button Value on the Sony Remote provided as part of the Induino R3 Learners Kit
    {
      if(i>0)        // if counter value is greater than 0 or 3 bits  
        i--;        // decrement counter value  
      else           
        i=7;        // reset counter to 7
    }


    int a=i%2;      // calculate LSB   
    int b=i/2 %2;     // calculate middle bit  
    int c=i/4 %2;     // calculate MSB   
    digitalWrite(11,a);  // write MSB  
    digitalWrite(12,b);  // write middle bit  
    digitalWrite(13,c);  // write LSB  
    delay(500);     // A remote press will normally generate 3 signal trains. This is to avoid reading duplicates
  }  
}  

// A Dedicated function that will calculate and return a decimal value for each of the buttons on a remote.
int remote()  
{  
  int value = 0;  // a Variable to store our final calculated value
  int time = pulseIn(15,LOW);  // we need to look for the duration of the LOW pulse as TSOP will invert the incoming HIGH pulse
  if(time>2000) // Checking if the Start Bit has been received. Start Bit Duration is 2.4ms  
  {  
    for(int counter1=0;counter1<12;counter1++) // A loop to receive the next 12 bits  
    {  
      if(pulseIn(15,LOW)>1000) // checking the duration of each pulse, if it is a '1' then we use it in our binary to decimal conversion, '0's can be ignored.  
      {  
        value = value + (1<< counter1);// binary to decimail conversion. 1<< i is nothing but 2 raised to the power of i  
      }  
    }  
  }  
  return value;  
}  
