#include <Servo.h>

//PINS
int SENSOR_PIN = A5;    // select the input pin for the potentiometer
int SPD_PIN = 9;     //Forward/Reverse
int DIR_PIN = 2;  //Left/Right
int PWM_PIN = 3;

int EN_SP_PIN = 7;
int EN_JS_PIN = 8;
int SP_X_PIN = A1;
int SP_Y_PIN = A0;
int JS_X_PIN = A2;
int JS_Y_PIN = A3;

int DISP_PIN = 5;
int SIP_PIN = 5;
int PUFF_PIN = 5;
int LONG_SIP_PIN = 5;
int LONG_PUFF_PIN = 5;

//SENSORS
int MARGIN = 5;
int SENSOR_MARGIN = 10;

int MAX_SENSOR = 1023;
int SENSOR_CENTER = 660;
int SENSOR_LEFT = 380;
int SENSOR_RIGHT = 930;

//SIP-N-PUFF
int MAX_SP = 1023;
int SP_LEFT = 256;
int SP_RIGHT = 768;

//JOYSTICK
int MAX_JS = 1023;
int JS_LEFT = 256;
int JS_RIGHT = 768;

//INPUT DEVICE
int InputXPin = SP_X_PIN;
int InputYPin = SP_Y_PIN;
int MaxInput = MAX_SP;
int QMaxInput = MAX_SP/4;
int InputLeft = SP_LEFT;
int InputRight = SP_RIGHT;

//MOTORS
int MIN_PWM = 32;
int MAX_PWM = 250;
int MAX_SPD = 90;
int Q_MAX_SPD = 25;

int MAX_FWD = 145;
int MAX_REV = 45;
int STOP = 91;

int SPD_OFFSET = 45;

Servo spdServo;

//DISPLAY
int SPD_BIT = 0x80;
int NEG_BIT = 0x40;
int VAL_SHIFT = 3;

int CMD_BIT = 0x04;
int SIP = 0;
int PUFF = 1;
int LONG_SIP = 2;
int LONG_PUFF = 3;

int DIR_INCR = (SENSOR_CENTER-SENSOR_LEFT)/4;
int SPD_INCR = (MAX_SPD-STOP)/4;

void setup() {
  Serial.begin(9600);
  pinMode(DIR_PIN, OUTPUT);  
  pinMode(SPD_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);  
   
  pinMode(DISP_PIN, INPUT);
  pinMode(SIP_PIN, INPUT);
  pinMode(PUFF_PIN, INPUT);
  pinMode(LONG_SIP_PIN, INPUT);
  pinMode(LONG_PUFF_PIN, INPUT);
  
  spdServo.attach(SPD_PIN);
}

void loop() {  

//INPUT DEVICE
  if(!inputDevice()) {
    spdServo.write(STOP);
  }
  
//SPEED
  unsigned int spd = Speed();
  displaySpd(spd); 
  //delay(100);

//DIRECTION
  signed int dir = Direction();
  displayDir(dir);
  //delay(100);
}

unsigned int inputDevice() {
  if (digitalRead(EN_SP)) {
    InputXPin = SP_X_PIN;
    InputYPin = SP_Y_PIN;
    MaxInput = MAX_SP;
    InputLeft = SP_LEFT;
    InputRight - SP_RIGHT;
  }
  else if (digitalRead(EN_JS) {
    InputXPin = JS_X_PIN;
    InputYPin = JS_Y_PIN;
    MaxInput = MAX_JS;
    InputLeft = JS_LEFT;
    InputRight - JS_RIGHT;
  }
  else {
    return 0;
  }
  return 1;
}

unsigned int Speed() {
  unsigned int inSpd = analogRead(InputYPin);  
  // spSpd * MAX_SPD / MAX_SP + SPD_OFFSET
  //unsigned int spd = spSpd*25/256 + 45;
  unsigned int spd = inSpd*Q_MAX_SPD/QMaxInput + SPD_OFFSET;
  
  spd = spd < MAX_REV ? MAX_REV: spd;
  spd = spd > MAX_FWD ? MAX_FWD : spd;

  if (spd < STOP+MARGIN && spd > STOP-MARGIN) {
    spd = STOP;
  }
  spdServo.write(spd);
  
  return spd;
}

void displaySpd(unsigned int spd) {
  unsigned char spdDisp = SPD_BIT | controlDisplay();
  
  if (spd < STOP) {
    spdDisp |= NEG_BIT;
  }
  if (spd > STOP) {
    spdDisp |= (spd-STOP/SPD_INCR) << VAL_SHIFT;
  }
  
  Serial.write(spdDisp);
}

signed int Direction() {
  unsigned int spDir = analogRead(InputXPin);
  signed int curDir = analogRead(SENSOR_PIN);
  
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
    
  signed int sensorTarget = SENSOR_CENTER;
  
  if(spDir < InputLeft) sensorTarget = SENSOR_LEFT;
  if(spDir > InputRight) sensorTarget = SENSOR_RIGHT;
  
  signed int error = curDir - sensorTarget;
  
  // if our error is big enough, turn the motor
  if(abs(error) > SENSOR_MARGIN) {
    if(error > 0) {
      digitalWrite(DIR_PIN, HIGH);
    } else {
      digitalWrite(DIR_PIN, LOW);
    }
    unsigned int out = abs(error);
    
    if(out < MIN_PWM) out = MIN_PWM;
    if(out > MAX_PWM) out = MAX_PWM;
    analogWrite(PWM_PIN, out); 
  } 
  else {
    analogWrite(PWM_PIN, 0);
  }
  
  return curDir;
}

void displayDir(signed int curDir) {
  unsigned char dirDisp = controlDisplay();
  
  dirDisp |= (abs(CENTER-curDir)/DIR_INCR) << VAL_SHIFT;
  
  if (curDir < CENTER) {
    dirDisp |= NEG_BIT;
  } 
  
  Serial.write(dirDisp);
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

