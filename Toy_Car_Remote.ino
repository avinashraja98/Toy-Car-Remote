#include <WebSockets.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>  
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 2);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const char index_page[] PROGMEM = R"rawliteral(
webpage
)rawliteral";

const int drive = 5;
const int button = 4;

boolean pressed = false;

void SetupAP();
void SetupDNS();
void SetupWebServer();
void SetupIO();
void SetupWebSocketServer();

void ServeWebPage();

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void buttonHandler();
void drive(const char * msg);

void setup()
{
  Serial.begin(115200);

  SetupAP();
  SetupDNS();
  SetupWebServer();
  SetupWebSocketServer();
  SetupIO();

}

void loop() {
  server.handleClient();
  webSocket.loop();
  buttonHandler();
}

void ServeWebPage() {
  String s = index_page;
  server.send(200, "text/html", s);
}

void SetupAP()
{
  Serial.print("Setting configuration... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting Access Point... ");
  Serial.println(WiFi.softAP("Car") ? "Ready" : "Failed!");

  Serial.print("IP address = ");
  Serial.println(WiFi.softAPIP());
}

void SetupDNS()
{
  if (!MDNS.begin("car")) {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
}

void SetupWebServer()
{
  server.on("/", ServeWebPage);
  server.begin();
  Serial.println("HTTP server started");
}

void SetupIO()
{
  pinMode(drive, OUTPUT);
  pinMode(button, INPUT);
  Serial.println("IO Pins setup");
}

void SetupWebSocketServer()
{
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch (type) {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\r\n", num);
    break;
  case WStype_CONNECTED:
    break;
  case WStype_TEXT:
    Serial.printf("[%u] get Text: %s\r\n", num, payload);
    drive((const char *)payload);
    break;
  default:
    Serial.printf("Invalid WStype [%d]\r\n", type);
    break;
  }
}

void buttonHandler(){
  if(digitalRead(button)==HIGH){
    pressed=true;
    drive("drive");
  }
  else if(digitalRead(button)==LOW && pressed){
    pressed=false;
    drive("stop");
  }
}

void drive(const char * msg)
{
  if (strcmp(msg, "drive") == 0)
  {
    digitalWrite(drive, HIGH);
  }
  else if (strcmp(msg, "stop") == 0)
  {
    digitalWrite(drive, LOW);
  }
}
