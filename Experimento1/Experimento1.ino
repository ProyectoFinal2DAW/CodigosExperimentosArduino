#include <WiFiS3.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "arduino_secrets.h"

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

const char* mqttServer = "172.17.40.11";
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient client(wifiClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

const int Sensor1_pin = 2;
const int Sensor2_pin = 3;

unsigned long timeSensor1 = 0;
unsigned long timeSensor2 = 0;

bool sensor1_state = false;  // Estado previo del sensor 1
bool sensor2_state = false;  // Estado previo del sensor 2

void setup() {
  Serial.begin(115200);
  pinMode(Sensor1_pin, INPUT);
  pinMode(Sensor2_pin, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");

  timeClient.begin();
  timeClient.update();

  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    if (client.connect("arduinoClient")) {
      Serial.println("¡Conectado a MQTT!");
    } else {
      Serial.print("Error: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  bool sensor1_actual = digitalRead(Sensor1_pin);
  bool sensor2_actual = digitalRead(Sensor2_pin);

  // Detecta flanco de subida en el sensor 1
  if (sensor1_actual == HIGH && sensor1_state == false) {
    timeClient.update();
    timeSensor1 = timeClient.getEpochTime();
    Serial.println("Objeto detectado por sensor 1");
  }
  
  // Detecta flanco de subida en el sensor 2
  if (sensor2_actual == HIGH && sensor2_state == false && timeSensor1 != 0) {
    timeClient.update();
    timeSensor2 = timeClient.getEpochTime();

    Serial.print("Tiempo sensor 1: ");
    Serial.print(timeSensor1);
    Serial.print(" | Tiempo sensor 2: ");
    Serial.print(timeSensor2);
    Serial.println(" s");

    char mensaje[150];
    sprintf(mensaje, "{\"tiempo_sensor1\": %lu, \"tiempo_sensor2\": %lu}", 
            timeSensor1, timeSensor2);
   
    client.publish("sensores/tiempo", mensaje);

    timeSensor1 = 0;
    timeSensor2 = 0;
  }

  // Actualiza los estados anteriores de los sensores
  sensor1_state = sensor1_actual;
  sensor2_state = sensor2_actual;
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("arduinoClient")) {
      Serial.println("Reconectado.");
    } else {
      Serial.print("Error: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}
