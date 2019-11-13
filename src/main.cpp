// Author: Tomas Barak <baryhoemail@gmail.com>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#include "credentials.h"

namespace pins
{
	int redLed = 0;
	int greenLed = 12;
	int button = 14;

	uint8_t ledOn = LOW;
	uint8_t ledOff = HIGH;
}

unsigned long lastTrigger = 0;
unsigned long buttonTime = 0;
const unsigned long triggerInterval = 1000;
bool wasConnectedToWifi = false;

// handlers must be in RAM
ICACHE_RAM_ATTR void onButtonReleased()
{
	buttonTime = millis();
}

void setup(void)
{
	// GPIO
	pinMode(pins::redLed, OUTPUT);
	pinMode(pins::greenLed, OUTPUT);
	digitalWrite(pins::redLed, pins::ledOff);
	digitalWrite(pins::greenLed, pins::ledOff);
	pinMode(pins::button, INPUT_PULLUP);
	attachInterrupt(pins::button, onButtonReleased, RISING);

	// UART
	Serial.begin(115200);

	// WiFi
	WiFi.mode(WIFI_STA);
	WiFi.begin(credentials::ssid, credentials::password);
	Serial.println("Setup done.");
}

void onConnectedToWifi()
{
	digitalWrite(pins::greenLed, pins::ledOn);

	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(WiFi.SSID());
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
}

void loop(void)
{
	// return if not connected
	if (WiFi.status() != WL_CONNECTED)
	{
		wasConnectedToWifi = false;
		return;
	}

	// we are connected to wifi
	if (wasConnectedToWifi == false)
	{
		wasConnectedToWifi = true;
		onConnectedToWifi();
	}

	if(buttonTime - lastTrigger >= triggerInterval)
	{
		lastTrigger = buttonTime;
		WiFiClient wifiClient;
		HTTPClient triggerClient;
		triggerClient.begin(wifiClient, credentials::webhooksUrlWithKey);
		int respCode = triggerClient.GET();
		Serial.println("http resp code:");
		Serial.println(respCode);
		triggerClient.end();
	}
}
