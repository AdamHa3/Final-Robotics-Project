/*
=====================================================
 MODE 4 – Victory Lap 
-----------------------------------------------------
 • Robot drives in a rectangular path using timed moves
 • Routine runs ONCE, then stops permanently
 • Uses only the motor driver pins (no LED code)
=====================================================
*/

// --------------------------------------------------
// Motor driver pins
// --------------------------------------------------
const uint8_t PWMA = 5;           // PWM pin for LEFT motor speed
const uint8_t AIN1 = 7;           // Direction pin for LEFT motor

const uint8_t PWMB = 6;           // PWM pin for RIGHT motor speed
const uint8_t BIN1 = 8;           // Direction pin for RIGHT motor

const uint8_t STBY = 3;           // Standby pin (HIGH enables motors)

// --------------------------------------------------
// Stop both motors immediately
// --------------------------------------------------
void stopMotors() {
  analogWrite(PWMA, 0);           // Stop left motor
  analogWrite(PWMB, 0);           // Stop right motor
}

// --------------------------------------------------
// Move robot straight forward
// --------------------------------------------------
void forward(int s) {
  digitalWrite(AIN1, HIGH);       // Left motor forward
  analogWrite(PWMA, s);           // Left motor speed

  digitalWrite(BIN1, HIGH);       // Right motor forward
  analogWrite(PWMB, s);           // Right motor speed
}

// --------------------------------------------------
// Spin robot left (in-place turn)
// --------------------------------------------------
void spinLeft(int s) {
  digitalWrite(AIN1, HIGH);       // Left motor forward
  analogWrite(PWMA, s);           // Left motor speed

  digitalWrite(BIN1, LOW);        // Right motor backward
  analogWrite(PWMB, s);           // Right motor speed
}

// --------------------------------------------------
// Setup (runs once)
// --------------------------------------------------
void setup() {
  Serial.begin(9600);             // Start Serial Monitor (debugging)

  // Configure motor pins as OUTPUT
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(STBY, OUTPUT);

  digitalWrite(STBY, HIGH);       // Enable motor driver

  stopMotors();                   // Start stopped
  Serial.println("MODE 4 READY (NO NEOPIXEL)");
}

// --------------------------------------------------
// Main loop (runs once, then stops forever)
// --------------------------------------------------
void loop() {

  // ------------------------------------------------
  // Victory Lap Path (Rectangle-like pattern)
  // ------------------------------------------------
  forward(140);                   // Drive forward (long side)
  delay(4000);

  spinLeft(130);                  // Turn left ~90°
  delay(600);

  forward(140);                   // Drive forward (short side)
  delay(2500);

  spinLeft(130);                  // Turn left ~90°
  delay(600);

  forward(140);                   // Drive forward (long side)
  delay(4000);

  spinLeft(130);                  // Turn left ~90°
  delay(600);

  forward(140);                   // Drive forward (short side)
  delay(2500);

  spinLeft(130);                  // Final turn to align back
  delay(600);

  stopMotors();                   // Stop after completing the lap

  // ------------------------------------------------
  // Stop forever so the routine does NOT repeat
  // ------------------------------------------------
  while (true) {
    // Do nothing
  }
}
