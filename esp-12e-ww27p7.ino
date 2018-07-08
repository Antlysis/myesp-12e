#include <ESP8266WiFi.h> 
#include <WiFiClient.h>
#include "Adafruit_MQTT.h" 
#include "Adafruit_MQTT_Client.h" 
/************************* WiFi Access Point *********************************/ 
#define WLAN_SSID       "Tenda_320CE0" 
#define WLAN_PASS       "0105654525afk" 
//#define WLAN_SSID       "thonyblue_5G@unifi"
//#define WLAN_PASS       "Viva@820725"
//#define WLAN_SSID       "1@southern park"
//#define WLAN_PASS       "0166606322"
//#define WLAN_SSID         "HUAWEI P10 lite"
//#define WLAN_PASS         "c5943f26-c6f"

//#define WLAN_SSID         "Yes @su xin"
//#define WLAN_PASS         "0124878756"
#define MQTT_SERVER      "192.168.0.110" // give static address
#define MQTT_PORT         1883                    
#define MQTT_USERNAME    "" 
#define MQTT_PASSWORD         "" 
#define COIN_PIN 12
#define SEL_PIN 13
#define TERMINAL 07038189344

// Create an ESP8266 WiFiClient class to connect to the MQTT server. 
WiFiClient client; 
 //Setup the MQTT client class by passing in the WiFi client and MQTT server and login details. 
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD); 
/****************************** Feeds ***************************************/ 
// Setup a feed called 'pi_led' for publishing. 

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname> 
Adafruit_MQTT_Publish pi_led = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME ); 
// Setup a feed called 'esp8266_led' for subscribing to changes. 

Adafruit_MQTT_Subscribe receivedPayment = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "07038189344"); 
/*************************** Sketch Code ************************************/ 

//void MQTT_connect(); 
void setup() {
 pinMode(COIN_PIN, OUTPUT);
 pinMode(SEL_PIN, OUTPUT);
 pinMode(LED_BUILTIN, OUTPUT);
 Serial.begin(115200); 
 delay(1000);
 Serial.print("its working");
 // always need to select low for the coin acceptor.
 digitalWrite(SEL_PIN, HIGH);
 digitalWrite(COIN_PIN, HIGH); 
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
} 
//uint32_t x=0; 
void loop() { 
 // Ensure the connection to the MQTT server is alive (this will make the first 
 // connection and automatically reconnect when disconnected).  See the MQTT_connect 
 MQTT_connect(); 
 // this is our 'wait for incoming subscription packets' busy subloop 
 // try to spend your time here 
 // Here its read the subscription 
 Adafruit_MQTT_Subscribe *subscription; 
 while ((subscription = mqtt.readSubscription())) { 
   if (subscription == &receivedPayment) { 
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
 Serial.println("MQTT Connected!"); 
} 

void payMachine (int amt) {
  if (amt <= 9) {
    digitalWrite(SEL_PIN,HIGH);
    for (int i=1; i <= amt; i++){
        delay(2000);
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(COIN_PIN, LOW);
        delay(2000);
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(COIN_PIN, HIGH); 
        delay(2000);
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


