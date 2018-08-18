#include <ESP8266WiFi.h> 
#include "Adafruit_MQTT.h" 
#include "Adafruit_MQTT_Client.h" 
/************************* WiFi Access Point *********************************/ 
#define WLAN_SSID       "Tenda_320CE0" 
#define WLAN_PASS       "0105654525afk" 
//#define WLAN_SSID       "1@southern park"
//#define WLAN_PASS       "0166606322"
//#define WLAN_SSID         "HUAWEI P10 lite"
//#define WLAN_PASS         "c5943f26-c6f"

//#define WLAN_SSID         "Yes @su xin"
//#define WLAN_PASS         "0124878756"
#define MQTT_SERVER      "192.168.0.106" // give static address
#define MQTT_PORT         1883                    
#define MQTT_USERNAME    "" 
#define MQTT_PASSWORD         "" 
#define COIN_PIN 0
#define SEL_PIN 2
// Create an ESP8266 WiFiClient class to connect to the MQTT server. 
WiFiClient client; 
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details. 
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD); 
/****************************** Feeds ***************************************/ 
// Setup a feed called 'pi_led' for publishing. 

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname> 
Adafruit_MQTT_Publish pi_led = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME ""); 
// Setup a feed called 'esp8266_led' for subscribing to changes. 

Adafruit_MQTT_Subscribe esp8266_led = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "11336"); 
/*************************** Sketch Code ************************************/ 

void MQTT_connect(); 
void setup() { 
 Serial.begin(115200); 
 delay(10); 
 pinMode(COIN_PIN, OUTPUT);
 pinMode(SEL_PIN, OUTPUT);
 // always need to select low for the coin acceptor.
 digitalWrite(SEL_PIN, LOW);
 digitalWrite(COIN_PIN, HIGH); 
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
 mqtt.subscribe(&esp8266_led); 
} 
uint32_t x=0; 
void loop() { 
  // Check if the button has been pressed by looking for a change from high to 
 // low signal (with a small delay to debounce). 
 //int button_first = digitalRead(BUTTON_PIN); 
 // Ensure the connection to the MQTT server is alive (this will make the first 
 // connection and automatically reconnect when disconnected).  See the MQTT_connect 
 MQTT_connect(); 
 // this is our 'wait for incoming subscription packets' busy subloop 
 // try to spend your time here 
 // Here its read the subscription 
 //digitalWrite(LED_PIN, HIGH);
 Adafruit_MQTT_Subscribe *subscription; 
 while ((subscription = mqtt.readSubscription())) { 
   if (subscription == &esp8266_led) { 
     char *message = (char *)esp8266_led.lastread; 
     //if (isDigit(message)) {
      // convert the incoming byte to a char and add it to the string:
      //strg_msg += (char)message;
    //}
     //int amount = strg_msg.toInt();
     float to_float = atof(message);
     int amount = int(to_float);
     Serial.print(F("Got: ")); 
     Serial.println(message);
     
     for (int i=1; i <= amount; i++){
      Serial.println("haha\n");
     }
        // Check if the message was ON, OFF, or TOGGLE. 
     //if (strncmp(message, "ON", 2) == 0) { 
       // Turn the LED on.
       //Serial.println("Yes it is matched"); 
       //digitalWrite(SEL_PIN, HIGH);
       //delay(3000);
       //digitalWrite(COIN_PIN, LOW);
       //delay(1000);
       //digitalWrite(COIN_PIN, HIGH); 
       //delay(2000);
       //digitalWrite(COIN_PIN, LOW);
       //delay(1000);
      // digitalWrite(COIN_PIN, HIGH); 
       //delay(2000);
       //digitalWrite(COIN_PIN, LOW);
       //delay(1000);
       //digitalWrite(COIN_PIN, HIGH);
       //digitalWrite(SEL_PIN,LOW);
     //} 
     //else if (strncmp(message, "OFF", 3) == 0) { 
       // Turn the LED off. 
     //  digitalWrite(COIN_PIN, LOW); 
     //} 
     //else if (strncmp(message, "TOGGLE", 6) == 0) { 
       // Toggle the LED. 
      // digitalWrite(COIN_PIN, !digitalRead(COIN_PIN)); 
} 
   } 
} 
 //delay(20); 
 //int button_second = digitalRead(BUTTON_PIN); 
 //if ((button_first == HIGH) && (button_second == LOW)) { 
   // Button was pressed! 
   //Serial.println("Button pressed!"); 
   //pi_led.publish("TOGGLE"); 
 //} 
// Function to connect and reconnect as necessary to the MQTT server. 

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


