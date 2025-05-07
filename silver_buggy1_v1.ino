#include <Adafruit_NeoPixel.h>

//neopiel led
#define LED_PIN 9
#define NUM_LEDS 1
Adafruit_NeoPixel pixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ==== Enum-based Color Dictionary ====
enum PixelColor {
  RED,
  GREEN,
  BLUE,
  YELLOW,
  MAGENTA,
  CYAN,
  ORANGE,
  PURPLE,
  WHITE,
  TEAL,
  NUM_DEFINED_COLORS
};

const uint32_t COLOR_LOOKUP[NUM_DEFINED_COLORS] = {
  pixel.Color(255, 0, 0),     // RED ❤️
  pixel.Color(0, 255, 0),     // GREEN 💚
  pixel.Color(0, 0, 255),     // BLUE 💙
  pixel.Color(255, 255, 0),   // YELLOW 💛
  pixel.Color(255, 0, 255),   // MAGENTA 💜
  pixel.Color(0, 255, 255),   // CYAN 💎
  pixel.Color(255, 165, 0),   // ORANGE 🧡
  pixel.Color(128, 0, 128),   // PURPLE 🍇
  pixel.Color(255, 255, 255), // WHITE 💡
  pixel.Color(0, 128, 128)    // TEAL 🐬
};


// === Pin Definitions ===
const int IR_LEFT = A0;
const int IR_RIGHT =A1;
const int GANTRY_PIN = 4;
const int TRIG_PIN = 13;
const int ECHO_PIN = 12;
const int MORTOR_DELAY_POT =A2;
const int MORTOR_DELAY_POT_LED = 11;
const int FALLBACK_DELAY_POT = A3;
const int FALLBACK_DELAY_POT_LED = 10;

// === Corrected Motor Control Pins ===
const int MOTOR_LEFT_FORWARD = 5;
const int MOTOR_LEFT_BACKWARD = 6;
const int MOTOR_RIGHT_FORWARD = 8;
const int MOTOR_RIGHT_BACKWARD = 7;

// === Constants for Gantry Detection ===
const int GANTRY_1_MIN = 500;
const int GANTRY_1_MAX = 1000;

const int GANTRY_2_MIN = 2500;
const int GANTRY_2_MAX = 3000;

const int GANTRY_3_MIN = 1500;
const int GANTRY_3_MAX = 2000;

// === Distance Threshold (Obstacle Stop) ===
const int OBSTACLE_DISTANCE = 10;  // cm

// === Globals ===
unsigned long lastLostTime = 0;
int count = 0;
int flag = 1;
int distance = 0;
bool debug = true;  // Toggle for Serial debugging
int dynamicMotorDelay = 15;
int fallbackDelay = 200;
String incoming = "";

// === Setup ===
void setup() {
  Serial.begin(9600);

  pixel.begin();
  pixel.show();  // Initialize off

  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(GANTRY_PIN, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);


  pinMode(MORTOR_DELAY_POT, INPUT);
  pinMode(MORTOR_DELAY_POT_LED, OUTPUT);
  pinMode(FALLBACK_DELAY_POT, INPUT);
  pinMode(FALLBACK_DELAY_POT_LED, OUTPUT);

  pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
  pinMode(MOTOR_LEFT_BACKWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_BACKWARD, OUTPUT);
}

//setting pixel color
void setPixelColorByEnum(PixelColor color) {
  if (color >= 0 && color < NUM_DEFINED_COLORS) {
    pixel.setPixelColor(0, COLOR_LOOKUP[color]);
    pixel.show();
  }
}


// === Movement Functions ===
void moveForward(int delayTime = 10) {
  digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
  Serial.println("Forward-->");
  delay(delayTime);
}
// === Movement Functions ===
void moveHardleft(int delayTime = 10) {
  digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
  digitalWrite(MOTOR_RIGHT_BACKWARD, HIGH);
  Serial.println("HARD LEFT-->");
  delay(delayTime);
}

void moveLeft(int delayTime = 10) {
  digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
  Serial.println("Left-->");
  delay(delayTime);
}

void moveRight(int delayTime = 10) {
  digitalWrite(MOTOR_LEFT_FORWARD, LOW);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
  Serial.println("Right-->");
  delay(delayTime);
}
void moveHardRight(int delayTime = 10) {
  digitalWrite(MOTOR_LEFT_FORWARD, LOW);
  digitalWrite(MOTOR_LEFT_BACKWARD, HIGH);
  digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
  Serial.println("Hard Right-->");
  delay(delayTime);
}

void stopMotors() {
  digitalWrite(MOTOR_LEFT_FORWARD, LOW);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
  Serial.println("Stopp-->");
}

// === Ultrasonic Sensor ===
void measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.0344 / 2;

  if (debug) {
    Serial.print("Distance: ");
    Serial.println(distance);
  }

  if (distance < OBSTACLE_DISTANCE) {
    stopMotors();
    delay(3000);
    }
}


//reducing delay caused by measure distance 
void delayedMeasureDistance() {
  static unsigned long lastPing = 0;
  if (millis() - lastPing > 300) {
    measureDistance();
    lastPing = millis();
  }
}

