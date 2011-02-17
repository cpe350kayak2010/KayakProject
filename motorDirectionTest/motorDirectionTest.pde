int sensorPin = A0;    // select the input pin for the potentiometer     
unsigned int sensorValue = 0;  // variable to store the value coming from the sensor
int motorDirPin = 12;
int pwmPin = 9;
int pwmValue;

void setup() {
  // declare the ledPin as an OUTPUT:
  Serial.begin(9600);  
  
  pinMode(motorDirPin, OUTPUT);  
  
  // fade in from min to max in increments of 5 points:
  for(int fadeValue = 0 ; fadeValue <= 255; fadeValue +=5) { 
    // sets the value (range from 0 to 255):
    analogWrite(pwmPin, fadeValue);    
    //Serial.println(pwmValue, DEC);    
    // wait for 30 milliseconds to see the dimming effect    
    delay(30);                            
  } 
}

void loop() {


  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);

  if (sensorValue <= 505) {
    digitalWrite(motorDirPin, LOW);
    pwmValue = (505-sensorValue)/2;
  }
  else if (sensorValue >=  525) {
    digitalWrite(motorDirPin, HIGH);
    pwmValue = (sensorValue-525)/2;
  }
  else {
    pwmValue = 0;
  }
  
  analogWrite(pwmPin, pwmValue);  
  Serial.println(pwmValue, DEC);
  Serial.println(motorDirPin, DEC);
  
  delay(100);                  
}


