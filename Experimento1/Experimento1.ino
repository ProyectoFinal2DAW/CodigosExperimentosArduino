#include <WiFiS3.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "arduino_secrets.h"  // Define SECRET_SSID y SECRET_PASS

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

// Estados anteriores
bool sensor1_state = false;
bool sensor2_state = false;
bool sensor3_state = false;
bool sensor4_state = false;

// Enviar tiempo del sensor vía MQTT
void enviarTiempoSensor(const char* sensorName, unsigned long tiempo) {
  char mensaje[200];
  snprintf(mensaje, sizeof(mensaje),
           "{\"sensor\": \"%s\", \"tiempo\": %lu}", sensorName, tiempo);

  client.publish("sensores/tiempo", mensaje);

  Serial.print(sensorName);
  Serial.print(" activado - tiempo enviado: ");
  Serial.println(tiempo);
}

void setup() {
  Serial.begin(115200);

  // Configurar pines como entrada
  pinMode(Sensor1_pin, INPUT);
  pinMode(Sensor2_pin, INPUT);
  pinMode(Sensor3_pin, INPUT);
  pinMode(Sensor4_pin, INPUT);

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");
  Serial.println(WiFi.localIP());

  // Sincronizar NTP
  timeClient.begin();
  timeClient.update();

  // Conectar a MQTT
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    if (client.connect("arduinoClient")) {
      Serial.println("¡Conectado a MQTT!");
    } else {
      Serial.print("Error MQTT: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Actualizar NTP cada 30 segundos
  static unsigned long lastNtpUpdate = 0;
  if (millis() - lastNtpUpdate > 30000) {
    if (timeClient.update()) {
      lastNtpUpdate = millis();
    }
  }

  // Leer sensores
  bool s1 = digitalRead(Sensor1_pin);
  bool s2 = digitalRead(Sensor2_pin);
  bool s3 = digitalRead(Sensor3_pin);
  bool s4 = digitalRead(Sensor4_pin);

  // Detectar flancos de subida y enviar tiempo individual (solo segundos)
  if (s1 == HIGH && !sensor1_state) {
    enviarTiempoSensor("sensor1", timeClient.getEpochTime());
  }
  if (s2 == HIGH && !sensor2_state) {
    enviarTiempoSensor("sensor2", timeClient.getEpochTime());
  }
  if (s3 == HIGH && !sensor3_state) {
    enviarTiempoSensor("sensor3", timeClient.getEpochTime());
  }
  if (s4 == HIGH && !sensor4_state) {
    enviarTiempoSensor("sensor4", timeClient.getEpochTime());
  }

  // Actualizar estados
  sensor1_state = s1;
  sensor2_state = s2;
  sensor3_state = s3;
  sensor4_state = s4;
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("arduinoClient")) {
      Serial.println("Conectado.");
    } else {
      Serial.print("Error MQTT: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}
