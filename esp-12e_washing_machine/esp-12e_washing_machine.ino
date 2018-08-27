#include <ESP8266WiFi.h> 
#include <WiFiClient.h>
#include "Adafruit_MQTT.h" 
#include "Adafruit_MQTT_Client.h" 

extern "C" {
#include "user_interface.h"
}

/************************* WiFi Access Point *********************************/ 
//#define WLAN_SSID       "Tenda_320CE0" 
//#define WLAN_PASS       "0105654525afk" 
#define WLAN_SSID       "antlysis_meadow_2.4G@unifi"
#define WLAN_PASS       "!Ath4ht@w4dt!"
//#define WLAN_SSID "MX SP"
//#define WLAN_PASS "ssot1178"
//#define WLAN_SSID       "1@southern park"
//#define WLAN_PASS       "0166606322"
//#define WLAN_SSID         "HUAWEI P10 lite"
//#define WLAN_PASS         "c5943f26-c6f"

//#define WLAN_SSID         "Yes @su xin"
//#define WLAN_PASS         "0124878756"
#define MQTT_SERVER      "192.168.1.22" // give static address
#define MQTT_PORT         1883                    
#define MQTT_USERNAME    "" 
#define MQTT_PASSWORD         "" 
#define LPSO_W_SEL 13
#define LPSO_W_IN 9
#define LPSO_W_STATUS 14  //JP1
#define LPSO_W_CTRL 12     //JP2
#define MODE_1 1
#define MODE_2 3 
#define MODE_3 15
#define DTG_CA1_CTRL 2
#define DTG_CA2_CTRL 4
#define DTG_CA1_IN 5
#define DTG_CA2_IN 16
#define DTG_MTR_Y1 14     //JP1
#define DTG_MTR_R1 12      //JP2
#define DTG_MTR_Y2 10      //JP3
#define DEX_D2_STATUS 10   //JP3
#define DEX_D_IN 11   //JP6
#define LPSO_D_IN 11  //JP6
#define DEX_W_IN 11   //JP6
#define DTG_MTR_R3 11 //JP6
#define DEX_D1_STATUS 7  //JP5
#define DEX_W_STATUS 7    //JP5
#define LPSO_D_STATUS 7   //JP5
#define DTG_MTR_Y3 7      //JP5
#define LPSO_D_CTRL 6     //JP4
#define DEX_D_CTRL  6     //JP4
#define DEX_W_CTRL  6     //JP4



/////////////////////////////////
////// MQTT SETUP ///////////////
/////////////////////////////////

// Create an ESP8266 WiFiClient class to connect to the MQTT server. 
WiFiClient client; 
 //Setup the MQTT client class by passing in the WiFi client and MQTT server and login details. 
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD); 
Adafruit_MQTT_Publish lastwill = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "connectivity/6786f3831d7a750bac397d8967b81044");

/****************************** Feeds ***************************************/ 
 

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname> 
Adafruit_MQTT_Publish doorLock = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "Lock/6786f3831d7a750bac397d8967b81044");
Adafruit_MQTT_Publish coinIn = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "coinIn/6786f3831d7a750bac397d8967b81044");
// Setup a feed called 'esp8266_led' for subscribing to changes. 
Adafruit_MQTT_Publish mqttConnected = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "connected/6786f3831d7a750bac397d8967b81044"); 
Adafruit_MQTT_Subscribe receivedPayment = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "6786f3831d7a750bac397d8967b81044");
/*************************** Sketch Code ************************************/ 

////////////////////////////////////
//////// TIME AND INTERRUPT ////////
////////////////////////////////////
// start the timer for the heartbeat 
os_timer_t myHeartBeat;

void timerCallback(void *pArg) {
  Serial.print("timer is functioning\b");
  lastwill.publish("ON");
}
void timerInit(void) {
  os_timer_setfn(&myHeartBeat, timerCallback, NULL);
  os_timer_arm(&myHeartBeat, 30000, true);
}

