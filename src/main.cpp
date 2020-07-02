#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <keys.h>

const char *mqtt_server = "192.168.1.183";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
unsigned long lastSensor = 0;

bool toggle;

#define SENSORPIN 16
#define DOORPIN 5

void setup_wifi()
{
    digitalWrite(DOORPIN, LOW);
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_NAME);
    WiFi.hostname("Smart_Garage");

    WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

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

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    payload[length] = '\0';
    String position = String((char *)payload);

    Serial.print(position);
    Serial.println();

    if (position == "close")
    {
        client.publish("home-assistant/garage_cover/response", "close");
        digitalWrite(DOORPIN, HIGH);
        delay(500);
        digitalWrite(DOORPIN, LOW);
    }
    if (position == "open")
    {
        client.publish("home-assistant/garage_cover/response", "open");
        digitalWrite(DOORPIN, HIGH);
        delay(500);
        digitalWrite(DOORPIN, LOW);
    }
    if (position == "stop")
    {
        client.publish("home-assistant/garage_cover/response", "stop");
        digitalWrite(DOORPIN, HIGH);
        delay(500);
        digitalWrite(DOORPIN, LOW);
    }
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        digitalWrite(DOORPIN, LOW);
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "Garage_Cover-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish("home-assistant/garage_cover/availability", "online");
            client.publish("home-assistant/garage/availability", "online");
            // ... and resubscribe
            client.subscribe("home-assistant/garage_cover/set");
            client.subscribe("home-assistant/garage_cover/set_position");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    pinMode(SENSORPIN, INPUT_PULLDOWN_16);
    pinMode(DOORPIN, OUTPUT);
    digitalWrite(DOORPIN, LOW);
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > 60000)
    {
        lastMsg = now;
        client.publish("home-assistant/garage_cover/availability", "online");
        client.publish("home-assistant/garage/availability", "online");
    }

    if (now - lastSensor > 2000)
    {
        lastSensor = now;
        if (digitalRead(SENSORPIN) == LOW && toggle == false)
        {
            client.publish("home-assistant/garage/contact", "OFF");
            toggle = true;
        }
        else if (digitalRead(SENSORPIN) == HIGH && toggle == true)
        {
            client.publish("home-assistant/garage/contact", "ON");
            toggle = false;
        }
    }
}