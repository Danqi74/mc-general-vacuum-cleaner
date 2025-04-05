#include <Wire.h>
#include <MPU6050.h>

#define EN_LEFT 1
#define EN_RIGHT 2
#define IN1 22
#define IN2 24
#define IN3 26
#define IN4 28
#define FAN_IN 48
#define BR_IN 49
#define IR_SENSOR_LEFT_IN 50
#define IR_SENSOR_RIGHT_IN 51
#define IR_SENSOR_LEFT_OUT 52
#define IR_SENSOR_RIGHT_OUT 53

int inPins[] = {IN1, IN2, IN3, IN4};

bool fanState = false;
bool brushState = false;

MPU6050 mpu;
float Kp = 1.0, Ki = 0.0, Kd = 0.5; 
float previous_error = 0, integral = 0;

bool collisionInProgress = false;
bool isRightCollision = false;
unsigned long collisionStartTime = 0;
const unsigned long reverseDuration = 1200;
const unsigned long turnDuration = 1000;

void setMotorState(int state[4]) {
    digitalWrite(IN1, state[0]);
    digitalWrite(IN2, state[1]);
    digitalWrite(IN3, state[2]);
    digitalWrite(IN4, state[3]);
}

void directionControl(String direction) {
    int step0[] = {LOW, LOW, LOW, LOW};
    setMotorState(step0);
    if (direction == "forward") {
        int step[] = {LOW, HIGH, LOW, HIGH};
        setMotorState(step);
    } else if (direction == "backward") {
        int step[] = {HIGH, LOW, HIGH, LOW};
        setMotorState(step);
    } else if (direction == "left") {
        int step[] = {LOW, HIGH, HIGH, LOW};
        setMotorState(step);
    } else if (direction == "right") {
        int step[] = {HIGH, LOW, LOW, HIGH};
        setMotorState(step);
    } else if (direction == "stop") {
        int step[] = {LOW, LOW, LOW, LOW};
        setMotorState(step);
    }
}

void speedControl(bool isLeft, uint8_t value) {
    analogWrite(isLeft ? EN_LEFT : EN_RIGHT, value);
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    mpu.initialize();

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

    if (!mpu.testConnection()) {
        Serial.println("Помилка підключення до MPU6050!");
        while (1);
    }
    Serial.println("MPU6050 підключено!");
}

float computePID(float error) {
    integral += error;
    float derivative = error - previous_error;
    float output = Kp * error + Ki * integral + Kd * derivative;
    previous_error = error;
    return output;
}

void adjustMotors() {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    float error = gx / 131.0;
    float correction = computePID(error);
    
    int leftSpeed = constrain(150 - correction, 0, 255);
    int rightSpeed = constrain(150 + correction, 0, 255);
    
    analogWrite(EN_LEFT, leftSpeed);
    analogWrite(EN_RIGHT, rightSpeed);
}

void handleCollision() {
    unsigned long currentMillis = millis();

    if (!collisionInProgress) {
        directionControl("stop");
        collisionStartTime = currentMillis;
        collisionInProgress = true;
    }

    if (collisionInProgress) {
        if (currentMillis - collisionStartTime <= reverseDuration) {
            directionControl("backward");
        } else if (currentMillis - collisionStartTime <= reverseDuration + turnDuration) {
            if (isRightCollision) {
                directionControl("left");
            } else {
                directionControl("right");
            }
        } else {
            collisionInProgress = false;
        }
    }
}

void handleInfraredSensors() {
    bool leftTriggered = digitalRead(IR_SENSOR_LEFT_IN) == HIGH;
    bool rightTriggered = digitalRead(IR_SENSOR_RIGHT_IN) == HIGH;

    if (rightTriggered) {
        isRightCollision = true;
        handleCollision();
    } else if (leftTriggered) {
        isRightCollision = false;
        handleCollision();
    }
}

void loop() {
    if (collisionInProgress) {
        handleCollision();
    } else {
        handleInfraredSensors();
        adjustMotors();
        directionControl("forward");
    }
}
