#include <Servo.h>

int SENSOR_PIN = A5;    // select the input pin for the potentiometer
int SP_X_PIN = A0;
int SP_Y_PIN = A1;
int LED_PIN = 13;      // select the pin for the LED
int SPD_PIN = 9;     //Forward/Reverse
int DIR_PIN = 2;  //Left/Right
int PWM_PIN = 3;

int PWMDEFAULT = 255;
int TURN_DUR = 30;
int MARGIN = 5;

int MAX_SPD = 180;
int MAX_SP = 256;
int MAX_SENSOR = 1024;

Servo spdServo;

void setup() {
  Serial.begin(9600);
  pinMode(DIR_PIN, OUTPUT);  
  pinMode(SPD_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);    
  pinMode(LED_PIN, OUTPUT);  
  spdServo.attach(SPD_PIN);
}

void loop() {  

//SPEED
  int spSpd = analogRead(SP_Y_PIN);  

  // spSpd * MAX_SPD / MAX_SP
  int spd = spSpd*45/64;
  
  if(spd >= 0 && spd <= 180){
    spdServo.write(spd);
  }     

//DIRECTION
  int spDir = analogRead(SP_X_PIN);
  int curDir = analogRead(SENSOR_PIN);
  curDir = curDir/(MAX_SENSOR/MAX_SP);
  
  if(curDir-spDir-MARGIN > 0) {
    digitalWrite(DIR_PIN, LOW);
  }
  else {
    digitalWrite(DIR_PIN, HIGH);
  }
  
  while (abs(spDir-curDir)-MARGIN > 0) {
      analogWrite(PWM_PIN, PWMDEFAULT); 
      delay(TURN_DUR);  
      curDir = analogRead(SENSOR_PIN);
  }     
}


