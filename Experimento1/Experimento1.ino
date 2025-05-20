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

// Pines de sensores
const int Sensor1_pin = 2;
const int Sensor2_pin = 3;
const int Sensor3_pin = 4;
const int Sensor4_pin = 5;

// Variables de tiempo
float timeSensor1 = 0;
float timeSensor2 = 0;
float timeSensor3 = 0;
float timeSensor4 = 0;

// Estados anteriores
bool sensor1_state = false;
bool sensor2_state = false;
bool sensor3_state = false;
bool sensor4_state = false;

// Variables para tiempo con milisegundos
unsigned long lastNtpEpoch = 0;
unsigned long lastNtpMillis = 0;

void setup() {
  Serial.begin(115200);
  pinMode(Sensor1_pin, INPUT);
  pinMode(Sensor2_pin, INPUT);
  pinMode(Sensor3_pin, INPUT);
  pinMode(Sensor4_pin, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.update();
  lastNtpEpoch = timeClient.getEpochTime();
  lastNtpMillis = millis();

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

// Función para obtener tiempo con milisegundos desde última sincronización NTP
float getEpochTimeWithMs() {
  unsigned long now = millis();
  unsigned long elapsed = now - lastNtpMillis;
  return lastNtpEpoch + (elapsed / 1000.0);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Actualiza sincronización NTP cada minuto
  if (millis() - lastNtpMillis > 60000) {
    if (timeClient.update()) {
      lastNtpEpoch = timeClient.getEpochTime();
      lastNtpMillis = millis();
    }
  }

  bool sensor1_actual = digitalRead(Sensor1_pin);
  bool sensor2_actual = digitalRead(Sensor2_pin);
  bool sensor3_actual = digitalRead(Sensor3_pin);
  bool sensor4_actual = digitalRead(Sensor4_pin);

  // Flanco de subida sensor 1
  if (sensor1_actual == HIGH && !sensor1_state) {
    timeSensor1 = getEpochTimeWithMs();
    Serial.println("Sensor 1 activado");
  }

  // Flanco de subida sensor 2
  if (sensor2_actual == HIGH && !sensor2_state && timeSensor1 != 0) {
    timeSensor2 = getEpochTimeWithMs();
    Serial.println("Sensor 2 activado");
  }

  // Flanco de subida sensor 3
  if (sensor3_actual == HIGH && !sensor3_state) {
    timeSensor3 = getEpochTimeWithMs();
    Serial.println("Sensor 3 activado");
  }

  // Flanco de subida sensor 4
  if (sensor4_actual == HIGH && !sensor4_state && timeSensor3 != 0) {
    timeSensor4 = getEpochTimeWithMs();
    Serial.println("Sensor 4 activado");

    // Enviar mensaje MQTT con los 4 tiempos
    Serial.print("Tiempos capturados: ");
    Serial.print("S1: "); Serial.print(timeSensor1, 3);
    Serial.print(" | S2: "); Serial.print(timeSensor2, 3);
    Serial.print(" | S3: "); Serial.print(timeSensor3, 3);
    Serial.print(" | S4: "); Serial.println(timeSensor4, 3);

    char mensaje[250];
    snprintf(mensaje, sizeof(mensaje),
      "{\"tiempo_sensor1\": %.3f, \"tiempo_sensor2\": %.3f, \"tiempo_sensor3\": %.3f, \"tiempo_sensor4\": %.3f}",
      timeSensor1, timeSensor2, timeSensor3, timeSensor4);

    client.publish("sensores/tiempo", mensaje);

    // Reiniciar tiempos
    timeSensor1 = timeSensor2 = timeSensor3 = timeSensor4 = 0;
  }

  // Actualiza estados previos
  sensor1_state = sensor1_actual;
  sensor2_state = sensor2_actual;
  sensor3_state = sensor3_actual;
  sensor4_state = sensor4_actual;
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
