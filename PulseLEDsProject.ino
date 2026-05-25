int PulseSensorPurplePin = 0; 
int LED = LED_BUILTIN;        


int Signal;          
int Threshold = 580;  

void setup() {
  pinMode(LED, OUTPUT);  
  Serial.begin(115200); 
}

void loop() {

  Signal = analogRead(PulseSensorPurplePin); 

  Serial.println("Signal " + String(Signal)); 

  if (Signal > Threshold) {  
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);  
  }

  delay(20);
}
