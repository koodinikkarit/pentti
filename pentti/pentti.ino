/*
 Name:		pentti.ino
 Created:	12/2/2016 4:58:11 PM
 Author:	jaska
*/

#include "EnableInterrupt.h"
#include <EtherCard.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

unsigned char buttonPins[12] = {
  47,45,43,41,39,37,35,33,31,22,24,26
};

bool buttonStates[12] = {
  0,0,0,0,0,0,0,0,0,0,0,0
};

const char website[] PROGMEM = "www.google.com";

byte Ethernet::buffer[700];
static uint32_t timer;



// called when the client request is complete
static void my_callback(byte status, word off, word len) {
	Serial.println(">>>");
	Ethernet::buffer[off + 300] = 0;
	Serial.print((const char*)Ethernet::buffer + off);
	Serial.println("...");
}

void setup() {
	Serial.begin(9600);

	if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
		Serial.println(F("Failed to access Ethernet controller"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));
    
	byte hisip[] = { 10, 70, 0, 50 };
	ether.copyIp(ether.hisip, hisip);
	ether.hisport = 3111;
  for (int i = 0; i < 12; i++) {
    Serial.println(buttonPins[i]);
    pinMode(buttonPins[i], INPUT);
    
  }

}

void buttonStateChanged(int pin, bool state) {
  ether.packetLoop(ether.packetReceive());
  String query = String("?pin=") + pin + String("&state=") + state;
  char varPart[20];
  query.toCharArray(varPart, 20);
  Serial.println(query);
  ether.browseUrl(PSTR("/"), varPart, website, my_callback);
}

void loop() {
  for (int i = 0; i < 12; i++) {
    bool newState = digitalRead(buttonPins[i]);
    if (buttonStates[i] != newState) {
      buttonStateChanged(i+1, newState);
      buttonStates[i] = newState;
    }
  }
  ether.packetLoop(ether.packetReceive());
}
