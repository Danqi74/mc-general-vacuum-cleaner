#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "GENERAL_SUCKER";
const char *password = "MaxGay777";

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);

String header;

//int enAB = 5;  // GPIO5 (D1)
//int IN1 = 4;   // GPIO4 (D2)
//int IN2 = 0;   // GPIO0 (D3)
//int IN3 = 2;   // GPIO2 (D4)
//int IN4 = 14;  // GPIO14 (D5)

#define EN_LEFT D1
#define EN_RIGHT D6

#define IN1 D2
#define IN2 D3
#define IN3 D4
#define IN4 D5

#define FAN_IN D7
#define BR_IN D8

#define COLLISION_BTN D0

int inPins[] = {IN1, IN2, IN3, IN4};

bool fanState = false;
bool brushState = false;
bool cleanState = false;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial;
            text-align: center;
            margin: 0px auto;
            padding-top: 30px;
        }

        .h1 {
            text-align: center;
            width: 100%;
        }

        .button {
            padding: 10px 20px;
            font-size: 24px;
            text-align: center;
            outline: none;
            color: #fff;
            background-color: #4d9441;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            min-width: 137px;
        }

        .button:hover {
            background-color: #45843a
        }

        .button:active {
            transform: translateY(2px);
        }

        .buttons {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
            width: max-content;
            margin: auto;
        }

        .button:nth-child(1) {
            grid-column: 2;
            grid-row: 1;
        }

        /* FORWARD */
        .button:nth-child(2) {
            grid-column: 2;
            grid-row: 3;
        }

        /* BACKWARD */
        .button:nth-child(3) {
            grid-column: 3;
            grid-row: 2;
        }

        /* RIGHT */
        .button:nth-child(4) {
            grid-column: 1;
            grid-row: 2;
        }

        /* LEFT */
        .button:nth-child(5) {
            grid-column: 2;
            grid-row: 2;
        }

        /* STOP */
        .indicator {
            width: 30px;
            height: 30px;
            background-color: red; /* Початковий колір */
            border-radius: 50%;
            border: 3px solid #ccc;
            transition: background-color 0.3s ease, border-color 0.3s ease;
            margin-top: 5px;
        }
        .clean-control {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            align-items: center;
            gap: 15px;
            padding: 40px;
            background: white;
            border-radius: 12px;
            width: max-content;
            margin: auto;
        }

        /* Контейнер для кнопок */
        .button-container {
            display: flex;
            flex-wrap: wrap;
            justify-content: space-evenly;
            gap: 15px;
            width: 100%;
        }

        /* Контейнер для індикаторів */
        .indicator-container {
            display: flex;
            justify-content: space-evenly;
            gap: 30px;
            width: 100%;
            margin-top: 10px;
        }

    </style>
</head>

