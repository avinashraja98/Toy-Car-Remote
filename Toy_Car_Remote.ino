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

void ServeWebPage();

void setup()
{
	Serial.begin(115200);
	Serial.println();

	Serial.print("Setting configuration... ");
	Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

	Serial.print("Setting Access Point... ");
	Serial.println(WiFi.softAP("Car") ? "Ready" : "Failed!");

	Serial.print("IP address = ");
	Serial.println(WiFi.softAPIP());

	if (!MDNS.begin("car")) {
		Serial.println("Error setting up MDNS responder!");
	}
	Serial.println("mDNS responder started");

	server.on("/", ServeWebPage);
	server.begin();
	Serial.println("HTTP server started");

}

void loop() {
	server.handleClient();
}

void ServeWebPage() {
	String s = index_page;
	server.send(200, "text/html", s);
}