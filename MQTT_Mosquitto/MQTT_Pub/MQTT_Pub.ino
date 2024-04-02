#ifdef ESP8266
#include <ESP8266WiFi.h> /* WiFi library for ESP8266 */
#else
#include <WiFi.h> /* WiFi library for ESP32 */
#endif
#include <Wire.h>
#include <PubSubClient.h>
#include "DHT.h" /* DHT11 sensor library */

#define DHTPIN 2
#define DHTTYPE DHT22  // DHT 11
DHT dht(DHTPIN, DHTTYPE);

#define wifi_ssid "TP-LINK-304"
#define wifi_password "0987654321"
#define mqtt_server "192.168.0.104"

#define humidity_topic "sensor/DHT11/humidity"
#define temperature_topic "sensor/DHT11/temperature"
#define gas_topic "sensor/MQ-2/gas"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

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
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

unsigned long delayMillis = 0;

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - delayMillis > 2000 || delayMillis == 0) {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    delayMillis = millis();
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    float mq2_value;

    int mq2 = analogRead(A0);
    float voltage = mq2 / 1024.0 * 5.0;
    float ratio = voltage / 1.4;
    mq2_value = 1000.0 * pow(10, ((log10(ratio) - 1.0278) / 0.6629));

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(mq2_value)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    if (isnan(mq2_value)) {
      Serial.println("Failed to read from MQ-2 sensor!");
      return;
    }

    Serial.println("Temperature: " + String(t) + "°C");
    client.publish(temperature_topic, String(t).c_str(), true);

    Serial.println("Humidity: " + String(h) + "%");
    client.publish(humidity_topic, String(h).c_str(), true);

    Serial.println("Gas: " + String(mq2_value, 0) + "ppm");
    client.publish(gas_topic, String(mq2_value, 0).c_str(), true);
  }
}