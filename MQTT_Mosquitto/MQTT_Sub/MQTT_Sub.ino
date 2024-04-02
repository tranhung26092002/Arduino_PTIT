#ifdef ESP8266
#include <ESP8266WiFi.h> /* WiFi library for ESP8266 */
#else
#include <WiFi.h> /* WiFi library for ESP32 */
#endif
#include <Wire.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>

#define wifi_ssid "IoT LAB"
#define wifi_password "kvt1ptit"
#define mqtt_server "192.168.1.13"

#define humidity_topic "sensor/DHT11/humidity"
#define temperature_topic "sensor/DHT11/temperature"
#define gas_topic "sensor/MQ-2/gas"

WiFiClient espClient;
PubSubClient client(espClient);

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD I2C address and size

const int out[3] = {5,4, 2};
const int btn[3] = {14,12, 13};
unsigned long timeDelay = millis();

bool buzzerState = false; // Trạng thái hiện tại của còi Buzzer

void setup_wifi() {
  delay(10);
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
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Sub")) {
      Serial.println("connected");
      client.subscribe(temperature_topic);
      client.subscribe(gas_topic);
      client.subscribe(humidity_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Convert byte array to string
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Check the topic and display the message accordingly
  if (strcmp(topic, temperature_topic) == 0) {
    lcd.setCursor(0, 0); // Set cursor to the second row
    lcd.print("T: ");
    lcd.print(message.toFloat(), 1);
    lcd.print("'C");
  } else if (strcmp(topic, humidity_topic) == 0) {
    lcd.setCursor(9, 0); // Set cursor to the second row, column 10
    lcd.print(" H: ");
    lcd.print(message.toInt());
    lcd.print("%");
    if(message.toInt() > 90){
      analogWrite(out[2], 20); // Setting pin to high
      delay(1000);
      analogWrite(out[2], 0); // Setting pin to low
      delay(1000);
    }
  } else if (strcmp(topic, gas_topic) == 0) {
    lcd.setCursor(0, 1); // Set cursor to the first row
    lcd.print("Gas: ");
    lcd.print(message);
    lcd.print("ppm");
  }
}

ICACHE_RAM_ATTR void handleBtn(){
  if(millis()-timeDelay>500){
    for(int i=0;i<2;i++){      
      if(digitalRead(btn[i])==LOW){
        digitalWrite(out[i],!digitalRead(out[i]));
      }
    }
    if (digitalRead(btn[2]) == LOW) { // If button is pressed
      // Toggle buzzer state
      buzzerState = !buzzerState;
      if (buzzerState) {
        // Turn on buzzer
        analogWrite(out[2], 20); // Setting pin to high
      } else {
        // Turn off buzzer
        analogWrite(out[2], 0); // Setting pin to low
      }
      timeDelay = millis(); // Remember the time of the last button press
    }
    timeDelay=millis();
  }
}

void setup() {
  Serial.begin(9600);
  lcd.init();                      
  lcd.backlight();                 

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  for(int i=0; i<3;i++){
    pinMode(out[i],OUTPUT);
    pinMode(btn[i],INPUT_PULLUP);
    attachInterrupt(btn[i],handleBtn,FALLING);
  }
}

unsigned long timeUpdata=millis();

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
