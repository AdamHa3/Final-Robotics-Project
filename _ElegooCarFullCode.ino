#include <Servo.h>                 // Servo control (ultrasonic scan)
#include <IRremote.h>              // IRremote library (RAW 32-bit codes)
#include <Adafruit_NeoPixel.h>     // NeoPixel RGB LED control

/*
====================================================================
ELEGoo Robot Car — Multi-Mode Program (Improved Structure)
--------------------------------------------------------------------
Modes:
1) Manual IR Drive (HOLD-to-move) + Speed Up/Down (Option 8/7)
2) Simple Analog Line Follow (A0/A1/A2 thresholds)
3) Ultrasonic Avoid (30cm): FRONT -> LEFT -> RIGHT -> Reverse + 180
4) Victory Lap (timed rectangle)
LED:
- Mode 1 = Blue
- Mode 2 = Green
- Mode 3 = Red
- Mode 4 = Purple
- Warning/Lost/Lift = Orange
====================================================================
*/

// =====================================================
// -------------------- NEOPIXEL ------------------------
// =====================================================
#define LED_PIN   4
#define NUM_LEDS  1
Adafruit_NeoPixel led(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  led.setPixelColor(0, led.Color(r, g, b));
  led.show();
}
void ledMode1()   { setLED(0, 0, 255); }     // Blue
void ledMode2()   { setLED(0, 255, 0); }     // Green
void ledMode3()   { setLED(255, 0, 0); }     // Red
void ledMode4()   { setLED(150, 0, 255); }   // Purple
void ledWarning() { setLED(255, 170, 0); }   // Orange (lost/lift/warning)

// =====================================================
// ---------------------- IR ----------------------------
// =====================================================
const int IR_RECEIVE_PIN = 9;

// Your REAL 32-bit RAW hex codes (from the start)
#define BTN_UP          0xB946FF00
#define BTN_RIGHT       0xBC43FF00
#define BTN_DOWN        0xEA15FF00
#define BTN_LEFT        0xBB44FF00

// Mode buttons (your earlier set)
#define BTN_MODE1       0xF30CFF00   // Option 1
#define BTN_MODE2       0xE718FF00   // Option 2
#define BTN_MODE3       0xA15EFF00   // Option 3
#define BTN_MODE4       0xF708FF00   // Option 4

// Speed buttons
#define BTN_SPEED_DOWN  0xBD42FF00   // Option 7
#define BTN_SPEED_UP    0xAD52FF00   // Option 8

// Hold-to-move timing
uint32_t lastCmd = 0;
unsigned long lastSignalTime = 0;
const unsigned long RELEASE_TIMEOUT = 200;

// Mode state
int mode = 1;

// =====================================================
// --------------------- MOTORS -------------------------
// =====================================================
const int PWMA = 5, AIN1 = 7;     // Left motor
const int PWMB = 6, BIN1 = 8;     // Right motor
const int STBY = 3;              // Enable motor driver

// Manual speed settings
int speedValue = 120;
const int SPEED_STEP = 20;
const int MIN_SPEED = 60;
const int MAX_SPEED = 255;

void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  digitalWrite(AIN1, LOW);
  digitalWrite(BIN1, LOW);
}

void forward(int s)  { digitalWrite(AIN1,HIGH); analogWrite(PWMA,s); digitalWrite(BIN1,HIGH); analogWrite(PWMB,s); }
void backward(int s) { digitalWrite(AIN1,LOW);  analogWrite(PWMA,s); digitalWrite(BIN1,LOW);  analogWrite(PWMB,s); }
void turnLeft(int s) { digitalWrite(AIN1,HIGH); analogWrite(PWMA,s); digitalWrite(BIN1,LOW);  analogWrite(PWMB,s); }
void turnRight(int s){ digitalWrite(AIN1,LOW);  analogWrite(PWMA,s); digitalWrite(BIN1,HIGH); analogWrite(PWMB,s); }

// =====================================================
// -------------- ULTRASONIC + SERVO --------------------
// =====================================================
Servo scanServo;
const int SERVO_PIN = 10;

const int TRIG_PIN = 13;
const int ECHO_PIN = 12;

const int SAFE_CM = 30;          // you asked for 30cm obstacle distance

float distanceCM() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long t = pulseIn(ECHO_PIN, HIGH, 25000);
  if (t == 0) return 0;

  float cm = (t * 0.0343f) / 2.0f;
  if (cm > 300) return 0;
  return cm;
}

// =====================================================
// ----------------- IR READ HELPER ---------------------
// =====================================================
uint32_t readIR() {
  if (!IrReceiver.decode()) return 0;

  uint32_t code = IrReceiver.decodedIRData.decodedRawData;

  // Some remotes send 0x0 during hold; reuse last command
  if (code == 0x0) code = lastCmd;
  else lastCmd = code;

  lastSignalTime = millis();
  IrReceiver.resume();
  return code;
}

