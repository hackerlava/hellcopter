#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define MOTOR1_PIN1 4
#define MOTOR1_PIN2 3
#define MOTOR2_PIN1 1
#define MOTOR2_PIN2 2
#define ESC_PIN 9

#define DEFAULT_SPEED 100   // Half speed (out of 255)
#define BOOST_SPEED 255     // Full speed

// ESC Configuration
#define MIN_SPEED 1000  // 1000μs (typical minimum)
#define MAX_SPEED 1500  // 2000μs (typical maximum)
#define ARM_SPEED 1000  // 1000μs for arming

// Global variables for left and right speeds
int globalLeftSpeed = 0;
int globalRightSpeed = 0;
float currentESCSpeed = 0; // Track current ESC speed for ramping

unsigned long lastCommandTime = 0;
uint8_t lastCommand = 0;
const unsigned long commandTimeout = 3000; // 3 seconds
const float rampStep = 0.5; // Ramp step size (%)
const unsigned long rampDelay = 20; // Delay between steps (ms)

void setESCSpeed(float targetPercent) {
  // Calculate turn-compensated target speed
  float compensatedPercent = targetPercent;
  if (targetPercent > 0) { // Only apply compensation when weapon is active
    compensatedPercent += (globalLeftSpeed / 2.55) - (globalRightSpeed / 2.55);
  }
  
  // Clamp the compensated value
  compensatedPercent = constrain(compensatedPercent, 0, 100);
  
  // Apply ramping to the compensated value
  if (compensatedPercent > currentESCSpeed) {
    currentESCSpeed += min(rampStep, compensatedPercent - currentESCSpeed);
  } else if (compensatedPercent < currentESCSpeed) {
    currentESCSpeed -= min(rampStep, currentESCSpeed - compensatedPercent);
  }
  
  // Final clamp to ensure we stay within bounds
  currentESCSpeed = constrain(currentESCSpeed, 0, 100);
  
  // Convert to pulse width and send to ESC
  uint32_t pulseWidth = map(currentESCSpeed, 0, 100, MIN_SPEED, MAX_SPEED);
  pulseWidth = constrain(pulseWidth, MIN_SPEED, MAX_SPEED);
  
  digitalWrite(ESC_PIN, HIGH);
  delayMicroseconds(pulseWidth);
  digitalWrite(ESC_PIN, LOW);
  delayMicroseconds(20000 - pulseWidth);
  
  Serial.print("ESC: Target=");
  Serial.print(targetPercent);
  Serial.print("%, Compensated=");
  Serial.print(compensatedPercent);
  Serial.print("%, Current=");
  Serial.print(currentESCSpeed);
  Serial.print("%, Pulse=");
  Serial.print(pulseWidth);
  Serial.println("μs");
}

void set_motor_pwm(int pin1, int pin2, int speed) {
  if (speed > 0) {
    analogWrite(pin1, speed);
    analogWrite(pin2, 0);
  } else if (speed < 0) {
    analogWrite(pin1, 0);
    analogWrite(pin2, -speed);
  } else {
    analogWrite(pin1, 0);
    analogWrite(pin2, 0);
  }
}

void stopMotors() {
  globalLeftSpeed = 0;
  globalRightSpeed = 0;
  currentESCSpeed = 0;
  set_motor_pwm(MOTOR1_PIN1, MOTOR1_PIN2, 0);
  set_motor_pwm(MOTOR2_PIN1, MOTOR2_PIN2, 0);
  setESCSpeed(0);
}

void executeCommand(uint8_t cmd) {
  bool boost = (cmd >= 18);
  int speed = boost ? BOOST_SPEED : DEFAULT_SPEED;

  // Movement decoding
  bool w = cmd == 2 || cmd == 6 || cmd == 7 || cmd == 10 || cmd == 14 || cmd == 15 || cmd == 18 || cmd == 22 || cmd == 23 || cmd == 26 || cmd == 30 || cmd == 31;
  bool a = cmd == 3 || cmd == 6 || cmd == 8 || cmd == 11 || cmd == 14 || cmd == 16 || cmd == 19 || cmd == 22 || cmd == 24 || cmd == 27 || cmd == 30 || cmd == 32;
  bool s = cmd == 4 || cmd == 8 || cmd == 9 || cmd == 12 || cmd == 16 || cmd == 17 || cmd == 20 || cmd == 24 || cmd == 25 || cmd == 28 || cmd == 32 || cmd == 33;
  bool d = cmd == 5 || cmd == 7 || cmd == 9 || cmd == 13 || cmd == 15 || cmd == 17 || cmd == 21 || cmd == 23 || cmd == 25 || cmd == 29 || cmd == 31 || cmd == 33;
  bool space = (cmd >= 1 && cmd <= 17) && !(cmd >= 2 && cmd <= 9);

  // Calculate motor speeds
  globalLeftSpeed = 0;
  globalRightSpeed = 0;

  if (w) {
    globalLeftSpeed = speed;
    globalRightSpeed = speed;
  } else if (s) {
    globalLeftSpeed = -speed;
    globalRightSpeed = -speed;
  }

  if (a) {
    globalLeftSpeed -= speed / 2;
    globalRightSpeed += speed / 2;
  } else if (d) {
    globalLeftSpeed += speed / 2;
    globalRightSpeed -= speed / 2;
  }

  // Apply constraints
  globalLeftSpeed = constrain(globalLeftSpeed, -255, 255);
  globalRightSpeed = constrain(globalRightSpeed, -255, 255);

  // Set motor outputs
  set_motor_pwm(MOTOR1_PIN1, MOTOR1_PIN2, globalLeftSpeed);
  set_motor_pwm(MOTOR2_PIN1, MOTOR2_PIN2, globalRightSpeed);

  // Weapon control
  setESCSpeed(space ? 100 : 0);
}

// BLE Setup
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value.length() != 1) {
      Serial.print("Invalid packet size: ");
      Serial.println(value.length());
      return;
    }

    uint8_t cmd = value[0];
    Serial.print("Received command: ");
    Serial.println(cmd);

    if (cmd < 34) {
      lastCommand = cmd;
      lastCommandTime = millis();
      executeCommand(cmd);
    }
  }
};

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR1_PIN1, OUTPUT);
  pinMode(MOTOR1_PIN2, OUTPUT);
  pinMode(MOTOR2_PIN1, OUTPUT);
  pinMode(MOTOR2_PIN2, OUTPUT);
  pinMode(ESC_PIN, OUTPUT);

  // BLE setup
  BLEDevice::init("Hellcopter");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      "beb5483e-36e1-4688-b7f5-ea07361b26a8",
      BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  // Arm ESC
  Serial.println("Arming ESC...");
  for (int i = 0; i < 150; i++) {
    digitalWrite(ESC_PIN, HIGH);
    delayMicroseconds(ARM_SPEED);
    digitalWrite(ESC_PIN, LOW);
    delayMicroseconds(20000 - ARM_SPEED);
    delay(20);
  }
  currentESCSpeed = 0;
  Serial.println("Ready");
}

void loop() {
  if (millis() - lastCommandTime > commandTimeout) {
    stopMotors();
  } else {
    executeCommand(lastCommand);
  }
  delay(rampDelay);
}
