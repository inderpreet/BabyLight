/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *******************************************************************************/

#define MQTTCLIENT_QOS2 1

#include <SPI.h>
#include <WiFi.h>
#include <WifiIPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>

typedef struct ip_v1{
    char SOF;
    char Length;
    char White2;
    char White1;
    char White0;
    char RedB2;      // Manual Red Value
    char RedB1;      // Manual Red Value
    char RedB0;      // Manual Red Value
    char GreenB2;    // Manual Green Value
    char GreenB1;    // Manual Green Value
    char GreenB0;    // Manual Green Value
    char BlueB2;     // Manual Blue Value
    char BlueB1;     // Manual Blue Value
    char BlueB0;     // Manual Blue Value
    char End;      // should be
} ip_v1;

ip_v1 myPacket;  //Create a global Object

unsigned char str2Byte(char Bit2, char Bit1, char Bit0){
      unsigned char temp;
      temp = Bit0 - 0x30;        // convert to number from char
      temp += 10 * (Bit1 - 0x30); // Tens place
      temp += 100* (Bit2 - 0x30); // Hunder's Place
      if(temp>255)
          temp=255;
      return temp;
}

// your network name also called SSID
char ssid[] = "Zangetsu";
// your network password
char password[] = "Shellysunny0987654321";

int WLED = 29;    // White LED Driver Connected to...

boolean myFlag = false;

char printbuf[100];

int arrivedcount = 0;

void messageArrived(MQTT::MessageData& md)
{
  unsigned char temp;
  MQTT::Message &message = md.message;
  // Copy to a local buffer.
  memcpy(&myPacket, message.payload, sizeof(myPacket));
  //Process the packet
  if(myPacket.SOF==':'){
    switch(myPacket.Length){
      case '1': //Just one byte of data
        temp = str2Byte(myPacket.White2, myPacket.White1, myPacket.White0);
        analogWrite(WLED, temp);
        break;
      // Add more cases here.
      default:
        break;
    }
  }
  
  sprintf(printbuf, "Message %d arrived: qos %d, retained %d, dup %d, packetid %d\n", 
		++arrivedcount, message.qos, message.retained, message.dup, message.id);
  Serial.print(printbuf);
  sprintf(printbuf, "Payload %s\n", (char*)message.payload);
  Serial.print(printbuf);
}


WifiIPStack ipstack;
MQTT::Client<WifiIPStack, Countdown> client = MQTT::Client<WifiIPStack, Countdown>(ipstack);

const char* topic = "ipv1/babylight/command";
const char* queryTopic = "ipv1/babylight/query";

void connect()
{
  char hostname[] = "iot.eclipse.org";
  int port = 1883;
  sprintf(printbuf, "Connecting to %s:%d\n", hostname, port);
  Serial.print(printbuf);
  int rc = ipstack.connect(hostname, port);
  if (rc != 1)
  {
    sprintf(printbuf, "rc from TCP connect is %d\n", rc);
    Serial.print(printbuf);
  }
 
  Serial.println("MQTT connecting");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"energia-cc3200";
  rc = client.connect(data);
  if (rc != 0)
  {
    sprintf(printbuf, "rc from MQTT connect is %d\n", rc);
    Serial.print(printbuf);
  }
  Serial.println("MQTT connected");
  
  rc = client.subscribe(topic, MQTT::QOS2, messageArrived);   
  if (rc != 0)
  {
    sprintf(printbuf, "rc from MQTT subscribe is %d\n", rc);
    Serial.print(printbuf);
  }
  Serial.println("MQTT subscribed");
}

void setup()
{
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  //Turn of the LEDs at reset.
  analogWrite(WLED, 0);
  Serial.begin(115200);
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nIP Address obtained");
  // We are connected and have an IP address.
  Serial.println(WiFi.localIP());
  
  Serial.println("MQTT Baby Light");
  connect();
}

void loop()
{
  if (!client.isConnected())
    connect();
  
  MQTT::Message message;
  
  if(arrivedcount>0){  // i.e. A message has been received...
    arrivedcount=0;
  }
  
  //arrivedcount = 0;

  // Send and receive QoS 0 message
//  char buf[100];
//  sprintf(buf, "Hello World! QoS 0 message");
//  Serial.println(buf);
//  message.qos = MQTT::QOS0;
//  message.retained = false;
//  message.dup = false;
//  message.payload = (void*)buf;
//  message.payloadlen = strlen(buf)+1;
//  int rc = client.publish(topic, message);
  while (arrivedcount == 0)
    client.yield(1000);
        
  // Send and receive QoS 1 message
//  sprintf(buf, "Hello World!  QoS 1 message");
//  Serial.println(buf);
//  message.qos = MQTT::QOS1;
//  message.payloadlen = strlen(buf)+1;
//  rc = client.publish(topic, message);
//  while (arrivedcount == 1)
//    client.yield(1000);
//        
//  // Send and receive QoS 2 message
//  sprintf(buf, "Hello World!  QoS 2 message");
//  Serial.println(buf);
//  message.qos = MQTT::QOS2;
//  message.payloadlen = strlen(buf)+1;
//  rc = client.publish(topic, message);
//  while (arrivedcount == 2)
//    client.yield(1000);
  
  delay(2000);
}