// =====================================================
// ---------------------- SETUP -------------------------
// =====================================================
void setup() {
  Serial.begin(9600);

  // Start NeoPixel
  led.begin();
  led.show();
  ledMode1();

  // Start IR receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  // Motor pins
  pinMode(PWMA, OUTPUT); pinMode(AIN1, OUTPUT);
  pinMode(PWMB, OUTPUT); pinMode(BIN1, OUTPUT);
  pinMode(STBY, OUTPUT); digitalWrite(STBY, HIGH);

  // Ultrasonic pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Servo
  scanServo.attach(SERVO_PIN);
  scanServo.write(90);

  stopMotors();
  Serial.println("System Ready.");
}

// =====================================================
// ----------------------- LOOP -------------------------
// =====================================================
void loop() {
  uint32_t code = readIR();

  // -------- Mode select (Option 2 toggles back to manual) --------
  if (code == BTN_MODE1) { mode = 1; ledMode1(); }
  if (code == BTN_MODE2) { mode = (mode == 2 ? 1 : 2); if(mode==2) ledMode2(); else ledMode1(); }
  if (code == BTN_MODE3) { mode = 3; ledMode3(); }
  if (code == BTN_MODE4) { mode = 4; ledMode4(); }

  // -------- Run the active mode --------
  if (mode == 1) modeManual(code);
  else if (mode == 2) modeAnalogLine();
  else if (mode == 3) modeUltrasonicAvoid();
  else if (mode == 4) modeVictoryLap();
}

// =====================================================
// MODE 1 — Manual IR (HOLD-to-move) + Speed buttons
// =====================================================
void modeManual(uint32_t code) {

  // Speed adjust on Option 8/7
  if (code == BTN_SPEED_UP) {
    speedValue = constrain(speedValue + SPEED_STEP, MIN_SPEED, MAX_SPEED);
    Serial.print("Speed -> "); Serial.println(speedValue);
  }
  else if (code == BTN_SPEED_DOWN) {
    speedValue = constrain(speedValue - SPEED_STEP, MIN_SPEED, MAX_SPEED);
    Serial.print("Speed -> "); Serial.println(speedValue);
  }

  // Stop when released (no signal recently)
  if (millis() - lastSignalTime > RELEASE_TIMEOUT) {
    stopMotors();
    return;
  }

  // Hold-to-move direction commands
  if      (lastCmd == BTN_UP)    forward(speedValue);
  else if (lastCmd == BTN_DOWN)  backward(speedValue);
  else if (lastCmd == BTN_LEFT)  turnLeft(speedValue);
  else if (lastCmd == BTN_RIGHT) turnRight(speedValue);
  else                           stopMotors();
}

// =====================================================
// MODE 2 — Simple Analog Line Follow (threshold logic)
// =====================================================
void modeAnalogLine() {

  int R = analogRead(A0);
  int M = analogRead(A1);
  int L = analogRead(A2);

  // Optional: print sometimes for tuning
  // Serial.print(L); Serial.print(" "); Serial.print(M); Serial.print(" "); Serial.println(R);

  // Lift / not on ground safety (very low on all sensors)
  if (L < 80 && M < 80 && R < 80) {
    ledWarning();
    stopMotors();
    return;
  }

  ledMode2();

  // Hard stop if strong middle reading
  if (M > 1000) { stopMotors(); return; }

  // Simple decisions
  if (M > 350) { forward(60); return; }
  if (L > 350) { turnLeft(60); return; }
  if (R > 350) { turnRight(60); return; }

  // Lost line -> gentle search turn (no backing up)
  ledWarning();
  turnRight(55);
}

// =====================================================
// MODE 3 — Ultrasonic Avoid (30cm): LEFT -> RIGHT -> Reverse+180
// =====================================================
void modeUltrasonicAvoid() {

  ledMode3();

  // Check front
  scanServo.write(90);
  delay(120);
  float front = distanceCM();

  if (front == 0 || front >= SAFE_CM) {
    forward(120);
    return;
  }

  stopMotors();
  delay(150);

  // Look LEFT first
  scanServo.write(150);
  delay(220);
  float leftD = distanceCM();
  if (leftD == 0) leftD = 300;

  if (leftD > SAFE_CM) {
    turnLeft(140);
    delay(500);
    return;
  }

  // Look RIGHT
  scanServo.write(30);
  delay(220);
  float rightD = distanceCM();
  if (rightD == 0) rightD = 300;

  if (rightD > SAFE_CM) {
    turnRight(140);
    delay(500);
    return;
  }

  // Both blocked -> reverse + 180
  backward(140);
  delay(600);

  turnLeft(150);
  delay(1200);

  stopMotors();
}

// =====================================================
// MODE 4 — Victory Lap (Timed rectangle)
// =====================================================
void modeVictoryLap() {

  ledMode4();

  forward(50); delay(12000);
  turnLeft(50); delay(600);

  forward(50); delay(5000);
  turnLeft(50); delay(600);

  forward(50); delay(12000);
  turnLeft(50); delay(600);

  forward(50); delay(5000);
  turnLeft(50); delay(600);

  // Spin celebration
  turnLeft(70); delay(2000);
  stopMotors();
}