int mainCounter = 0;
int Counter = 0;
int lockState = HIGH;
int lastLockState = HIGH;
char myMode = ' ';
void handleInterrupt() {
    Serial.print("One coin inserted\n");
    coinIn.publish("1COIN");
    //delay(3000);
}

///////////////////////////////////
///////// INITIALISATION //////////
///////////////////////////////////

void setup() {
 pinMode(STATUS_PIN, INPUT_PULLUP);
 pinMode(COIN_INPIN, INPUT);
 attachInterrupt(digitalPinToInterrupt(COIN_INPIN), handleInterrupt, FALLING);
 pinMode(COIN_OUTPIN, OUTPUT);
 pinMode(SEL_PIN, OUTPUT);
 pinMode(LED_BUILTIN, OUTPUT);
 Serial.begin(115200); 
 delay(1000);
 // always need to select low for the coin acceptor.
 digitalWrite(SEL_PIN, LOW);
 digitalWrite(COIN_OUTPIN, LOW); 
 digitalWrite(LED_BUILTIN, HIGH);
 // Read the mode of the hardware //
 digitalRead(MODE_1);
 digitalRead(MODE_2);
 digitalRead(MODE_3);
 if (MODE_1 == LOW && MODE_2 == LOW && MODE_3 == LOW) {
    myMode = 'lyso_washer';
 } else if (MODE_1 == LOW && MODE_2 == LOW && MODE_3 == HIGH) {
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
 mqtt.will(MQTT_USERNAME "connectivity/6786f3831d7a750bac397d8967b81044", "OFF");
 timerInit();
}
int counter = 0;
int checkOne=0;
int SendOne = 0;
int moneyR = 0;

///////////////////////////////////
/////// MAIN LOOP /////////////////
///////////////////////////////////

void loop() { 
 // Ensure the connection to the MQTT server is alive (this will make the first 
 // connection and automatically reconnect when disconnected).  See the MQTT_connect 
 
  MQTT_connect();
 mainCounter++;
 if (mainCounter > 1000) {
    lockState =  digitalRead(STATUS_PIN);
    mainCounter = 0;
 }
 if (lockState != lastLockState) {
    Counter++;
//  Serial.print("enter state 1");
    if(SendOne <= 5) {
      if(Counter > 2000){
        Serial.print("I found interrupt lur\n");
        doorLock.publish("Locked");
        Counter = 0;
        SendOne++;
      }
    }
    checkOne = 0;
 } else if (lockState == HIGH){
    if (checkOne == 0) {
      doorLock.publish("Unlocked");
      checkOne = 1;
      SendOne = 0;
    }
 }

 
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
         payMachine_NC(amount);
       }
     } 
    }
 }
} 

//////////////////////////////////
////// FUNCTIONS /////////////////
//////////////////////////////////

void MQTT_connect() { 
 int8_t ret; 
 int mycount;
 mycount = 0;
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
        //basically die and wait for WDT to reset me 
        while (1); 
     } 
 }
 Serial.println("MQTT Connected!");
 //lastwill.publish("ON");
} 

void payMachine_NC (int amt) {
  moneyR = moneyR + amt;
  if (amt <= 9) {
    digitalWrite(SEL_PIN,HIGH);
    for (int i=1; i <= amt; i++){
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(COIN_OUTPIN, LOW);
      delay(10);
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(COIN_OUTPIN, HIGH); 
      delay(10);
    }
    digitalWrite(SEL_PIN,LOW);
    Serial.print((int)moneyR);
    Serial.print("\n");
  }
}

void payMachine_NO (int amt) {
  moneyR = moneyR + amt;
  if (amt <= 9) {
    digitalWrite(SEL_PIN,HIGH);
    for (int i=1; i <= amt; i++){
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(COIN_OUTPIN, HIGH);
      delay(50);
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(COIN_OUTPIN, LOW); 
      delay(2000);
    }
    digitalWrite(SEL_PIN,LOW);
    Serial.print((int)moneyR);
    Serial.print("\n");
  }
}

boolean isValidNumber(String str){
  boolean isNum=false;
  for(byte i=0;i<str.length();i++) {
    isNum = isDigit(str.charAt(i));
  }
  return isNum;
} 


