#include "WiFiS3.h"
#include <ArduinoMqttClient.h>

int redPin = 2;
int greenPin = 3;
int bluePin = 4;
int redPinIn = 5;
int greenPinIn = 6;
int bluePinIn = 7;
int redPinOut = 8;
int greenPinOut = 9;
int bluePinOut = 10;
byte mac[6];

// WiFi work
char ssid[] = "DATA"; // Enter your WiFi name
char pass[] = "XXXXXXXX";  // Enter WiFi password

char mqtt_user[] = "username";
char mqtt_pass[] = "XXXXXXX";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
  float temperature = 0.0;
  float temperatureIn = 0.0;
  float temperatureOut = 0.0;

const char broker[] = "xxx.xxx.xxx.xxx";
int        port     = 1883;
const char subscribe_topic[]  = "Omiros/Home/temperature";
const char subscribe_topicIn[]  = "Omiros/Work/temperatureIn";
const char subscribe_topicOut[]  = "Omiros/Work/temperatureOut";

//const char publish_topic[]  = "/hello/world";
void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(redPinIn, OUTPUT);
  pinMode(greenPinIn, OUTPUT);
  pinMode(bluePinIn, OUTPUT);
  pinMode(redPinOut, OUTPUT);
  pinMode(greenPinOut, OUTPUT);
  pinMode(bluePinOut, OUTPUT);
  // Create serial connection and wait for it to become available.
  Serial.begin(9600);
  while (!Serial) {
    ; 
  }

  // Connect to WiFi
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.print("Connected, IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print(", RRSI: ");
  Serial.println(WiFi.RSSI());
  Serial.print(" MAC address : ");
  WiFi.macAddress(mac);
  for (int i=1;i<5;i++){
  Serial.print(mac[i],HEX);Serial.print(":");
  }Serial.println();
  // You can provide a username and password for authentication
  mqttClient.setUsernamePassword(mqtt_user, mqtt_pass);
  Serial.print("Attempting to connect to the MQTT broker.");
  while (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    delay(4000);

    //while (1);
  }

  Serial.println("You're connected to the MQTT broker!");

  Serial.print("Subscribing to topic: ");
  Serial.println(subscribe_topic);
  Serial.println(subscribe_topicIn);
  Serial.println(subscribe_topicOut);

  // subscribe to a topic
  mqttClient.subscribe(subscribe_topic);
  mqttClient.subscribe(subscribe_topicIn);
  mqttClient.subscribe(subscribe_topicOut);
  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(topic);

  Serial.println("Waiting for messages on topic: ");
  Serial.println(subscribe_topic);
  Serial.println(subscribe_topicIn);
  Serial.println(subscribe_topicOut);
}

void loop() {
  int messageSize = mqttClient.parseMessage();

  if (messageSize) {
    // we received a message, print out the topic and contents
    String topicRecieved = mqttClient.messageTopic();
    Serial.print("Received a message with topic :");
    Serial.print(topicRecieved);
    Serial.print(", length ");
    Serial.print(messageSize);
    Serial.print(" bytes:");

    if(topicRecieved=="Omiros/Home/temperature"){

    String message = mqttClient.readStringUntil('\n');
    temperature = atof(message.c_str());
    Serial.println(message);
    
    if (temperature <= 25 ){
  digitalWrite(redPin, HIGH);//green
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, LOW);
}
else if(temperature > 30){
  digitalWrite(redPin, LOW);//red
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);
}
else {
   digitalWrite(redPin, LOW);//magenta
   digitalWrite(greenPin, LOW);
   digitalWrite(bluePin, HIGH); 
}
    }
    else if(topicRecieved =="Omiros/Work/temperatureIn"){
    String messageIn = mqttClient.readStringUntil('\n');
    temperatureIn = atof(messageIn.c_str());
    Serial.println(messageIn);
    if (temperatureIn <= 25 ){
  digitalWrite(redPinIn, HIGH);//green
  digitalWrite(greenPinIn, HIGH);
  digitalWrite(bluePinIn, LOW);
}
else if(temperatureIn > 30){
  digitalWrite(redPinIn, LOW);//red
  digitalWrite(greenPinIn, HIGH);
  digitalWrite(bluePinIn, HIGH);
}
else {
   digitalWrite(redPinIn, LOW);//magenta
   digitalWrite(greenPinIn, LOW);
   digitalWrite(bluePinIn, HIGH);  
}
    }
    else if(topicRecieved =="Omiros/Work/temperatureOut"){
    String messageOut = mqttClient.readStringUntil('\n');
    temperatureOut = atof(messageOut.c_str());
    Serial.println(messageOut);
    if (temperatureOut <= 25 ){
  digitalWrite(redPinOut, HIGH);//green
  digitalWrite(greenPinOut, HIGH);
  digitalWrite(bluePinOut, LOW);
}
else if(temperatureOut > 30){
  digitalWrite(redPinOut, LOW);//red
  digitalWrite(greenPinOut, HIGH);
  digitalWrite(bluePinOut, HIGH);
}
else {
   digitalWrite(redPinOut, LOW);//magenta
   digitalWrite(greenPinOut, LOW);
   digitalWrite(bluePinOut, HIGH);  
}
    }
  }

  delay(1000);

 

}

