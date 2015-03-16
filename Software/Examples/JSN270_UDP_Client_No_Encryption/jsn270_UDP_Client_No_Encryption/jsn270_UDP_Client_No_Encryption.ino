#include <Debug.h>
#include <JSN270.h>
#include <Arduino.h>
#include <SoftwareSerial.h>

#define SSID      "PJW_2G"
#define KEY       "12345678"
#define AUTH       "NONE" 

#define USE_DHCP_IP 1

#if !USE_DHCP_IP
#define MY_IP          "192.168.1.133"
#define SUBNET         "255.255.255.0"
#define GATEWAY        "192.168.1.254"
#endif

#define HOST_IP        "192.168.0.7"
#define REMOTE_PORT    1234
#define PROTOCOL       "UDP"

SoftwareSerial mySerial(3, 2); // RX, TX
 
JSN270 JSN270(&mySerial);

void setup() {
    char c;
        
    mySerial.begin(9600);
    Serial.begin(9600);

    Serial.println("--------- JSN270 UDP Client with No Encryption Test --------");
    
    // wait for initilization of JSN270
    delay(1000);
//    JSN270.reset();
    delay(1000);

    JSN270.prompt();

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
        delay(2000);
        JSN270.sendCommand("at+exit\r");
        delay(5);
//        Serial.println("Enter Data mode successfully. You can send or receive data with host server");
    }
}

void loop() {
  if(JSN270.available()) {
    Serial.print((char)JSN270.read());
  }
   if(Serial.available()) {
    JSN270.print((char)Serial.read());
  }
  
}
