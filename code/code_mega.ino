
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

unsigned long collisionStartTime = 0;
unsigned long reverseDuration = 1200;
unsigned long turnDuration = 1000;
bool collisionInProgress = false;
bool isRightCollision = false;

void setMotorState(int state[4]) {
    for (int i = 0; i < 4; i++) {
        digitalWrite(inPins[i], state[i]);
    }
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
    if (isLeft) {
        analogWrite(EN_LEFT, value);
    } else {
        analogWrite(EN_RIGHT, value);
    }
}

void setup() {
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

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    
    analogWrite(EN_LEFT, 255);
    analogWrite(EN_RIGHT, 255);

    digitalWrite(FAN_IN, LOW);
    digitalWrite(BR_IN, LOW);

    digitalWrite(IR_SENSOR_LEFT_OUT, HIGH);
    digitalWrite(IR_SENSOR_RIGHT_OUT, HIGH);

    Serial.begin(115200);
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
            delay(20);
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
    // digitalWrite(BR_IN, HIGH);
    // digitalWrite(FAN_IN, HIGH);
    if (collisionInProgress) handleCollision();
    handleInfraredSensors();
    directionControl("forward");

}
