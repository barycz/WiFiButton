// Author: Tomas Barak <baryhoemail@gmail.com>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#include "credentials.h"
#include "LamaxAPI.h"

namespace pins
{
	int smallBlueLed = LED_BUILTIN;
	int bigBlueLed = LED_BUILTIN_AUX;
	int button = D1;

	uint8_t ledOn = LOW;
	uint8_t ledOff = HIGH;
}

unsigned long lastTrigger = 0;
unsigned long buttonTime = 0;
const unsigned long triggerInterval = 800;
bool wasConnectedToWifi = false;
bool isRecording = false;

// handlers must be in RAM
ICACHE_RAM_ATTR void onButtonPressed()
{
	buttonTime = millis();
}

void setup(void)
{
	// GPIO
	pinMode(pins::smallBlueLed, OUTPUT);
	pinMode(pins::bigBlueLed, OUTPUT);
	digitalWrite(pins::smallBlueLed, pins::ledOff);
	digitalWrite(pins::bigBlueLed, pins::ledOff);
	pinMode(pins::button, INPUT_PULLUP);
	attachInterrupt(pins::button, onButtonPressed, FALLING);

	// UART
	Serial.begin(115200);

	// WiFi
	WiFi.mode(WIFI_STA);
	WiFi.begin(credentials::ssid, credentials::password);
	Serial.println("Setup done.");
}

void onConnectedToWifi()
{
	digitalWrite(pins::smallBlueLed, pins::ledOn);

	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(WiFi.SSID());
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
}

void onDisconnectedFromWifi()
{
	digitalWrite(pins::smallBlueLed, pins::ledOff);
	Serial.println("Disconnected");
}

void loop(void)
{
	// return if not connected
	if (WiFi.status() != WL_CONNECTED)
	{
		if (wasConnectedToWifi)
		{
			onDisconnectedFromWifi();
		}

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
		if (isRecording)
		{
			Serial.println("stopping");
			triggerClient.begin(wifiClient, LamaxAPI::stopUrl);
		}
		else
		{
			Serial.println("starting");
			triggerClient.begin(wifiClient, LamaxAPI::startUrl);
		}

		int respCode = triggerClient.GET();
		if (respCode == 200)
		{
			isRecording = !isRecording;
			digitalWrite(pins::bigBlueLed, isRecording ? pins::ledOn : pins::ledOff);
		}

		Serial.println("http resp code:");
		Serial.println(respCode);
		triggerClient.end();
	}
}
