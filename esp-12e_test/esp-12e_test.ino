#include <ESP8266WiFi.h>

const short int BUILTIN_LED1 = 2; //GPIO2
const short int BUILTIN_LED2 = 16;//GPIO16

#define WLAN_SSID       "Tenda_320CE0" 
#define WLAN_PASS       "0105654525afk" 
#define COIN_PIN 1
#define SEL_PIN 3
#define TERMINAL 11336

void setup() {
pinMode(COIN_PIN, OUTPUT);
pinMode(SEL_PIN, OUTPUT);
pinMode(BUILTIN_LED1, OUTPUT); // Initialize the BUILTIN_LED1 pin as an output
pinMode(BUILTIN_LED2, OUTPUT); // Initialize the BUILTIN_LED2 pin as an output
Serial.begin(115200);
delay(1000);
Serial.print("its working");
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
}

void loop() {

digitalWrite(BUILTIN_LED2, LOW); // Turn the LED ON by making the voltage LOW digitalWrite(BUILTIN_LED2, HIGH); // Turn the LED off by making the voltage HIGH delay(1000); // Wait for a second
delay(2000);
digitalWrite(BUILTIN_LED2, HIGH); // Turn the LED off by making the voltage HIGH
//digitalWrite(BUILTIN_LED1, HIGH); // Turn the LED ON by making the voltage LOW
delay(2000); // Wait for two seconds
//Serial.print("Hello");

}

void payMachine (int amt) {
  if (amt <= 9) {
    digitalWrite(SEL_PIN,HIGH);
    for (int i=1; i <= amt; i++){
        delay(2000);
        digitalWrite(COIN_PIN, LOW);
        delay(50);
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

