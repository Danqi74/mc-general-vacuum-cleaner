#include <Wire.h>
#include <MPU6050.h>
#include <SerialTransfer.h>

#define EN_LEFT 2
#define EN_RIGHT 3
#define IN1 22
#define IN2 24
#define IN3 26
#define IN4 28
#define FAN_IN 30
#define BR_IN 32
#define IR_SENSOR_LEFT_IN 50
#define IR_SENSOR_RIGHT_IN 51
#define IR_SENSOR_LEFT_OUT 52
#define IR_SENSOR_RIGHT_OUT 53

MPU6050 mpu;
SerialTransfer transfer;

bool newReceivedData = false;

float Kp = 11.0, Ki = 2.0, Kd = 1.0;
float previous_error = 0, integral = 0;

bool collisionInProgress = false;
bool isRightCollision = false;
unsigned long collisionStartTime = 0;
const unsigned long reverseDuration = 1200;
const unsigned long turnDuration = 1000;

float yaw = 0;
unsigned long lastUpdate = 0;

float gyroBias = 0;
float smoothAx = 0, smoothAy = 0, smoothAz = 0;
float smoothGx = 0, smoothGy = 0, smoothGz = 0;

const float alpha = 0.05;  // Коефіцієнт згладжування (0..1, де ближче до 0 — більше згладжування)

struct ToSend {
  int16_t ax = 0;
  int16_t ay = 0;
  int16_t az = 0;
  int16_t gx = 0;
  int16_t gy = 0;
  int16_t gz = 0;
  bool leftTrigg = false;
  bool rightTrigg = false;
  float gzBias = 0.0;
} txData;

struct ToReceive {
  char command;
  bool fan;
  bool brush;
  bool clean;
} rxData;

void setMotorState(int state[4]) {
  digitalWrite(IN1, state[0]);
  digitalWrite(IN2, state[1]);
  digitalWrite(IN3, state[2]);
  digitalWrite(IN4, state[3]);
}

void directionControl(char direction) {
  int step0[] = {LOW, LOW, LOW, LOW};
  setMotorState(step0);
  if (direction == 'f') {
    int step[] = {LOW, HIGH, LOW, HIGH};
    setMotorState(step);
  } else if (direction == 'b') {
    int step[] = {HIGH, LOW, HIGH, LOW};
    setMotorState(step);
  } else if (direction == 'l') {
    int step[] = {LOW, HIGH, HIGH, LOW};
    setMotorState(step);
  } else if (direction == 'r') {
    int step[] = {HIGH, LOW, LOW, HIGH};
    setMotorState(step);
  } else if (direction == 's') {
    int step[] = {LOW, LOW, LOW, LOW};
    setMotorState(step);
  }
}

void speedControl(bool isLeft, uint8_t value) {
  analogWrite(isLeft ? EN_LEFT : EN_RIGHT, value);
}

void calculateGyroBias(){
  long sum = 0;
  const int samples = 200;
  for (int i = 0; i < samples; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    sum += gz;
  }
  txData.gzBias = sum / (float)samples;
}

void setup() {
  Serial1.begin(115200);
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();
  transfer.begin(Serial1);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(EN_LEFT, OUTPUT);
  pinMode(EN_RIGHT, OUTPUT);
  pinMode(FAN_IN, OUTPUT);
  pinMode(BR_IN, OUTPUT);
  pinMode(IR_SENSOR_LEFT_IN, INPUT);
  pinMode(IR_SENSOR_RIGHT_IN, INPUT);
  pinMode(IR_SENSOR_LEFT_OUT, OUTPUT);
  pinMode(IR_SENSOR_RIGHT_OUT, OUTPUT);

  digitalWrite(FAN_IN, LOW);
  digitalWrite(BR_IN, LOW);
  digitalWrite(IR_SENSOR_LEFT_OUT, HIGH);
  digitalWrite(IR_SENSOR_RIGHT_OUT, HIGH);

  // analogWrite(EN_LEFT, 250);
  // analogWrite(EN_RIGHT, 250);

  

  if (!mpu.testConnection()) {
    Serial.println("Помилка підключення до MPU6050!");
    while (1);
  }
  Serial.println("MPU6050 підключено!");

  lastUpdate = millis();
}

float computePID(float error) {
  integral += error;
  float derivative = error - previous_error;
  float output = Kp * error + Ki * integral + Kd * derivative;
  previous_error = error;
  return output;
}

void updateYaw() {
  unsigned long now = millis();
  float dt = (now - lastUpdate) / 1000.0;
  lastUpdate = now;

  int16_t gz = txData.gz;
  yaw += (gz - txData.gzBias / 131.0) * dt;
}

void adjustMotors() {
  updateYaw();
  float correction = computePID(yaw);

  int leftSpeed = constrain(50 - correction, 0, 255);
  int rightSpeed = constrain(250 + correction, 0, 255);

  analogWrite(EN_LEFT, leftSpeed);
  analogWrite(EN_RIGHT, rightSpeed);
}


void checkMPU() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Експоненціальне згладжування
  smoothAx = alpha * ax + (1 - alpha) * smoothAx;
  smoothAy = alpha * ay + (1 - alpha) * smoothAy;
  smoothAz = alpha * az + (1 - alpha) * smoothAz;

  smoothGx = alpha * gx + (1 - alpha) * smoothGx;
  smoothGy = alpha * gy + (1 - alpha) * smoothGy;
  smoothGz = alpha * gz + (1 - alpha) * smoothGz;

  // Передаємо згладжені значення
  txData.ax = smoothAx;
  txData.ay = smoothAy;
  txData.az = smoothAz;
  txData.gx = smoothGx;
  txData.gy = smoothGy;
  txData.gz = smoothGz;
}

void handleCollision() {
  unsigned long currentMillis = millis();

  if (!collisionInProgress) {
    directionControl('s');
    collisionStartTime = currentMillis;
    collisionInProgress = true;
  }

  if (collisionInProgress) {
    if (currentMillis - collisionStartTime <= reverseDuration) {
      directionControl('b');
    } else if (currentMillis - collisionStartTime <= reverseDuration + turnDuration) {
      directionControl(isRightCollision ? 'l' : 'r');
    } else {
      collisionInProgress = false;
    }
  }
}

void handleInfraredSensors() {
  bool leftTriggered = digitalRead(IR_SENSOR_LEFT_IN) == HIGH;
  bool rightTriggered = digitalRead(IR_SENSOR_RIGHT_IN) == HIGH;

  txData.leftTrigg = leftTriggered;
  txData.rightTrigg = rightTriggered;

  if (rightTriggered) {
    isRightCollision = true;
    handleCollision();
  } else if (leftTriggered) {
    isRightCollision = false;
    handleCollision();
  }
}

void loop() {
  if (transfer.available()) {
    transfer.rxObj(rxData);
    newReceivedData = true;
  }

  checkMPU();

  if (newReceivedData) {
    if (rxData.fan) {
      digitalWrite(FAN_IN, HIGH);
    } else {
      digitalWrite(FAN_IN, LOW);
    }
    
    if (rxData.brush) {
      digitalWrite(BR_IN, HIGH);
    } else {
      digitalWrite(BR_IN, LOW);
    }

    newReceivedData = false;
  }

  if (rxData.clean) {
    if (collisionInProgress) {
      handleCollision();
    } else {
      handleInfraredSensors();
      adjustMotors();
      directionControl('f');
    }
  } else {
      directionControl(rxData.command);
      if (rxData.command == 's'){
        calculateGyroBias();
      }
    }
  transfer.sendDatum(txData);
}