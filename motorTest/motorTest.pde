#include <Servo.h>

Servo myservo;
int SP_Y_PIN = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
unsigned int spSpd = 0;  // variable to store the value coming from the sensor
int SPD_PIN = 9;
unsigned int spd;

void setup() {
  // declare the ledPin as an OUTPUT:
  Serial.begin(9600);
  pinMode(SPD_PIN, OUTPUT);  
  
  
  pinMode(ledPin, OUTPUT);  
  myservo.attach(SPD_PIN);
  
}

void loop() {
  // read the value from the sensor:
  spSpd = analogRead(SP_Y_PIN);    
  spd = (spSpd*45)/256;
  // turn the ledPin on
  digitalWrite(ledPin, HIGH);  
  
  if(spd >= 0 && spd <= 180){
    myservo.write(spd);
    Serial.println(spd, DEC);
  }
    
  
  
  delay(100);                  
}


