#include <ESP8266WiFi.h> 
#include <WiFiClient.h>
#include "Adafruit_MQTT.h" 
#include "Adafruit_MQTT_Client.h" 

/************************* WiFi Access Point *********************************/ 
//#define WLAN_SSID       "Tenda_320CE0" 
//#define WLAN_PASS       "0105654525afk" 
#define WLAN_SSID       "antlysis_meadow_2.4G@unifi"
#define WLAN_PASS       "!Ath4ht@w4dt!"
//#define WLAN_SSID       "1@southern park"
//#define WLAN_PASS       "0166606322"
//#define WLAN_SSID         "HUAWEI P10 lite"
//#define WLAN_PASS         "c5943f26-c6f"

//#define WLAN_SSID         "Yes @su xin"
//#define WLAN_PASS         "0124878756"
#define MQTT_SERVER      "192.168.1.17" // give static address
//#define MQTT_SERVER      "192.168.1.161"
#define MQTT_PORT         1883                    
#define MQTT_USERNAME    "" 
#define MQTT_PASSWORD         "" 
#define COIN_OUTPIN 12
#define SEL_PIN 13
#define COIN_INPIN 9
#define DOORLOCK_PIN 10
#define TERMINAL 07038189344
//const COIN_TOPIC "coin/" + TERMINAL
// Create an ESP8266 WiFiClient class to connect to the MQTT server. 
WiFiClient client; 
 //Setup the MQTT client class by passing in the WiFi client and MQTT server and login details. 
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD); 
// Define a will
//const char WILL_FEED[] PROGMEM = MQTT_USERNAME "connectivity/07038189344";
Adafruit_MQTT_Publish lastwill = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "connectivity/07038189344");

/****************************** Feeds ***************************************/ 
 

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname> 
Adafruit_MQTT_Publish coinInserted = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "coin/07038189344"); 
// Setup a feed called 'esp8266_led' for subscribing to changes. 
Adafruit_MQTT_Publish mqttConnected = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "connected/07038189344"); 
Adafruit_MQTT_Subscribe receivedPayment = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "07038189344");
Adafruit_MQTT_Subscribe connectivityCheck = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "connectivity2");
/*************************** Sketch Code ************************************/ 
volatile byte interruptCounter = 0;

//void MQTT_connect(); 
void setup() {
 pinMode(DOORLOCK_PIN, INPUT_PULLUP);
 attachInterrupt(digitalPinToInterrupt(DOORLOCK_PIN), handleInterrupt, FALLING);
 pinMode(COIN_OUTPIN, OUTPUT);
 pinMode(SEL_PIN, OUTPUT);
 pinMode(LED_BUILTIN, OUTPUT);
 Serial.begin(115200); 
 delay(1000);
 Serial.print("its working");
 // always need to select low for the coin acceptor.
 digitalWrite(SEL_PIN, LOW);
 digitalWrite(COIN_OUTPIN, HIGH); 
 digitalWrite(LED_BUILTIN, HIGH);
 // Setup button as an input with internal pull-up.  
 Serial.println(F("RPi-ESP-MQTT")); 
 // Connect to WiFi access point. 
 Serial.println();
 Serial.print("Connecting to "); 
 Serial.println(WLAN_SSID); 
 WiFi.begin(WLAN_SSID, WLAN_PASS); 
 while (WiFi.status() != WL_CONNECTED) { 
   delay(500); 
   Serial.print("."); 
 } 
 Serial.println(); 
 Serial.println("WiFi connected"); 
 Serial.println("IP address: "); Serial.println(WiFi.localIP()); 
 // Setup MQTT subscription for esp8266_led feed. 
 mqtt.subscribe(&receivedPayment);
 mqtt.subscribe(&connectivityCheck);
 mqtt.will(MQTT_USERNAME "connectivity/07038189344", "OFF");
} 
//uint32_t x=0; 
void loop() { 
 // Ensure the connection to the MQTT server is alive (this will make the first 
 // connection and automatically reconnect when disconnected).  See the MQTT_connect 
 MQTT_connect(); 

 lastwill.publish("ON");  // make sure we publish ON first thing after connecting
 // this is our 'wait for incoming subscription packets' busy subloop 
 // try to spend your time here 
 // Here its read the subscription 
 Adafruit_MQTT_Subscribe *subscription; 
 while ((subscription = mqtt.readSubscription())) { 
   if (subscription == &receivedPayment) {
     Serial.println("test124");
     char *message = (char *)receivedPayment.lastread; 
     float to_float = atof(message);
     int amount = int(to_float);
     Serial.print(F("Got: ")); 
     Serial.println(message);
     
     // Check if the message was a number  
     if (isValidNumber(message) == true) { 
       if (amount <= 10) { 
         payMachine(amount);
       }
     } 
    }
   else if (subscription == &connectivityCheck) { 
     char *message = (char *)connectivityCheck.lastread;
     String myMessage = message;
     Serial.println(message);
     if (myMessage.equals("check")) {
        Serial.println("reply yes");
        mqttConnected.publish("07038189344");
     }
   } 
 }
   if(interruptCounter>0){
 
      interruptCounter--;
      Serial.print("An interrupt has occurred. Total: ");
  } 
} 
 
void MQTT_connect() { 
 int8_t ret; 
 // Stop if already connected. 
 if (mqtt.connected()) {
   return; 
 } 
 Serial.print("Connecting to MQTT... "); 
 uint8_t retries = 3; 
 while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected 
      Serial.println(mqtt.connectErrorString(ret)); 
      Serial.println("Retrying MQTT connection in 5 seconds..."); 
      mqtt.disconnect(); 
      delay(5000);  // wait 5 seconds 
      retries--; 
      if (retries == 0) { 
        // basically die and wait for WDT to reset me 
        while (1); 
      } 
 }
 //mqttConnected.publish("Hi i have connected you");
 Serial.println("MQTT Connected!"); 
} 

void payMachine (int amt) {
  if (amt <= 9) {
    digitalWrite(SEL_PIN,HIGH);
    for (int i=1; i <= amt; i++){
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(COIN_OUTPIN, LOW);
        delay(1000);
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(COIN_OUTPIN, HIGH); 
        //delay(2000);
    }
    digitalWrite(SEL_PIN,LOW);
  }
}

boolean isValidNumber(String str){
   boolean isNum=false;
   for(byte i=0;i<str.length();i++)
   {
       isNum = isDigit(str.charAt(i));
   }
   return isNum;
} 

void handleInterrupt() {
  Serial.print("I found interrupt lur");
  coinInserted.publish("Received_1");
}

