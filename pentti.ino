
#include <EtherCard.h>
#include "CmdConst.h"
#include "states.h"

uint16_t pekkaPort = 3111;
byte pekkaIp[] = {10, 70, 0, 50};
byte mac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

unsigned char buttonPins[12] = {
  28,26,24,22,36,34,32,30,44,42,40,38
};

bool buttonStates[12] = {
  0,0,0,0,0,0,0,0,0,0,0,0
};

void buttonStateChanged(int pin, bool state);
void serveSerial();
void sendMacToSerial();
void sendPekkaPortToSerial();
void sendPekkaIpToSerial();
void sendAllConfigToSerial();

byte Ethernet::buffer[700];

void setup()
{
    Serial.begin(9600);
   for (int i = 0; i < 12; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    
  }
          if (ether.begin(sizeof Ethernet::buffer, mac) == 0)
          Serial.println(F("Failed to access Ethernet controller"));
        if (!ether.dhcpSetup())
          Serial.println(F("DHCP failed"));
          ether.copyIp(ether.hisip, pekkaIp);
          ether.hisport = pekkaPort;
}

int incomingByte = 0;

void loop()
{
    static int state = RECEIVE_COMMAND_FROM_SERIAL;
    switch (state)
    {
    case RECEIVE_COMMAND_FROM_SERIAL:
		serveSerial();
    state = RECEIVE_BUTTON_PRESSED;
		break;
	case RECEIVE_BUTTON_PRESSED:
    for (int i = 0; i < 12; i++) {
      bool newState = digitalRead(buttonPins[i]);
      if (buttonStates[i] != newState) {
        buttonStateChanged(i+1, newState);
        buttonStates[i] = newState;
      }
    }
    state = RECEIVE_COMMAND_FROM_SERIAL;
		break;
    }
    ether.packetLoop(ether.packetReceive());
}

static void my_callback(byte status, word off, word len) {
  //Serial.println(">>>");
  Ethernet::buffer[off + 300] = 0;
  /*Serial.print((const char*)Ethernet::buffer + off);
  Serial.println("...");*/
}

void buttonStateChanged(int pin, bool state) {
  ether.packetLoop(ether.packetReceive());
  String query = String("?pin=") + pin + String("&state=") + state;
  char varPart[20];
  query.toCharArray(varPart, 20);
  //Serial.println(query);
  ether.browseUrl(PSTR("/"), varPart, "", my_callback);
}

void serveSerial()
{
    if (Serial.available())
    {
	static byte command = 0;
	byte readByte = Serial.read();
	if (command == 0)
	    command = readByte;
	switch (command)
	{
	case REPORT_ALL_CONFIG:
	    sendAllConfigToSerial();
	    command = 0;
	    break;
	case SET_PEKKA_IP:
	{
	    static int ipIndex = 0;
	    static bool start = false;
	    if (start)
	    {
		pekkaIp[ipIndex] = readByte;
		ipIndex++;
		if (ipIndex > 3)
		{
		    command = 0;
		    ipIndex = 0;
		    start = false;
		    sendPekkaIpToSerial();
        ether.copyIp(ether.hisip, pekkaIp);
		}
	    }
	    else
	    {
		start = true;
	    }
	}
	break;
	case SET_PEKKA_PORT:
	{
	    static int portIndex = 0;
	    static bool start = false;
	    static uint16_t port = 0;
	    if (start)
	    {
		if (portIndex == 0)
		{
		    port |= (readByte << 8);
		    portIndex++;
		}
		else if (portIndex == 1)
		{
		    port |= readByte;
		    start = false;
		    pekkaPort = port;
		    port = 0;
		    portIndex = 0;
		    command = 0;
		    sendPekkaPortToSerial();
         ether.hisport = pekkaPort;
		}
	    }
	    else
	    {
		start = true;
	    }
	}
	break;
	case SET_MAC:
	{
	    static int macIndex = 0;
	    static bool start = false;
	    static uint16_t port = 0;
	    if (start)
	    {
		mac[macIndex] = readByte;
		macIndex++;
		if (macIndex > 5)
		{
		    command = 0;
		    macIndex = 0;
		    start = false;
		    sendMacToSerial();
        if (ether.begin(sizeof Ethernet::buffer, mac) == 0)
          Serial.println(F("Failed to access Ethernet controller"));
        if (!ether.dhcpSetup())
          Serial.println(F("DHCP failed"));
		}
	    }
	    else
	    {
		start = false;
	    }
	}
	break;
	case REPORT_PEKKA_IP:
	    sendPekkaIpToSerial();
      /*
      Serial.println(pekkaIp[0], DEC);
      Serial.println(pekkaIp[1], DEC);
      Serial.println(pekkaIp[2], DEC);
      Serial.println(pekkaIp[3], DEC);*/
	    command = 0;
	    break;
	case REPORT_PEKKA_PORT:
	    sendPekkaPortToSerial();
      //Serial.println(pekkaPort, DEC);
	    command = 0;
	    break;
	case REPORT_MAC:
	    sendMacToSerial();
      /*
      Serial.println(mac[0], DEC);
      Serial.println(mac[1], DEC);
      Serial.println(mac[2], DEC);
      Serial.println(mac[3], DEC);
      Serial.println(mac[4], DEC);
      Serial.println(mac[5], DEC);*/
	    command = 0;
	    break;
	default:
	    command = 0;
	    break;
	}
    }
}

void sendMacToSerial()
{
    byte message[] = {
	MAC,
	mac[0],
	mac[1],
	mac[2],
	mac[3],
	mac[4],
	mac[5]};
    Serial.write(message, 7);
}

void sendPekkaIpToSerial()
{
    byte message[] = {
	PEKKA_IP,
	pekkaIp[0],
	pekkaIp[1],
	pekkaIp[2],
	pekkaIp[3],
    };
    Serial.write(message, 5);
}

void sendPekkaPortToSerial()
{
    byte pekkaPortPart1 = (byte)(pekkaPort >> 8);
    byte pekkaPortPart2 = (byte)(pekkaPort & 0xff);
    byte message[] = {
	PEKKA_PORT,
	pekkaPortPart1,
	pekkaPortPart2,
    };
    Serial.write(message, 3);
}

void sendAllConfigToSerial()
{
    byte pekkaPortPart1 = (byte)(pekkaPort >> 8);
    byte pekkaPortPart2 = (byte)(pekkaPort & 0xff);
    byte message[] = {
	PEKKA_IP,
	pekkaIp[0],
	pekkaIp[1],
	pekkaIp[2],
	pekkaIp[3],
	PEKKA_PORT,
	pekkaPortPart1,
	pekkaPortPart2,
	MAC,
	mac[0],
	mac[1],
	mac[2],
	mac[3],
	mac[4],
	mac[5],
    };
    Serial.write(message, 15);
}
