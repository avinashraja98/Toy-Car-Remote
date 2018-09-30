#include <WebSockets.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Servo.h>

#define pwm 5			// pwm pin
#define dir 4			// dir pin
#define steer_pwm 14		// pwm pin for the steering servo motor
#define button 0		// pin for the big red button

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 2);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const char index_page[] PROGMEM = R"rawliteral(
webpage
)rawliteral";

//const int pwm = 5;
//const int dir = 4;

Servo steer_servo;
//const int steer_pwm = 14;

int speed = 1023;

//const int button = 0;
int buttonEnabled = 1;
int buttonDir = 1;

boolean pressed = false;
boolean connected = false;
boolean drive = false;

void SetupAP();
void SetupDNS();
void SetupWebServer();
void SetupIO();
void SetupWebSocketServer();

void ServeWebPage();

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void buttonHandler();
void setButton(String msg);
void setButtonDir(String msg);
void drive(String msg);
void turn(String msg);
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
	
  if(drive && !connected) {
    stop_drive("forward");	  
  }
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
  steer_servo.attach(steer_pwm);
  pinMode(button, INPUT_PULLUP);
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
  {
    connected = false;
    Serial.printf("[%u] Disconnected!\r\n", num);
    break;
  }		  
  case WStype_CONNECTED:
  {
    connected = true;
    break;
  }
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
else if(msg_prefix=="slid_")
turn(temp.substring(5,temp.length()));
else if(msg_prefix=="togg_")
setButton(temp.substring(5,temp.length()));
else if(msg_prefix=="bDir_")
setButtonDir(temp.substring(5,temp.length()));
else if(msg_prefix=="sped_")
speed = atoi(temp.substring(5,temp.length())); //setSpeed
else
drive(temp);

break;
  }
  default:
    Serial.printf("Invalid WStype [%d]\r\n", type);
    break;
  }
}

void setSpeed(String msg){
	
}
	
void setButton(String msg){
  if(msg=="1")
  buttonEnabled=1;
  else if(msg=="0")
  buttonEnabled=0;
}

void setButtonDir(String msg){
  if(msg=="r")
  buttonDir=1;
  else if(msg=="l")
  buttonDir=0;
}

void buttonHandler(){
  if(digitalRead(button)==LOW && !pressed && buttonEnabled){
    pressed=true;
    if(buttonDir)
    drive("forward");
    else
    drive("reverse");
  }
  else if(digitalRead(button)==HIGH && pressed && buttonEnabled){
    pressed=false;
    if(buttonDir)
    stop_drive("forward");
    else
    stop_drive("reverse"); 
  }
}

void drive(String msg)
{
  if (msg=="forward")
  {
    analogWrite(pwm, speed);
    digitalWrite(dir, HIGH);
    drive = true;
  }else if (msg=="reverse"){
    analogWrite(pwm, speed);
    digitalWrite(dir, LOW);
    drive = true;
  }
}

void turn(String msg){
  steer_servo.write(msg.toInt());
}

void stop_drive(String msg)
{
  if (msg=="forward")
  {
    analogWrite(pwm, 0);
  }else if (msg=="reverse"){
    analogWrite(pwm, 0);
  }
  drive = false;
}
