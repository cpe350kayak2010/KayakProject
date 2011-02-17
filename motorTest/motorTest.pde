#include <Servo.h>

Servo myservo;
int sensorPin = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
unsigned int sensorValue = 0;  // variable to store the value coming from the sensor
int servoPin = 9;
unsigned int ServoValue;

void setup() {
  // declare the ledPin as an OUTPUT:
  Serial.begin(9600);
  pinMode(servoPin, OUTPUT);  
  
  
  pinMode(ledPin, OUTPUT);  
  myservo.attach(servoPin);
  
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);    
  ServoValue = (sensorValue*45)/256;
  // turn the ledPin on
  digitalWrite(ledPin, HIGH);  
  
  if(ServoValue >= 0 && ServoValue <= 180){
    myservo.write(ServoValue);
  }
    Serial.println(ServoValue, DEC);
  
  
  delay(100);                  
}


