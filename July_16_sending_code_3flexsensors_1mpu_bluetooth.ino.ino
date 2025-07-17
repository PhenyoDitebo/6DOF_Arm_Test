#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <SoftwareSerial.h>

SoftwareSerial BTserial(2, 3); // RX, TX for HC-05
Adafruit_MPU6050 mpu;

// Flex sensor analog pins
#define FLEX_SHOULDER A0
#define FLEX_ELBOW    A1
#define FLEX_WRIST    A2 // Flick (up/down)

// Raw flex range
const int minFlex = 200;
const int maxFlex = 800;

void setup() {
  Serial.begin(9600);
  BTserial.begin(9600);

  // Initialize MPU
  if (!mpu.begin()) {
    Serial.println("MPU6050 not detected. Halting.");
    while (1) delay(10);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("Glove transmitter with MPU ready.");
}

void loop() {
  //Read flex sensors
  int shoulderRaw = analogRead(FLEX_SHOULDER);
  int elbowRaw    = analogRead(FLEX_ELBOW);
  int wristRaw    = analogRead(FLEX_WRIST);

  //Map to angles
  int shoulderAngle = map(shoulderRaw, minFlex, maxFlex, 45, 120);
  int elbowAngle    = map(elbowRaw,    minFlex, maxFlex, 90, 160);
  int wristAngle    = map(wristRaw,    minFlex, maxFlex, 60, 120); // Flick

  shoulderAngle = constrain(shoulderAngle, 0, 180);
  elbowAngle    = constrain(elbowAngle,    0, 180);
  wristAngle    = constrain(wristAngle,    0, 180);

  //MPU6050 → roll for wrist rotation
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float roll = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
  int wristRotation = constrain(map((int)roll, -90, 90, 0, 180), 0, 180);

  //Format: S:<x>|E:<x>|W:<x>|R:<x>
  String message = "S:" + String(shoulderAngle);
  message += "|E:" + String(elbowAngle); 
  message += "|W:" + String(wristAngle);       // Flick
  message += "|R:" + String(wristRotation);    // Rotation

  //Send
  BTserial.println(message);
  BTserial.flush();

  //Debug
  Serial.print("Sent: ");
  Serial.println(message);

  delay(300);
}