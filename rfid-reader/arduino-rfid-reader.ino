#include <SPI.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h> // See https://pubsubclient.knolleary.net/

//The following libraries must be installed into your arduino libraries folder
#include <MFRC522.h>     //from the library manager, search for frc and install MFRC522 By GithubCommunity Version 1.4.3
#include <SimpleTimer.h> //from github, https://github.com/schinken/SimpleTimer, download the cpp and h files (raw) and place into a folder named "SimpleTimer" in the arduino libraries folder

#include "secrets.h" // This contains the "SSID", "PASSWORD" and "MQTT_SERVER" values.

// RFID reader config
#define SS_PIN 15 // D8 => GPIO 15
#define RST_PIN 0 // D3 => GPIO 0
#define CARD_READ_INTERVAL (200)

#define DEVICE_TYPE "rfid-reader" // That's me

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
SimpleTimer timer;                //a timer object to handle periodic tasks without blocking delay()

const bool debugRfid = false;
const bool debugMQTT = false;

// the setup function runs once when you press reset or power the board
void setup()
{
    initializeSerial();
    initializeWiFi();
    initializeMQTT();
    initializeRfidReader();
}

void loop()
{
    timer.run();
}

void initializeSerial()
{
    Serial.begin(115200);
    while (!Serial)
    {
        ; // wait for serial attach
    }
}

void initializeWiFi()
{

    delay(100);

    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void initializeMQTT()
{
    mqtt_client.setServer(MQTT_SERVER, 1883);

    timer.setInterval(100, runMQTT);
}

void initializeRfidReader()
{
    Serial.println(F("Init rfid reader"));
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522

    if (debugRfid)
    {
        mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
        Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
    }
    timer.setInterval(CARD_READ_INTERVAL, readRfidToSerial);
}

void readRfidToSerial()
{

    if (!mfrc522.PICC_IsNewCardPresent())
    {
        if (debugRfid)
        {
            Serial.println(F("No card present"));
        }
        return;
    }
    if (debugRfid)
    {
        Serial.println(F("Card present"));
    }

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial())
    {
        if (debugRfid)
        {
            Serial.println(F("Failed to select card"));
        }
        return;
    }

    // Dump debug info about the card; PICC_HaltA() is automatically called
    //   mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

    String rfidUIDValue = "";
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        rfidUIDValue.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
        rfidUIDValue.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    rfidUIDValue.toUpperCase();
    publishCard(rfidUIDValue.c_str());

    if (debugRfid)
    {
        Serial.print("UID tag:");
        Serial.println(rfidUIDValue);
    }
}

void reconnect()
{
    // Loop until we're reconnected
    while (!mqtt_client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (mqtt_client.connect(clientId.c_str()))
        {
            Serial.println("connected");
            // Once connected, publish an announcement...
            mqtt_client.publish("outTopic", "hello world");
            // ... and resubscribe
            mqtt_client.subscribe("inTopic");

            // Register to the server
            publishRegistration();
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void runMQTT()
{
    if (!mqtt_client.connected())
    {
        reconnect();
    }
    mqtt_client.loop();
}

void publishCard(const char *uid)
{
    publishTopic("card", uid);
}

void publishRegistration()
{
    publishTopic("register", DEVICE_TYPE);
}

void getMac(char mac[13])
{
    String s = WiFi.macAddress();
    for (int i = 0, j = 0; i < 12; i += 1)
    {
        mac[i] = s.charAt(j);
        if (s.charAt(j + 1) == ':')
        {
            j += 2;
        }
        else
        {
            j += 1;
        }
    }
    mac[12] = 0x00;
}

void publishTopic(const char *cmd, const char *value)
{
    char pub_topic[128];
    char mac[13];
    getMac(mac);
    snprintf(pub_topic, sizeof(pub_topic), "/device/%s/%s", mac, cmd);

    mqtt_client.publish(pub_topic, value);

    if (debugMQTT)
    {
        Serial.print("Published '");
        Serial.print(value);
        Serial.print("' to topic: ");
        Serial.println(pub_topic);
    }
}
