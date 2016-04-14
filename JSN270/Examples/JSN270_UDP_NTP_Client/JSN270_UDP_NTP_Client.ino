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

#define HOST_IP        "211,233,84,186"	// 2.kr.pool.ntp.org
#define REMOTE_PORT    123
#define PROTOCOL       "UDP"

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

SoftwareSerial mySerial(3, 2); // RX, TX
 
JSN270 JSN270(&mySerial);

void setup() {
	char c;
	String hostname;
	char hostip[32];
	
	mySerial.begin(9600);
	Serial.begin(9600);

	Serial.println("--------- JSN270 NTP Client --------");

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

	JSN270.sendCommand("at+nslookup=2.kr.pool.ntp.org\r");
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
	
	Serial.print("NTP Server IP is ");
	Serial.println(hostname);
	hostname.toCharArray(hostip, hostname.length()+1);	// convert string to char array

	if (!JSN270.client(hostip, REMOTE_PORT, PROTOCOL)) {
		Serial.print("Failed connect to ");
		Serial.println(hostip);
		Serial.println("Restart System");
	} else {
		Serial.print("Socket connect to ");
		Serial.println(hostip);
		delay(2000);
	}
}

void loop() {
	char c;
	
	// Enter data mode
	JSN270.sendCommand("at+exit\r");

	// consume [OK] message
	while (JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
	}
        
	sendNTPpacket(); // send an NTP packet to a time server
	// wait to see if a reply is available

	if (mySerial.overflow()) {
		Serial.println("SoftwareSerial overflow!");
	}

	if (JSN270.receive((uint8_t *)&packetBuffer, NTP_PACKET_SIZE, 1000) > 0) {
		//the timestamp starts at byte 40 of the received packet and is four bytes,
		// or two words, long. First, esxtract the two words:

		unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
		unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
		// combine the four bytes (two words) into a long integer
		// this is NTP time (seconds since Jan 1 1900):
		unsigned long secsSince1900 = highWord << 16 | lowWord;

		if (secsSince1900) {
//			Serial.print("Seconds since Jan 1 1900 = " );
//			Serial.println(secsSince1900);

			// now convert NTP time into everyday time:
//			Serial.print("Unix time = ");
			// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
			const unsigned long seventyYears = 2208988800UL;
			// subtract seventy years:
			unsigned long epoch = secsSince1900 - seventyYears;
			// print Unix time:
//			Serial.println(epoch);

			epoch = epoch + (3600 * 9);    // Korean time is UTC + 9
			// print the hour, minute and second:
			Serial.print("The UTC+9 time is ");       // UTC is the time at Greenwich Meridian (GMT)
			Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
			Serial.print(':');
			if ( ((epoch % 3600) / 60) < 10 ) {
				// In the first 10 minutes of each hour, we'll want a leading '0'
				Serial.print('0');
			}
			Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
			Serial.print(':');
			if ( (epoch % 60) < 10 ) {
				// In the first 10 seconds of each minute, we'll want a leading '0'
				Serial.print('0');
			}
			Serial.println(epoch % 60); // print the second        
		}
	}

	// Enter command mode:
	JSN270.print("+++");
	// wait ten seconds before asking for the time again
	delay(9000);
}

// send an NTP request to the time server at the given address
void sendNTPpacket()
{
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)

	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	JSN270.send((uint8_t*)packetBuffer, NTP_PACKET_SIZE);
}
