#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>

// Replace with your network credentials
const char *ssid = "Valtion tiedusteluauto";
const char *password = "0LEDT19BYF3E3";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create a WebSocket object

AsyncWebSocket ws("/ws");

String message = "";
String sliderValue1 = "0";

int dutyCycle1;

// setting PWM properties
const int freq = 5000;
const int ledChannel1 = 0;

const int resolution = 8;

// Json Variable to Hold Slider Values
JSONVar sliderValues;
// DynamicJsonDocument sliderValues(512);

// Get Slider Values
String getSliderValues()
{
  sliderValues["sliderValue1"] = String(sliderValue1);

  // sliderValues.toStyledString();
  // String jsonString;
  // serializeJson(sliderValues, jsonString);

  // String jsonString = '{ "sliderValue1": ' + String(sliderValue1) + ' }'; //JSON.stringify(sliderValues);
  String jsonString = JSON.stringify(sliderValues);
  return jsonString;
}

// Initialize SPIFFS
void initFS()
{
  if (!SPIFFS.begin())
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  else
  {
    Serial.println("SPIFFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int limitcounter = 0;
  Serial.print("Connecting to WiFi ..");
  while (((WiFi.status() != WL_CONNECTED)) && (limitcounter < 30))
  {
    Serial.print('.');
    limitcounter++;
    delay(1000);
  }
  Serial.print("Connected! Go to: ");
  Serial.println(WiFi.localIP());
}

void notifyClients(String sliderValues)
{
  ws.textAll(sliderValues);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    message = (char *)data;
    if (message.indexOf("1s") >= 0)
    {
      sliderValue1 = message.substring(2);
      dutyCycle1 = map(sliderValue1.toInt(), 0, 100, 0, 255);
      Serial.println(dutyCycle1);
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }
    if (strcmp((char *)data, "getValues") == 0)
    {
      notifyClients(getSliderValues());
    }
  }
}
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT); // set the digital pin as output
  initFS();
  initWiFi();

  // configure LED PWM functionalitites
  // ledcSetup(ledChannel1, freq, resolution);
  ledcSetup(ledChannel1, freq, resolution);

  // attach the channel to the GPIO to be controlled
  // ledcAttachPin(LED_BUILTIN, ledChannel1);
  ledcAttachPin(LED_BUILTIN, ledChannel1);

  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  server.serveStatic("/", SPIFFS, "/");

  // Start server
  server.begin();
}

void loop()
{
  ledcWrite(ledChannel1, dutyCycle1);
  // analogWrite(LED_BUILTIN, dutyCycle1);

  ws.cleanupClients();
}