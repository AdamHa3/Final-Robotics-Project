// ============================================================
//  SIMPLE LINE FOLLOWER (Elegoo V4) — rewritten version
//  - Reads 3 analog line sensors (A0/A1/A2)
//  - Steers based on which sensor sees the line
//  - Prints sensor values for debugging
//  - Has a safety stop when the middle sensor is extremely high
// ============================================================

// ---------------------------
// Motor driver pins
// ---------------------------
const int PWMA = 5;   // Left motor speed (PWM)
const int AIN1 = 7;   // Left motor direction
const int PWMB = 6;   // Right motor speed (PWM)
const int BIN1 = 8;   // Right motor direction
const int STBY = 3;   // Driver enable (HIGH = motors enabled)

// Group motor pins for easy setup
const int motorPins[] = {PWMA, AIN1, PWMB, BIN1, STBY};

// ---------------------------
// Line sensor pins
// ---------------------------
const int S_RIGHT  = A0;   // Right sensor
const int S_MIDDLE = A1;   // Middle sensor
const int S_LEFT   = A2;   // Left sensor

// ---------------------------
// Tuning values
// ---------------------------
const int THRESH_LINE = 400;    // Line detection threshold
const int STOP_HIGH   = 1000;   // Emergency stop threshold (middle sensor)
const int BASE_SPEED  = 55;     // Normal driving speed (PWM)

// ---------------------------
// Motor helper functions
// ---------------------------

// Drive both motors with a single speed value (forward)
void driveForward(int spd) {
  digitalWrite(AIN1, HIGH);
  analogWrite(PWMA, spd);

  digitalWrite(BIN1, HIGH);
  analogWrite(PWMB, spd);
}

// Drive both motors backward
void driveBackward(int spd) {
  digitalWrite(AIN1, LOW);
  analogWrite(PWMA, spd);

  digitalWrite(BIN1, LOW);
  analogWrite(PWMB, spd);
}

// Turn left (spin-in-place style)
void turnLeft(int spd) {
  digitalWrite(AIN1, HIGH);
  analogWrite(PWMA, spd);

  digitalWrite(BIN1, LOW);
  analogWrite(PWMB, spd);
}

// Turn right (spin-in-place style)
void turnRight(int spd) {
  digitalWrite(AIN1, LOW);
  analogWrite(PWMA, spd);

  digitalWrite(BIN1, HIGH);
  analogWrite(PWMB, spd);
}

// Stop motors safely
void motorsStop() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  // Direction pins low (optional safety)
  digitalWrite(AIN1, LOW);
  digitalWrite(BIN1, LOW);
}

// ---------------------------
// Setup
// ---------------------------
void setup() {
  Serial.begin(9600);               // Debug output

  // Configure motor pins as outputs
  for (int i = 0; i < 5; i++) {
    pinMode(motorPins[i], OUTPUT);
  }

  digitalWrite(STBY, HIGH);         // Enable the motor driver
  motorsStop();                     // Start stopped
}

// ---------------------------
// Main loop (line following)
// ---------------------------
void loop() {

  // Read sensors
  int rVal = analogRead(S_RIGHT);
  int mVal = analogRead(S_MIDDLE);
  int lVal = analogRead(S_LEFT);

  // Print values (Left Middle Right)
  Serial.print(lVal);
  Serial.print("   ");
  Serial.print(mVal);
  Serial.print("   ");
  Serial.println(rVal);

  // 1) Emergency stop (strong reading on middle sensor)
  if (mVal > STOP_HIGH) {
    motorsStop();
    return;
  }

  // 2) Decide movement (priority: middle first, then left/right)
  if (mVal > THRESH_LINE) {
    // Best case: centered on line
    driveForward(BASE_SPEED);
  }
  else if (lVal > THRESH_LINE) {
    // Line is under the left sensor -> steer left
    turnLeft(BASE_SPEED);
  }
  else if (rVal > THRESH_LINE) {
    // Line is under the right sensor -> steer right
    turnRight(BASE_SPEED);
  }
  else {
    // 3) Lost line: keep turning slightly right to try to find it again
    turnRight(BASE_SPEED);
  }
}
