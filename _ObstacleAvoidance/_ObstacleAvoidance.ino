#include <Servo.h>                 // Servo library (used to rotate the ultrasonic sensor)

// =====================================================
//      OBSTACLE AVOIDANCE (SERVO SCAN + ULTRASONIC)
//      Logic: Forward check -> Left check -> Right check
//             If both blocked -> reverse + 180 turn
// =====================================================

// ---------------- Servo ----------------
Servo scanServo;                   // Servo object for scanning left/right
const int SERVO_PIN = 10;          // Servo signal pin

// ---------------- Ultrasonic pins ----------------
const int TRIG_PIN = 13;           // Trigger sends the pulse
const int ECHO_PIN = 12;           // Echo receives the reflection

// ---------------- Motor driver pins ----------------
const int L_PWM = 5;               // Left motor speed (PWM)
const int L_DIR = 7;               // Left motor direction
const int R_PWM = 6;               // Right motor speed (PWM)
const int R_DIR = 8;               // Right motor direction
const int STBY  = 3;               // Motor driver enable

// ---------------- Tuning values ----------------
const int SAFE_CM = 25;            // Minimum safe distance before avoiding
const int DRIVE_SPD = 70;          // Normal movement speed
const int TURN_SPD  = 70;          // Turning speed

void setup() {
  Serial.begin(9600);              // Debug output

  // Servo setup
  scanServo.attach(SERVO_PIN);     // Connect servo to pin 10

  // Ultrasonic setup
  pinMode(TRIG_PIN, OUTPUT);       // Trigger is output
  pinMode(ECHO_PIN, INPUT);        // Echo is input

  // Motor setup
  pinMode(L_PWM, OUTPUT);
  pinMode(L_DIR, OUTPUT);
  pinMode(R_PWM, OUTPUT);
  pinMode(R_DIR, OUTPUT);
  pinMode(STBY, OUTPUT);

  digitalWrite(STBY, HIGH);        // Enable motor driver
  stopMotors();                    // Start stopped
}

void loop() {

  // =================================================
  // 1) CHECK FRONT
  // =================================================
  scanServo.write(90);             // Face forward
  delay(150);                      // Let servo settle

  float front = getDistanceCM();   // Read distance ahead

  // 0 means "no echo / out of range" -> treat as clear
  if (front == 0 || front >= SAFE_CM) {
    forward(DRIVE_SPD);            // Keep moving
    return;
  }

  // Something is too close
  stopMotors();
  delay(150);

  // =================================================
  // 2) CHECK LEFT FIRST
  // =================================================
  scanServo.write(150);            // Look left
  delay(250);

  float leftD = getDistanceCM();
  if (leftD == 0) leftD = 300;     // Treat out-of-range as clear

  if (leftD > SAFE_CM) {
    Serial.println("Clear LEFT -> turning left");
    turnLeft(TURN_SPD);
    delay(500);
    return;
  }

  // =================================================
  // 3) THEN CHECK RIGHT
  // =================================================
  scanServo.write(30);             // Look right
  delay(250);

  float rightD = getDistanceCM();
  if (rightD == 0) rightD = 300;   // Treat out-of-range as clear

  Serial.print("Right distance: ");
  Serial.println(rightD);

  if (rightD > SAFE_CM) {
    Serial.println("Clear RIGHT -> turning right");
    turnRight(TURN_SPD);
    delay(500);
    return;
  }

  // =================================================
  // 4) BOTH SIDES BLOCKED -> ESCAPE MOVE
  // =================================================
  Serial.println("Blocked left+right -> reverse + 180");

  backward(DRIVE_SPD);
  delay(600);

  // 180-degree turn (tune delay to match your robot)
  turnLeft(TURN_SPD);
  delay(1000);

  stopMotors();
}

// =====================================================
// Ultrasonic distance function (cm)
// Returns 0 if no echo received / out of range
// =====================================================
float getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long t = pulseIn(ECHO_PIN, HIGH, 25000);  // Timeout ~25ms
  if (t == 0) return 0;

  float cm = (t * 0.0343f) / 2.0f;          // Convert time to distance
  if (cm > 300) return 0;                   // Ignore weird far readings

  return cm;
}

// =====================================================
// Motor helpers
// =====================================================
void forward(int s){
  digitalWrite(L_DIR, HIGH);  analogWrite(L_PWM, s);
  digitalWrite(R_DIR, HIGH);  analogWrite(R_PWM, s);
}

void backward(int s){
  digitalWrite(L_DIR, LOW);   analogWrite(L_PWM, s);
  digitalWrite(R_DIR, LOW);   analogWrite(R_PWM, s);
}

void turnLeft(int s){
  digitalWrite(L_DIR, HIGH);  analogWrite(L_PWM, s);
  digitalWrite(R_DIR, LOW);   analogWrite(R_PWM, s);
}

void turnRight(int s){
  digitalWrite(L_DIR, LOW);   analogWrite(L_PWM, s);
  digitalWrite(R_DIR, HIGH);  analogWrite(R_PWM, s);
}

void stopMotors(){
  analogWrite(L_PWM, 0);
  analogWrite(R_PWM, 0);
}
