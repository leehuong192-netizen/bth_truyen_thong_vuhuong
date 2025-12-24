#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <WiFiUdp.h>
#include <coap-simple.h>

/* ========== CONFIG ========== */
const char* WIFI_SSID = "vh";
const char* WIFI_PASS = "vhvhvhvh";

/* CoAP Server (Receiver) */
IPAddress coap_server_ip(172, 20, 10, 2);
const int coap_port = 5683;

/* CoAP Resource */
const char* coap_resource = "sensor/temp";

/* Send interval (ms) */
const unsigned long SEND_INTERVAL = 5000;
/* ============================ */

WiFiUDP udp;
Coap coap(udp);

unsigned long lastSend = 0;

/* ============================ */
void setup_wifi() {
Serial.print("Connecting WiFi");
WiFi.begin(WIFI_SSID, WIFI_PASS);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
Serial.println("\nWiFi connected");
Serial.print("IP: ");
Serial.println(WiFi.localIP());
}

/* ============================ */

void setup() {
Serial.begin(115200);
setup_wifi();

coap.start();
Serial.println("CoAP client started");
}

/* ============================ */

void loop() {
  if (millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();

    /* Payload mẫu */
    String payload = "temp=30.5";
    
    Serial.print(coap_resource);

    Serial.print(" payload: ");
    Serial.println(payload);

    // Serial.print("Sending CoAP NON POST to ");
    // coap.send(
    //   coap_server_ip,
    //   coap_port,
    //   coap_resource,
    //   COAP_NONCON,
    //   COAP_POST,
    //   NULL,
    //   0,
    //   (uint8_t*)payload.c_str(),
    //   payload.length()
    // );

    Serial.print("Sending CoAP CON POST to ");
    coap.send(
      coap_server_ip,
      coap_port,
      coap_resource,
      COAP_CON, // Confirmable message
      COAP_POST, // Method: POST
      NULL,
      0,
      (uint8_t*)payload.c_str(), // Payload
      payload.length() // Độ dài payload
    );

  }
}