// === Gantry Check ===
void checkGantry() {
  if (digitalRead(GANTRY_PIN)) {
    unsigned long pulseValue = pulseIn(GANTRY_PIN, HIGH);

    if (debug) {
      Serial.print("Pulse = ");
      Serial.println(pulseValue);
    }

    if (pulseValue > GANTRY_1_MIN && pulseValue < GANTRY_1_MAX) {
      Serial.println("Gantry 1 Crossed");
      stopMotors();
      delay(1000);
    }
    if (pulseValue > GANTRY_2_MIN && pulseValue < GANTRY_2_MAX) {
      Serial.println("Gantry 2 Crossed");
      stopMotors();
      delay(1000);
    }
    if (pulseValue > GANTRY_3_MIN && pulseValue < GANTRY_3_MAX) {
      Serial.println("Gantry 3 Crossed");
      stopMotors();
      Serial.println("N");
      flag = 0;
      delay(1000);
    }
  }
}

//===patlogic when one count starting one not counted
// === Fallback Path Logic ===
void handleLostPath_fixed(int motorDelay , int fallDelay ) {
  if (millis() - lastLostTime > 1000) {
    count++;
    lastLostTime = millis();
    if (debug) {
      Serial.print("Lost IR - Count: ");
      Serial.println(count);
    }
  }

  switch (count) {
    case 1:
      //moveForward(motorDelay);
      setPixelColorByEnum(TEAL);
      moveForward(motorDelay);
      break;
    case 2:
      setPixelColorByEnum(GREEN);
      moveLeft(fallDelay);
      //moveLeft(fallDelay);
      break;
    case 3:
      setPixelColorByEnum(BLUE);
      moveForward(motorDelay);
      break;
    case 4:
          setPixelColorByEnum(BLUE);
          moveForward(motorDelay);
      // setPixelColorByEnum(CYAN);
      // moveForward(motorDelay);  //longer delay on count 5 => 4 times orignal fallback
      break;
      // setPixelColorByEnum(MAGENTA);
      // moveForward(motorDelay)-+;
      // break;
    case 5:
      setPixelColorByEnum(ORANGE);
      moveLeft(fallDelay*1.15);
      //moveForward();
      break;
    case 6:
    setPixelColorByEnum(MAGENTA);
    moveForward(motorDelay);
    break;
    case 7:
    stopMotors();
      if (debug) Serial.println("Final Stop");
      setPixelColorByEnum(RED);
      flag = 0;
      break;
    default :
      setPixelColorByEnum(WHITE);
      moveForward(motorDelay);
    
  }
}


// === Fallback Path Logic ===
void handleLostPath(int motorDelay , int fallDelay ) {
  if (millis() - lastLostTime > 1000) {
    count++;
    lastLostTime = millis();
    if (debug) {
      Serial.print("Lost IR - Count: ");
      Serial.println(count);
    }
  }

  switch (count) {
    case 1:
      //moveForward(motorDelay);
      setPixelColorByEnum(TEAL);
      moveForward(motorDelay);
      break;
    case 2:
      setPixelColorByEnum(GREEN);
      moveLeft(fallDelay);
      //moveLeft(fallDelay);
      break;
    case 3:
      setPixelColorByEnum(BLUE);
      moveForward(motorDelay);
      break;
    case 4:
      setPixelColorByEnum(CYAN);
      moveForward(motorDelay);  //longer delay on count 5 => 4 times orignal fallback
      break;
      // setPixelColorByEnum(MAGENTA);
      // moveForward(motorDelay);
      // break;
    case 5:
      setPixelColorByEnum(ORANGE);
      moveLeft(fallDelay*1.15);
      //moveForward();
      break;
    case 6:
    setPixelColorByEnum(MAGENTA);
    moveForward(motorDelay);
    break;
    case 7:
    stopMotors();
      if (debug) Serial.println("Final Stop");
      setPixelColorByEnum(RED);
      flag = 0;
      break;
    default :
      setPixelColorByEnum(WHITE);
      moveForward(motorDelay);
    
  }
}

//====Getting values form delay pot====

int getValueFromPot(int pin, int minVal, int maxVal) {
  int raw = analogRead(pin);
  int value = constrain(map(raw, 0, 1023, minVal, maxVal), minVal, maxVal);
  int pwm = map(raw, 0, 1023, 0, 255);
  analogWrite(pin == MORTOR_DELAY_POT ? MORTOR_DELAY_POT_LED : FALLBACK_DELAY_POT_LED, pwm);
  return value;
}

// === Main Loop ===
void loop() {

  if (Serial.available()) {
    if(Serial.read()=='M'){
      flag =1;
    }
    // incoming = Serial.readStringUntil('\n');
    // incoming.trim();
    // if (incoming.startsWith("[CMD:") && incoming.endsWith("]")) {
    //   if (incoming == "[CMD:M]")flag = 1;  // Resume if 'M' received
    // }
  }

  if (flag == 1) {

    //measuring ultrasonic distance;
    delayedMeasureDistance();
    
    checkGantry();

    int leftIR = digitalRead(IR_LEFT);
    int rightIR = digitalRead(IR_RIGHT);

    if (leftIR == 1 && rightIR == 1) {
      moveForward(dynamicMotorDelay);
    } else if (leftIR == 0 && rightIR == 1) {
      moveLeft(dynamicMotorDelay);
    } else if (leftIR == 1 && rightIR == 0) {
      moveRight(dynamicMotorDelay);
    } else {
      if (count == 0){
        count =1;
      }
      handleLostPath(dynamicMotorDelay, fallbackDelay);
    }
  }
}







