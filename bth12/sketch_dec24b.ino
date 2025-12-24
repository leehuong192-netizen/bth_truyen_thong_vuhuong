#include <dummy.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* device_name = "vuhuong";
const char* ssid = "vhh";
const char* password = "vhvhvhvh";

const char* mqtt_server = "172.20.10.2";
const int   mqtt_port   = 1883;

const char* mqtt_user = "vungochuong";
const char* mqtt_pass = "vuhuong2005";

const char* mqtt_topic = "iot/lab1/10123182/sensor";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect() {
  while (!client.connected()) {
    String clientId = String(device_name) + "-10123182";
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      // connected
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float temp = random(250, 350) / 10.0;
  float hum  = random(400, 800) / 10.0;

  char payload[80];
  snprintf(payload, sizeof(payload), "{\"temp\":%.1f,\"hum\":%.1f}", temp, hum);

  client.publish(mqtt_topic, payload, true); // retain=true
  delay(5000);
}
