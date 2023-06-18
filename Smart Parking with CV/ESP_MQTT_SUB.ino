#include <WiFi.h>
#include "PubSubClient.h"
#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN    5  // ESP32 pin GIOP5 
#define RST_PIN   4 // ESP32 pin GIOP27 
#define SERVO_PIN 22 // ESP32 pin GIOP22 connects to relay

const char* ssid = "Mekanik";
const char* password = "duarmekanik";
const char* mqttServer = "mqtt.eclipseprojects.io";
bool AwalRFID = false;
int port = 1883;
int pinled = 2;
char clientId[50];

Servo servoMotor;
MFRC522 rfid(SS_PIN, RST_PIN);

byte keyTagUID[4] = {0x51, 0x77, 0x35, 0x26};

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(pinled, OUTPUT);
  delay(10);

  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  wifiConnect();
  Serial.println("");
  Serial.println("Wifi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, port);
  client.setCallback(callback);

  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
  servoMotor.attach(SERVO_PIN);

}

void wifiConnect(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(200);
    Serial.print(".");
  }
}

void mqttReconnect() {
 while (!client.connected()) {
   Serial.print("Attempting MQTT connection...");
   long r = random(1000);
   sprintf(clientId, "clientId-%ld", r);
   if (client.connect(clientId)) {
     Serial.print(clientId);
     Serial.println(" connected");
     client.subscribe("space_count"); //Subscribe the topic
   } else {
     Serial.print("failed, rc=");
     Serial.print(client.state());
     Serial.println(" try again in 5 seconds");
     delay(5000);
   }
 }
}
void RFID(){
  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

      if (rfid.uid.uidByte[0] == keyTagUID[0] &&
          rfid.uid.uidByte[1] == keyTagUID[1] &&
          rfid.uid.uidByte[2] == keyTagUID[2] &&
          rfid.uid.uidByte[3] == keyTagUID[3] ) {
        Serial.println("Access is granted");
        AwalRFID = true;
        delay(1200);
      }
      else
      {
        Serial.print("Access denied, UID:");
        AwalRFID = false;
        for (int i = 0; i < rfid.uid.size; i++) {
          Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(rfid.uid.uidByte[i], HEX);
        }
        Serial.println();
        
      }

      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
   } //else{
  //   AwalRFID = false;
  // }
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received message: ");
  int i = 0;
  byte a = 0;
  String messageTemp;
  for (i; i < length; i++) {
    a = payload[i];
    Serial.print((char)a);
    messageTemp += (char)a;
  }
  //Serial.print((char)a);
 
  Serial.println();

   if (messageTemp > "14" && AwalRFID == true){
      servoMotor.write(90); // unlock the door for 2 seconds
      delay(1000);
      servoMotor.write(0); // lock the door
      delay(1000);
      servoMotor.write(90); // lock the door
      Serial.print("++++++++++++++++++++++++++++++++++++++++++++++++++++++");
      delay(500);
      digitalWrite(pinled, HIGH);
      delay(500);
      digitalWrite(pinled, LOW);
  }
  else if (messageTemp > "14" && AwalRFID == false){
    
  }
  else{
    Serial.print("Parking Space is Full..");
  }
}

void loop() {
RFID();
 delay(10);
 if (!client.connected()) {
   mqttReconnect();
 }
 client.loop();
}