<body>
    <h1>GENERAL SUCKER</h1>
    <div class="buttons">
        <button class="button" onClick="runCommand('forward');">FORWARD</button>
        <button class="button" onClick="runCommand('backward');">BACKWARD</button>
        <button class="button" onClick="runCommand('right');">RIGHT</button>
        <button class="button" onClick="runCommand('left');">LEFT</button>
        <button class="button" onClick="runCommand('stop');">STOP</button>
        <div class="slidecontainer">
            <input type="range" min="0" max="255" value="255" class="slider" id="MotorLeft">
        </div>
        <div class="slidecontainer">
            <input type="range" min="0" max="255" value="255" class="slider" id="MotorRight">
        </div>
        <p><input type="number" id="valueLeft" value="255"></p>
        <p><input type="number" id="valueRight" value="255"></p>
    </div>
    <div class="clean-control">
        <div class="button-container">
            <button class="button" onClick="runCommandSwitch('fan');">FAN</button>
            <button class="button" onClick="runCommandSwitch('brush');">BRUSH</button>
            <button class="button" onClick="runCommandSwitch('clean');">CLEAN</button>
        </div>
    
        <div class="indicator-container">
            <div class="indicator" id="fan-indc"></div>
            <div class="indicator" id="brush-indc"></div>
            <div class="indicator" id="clean-indc"></div>
        </div>
    </div>
    <script>
        function runCommand(x) {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/" + x, true);
            xhr.send();
        }


        let debounceTimeout;

        function changeMotorSpeed(value, side) {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/changeMotorSpeed?value=" + value + "&side=" + side, true);
            xhr.send();
        }

        var sliderRight = document.getElementById("MotorRight");
        var outputRight = document.getElementById("valueRight");

        var sliderLeft = document.getElementById("MotorLeft");
        var outputLeft = document.getElementById("valueLeft");

        function handleSliderInput(slider, output, side) {
            output.value = slider.value; // Update number input when slider changes
            
            clearTimeout(debounceTimeout);
            debounceTimeout = setTimeout(function() {
                changeMotorSpeed(slider.value, side);
            }, 300);
        }

        function handleNumberInput(output, slider, side) {
            if (output.value >= 0 && output.value <= 255) {
                slider.value = output.value; // Update slider when number input changes
                clearTimeout(debounceTimeout);
                debounceTimeout = setTimeout(function() {
                    changeMotorSpeed(output.value, side);
                }, 300);
            }
        }

        sliderRight.oninput = function() {
            handleSliderInput(this, outputRight, "right");
        }

        sliderLeft.oninput = function() {
            handleSliderInput(this, outputLeft, "left");
        }

        outputRight.oninput = function() {
            handleNumberInput(this, sliderRight, "right");
        }

        outputLeft.oninput = function() {
            handleNumberInput(this, sliderLeft, "left");
        }


        function runCommandSwitch(x) {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/" + x, true);
            xhr.send();

            var indicator = document.getElementById(x + "-indc");

            if (indicator.style.backgroundColor === "green") {
                indicator.style.backgroundColor = "red";
            } else {
                indicator.style.backgroundColor = "green";
            }
        }
        </script>
</body>

</html>
)rawliteral";

void setMotorState(int state[4]) {
    for (int i = 0; i < 4; i++) {
        digitalWrite(inPins[i], state[i]);
    }
}

// This function lets you control spinning direction of motors
void directionControl(String direction, int angle = 90)
{
    unsigned long currentMillis = millis();
    short int type;
    if (direction == "forward")
    {
        type = 1;
        int step[] = {LOW, HIGH, LOW, HIGH};
        setMotorState(step);
        // digitalWrite(IN1, LOW);
        // digitalWrite(IN2, HIGH);
        // digitalWrite(IN3, LOW);
        // digitalWrite(IN4, HIGH);
    }
    else if (direction == "backward")
    {
        type = 2;
        int step[] = {HIGH, LOW, HIGH, LOW};
        setMotorState(step);
        // digitalWrite(IN1, HIGH);
        // digitalWrite(IN2, LOW);
        // digitalWrite(IN3, HIGH);
        // digitalWrite(IN4, LOW);
    }
    else if (direction == "left")
    {
        type = 3;
        int step[] = {LOW, HIGH, HIGH, LOW};
        setMotorState(step);
        // digitalWrite(IN1, LOW);
        // digitalWrite(IN2, HIGH);
        // digitalWrite(IN3, HIGH);
        // digitalWrite(IN4, LOW);
        while (millis() - currentMillis < 12 * angle) {
            if (type != 3) {
                return;
            }
            if (millis() - currentMillis > 5) {
                delay(1);
            }
        }
        int step2[] = {LOW, LOW, LOW, LOW};
        setMotorState(step);
        // digitalWrite(IN1, LOW);
        // digitalWrite(IN2, LOW);
        // digitalWrite(IN3, LOW);
        // digitalWrite(IN4, LOW);
    }
    else if (direction == "right")
    {
        type = 4;
        int step[] = {HIGH, LOW, LOW, HIGH};
        setMotorState(step);
        // digitalWrite(IN1, HIGH);
        // digitalWrite(IN2, LOW);
        // digitalWrite(IN3, LOW);
        // digitalWrite(IN4, HIGH);
        while (millis() - currentMillis < 12 * angle) {
            if (type != 4) {
                return;
            }
            if (millis() - currentMillis > 5) {
                delay(1);
            }
        }
        int step2[] = {LOW, LOW, LOW, LOW};
        setMotorState(step);
        // digitalWrite(IN1, LOW);
        // digitalWrite(IN2, LOW);
        // digitalWrite(IN3, LOW);
        // digitalWrite(IN4, LOW);
    }
    else if (direction == "stop")
    {
        type = 0;
        int step[] = {LOW, LOW, LOW, LOW};
        setMotorState(step);
        // digitalWrite(IN1, LOW);
        // digitalWrite(IN2, LOW);
        // digitalWrite(IN3, LOW);
        // digitalWrite(IN4, LOW);
    }
}

