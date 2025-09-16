#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>


const char *ssid = "YOUR_SSID_HERE";
const char *password = "YOUR_PASSWORD_HERE";

AsyncWebServer server(80);
AsyncWebSocket myWebSocket("/ws");  // Use a different name for the AsyncWebSocket instance

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;
Servo servo6;

bool clientConnected = false;

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");
    clientConnected = true;
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket client disconnected");
    clientConnected = false;
  } else if (type == WS_EVT_DATA && clientConnected) {
    // Parse JSON data from the WebSocket message
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, (const char *)data);

    // Update servo positions
    servo1.write(doc["servo1"]);
    servo2.write(doc["servo2"]);
    servo3.write(doc["servo3"]);
    servo4.write(doc["servo4"]);
    servo5.write(doc["servo5"]);
    servo6.write(doc["servo6"]);
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Configure servos
  servo1.attach(2);
  servo2.attach(33);
  servo3.attach(4);
  servo4.attach(5);
  servo5.attach(32);
  servo6.attach(27);

  // Serve HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "<html><head><script>var socket = new WebSocket('ws://' + window.location.hostname + '/ws'); function updateServos() { var servo1 = document.getElementById('servo1').value; var servo2 = document.getElementById('servo2').value; var servo3 = document.getElementById('servo3').value; var servo4 = document.getElementById('servo4').value; var servo5 = document.getElementById('servo5').value; var servo6 = document.getElementById('servo6').value; var data = { servo1: parseInt(servo1), servo2: parseInt(servo2), servo3: parseInt(servo3), servo4: parseInt(servo4), servo5: parseInt(servo5), servo6: parseInt(servo6) }; socket.send(JSON.stringify(data)); } </script></head><body><h1>Servo Control</h1><label for='servo1'>Servo 1</label><input type='range' id='servo1' min='0' max='180' value='90' oninput='updateServos()'><br><label for='servo2'>Servo 2</label><input type='range' id='servo2' min='0' max='180' value='90' oninput='updateServos()'><br><label for='servo3'>Servo 3</label><input type='range' id='servo3' min='0' max='180' value='90' oninput='updateServos()'><br><label for='servo4'>Servo 4</label><input type='range' id='servo4' min='0' max='180' value='90' oninput='updateServos()'><br><label for='servo5'>Servo 5</label><input type='range' id='servo5' min='0' max='180' value='90' oninput='updateServos()'><br><label for='servo6'>Servo 6</label><input type='range' id='servo6' min='0' max='180' value='90' oninput='updateServos()'></body></html>");
  });

  // Start WebSocket server
  myWebSocket.onEvent(onWebSocketEvent);
  server.addHandler(&myWebSocket);

  // Start the server
  server.begin();
}

void loop() {
  // Your additional loop code, if any
}
