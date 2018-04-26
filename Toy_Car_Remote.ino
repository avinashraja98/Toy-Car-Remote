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

const int pwm = 5;
const int dir = 4;

const int steer_pwm = 14;
const int steer_dir = 12;

const int button = 16;

boolean pressed = false;

void SetupAP();
void SetupDNS();
void SetupWebServer();
void SetupIO();
void SetupWebSocketServer();

void ServeWebPage();

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void buttonHandler();
void drive(String msg);
void stop_drive(String msg);

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
  pinMode(pwm, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(steer_pwm, OUTPUT);
  pinMode(steer_dir, OUTPUT);
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
  {
    Serial.printf("[%u] get Text: %s\r\n", num, payload);
    char *msg;
    
     Serial.println(strlen((const char*)payload));
    
      String temp = (char*)payload;
     String msg_prefix = temp.substring(0,5);
    
     Serial.println(msg_prefix);
    
    if(msg_prefix=="stop_")
    stop_drive(temp.substring(5,temp.length()));
    else
    drive(temp);
    
    break;
  }
  default:
    Serial.printf("Invalid WStype [%d]\r\n", type);
    break;
  }
}

void buttonHandler(){
  if(digitalRead(button)==LOW && !pressed){
    pressed=true;
    drive("forward");
  }
  else if(digitalRead(button)==HIGH && pressed){
    pressed=false;
    stop_drive("forward");
  }
}

void drive(String msg)
{
  if (msg=="forward")
  {
    analogWrite(pwm, 1023);
    digitalWrite(dir, HIGH);
  }else if (msg=="reverse")  {
    analogWrite(pwm, 1023);
    digitalWrite(dir, LOW);
  }else if (msg=="left")  {
    analogWrite(steer_pwm, 1023);
    digitalWrite(steer_dir, LOW);
  }else if (msg=="right")  {
    analogWrite(steer_pwm, 1023);
    digitalWrite(steer_dir, HIGH);
  }
}

void stop_drive(String msg){

  if (msg=="forward")
  {
    analogWrite(pwm, 0);
  }else if (msg=="reverse")  {
    analogWrite(pwm, 0);
  }else if (msg=="left")  {
    analogWrite(steer_pwm, 0);
  }else if (msg=="right")  {
    analogWrite(steer_pwm, 0);
  }
}
