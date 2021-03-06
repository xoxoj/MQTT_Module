/*
 * MQTT_Module.cpp
 *
 * Created: 11.08.2014 15:25:45
 *  Author: gsteindl
 */ 

#define F_CPU 16000000

#include <Arduino.h>
#include <stdlib.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "TempSensor.h"
#include "ADConverter.h"
#include "Config.h"
#include "WifiCommunicator.h"
#include "MQTTHandler.h"
#include "MemoryFree.h"

//prototypes
void setup();
void loop();

//Globals
extern config_t globalConfig;

char ssid[] = "ArduinoNet";     //  your network SSID (name)
char pass[] = "ArduinoNet";  // your network password


void setup() {

	Serial.begin(9600);
	
	Serial.print(F("Free memory1: "));
	Serial.println(freeMemory());
	delay(200);
	
	adcInit();
	configInit();
	
	tempSenosrsInit(globalConfig.resolution);
	wifiComInit(globalConfig.ssid, globalConfig.pass);
	mqttInit();

	Serial.println(F("Initialization done"));
	
}
void loop() {
	unsigned long now;
	static unsigned long lastSendTimeTemp, lastSendTimeAdc;
	
	if(checkConnectionAndReconnectMQTT()){
		//error: we should restart 
		Serial.println(F("Stop and restart MQTT... "));
		mqttStop();
		delay(1000);
		mqttInit();
		//check again
		if(checkConnectionAndReconnectMQTT()){
			Serial.println(F("FAILED!"));
			delay(1000);
			return;
		}
		else{
			Serial.println(F("done!"));		
		}
	}
	
	now=millis();
	
	//temp
	if((now-lastSendTimeTemp) >= globalConfig.sendIntervalTemp){	//the overflow of millis() is not harming the calculation

		tempSensorRead();
		tempSensorPrintTable();
		mqttSendTemp();
		Serial.println("------------");
		lastSendTimeTemp=now;
	}
	//adc
	if((now-lastSendTimeAdc) >= globalConfig.sendIntervalAdc){	//the overflow of millis() is not harming the calculation

		adcRead();
		adcPrintTable();
		mqttSendADC();
		Serial.println("------------");
		lastSendTimeAdc=now;
	}
	
	
	mqttLoopFunction();	//MQTT client loop processing for keep alive messages
}
