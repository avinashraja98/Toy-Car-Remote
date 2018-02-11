#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>  
#include <ESP8266WebServer.h>

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 2);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

const char index_page[] PROGMEM = R"=====(
webpage
)=====";

const int inp1 = 5;
const int inp2 = 4;

void SetupAP();
void SetupDNS();
void SetupWebServer();
void SetupIO();

void ServeWebPage();

void setup()
{
	Serial.begin(115200);	

	SetupAP();
	SetupDNS();
	SetupWebServer();
	SetupIO();
	
}

void loop() {
	server.handleClient();
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
	pinMode(inp1, OUTPUT);
	pinMode(inp2, OUTPUT);
}