void speedControl(bool isLeft, u_int8_t value){
    if (isLeft){
        analogWrite(EN_LEFT, value);
    } else{
        analogWrite(EN_RIGHT, value);
    }
}

void setup(){
    //-------------------------------- Initializing pins
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(EN_LEFT, OUTPUT);
    pinMode(EN_RIGHT, OUTPUT);
    pinMode(FAN_IN, OUTPUT);
    pinMode(BR_IN, OUTPUT);
    pinMode(COLLISION_BTN, INPUT);

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);

    analogWrite(EN_LEFT, 255);
    analogWrite(EN_RIGHT, 255);

    digitalWrite(FAN_IN, LOW);
    digitalWrite(BR_IN, LOW);

    //--------------------------------- Connecting to WiFi
    Serial.begin(115200);

    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
        { request->send_P(200, "text/html", index_html); });

    server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request)
        { directionControl("forward");
        request->send_P(200, "text/html", "ok"); });

    server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request)
        { directionControl("backward");
        request->send_P(200, "text/html", "ok"); });

    server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request)
        { directionControl("right");
        request->send_P(200, "text/html", "ok"); });

    server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request)
        { directionControl("left");
        request->send_P(200, "text/html", "ok"); });

    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request)
        { directionControl("stop");
        request->send_P(200, "text/html", "ok"); });

    server.on("/changeMotorSpeed", HTTP_GET, [](AsyncWebServerRequest *request){
        String value = request->getParam("value")->value();  // Get 'value' from URL
        String side = request->getParam("side")->value();    // Get 'side' from URL

        uint8_t speedValue = value.toInt();

        if (side == "left") {
            speedControl(true, speedValue);
        } else if (side == "right") {
            speedControl(false, speedValue);
        }

        request->send_P(200, "text/html", "ok");
    });

    server.on("/fan", HTTP_GET, [](AsyncWebServerRequest *request){
        if (fanState){
            fanState = false;
            digitalWrite(FAN_IN, LOW);
        } else{
            fanState = true;
            digitalWrite(FAN_IN, HIGH);
        }

        request->send_P(200, "text/html", "ok");
    });

    server.on("/brush", HTTP_GET, [](AsyncWebServerRequest *request){
        if (brushState){
            brushState = false;
            digitalWrite(BR_IN, LOW);
        } else{
            brushState = true;
            digitalWrite(BR_IN, HIGH);
        }

        request->send_P(200, "text/html", "ok");
    });

    server.on("/clean", HTTP_GET, [](AsyncWebServerRequest *request){
        if (cleanState){
            cleanState = false;
        } else{
            cleanState = true;
        }
        request->send_P(200, "text/html", "ok");
    });

    server.begin();
}

void handleCollision(){
    directionControl("stop");
    directionControl("backward");
    unsigned long currentMillis = millis();
    while (millis() - currentMillis < 500) {}
    directionControl("left", 45);
}

void read_collision_button(){
    static bool flag_read_button = true;

    if (flag_read_button)                    {
        if (digitalRead(COLLISION_BTN) == LOW){
        flag_read_button = false;
        handleCollision();
        }
    }
    else{
        if (digitalRead(COLLISION_BTN) == HIGH){
        flag_read_button = true;
        }
    }
}


void loop(){
    unsigned long lastMillis = millis();
    if (cleanState) {
        directionControl("forward");
        read_collision_button();
    }
}

