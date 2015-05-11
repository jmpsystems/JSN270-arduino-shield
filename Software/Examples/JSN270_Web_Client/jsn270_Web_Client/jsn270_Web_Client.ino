#include <Debug.h>
#include <JSN270.h>
#include <Arduino.h>
#include <SoftwareSerial.h>

#define SSID      "PJW_2G"
#define KEY       "12345678"
#define AUTH       "WPA2" 

#define USE_DHCP_IP 1

#if !USE_DHCP_IP
#define MY_IP          "192.168.1.133"
#define SUBNET         "255.255.255.0"
#define GATEWAY        "192.168.1.254"
#endif

#define HOST_IP        "74,125,232,128"	// www.google.com
#define REMOTE_PORT    80
#define PROTOCOL       "TCP"

SoftwareSerial mySerial(3, 2); // RX, TX
 
JSN270 JSN270(&mySerial);

void setup() {
	char c;

	mySerial.begin(9600);
	Serial.begin(9600);

	Serial.println("--------- JSN270 Web Client Test --------");

	// wait for initilization of JSN270
	delay(1000);
	//JSN270.reset();
	delay(1000);

	//JSN270.prompt();
	JSN270.sendCommand("at+ver\r");
	delay(5);
	while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
		Serial.print((char)c);
	}
	delay(1000);

#if USE_DHCP_IP
	JSN270.dynamicIP();
#else
	JSN270.staticIP(MY_IP, SUBNET, GATEWAY);
#endif    
    
	if (JSN270.join(SSID, KEY, AUTH)) {
		Serial.println("WiFi connect to " SSID);
	}
	else {
		Serial.println("Failed WiFi connect to " SSID);
		Serial.println("Restart System");

		return;
	}
    
	delay(1000);

	JSN270.sendCommand("at+wstat\r");
	delay(5);
	while(JSN270.receive((uint8_t *)&c, 1, 100) > 0) {
		Serial.print((char)c);
	}

	delay(1000);        

	JSN270.sendCommand("at+nstat\r");
	delay(5);
	while(JSN270.receive((uint8_t *)&c, 1, 100) > 0) {
		Serial.print((char)c);
	}

	delay(1000);

	if (!JSN270.client(HOST_IP, REMOTE_PORT, PROTOCOL)) {
		Serial.println("Failed connect to " HOST_IP);
		Serial.println("Restart System");
	} else {
		Serial.println("Socket connect to " HOST_IP);
		//delay(2000);
		// Enter data mode
		JSN270.sendCommand("at+exit\r");
		delay(100);

		Serial.println("connected to server");
		// Make a HTTP request:
		JSN270.println("GET /search?q=JSN270 HTTP/1.1");
		JSN270.println("Host: www.google.com");
		JSN270.println("Connection: close");
		JSN270.println();
	}
}

void loop() {
	if(JSN270.available()) {
		Serial.print((char)JSN270.read());
	}
}