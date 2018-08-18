//MQTT Payloads Rule:
// 16位2进制，每2位表示一个pin的开与关
// 比如:
// 10 00 00 00 00 00 00 00 表示 1# 开
// 01 00 00 00 00 00 00 00 表示 1# 关
// 比如:
// 10 10 10 10 10 10 10 10 表示 1# 全开
// 01 01 01 01 01 01 01 01 表示 1# 全关

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "TT";
const char* password = "13278899069";
const char* chip_id = "tt_home_livingroom_switch_01";
const int bps = 9600;
const int trigger = HIGH; //触发电平

#define mqtt_server  "192.168.0.13"
#define mqtt_server_port 1883
#define mqtt_user "ttiot"
#define mqtt_password "cl3bkm4fuc"

const int dataLength = 4;
const int pins[dataLength] = {D1, D2, D3, D4};
const int switch01 = D5;
const int switch02 = D6;
int switch01status = LOW;
int switch02status = LOW;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {

  Serial.begin(9600);

  for (int i = 0; i < dataLength; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], !trigger);
  }

  setup_wifi();

  client.setServer(mqtt_server, mqtt_server_port);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

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
    // Attempt to connect
    if (client.connect(chip_id, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(chip_id);
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//When received MQTT message
void callback(char* topic, byte* payload, unsigned int length) {
  String payloadData = "";
  for (int i = 0; i < length; i++) {
    payloadData += String((char)payload[i]);
  }
  //the message received.
  String message = payloadData.substring(0, payloadData.lastIndexOf("@"));
  //the message who send it.
  String sender = payloadData.substring(payloadData.lastIndexOf("@") + 1, payloadData.length());

  if (message.length() <= 16)
  {
    for (int i = 0; i < message.length(); i++) {
      //Serial.print((char)payload[i]);
      //Serial.print("/");
      //Serial.print(i % 2);
      if ((char)message[i] == '1' && (i + 1) % 2 == 1)
        digitalWrite(pins[i / 2], HIGH);
      if ((char)message[i] == '1' && (i + 1) % 2 == 0)
        digitalWrite(pins[i / 2], LOW);
    }
  }
  
  //send the result to the sender
  //1 for on, 0 for off
  String result;
  for (int i = 0; i < dataLength; i++) {
    if (digitalRead(pins[i]) == LOW) {
      result += '0';
    }
    else {
      result += '1';
    }
  }
  String data = "{\"type\":\"Simple\",\"data\":\"" + result + "\"}";
  client.publish(sender.c_str(), data.c_str());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (digitalRead(switch01) == HIGH) {
    switch01status = !switch01status;
    digitalWrite(D2, switch01status);
    delay(200);
  }
  if (digitalRead(switch02) == HIGH) {
    switch02status = !switch02status;
    digitalWrite(D3, !switch02status);
    delay(200);
  }
}
