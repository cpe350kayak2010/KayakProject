#include <Servo.h>
//DISPLAY
int SPD_BIT = 0x80;
int NEG_BIT = 0x40;
int VAL_SHIFT = 3;

int CMD_BIT = 0x04;
int SIP = 0;
int PUFF = 1;
int LONG_SIP = 2;
int LONG_PUFF = 3;

int DIR_INCR = 70;

//PINS
int SENSOR_PIN = A5;    // select the input pin for the potentiometer
int SP_X_PIN = A1;
int SP_Y_PIN = A0;
int SPD_PIN = 9;     //Forward/Reverse
int DIR_PIN = 2;  //Left/Right
int PWM_PIN = 3;

int DISP_PIN = 5;
int SIP_PIN = 5;
int PUFF_PIN = 5;
int LONG_SIP_PIN = 5;
int LONG_PUFF_PIN = 5;

//MOTORS
/*int PWMDEFAULT = 255;
int TURN_DUR = 30;*/
int MARGIN = 5;
int SENSOR_MARGIN = 10;
/*
int MAX_SPD = 90;
int MAX_SP = 1023;
int MAX_SENSOR = 1023;*/
int MAX_FWD = 145;
int MAX_REV = 45;
int STOP = 91;

Servo spdServo;

void setup() {
  Serial.begin(9600);
  pinMode(DIR_PIN, OUTPUT);  
  pinMode(SPD_PIN, OUTPUT);
  
  pinMode(DISP_PIN, INPUT);
  pinMode(SIP_PIN, INPUT);
  pinMode(PUFF_PIN, INPUT);
  pinMode(LONG_SIP_PIN, INPUT);
  pinMode(LONG_PUFF_PIN, INPUT);

  /* pinMode(PWM_PIN, OUTPUT);    
  pinMode(LED_PIN, OUTPUT);  */
  spdServo.attach(SPD_PIN);
  delay(40000);
}

void loop() {  

//SPEED
  unsigned int spSpd = analogRead(SP_Y_PIN);  
  // spSpd * MAX_SPD / MAX_SP
  unsigned int spd = spSpd*25/256 + 45;
  
  spd = spd < MAX_REV ? MAX_REV: spd;
  spd = spd > MAX_FWD ? MAX_FWD : spd;

  if (spd < STOP+MARGIN && spd > STOP-MARGIN) {
    spd = STOP;
  }
  spdServo.write(spd);
    
//DISPLAY SPD
  unsigned char spdDisp = SPD_BIT | controlDisplay();
  
  if (spd < STOP) {
    spdDisp |= NEG_BIT;
  }
  if (spd > STOP) {
    
  }
  
  Serial.write(spdDisp);
  
  delay(100);

//DIRECTION
  unsigned int spDir = analogRead(SP_X_PIN);
  signed int curDir = analogRead(SENSOR_PIN);
  //Serial.println(spDir, DEC);

  /* Direction sensor:
     center is about 660; variations of about +/- 10 are normal
     full left is about 380
     full right is about 930
   */
   
   /* X input:
      center at 512, +/- 2
      long sip (left?) 14 +/- 2
      long puff (right?) 1006 +/- 2
    */
    
  signed int sensorTarget = 660;
  
  if( spDir < 256 ) sensorTarget = 380;
  if( spDir > 768 ) sensorTarget = 930;
  
  signed int error = curDir - sensorTarget;

 /* Serial.println();
  Serial.println(curDir, DEC);  
  Serial.println(sensorTarget, DEC);  
  Serial.println(error, DEC);  */

  // if our error is big enough, turn the motor
  if( abs(error) > SENSOR_MARGIN ) {
    if( error > 0 ) {
      digitalWrite(DIR_PIN, HIGH);
    } else {
      digitalWrite(DIR_PIN, LOW);
    }
    unsigned int out = abs(error);
    
    //out /= 2;
    if( out < 32 ) out = 32;
    if( out > 250 ) out = 250;
    analogWrite(PWM_PIN, out); 
  } else {
    analogWrite(PWM_PIN, 0);
  }

/*if(curDir > spDir-MARGIN) {
    digitalWrite(DIR_PIN, LOW);
  }
  else if (spDir > curDir-MARGIN){
    digitalWrite(DIR_PIN, HIGH);
  }
  
  while (abs(spDir-curDir)-MARGIN > 0) {
      analogWrite(PWM_PIN, PWMDEFAULT); 
      delay(TURN_DUR);  
      curDir = analogRead(SENSOR_PIN);
  }  */   

//DISPLAY DIR
  unsigned char dir = controlDisplay();
  
  dir |= (abs(660-currDir)/70) << VAL_SHIFT;
  
  if (error < 0) {
    dir |= NEG_BIT;
  } 
  Serial.write(dir);
  
  delay(100);
}


unsigned char controlDisplay() {
  unsigned char disp = 0;
  
  if (digitalRead(LONG_SIP_PIN)) {
    disp |= CMD_BIT | LONG_SIP;
  }  
  else if (digitalRead(LONG_PUFF_PIN)) {
    disp |= CMD_BIT | LONG_PUFF;
  }
  else if (digitalRead(SIP_PIN)) {
    disp |= CMD_BIT | SIP;
  }  
  else if (digitalRead(PUFF_PIN)) {
    disp |= CMD_BIT | PUFF;
  } 
  
  return disp;
}

