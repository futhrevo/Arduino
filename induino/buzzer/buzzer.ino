const int buzzer = 3; //buzzer to arduino pin 9


void setup(){
 
  pinMode(buzzer, OUTPUT); // Set buzzer - pin 9 as an output

}

void loop(){
 
tone(buzzer,15000,5);

    delay(1000);
  
}
