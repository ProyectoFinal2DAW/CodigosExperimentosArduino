#include <WiFiS3.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "arduino_secrets.h"

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

const char* mqttServer = "172.16.30.123";
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient client(wifiClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

const int Sensor1_pin = 2;
const int Sensor2_pin = 3;

unsigned long timeSensor1 = 0;
unsigned long timeSensor2 = 0;

bool sensor1_state = false;
bool sensor2_state = false;

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
  Serial.println(WiFi.localIP());

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

  // Flanco de subida sensor 1
  if (sensor1_actual == HIGH && !sensor1_state) {
    timeClient.update();
    timeSensor1 = timeClient.getEpochTime();
    Serial.println("Sensor 1 activado");
  }

  // Flanco de subida sensor 2
  if (sensor2_actual == HIGH && !sensor2_state && timeSensor1 != 0) {
    timeClient.update();
    timeSensor2 = timeClient.getEpochTime();
    Serial.println("Sensor 2 activado");

    // Enviar mensaje MQTT con los 2 tiempos
    Serial.print("Tiempos capturados: ");
    Serial.print("S1: "); Serial.print(timeSensor1);
    Serial.print(" | S2: "); Serial.println(timeSensor2);

    char mensaje[150];
    sprintf(mensaje, "{\"tiempo_sensor1\": %lu, \"tiempo_sensor2\": %lu}",
            timeSensor1, timeSensor2);

    client.publish("sensores/tiempo", mensaje);

    // Reiniciar tiempos
    timeSensor1 = 0;
    timeSensor2 = 0;
  }

  // Actualiza estados previos
  sensor1_state = sensor1_actual;
  sensor2_state = sensor2_actual;
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("arduinoClient")) {
      Serial.println("Conectado.");
      client.subscribe("sensores/tiempo");
    } else {
      Serial.print("Error MQTT: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}
