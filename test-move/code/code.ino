#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "Hnyp";
const char *password = "12345777";

AsyncWebServer server(80);

String header;

//int enAB = 5;  // GPIO5 (D1)
//int in1 = 4;   // GPIO4 (D2)
//int in2 = 0;   // GPIO0 (D3)
//int in3 = 2;   // GPIO2 (D4)
//int in4 = 14;  // GPIO14 (D5)

int in1 = 4;
int in2 = 0;

int in3 = 2;
int in4 = 14;

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
    </div>
    <script>
        function runCommand(x) {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/" + x, true);
            xhr.send();
        }
    </script>
</body>

</html>
)rawliteral";

// This function lets you control spinning direction of motors
void directionControl(String direction, int angle = 90)
{
    if (direction == "backward")
    {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
    }
    else if (direction == "forward")
    {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
    }
    else if (direction == "left")
    {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
        delay(angle * 12);
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        digitalWrite(in3, LOW);
        digitalWrite(in4, LOW);
    }
    else if (direction == "right")
    {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
        delay(angle * 12);
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        digitalWrite(in3, LOW);
        digitalWrite(in4, LOW);
    }
    else if (direction == "stop")
    {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        digitalWrite(in3, LOW);
        digitalWrite(in4, LOW);
    }
}

void setup(){
    //-------------------------------- Initializing pins
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);

    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);


    //--------------------------------- Connecting to WiFi
    Serial.begin(115200);
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

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
    server.begin();
}

void loop(){
  yield();
}

