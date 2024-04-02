#include <Servo.h>

#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#define RX_PIN 10 // Chân RX của cổng nối tiếp mềm
#define TX_PIN 11 // Chân TX của cổng nối tiếp mềm

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Tạo một đối tượng cổng nối tiếp mềm

#include <DHT.h>
#include <DHT_U.h>


#define DHT_Data 2
#define Temp_Led 3
#define DHT_Fan 16

DHT dht(DHT_Data, DHT22);

#define PIR_Data 4
#define PIR_Led 5
#define PIR_Button 6
#define PIR_Light 7
int light_value = 0;
unsigned long timeDelay = 0;

Servo Door_Servo;
bool Door_Pos = true;


#define MQ_Data A0
Servo Window_Servo;
#define MQ_Fan 18
int MQ_Led = 17;
bool Window_Pos = true;

unsigned long period = 1000;

void setup() {
  dht.begin();
  pinMode(DHT_Fan, OUTPUT);
  pinMode(Temp_Led, OUTPUT);
  Serial.begin(9600);
  mySerial.begin(9600);
  while (!Serial) delay(1);

  pinMode(PIR_Data, INPUT_PULLUP);
  pinMode(PIR_Led, OUTPUT);
  pinMode(PIR_Button, INPUT_PULLUP);
  pinMode(PIR_Light, OUTPUT);

  Door_Servo.attach(14);

  pinMode(MQ_Data, INPUT);
  Window_Servo.attach(15);
  pinMode(MQ_Fan, OUTPUT);
  pinMode(MQ_Led, OUTPUT);
}

void getDHT(float &temp, float &hum) {
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  //Serial.println(temp);
  if (temp >= 35)
    digitalWrite(DHT_Fan, HIGH);
  else
    digitalWrite(DHT_Fan, LOW);
  if (temp < 35)
    digitalWrite(Temp_Led, HIGH);
  else
    digitalWrite(Temp_Led, LOW);
}

void getPIR() {
  if (digitalRead(PIR_Data) == HIGH) {
    digitalWrite(PIR_Led, LOW);
  } else {
    digitalWrite(PIR_Led, HIGH);
  }

  if (digitalRead(PIR_Button) == LOW) {
    if (millis() - timeDelay > 50) {
      if (digitalRead(PIR_Button) == LOW) {
        digitalWrite(PIR_Light, !digitalRead(PIR_Light));
      }
      timeDelay = millis();
    }
  }
}

void getMQ(float &value) {
  value = analogRead(MQ_Data);
  if (value >= 500) {
    getServoTrue(Window_Pos, Window_Servo);
    getServoTrue(Door_Pos, Door_Servo);
    digitalWrite(MQ_Fan, HIGH);
    blinkLed(period, MQ_Led);
  } else {
    getServoFalse(Door_Pos, Door_Servo);
    getServoFalse(Window_Pos, Window_Servo);
    digitalWrite(MQ_Fan, LOW);
    digitalWrite(MQ_Led, LOW);
  }
}

void blinkLed(unsigned long &period, int &ledPin) {
  unsigned long currentMillis;
  if (millis() - currentMillis >= period)  //test whether the period has elapsed
  {
    digitalWrite(ledPin, !digitalRead(ledPin));  //if so, change the state of the LED.  Uses a neat trick to change the state
    currentMillis = millis();                    //IMPORTANT to save the start time of the current LED state.
  }
}

void getServoTrue(bool &value, Servo &servo) {
  value = !value;
  servo.write(0);
  delay(500);
}

void getServoFalse(bool &value, Servo &servo) {
  value = !value;
  servo.write(180);
  delay(500);
}

DynamicJsonDocument doc(1024);
DynamicJsonDocument docBtn(1024);

void loop() {
  // put your main code here, to run repeatedly:
  float t, h, g;
  getDHT(t, h);
  getPIR();
  // getRFID();
  getMQ(g);
  doc["temperature"] = t;
  doc["humidity"] = h;
  doc["gas"] = g;
  doc["led"] = digitalRead(PIR_Light);
  doc["fan"] = digitalRead(MQ_Fan);
  doc["warning"] = digitalRead(MQ_Led);

  serializeJson(doc, mySerial);
  serializeJson(doc, Serial);
  mySerial.println();
  Serial.println();

  if (mySerial.available()) {
    String data = mySerial.readStringUntil('\n');
    DeserializationError error = deserializeJson(docBtn, data);

    int l = docBtn["led"];
    int f = docBtn["fan"];
    int w = docBtn["warning"];

    digitalWrite(PIR_Light, l);
    digitalWrite(MQ_Fan, f);
    digitalWrite(MQ_Led, w);

    serializeJson(docBtn, Serial);
    Serial.println();
  }

}