#include <Adafruit_MPU6050.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <SoftwareSerial.h>

SoftwareSerial BTserial(2, 3); // RX, TX for Bluetooth
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// ✅ Servo channel assignments
const int shoulderMain   = 0; // Channel 0
const int shoulderMirror = 1; // Channel 1
const int elbowJoint     = 2; // Channel 2
const int wristFlick     = 3; // Channel 3 ← Up/down
const int wristRotation  = 4; // Channel 4 ← Rotation
const int gripper        = 5; // Channel 5 ← Gripper

// 🧠 Current positions
int posShoulder = 0;     // Start main at 0
int posMirror   = 180;   // Mirror at 180 for perfect inverse
int posElbow    = 90;
int posWrist    = 90;
int posRotate   = 90;
int posGripper  = 90;

// 📉 Smooth movement from current to target
void smoothMove(int channel, int &current, int target, int delayStep = 10) {
  if (current == target) return;

  int step = (target > current) ? 1 : -1;

  while (current != target) {
    current += step;
    int pulse = map(current, 0, 180, 150, 600);
    pwm.setPWM(channel, 0, pulse);
    delay(delayStep);
  }
}

void setup() {
  Serial.begin(9600);
  BTserial.begin(9600);

  pwm.begin();
  pwm.setPWMFreq(50); // 50 Hz for standard servos
  delay(1000);

  // Move to initial shoulder positions
  pwm.setPWM(shoulderMain, 0, map(posShoulder, 0, 180, 150, 600));
  pwm.setPWM(shoulderMirror, 0, map(posMirror, 0, 180, 150, 600));

  Serial.println("Receiver Ready: Smooth Motion, Shoulder Starts at 0/180");
}

void loop() {
  if (BTserial.available()) {
    String input = BTserial.readStringUntil('\n');
    Serial.print("Raw Input: ");
    Serial.println(input);

    // 🔍 Extract joint angles
    int sAngle = extractAngle(input, "S:");
    int eAngle = extractAngle(input, "E:");
    int wAngle = extractAngle(input, "W:"); // Wrist flick
    int rAngle = extractAngle(input, "R:"); // Wrist rotation
    int gAngle = extractAngle(input, "G:"); // Gripper

    // ✅ Shoulder & Mirror
    smoothMove(shoulderMain, posShoulder, sAngle);
    int mirrorDeg = 180 - sAngle; // exact mirror
    smoothMove(shoulderMirror, posMirror, mirrorDeg);

    // 💪 Elbow
    int limitedElbow = constrain(eAngle, 75, 180);
    smoothMove(elbowJoint, posElbow, limitedElbow);

    // ✋ Wrist Flick
    smoothMove(wristFlick, posWrist, wAngle);

    // 🔁 Wrist Rotation
    smoothMove(wristRotation, posRotate, rAngle);

    // 🤏 Gripper
    smoothMove(gripper, posGripper, gAngle);
  }

  delay(10); // Small refresh pause
}

// 🧠 Utility to extract angle value from string
int extractAngle(String input, String label) {
  int labelIndex = input.indexOf(label);
  if (labelIndex == -1) return 90;

  int separatorIndex = input.indexOf("|", labelIndex);
  String value;
  if (separatorIndex == -1) {
    value = input.substring(labelIndex + 2);
  } else {
    value = input.substring(labelIndex + 2, separatorIndex);
  }

  return constrain(value.toInt(), 0, 180);
}
