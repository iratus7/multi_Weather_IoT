#include <WiFi.h>
#include <PubSubClient.h>
#include <MQUnifiedsensor.h>
#include "DHT.h"
#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
#define placa "ESP-32"
#define Voltage_Resolution 3.3
#define pin 26
#define type "MQ-135"
#define ADC_Bit_Resolution 12
#define RatioMQ135CleanAir 3.6  
double CO2 = (0);  
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

#define LIGHT_SENSOR_PIN 34

// WiFi work
const char *ssid = "Jungle Chaos"; // Enter your WiFi name
const char *password = "XXXXXXX;  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "myhost.com";
const char *topicTHome = "Omiros/Home/temperature";
const char *topicHHome = "Omiros/Home/humidity";
const char *topicAHome = "Omiros/Home/air";
const char *topicTolueneHome = "Omiros/Home/airToluene";
const char *topicNH4Home = "Omiros/Home/NH4";
const char *topicLightHome = "Omiros/Home/light";

const char *mqtt_username = "perseas";
const char *mqtt_password = "veronica";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

//milis time events
const long eventTime_clientLoop = 2000;
const long eventTime_readSensors = 30000;
unsigned long previousTime_clientLoop,previousTime_readSensors = 0;

void setup() {
 // Set software serial baud to 115200;
 Serial.begin(115200);
 dht.begin();
 //WiFi.enableIpV6();

//Set math model to calculate the PPM concentration and the value of constants
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
 
  MQ135.init(); 
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
  MQ135.serialDebug(false);
  
  connectToWiFi(); 

 //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 while (!client.connected()) {
     String client_id = "esp32-client-";
     client_id += String(WiFi.macAddress());
     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
         Serial.println("Perseas MQTT broker connected");
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }
 }
 // publish and subscribe
 //client.publish(topic, "Temperature is 20!");
 //client.subscribe(topic);
}

void callback(char *topicTHome, byte *payload, unsigned int length) {
 Serial.print("Message arrived in topicTHome: ");
 Serial.println(topicTHome);
 Serial.print("Message:");
 for (int i = 0; i < length; i++) {
     Serial.print((char) payload[i]);
 }
 Serial.println();
 Serial.println("-----------------------");
}

void loop() {
  unsigned long currentTime = millis();

  //Client Loop delay 2secs
if (currentTime - previousTime_clientLoop >= eventTime_clientLoop) 
{
    client.loop();
    previousTime_clientLoop = currentTime;
}

//Read Sensors delay 20secs
if (currentTime - previousTime_readSensors >= eventTime_readSensors) 
{
    ReadSensorsAndPublish();
    previousTime_readSensors = currentTime;
}

}

void ReadSensorsAndPublish(){
   // Ελέγξε αν η σύνδεση WiFi χάθηκε και προσπάθησε να συνδεθείς ξανά
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost, attempting to reconnect...");
    connectToWiFi(); // Προσπάθησε να συνδεθείς στο WiFi ξανά
  }
  // Ελέγξε αν η σύνδεση με τον broker MQTT χάθηκε και προσπάθησε να επανασυνδεθείς
  if (!client.connected()) {
    reconnectMqtt(); // Προσπάθησε να επανασυνδεθείς στον broker MQTT
  }

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

 //MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
  MQ135.setA(110.47); MQ135.setB(-2.862);   
  CO2 = MQ135.readSensor(); // Sensor will read CO2 concentration using the model and a and b values setted before or in the setup   

  // MQ135.setA(605.18); MQ135.setB(-3.937);
  // float CO = MQ135.readSensor();

  // MQ135.setA(77.255); MQ135.setB(-3.18);
  // float Alcohol = MQ135.readSensor();

  MQ135.setA(44.947); MQ135.setB(-3.445);
  float Toluene = MQ135.readSensor();

  MQ135.setA(102.2 ); MQ135.setB(-2.473);
  float NH4 = MQ135.readSensor();

  // MQ135.setA(34.668); MQ135.setB(-3.369);
  // float Acetone = MQ135.readSensor();

  int lightPercentage = (analogRead(LIGHT_SENSOR_PIN)*100)/4095;

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("% Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print("CO2: ");   
  Serial.print(CO2);
  Serial.print(" Toluene: ");   
  Serial.print(Toluene);
  Serial.print(" NH4: ");   
  Serial.print(NH4);
  Serial.print(" Light: ");   
  Serial.print(lightPercentage);
  Serial.println("%");

  // Publish temperature to MQTT topic
  String temperature = String(t);
  String humidity = String(h);
  String air = String(CO2);
  String TolueneAir = String(Toluene);
  String NH4Air = String(NH4);
  String lightP = String(lightPercentage);
  client.publish(topicTHome, temperature.c_str());
  client.publish(topicHHome, humidity.c_str());
  client.publish(topicAHome, air.c_str());
  client.publish(topicTolueneHome, TolueneAir.c_str());
  client.publish(topicNH4Home, NH4Air.c_str());
  client.publish(topicLightHome, lightP.c_str());
 
 //print the state of the connection
  if (client.state()==0){
    Serial.println("Connected to Broker");}
  else{
    Serial.print("No connection with broker. State : ");
    Serial.println(client.state());}
}

void connectToWiFi(){ 
  
 // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
    }
    Serial.print("Connected, IP address: ");
    Serial.print(WiFi.localIP());
    Serial.print(", RRSI: ");
    Serial.println(WiFi.RSSI());
    //Serial.print(", IPv6: ");
    //Serial.print(WiFi.localIPv6());
  
}

void reconnectMqtt() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker");      
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}
