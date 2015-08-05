#include <Debug.h>
#include <JSN270.h>
#include <Arduino.h>
#include <SoftwareSerial.h>

#define SSID      "JSN270_2G"		// your wifi network SSID
#define KEY       "12345678"		// your wifi network password
#define AUTH       "WPA2" 		// your wifi network security (NONE, WEP, WPA, WPA2)

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
	String hostname;
	char hostip[32];

	mySerial.begin(9600);
	Serial.begin(9600);

	Serial.println("--------- JSN270 Web Client Test --------");

	// wait for initilization of JSN270
	delay(5000);
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
	while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
		Serial.print((char)c);
	}
	delay(1000);        

	JSN270.sendCommand("at+nstat\r");
	delay(5);
	while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
		Serial.print((char)c);
	}
	delay(1000);

	JSN270.sendCommand("at+nslookup=www.google.com\r");
	delay(5);
	while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
		if (c == '[') {
			break; 
		}
		else if ((c != '\r') && (c != '\n')) {
			if (c >= 32) {	// discard non-printable character
				hostname += c;
			}
		}
	}
	delay(1000);
	
	Serial.print("Host IP is ");
	Serial.println(hostname);
	hostname.toCharArray(hostip, hostname.length()+1);	// convert string to char array

	if (!JSN270.client(hostip, REMOTE_PORT, PROTOCOL)) {
		Serial.print("Failed connect to ");
		Serial.println(hostip);
		Serial.println("Restart System");
	} else {
		Serial.print("Socket connect to ");
		Serial.println(hostip);
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
