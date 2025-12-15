#include <IRremote.h>   // IRremote library (reads your 32-bit RAW IR codes)

/*
=========================================================
MODE 1 — MANUAL IR DRIVE (HOLD-TO-MOVE) + SPEED UP/DOWN
---------------------------------------------------------
- Uses RAW 32-bit HEX codes 
- Car moves ONLY while you keep holding a direction button
- Option 8 = Speed Increase, Option 7 = Speed Decrease
- Auto-stop happens when you release the button (timeout)
=========================================================
*/

// ---------------- IR receiver pin ----------------
const int IR_RECEIVE_PIN = 9;   // IR receiver signal pin

// ---------------- Motor driver pins ----------------
const int PWMA = 5;             // Left motor PWM (speed)
const int AIN1 = 7;             // Left motor direction
const int PWMB = 6;             // Right motor PWM (speed)
const int BIN1 = 8;             // Right motor direction
const int STBY = 3;             // Motor driver enable (HIGH = ON)

// ---------------- Your REAL 32-bit RAW HEX codes ----------------
#define BTN_UP         0xB946FF00
#define BTN_RIGHT      0xBC43FF00
#define BTN_DOWN       0xEA15FF00
#define BTN_LEFT       0xBB44FF00

// Speed buttons (from your “Option 7/8”)
#define BTN_SPEED_DOWN 0xBD42FF00   // Option 7
#define BTN_SPEED_UP   0xAD52FF00   // Option 8

// ---------------- Speed settings ----------------
int speedValue = 120;           // starting speed
const int SPEED_STEP = 20;      // how much speed changes per press
const int MIN_SPEED = 60;       // minimum usable speed (prevents stall)
const int MAX_SPEED = 255;      // max PWM

// ---------------- Hold-to-move timing ----------------
const unsigned long RELEASE_TIMEOUT = 200; // ms without IR -> stop
unsigned long lastSignalTime = 0;          // last time we saw a signal
uint32_t lastCmd = 0;                      // last command we received

// =====================================================
// Motor movement helpers
// =====================================================
void forward(int s){
  digitalWrite(AIN1, HIGH);  analogWrite(PWMA, s);
  digitalWrite(BIN1, HIGH);  analogWrite(PWMB, s);
}

void backward(int s){
  digitalWrite(AIN1, LOW);   analogWrite(PWMA, s);
  digitalWrite(BIN1, LOW);   analogWrite(PWMB, s);
}

void turnLeft(int s){
  digitalWrite(AIN1, HIGH);  analogWrite(PWMA, s);
  digitalWrite(BIN1, LOW);   analogWrite(PWMB, s);
}

void turnRight(int s){
  digitalWrite(AIN1, LOW);   analogWrite(PWMA, s);
  digitalWrite(BIN1, HIGH);  analogWrite(PWMB, s);
}

void stopMotors(){
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  digitalWrite(AIN1, LOW);
  digitalWrite(BIN1, LOW);
}

// =====================================================
// Setup
// =====================================================
void setup() {
  Serial.begin(9600);

  // Start IR receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.println("Manual IR mode ready.");

  // Motor pins
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(STBY, OUTPUT);

  // Enable motor driver
  digitalWrite(STBY, HIGH);

  stopMotors();
}

// =====================================================
// Main loop
// =====================================================
void loop() {

  // ---------------- Read IR (if available) ----------------
  if (IrReceiver.decode()) {

    // RAW 32-bit button code
    uint32_t code = IrReceiver.decodedIRData.decodedRawData;

    // Some remotes send "repeat frames" instead of repeating the same code.
    // If your remote sends 0x0 on hold, we reuse the previous command.
    if (code == 0x0) code = lastCmd;
    else lastCmd = code;

    // Update "last seen" time so hold-to-move keeps going
    lastSignalTime = millis();

    // Debug print
    Serial.print("RAW 32-bit: 0x");
    Serial.println(code, HEX);

    // ---------------- Speed controls (tap buttons) ----------------
    if (code == BTN_SPEED_UP) {
      speedValue = constrain(speedValue + SPEED_STEP, MIN_SPEED, MAX_SPEED);
      Serial.print("Speed -> "); Serial.println(speedValue);
    }
    else if (code == BTN_SPEED_DOWN) {
      speedValue = constrain(speedValue - SPEED_STEP, MIN_SPEED, MAX_SPEED);
      Serial.print("Speed -> "); Serial.println(speedValue);
    }

    // ---------------- Movement controls (hold buttons) ----------------
    else if (code == BTN_UP)        forward(speedValue);
    else if (code == BTN_DOWN)      backward(speedValue);
    else if (code == BTN_LEFT)      turnLeft(speedValue);
    else if (code == BTN_RIGHT)     turnRight(speedValue);
    else                            stopMotors();

    // Ready for next IR frame
    IrReceiver.resume();
  }

  // ---------------- Stop when button released ----------------
  if (millis() - lastSignalTime > RELEASE_TIMEOUT) {
    stopMotors();
  }
}
