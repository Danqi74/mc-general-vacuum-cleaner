#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SerialTransfer.h>

const char *ssid = "GENERAL_SUCKER";
const char *password = "MaxGay777";

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);

String header;

SerialTransfer transfer;

struct ToSend {
    char command = ' ';
    bool fan = false;
    bool brush = false;
    bool clean = false;
} txData;

struct ToReceive {
    int16_t val1;
    int16_t val2;
    int16_t val3;
    int16_t val4;
    int16_t val5;
    int16_t val6;
    bool flag1;
    bool flag2;
} rxData;

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

    
    <div class="clean-control">
        <div class="button-container">
            <button class="button" onClick="runCommandSwitch('clean');">CLEAN</button>
        </div>
        <div class="indicator-container">
            <div class="indicator" id="clean-indc"></div>
        </div>
    </div>
    <div class="map-container">
        <p>MAPPPP</p>
    </div>
    <div class="buttons">
        <button class="button" onClick="runCommand('forward');">FORWARD</button>
        <button class="button" onClick="runCommand('backward');">BACKWARD</button>
        <button class="button" onClick="runCommand('right');">RIGHT</button>
        <button class="button" onClick="runCommand('left');">LEFT</button>
        <button class="button" onClick="runCommand('stop');">STOP</button>
    </div>
    <div class="clean-control">
        <div class="button-container">
            <button class="button" onClick="runCommandSwitch('fan');" id="fan-indc" style="background-color: red;">FAN</button>
            <button class="button" onClick="runCommandSwitch('brush');" id="brush-indc" style="background-color: red;">BRUSH</button>
        </div>
    </div>
    <script>
        function runCommand(x) {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/" + x, true);
            xhr.send();
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


void setup(){
    Serial.begin(115200);
    Serial.swap(); // D7/D8
    transfer.begin(Serial);

    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
        { request->send_P(200, "text/html", index_html); });

    server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request)
        { txData.command = 'f';
        request->send_P(200, "text/html", "ok"); });

    server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request)
        { txData.command = 'b';
        request->send_P(200, "text/html", "ok"); });

    server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request)
        { txData.command = 'r';
        request->send_P(200, "text/html", "ok"); });

    server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request)
        { txData.command = 'l';
        request->send_P(200, "text/html", "ok"); });

    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request)
        { txData.command = 's';
        request->send_P(200, "text/html", "ok"); });

    server.on("/fan", HTTP_GET, [](AsyncWebServerRequest *request){
        if (txData.fan){
            txData.fan = false;
        } else{
            txData.fan = true;
        }

        request->send_P(200, "text/html", "ok");
    });

    server.on("/brush", HTTP_GET, [](AsyncWebServerRequest *request){
        if (txData.brush){
            txData.brush = false;
        } else{
            txData.brush = true;
        }

        request->send_P(200, "text/html", "ok");
    });

    server.on("/clean", HTTP_GET, [](AsyncWebServerRequest *request){
        if (txData.clean){
            txData.clean = false;
        } else{
            txData.clean = true;
        }
        request->send_P(200, "text/html", "ok");
    });

    server.begin();
}

void loop(){
    transfer.sendDatum(txData);

    delay(50); // коротка пауза

  // Приймаємо
if (transfer.available()) {
    transfer.rxObj(rxData);
    Serial.print("Прийнято: ");
    Serial.print(rxData.val1); Serial.print(", ");
    Serial.print(rxData.val2); Serial.print(", ");
    Serial.print(rxData.val3); Serial.print(", ");
    Serial.print(rxData.val4); Serial.print(", ");
    Serial.print(rxData.val5); Serial.print(", ");
    Serial.print(rxData.val6);
    Serial.print(" | Флаги: ");
    Serial.print(rxData.flag1); Serial.print(", ");
    Serial.println(rxData.flag2);
    Serial.println("=====");
}
}

