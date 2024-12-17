#include <Wire.h>
#include <MFRC522.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define SS_PIN 4
#define RST_PIN 16

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

Servo s1;

const char* ssid = "bijeli";
const char* password = "ferit.2023";
const char* mqttServer = "192.168.23.106"; 
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

int alcoholValue;
byte nuidPICC[4];
unsigned long lastReadTime = 0;
const unsigned long readInterval = 3000; 

void setup() {
    Serial.begin(19200); 
    s1.attach(5);
    rfid.PCD_Init();
    setupWifi();
    client.setServer(mqttServer, mqttPort);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }

    unsigned long currentMillis = millis();

    if (currentMillis - lastReadTime >= readInterval) {
        if (!rfid.PICC_IsNewCardPresent()) {
            return;
        }

        if (!rfid.PICC_ReadCardSerial()) {
            return;
        }

        MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

        if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
            piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
            piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
            return;
        }

        if (rfid.uid.uidByte[0] != nuidPICC[0] ||
            rfid.uid.uidByte[1] != nuidPICC[1] ||
            rfid.uid.uidByte[2] != nuidPICC[2] ||
            rfid.uid.uidByte[3] != nuidPICC[3]) {
            for (byte i = 0; i < 4; i++) {
                nuidPICC[i] = rfid.uid.uidByte[i];
            }

            rotateServo();

            if (Serial.available() > 0) {
                String message = Serial.readStringUntil('\n');
                Serial.print("Received message: ");
                Serial.println(message);

                client.publish("sensors/sensor1", message.c_str());
            }

            lastReadTime = currentMillis;
        }

        rfid.PICC_HaltA();

        rfid.PCD_StopCrypto1();
    }
    delay(5000); 
}

void rotateServo() {
    s1.write(90);
    delay(2000); 
    s1.write(0); 
    delay(2000); 
}

void setupWifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");
        if (client.connect("ESP8266Client")) {
            Serial.println("Connected to MQTT broker");
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}
