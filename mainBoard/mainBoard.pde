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

int SIP_PIN = 10;
int PUFF_PIN = 11;
int LONG_SIP_PIN = 12;
int LONG_PUFF_PIN = 13;

//SENSORS
int MARGIN = 5;
int SENSOR_MARGIN = 10;

int MAX_SENSOR = 1023;
int SENSOR_CENTER = 660;
int SENSOR_LEFT = 380;
int SENSOR_RIGHT = 930;

//SIP-N-PUFF
int MAX_SP = 1023;
int MIN_SP = 0;
int SP_LEFT = 256;
int SP_RIGHT = 768;
int SP_RANGE = 1024/4;

//JOYSTICK
int MAX_JS = 512+100;
int MIN_JS = 512-100;
int JS_LEFT = 512-100;
int JS_RIGHT = 512+100;
int JS_RANGE = 200/4;

//MOTORS
int MIN_PWM = 32;
int MAX_PWM = 250;

int MAX_DIR = 560;
int Q_MAX_DIR = MAX_DIR/4;

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
   
  pinMode(SIP_PIN, INPUT);
  pinMode(PUFF_PIN, INPUT);
  pinMode(LONG_SIP_PIN, INPUT);
  pinMode(LONG_PUFF_PIN, INPUT);
  
  pinMode(EN_JS_PIN, INPUT);
  pinMode(EN_SP_PIN, INPUT);
  
  spdServo.attach(SPD_PIN);
}

  
/* Direction sensor:
   center is about 660; variations of about +/- 10 are normal
   full left is about 380
   full right is about 930
 */
 
 
void loop() {  
  signed int sensorTarget = SENSOR_CENTER;
  unsigned int spd = STOP;
  
  if (digitalRead(EN_SP_PIN)) {
    // read sip-n-puff steering and set sensor target
     /* X input:
        center at 512, +/- 2
        long sip (left?) 14 +/- 2
        long puff (right?) 1006 +/- 2
      */
    signed int inDir = analogRead(SP_X_PIN);
    if(inDir < SP_LEFT) sensorTarget = SENSOR_LEFT;
    if(inDir > SP_RIGHT) sensorTarget = SENSOR_RIGHT;
    
    // read sip-n-puff speed input and generate speed
    unsigned int inSpd = analogRead(SP_Y_PIN);
    spd = (inSpd-MIN_SP)*Q_MAX_SPD/SP_RANGE + SPD_OFFSET;
    
    
  } else if (digitalRead(EN_JS_PIN)) {
    // read joystick steering and set sensor target
    signed int inDir = analogRead(JS_X_PIN);
    if(inDir < JS_LEFT ) inDir = JS_LEFT;
    if(inDir > JS_RIGHT ) inDir = JS_RIGHT;
    signed int dir = (inDir-MIN_JS)*Q_MAX_DIR/JS_RANGE;
    
    sensorTarget = dir + SENSOR_LEFT; 

    // read joystick speed and set sensor target
    unsigned int inSpd = analogRead(JS_Y_PIN);
    inSpd = MAX_SENSOR - inSpd;  
    spd = (inSpd-MIN_JS)*Q_MAX_SPD/JS_RANGE + SPD_OFFSET;

  } else {
    sensorTarget = SENSOR_CENTER;
    spd = STOP;
  }

  // speed control and limiting
  spd = spd < MAX_REV ? MAX_REV: spd;
  spd = spd > MAX_FWD ? MAX_FWD : spd;

  if (spd < STOP+MARGIN && spd > STOP-MARGIN) {
    spd = STOP;
  }
  spdServo.write(spd);
  // done with speed control
  
  // direction control and limiting
  signed int curDir = analogRead(SENSOR_PIN); // read direction sensor
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
  // done with direction control
  
  // write control data to display
  displaySpd(spd);
  displayDir(curDir);
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

void displayDir(signed int curDir) {
  unsigned char dirDisp = controlDisplay();
  
  dirDisp |= (abs(SENSOR_CENTER-curDir)/DIR_INCR) << VAL_SHIFT;
  
  if (curDir < SENSOR_CENTER) {
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

