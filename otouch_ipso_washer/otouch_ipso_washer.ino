#include <ESP8266WiFi.h> 
#include <WiFiClient.h>
#include "Adafruit_MQTT.h" 
#include "Adafruit_MQTT_Client.h" 

extern "C" {
#include "user_interface.h"
}

/////////////////////////////////
////// WIFI SETTING /////////////
/////////////////////////////////

//#define WLAN_SSID       "Tenda_320CE0" 
//#define WLAN_PASS       "0105654525afk" 
//#define WLAN_SSID       "antlysis_meadow_2.4G@unifi"
//#define WLAN_PASS       "!Ath4ht@w4dt!"
//#define WLAN_SSID "MX SP"
//#define WLAN_PASS "ssot1178"
#define WLAN_SSID         "HUAWEI P10 lite"
#define WLAN_PASS         "c5943f26-c6f"

/////////////////////////////////
////// PIN SETUP ///////////////
/////////////////////////////////

#define LPSO_W_SEL 13
#define LPSO_W_IN 10
#define LPSO_W_STATUS 14   //JP1
#define LPSO_W_CTRL 12     //JP2
#define DTG_CA_CTRL 16     //JP3
#define DTG_CA1_IN 5       //JP6
#define DTG_CA2_IN 12      //JP2
#define DTG_MTR_SFT 14     //JP1
#define DTG_MTR_DTG 2      //JP2
#define DTG_MTR_BEG 4      //JP3
#define DEX_D2_STATUS 16   //JP3
#define DEX_D_IN 5         //JP6
#define LPSO_D_IN 5        //JP6
#define DEX_W_IN 5         //JP6
#define DEX_D1_STATUS 4    //JP5
#define DEX_W_STATUS 4     //JP5
#define LPSO_D_STATUS 4    //JP5
#define LPSO_D_CTRL 2      //JP4
#define DEX_D_CTRL  2      //JP4
#define DEX_W_CTRL  2      //JP4


/////////////////////////////////
////// MQTT SETUP ///////////////
/////////////////////////////////

#define MQTT_SERVER      "192.168.43.141" // give static address
#define MQTT_PORT         1883                    
#define MQTT_USERNAME    "" 
#define MQTT_PASSWORD         "" 

// Create an ESP8266 WiFiClient class to connect to the MQTT server. 
WiFiClient client; 
 //Setup the MQTT client class by passing in the WiFi client and MQTT server and login details. 
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD); 
Adafruit_MQTT_Publish lastwill = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "connectivity/6786f3831d7a750bac397d8967b81044");

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname> 
Adafruit_MQTT_Publish runStatus = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "Lock/6786f3831d7a750bac397d8967b81044");
Adafruit_MQTT_Publish coinIn = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "coinIn/6786f3831d7a750bac397d8967b81044");
// Setup a feed called 'esp8266_led' for subscribing to changes. 
Adafruit_MQTT_Subscribe receivedPayment = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "6786f3831d7a750bac397d8967b81044");


////////////////////////////////////
//////// TIME AND INTERRUPT ////////
////////////////////////////////////
// start the timer for the heartbeat 
os_timer_t myHeartBeat;

void timerCallback(void *pArg) {
  //Serial.print("timer is functioning\b");
  lastwill.publish("ON");
}
void timerInit(void) {
  os_timer_setfn(&myHeartBeat, timerCallback, NULL);
  os_timer_arm(&myHeartBeat, 30000, true);
}


///////////////////////////////////
///////// INITIALISATION //////////
///////////////////////////////////

void setup() {
 pinMode(LPSO_W_SEL, OUTPUT);
 pinMode(LPSO_W_IN, INPUT);
 pinMode(LPSO_W_STATUS, INPUT);
 pinMode(LPSO_W_CTRL, OUTPUT);
 pinMode(DEX_D_CTRL, OUTPUT);
 pinMode(LED_BUILTIN, OUTPUT);
 //blink(3);
 Serial.begin(115200); 
 delay(1000);
 // always need to select low for the coin acceptor.
 //digitalWrite(SEL_PIN, LOW);
 //digitalWrite(COIN_OUTPIN, LOW); 
 digitalWrite(LED_BUILTIN, HIGH);
 // Read the mode of the hardware //
 
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
int lockCounter = 0;
int lockCount = 0;
int coin_input = 0;
int coinCounter = 0;
int unlockCounter = 0;
int high = 0;
int lockState = HIGH;

///////////////////////////////////
/////// MAIN LOOP /////////////////
///////////////////////////////////

void loop() { 
 // Ensure the connection to the MQTT server is alive (this will make the first 
 // connection and automatically reconnect when disconnected).  See the MQTT_connect 
 
 MQTT_connect();
 lockState =  digitalRead(LPSO_W_STATUS);
 coin_input = digitalRead(LPSO_W_IN);

 if (high == 0) {
  if(coin_input == LOW) {
    coinCounter++;
    if (coinCounter > 5) {
      high = 1;
      coinCounter = 0;
      //Serial.print("One coin inserted\n");
      //coinIn.publish("1COIN");
    }
  } else {
    high = 0;
  }
 } else if (high == 1) {
   if (coin_input == HIGH) {
     coinCounter++;
     if (coinCounter >20) {
       high = 0;
       coinIn.publish("1COIN");  
     } 
   } else {
     high = 1;
   }
 }

 if (lockState == LOW) {
  if (SendOne <= 5) {
    lockCounter++;
    //Serial.print("enter state 1");
    if(lockCounter > 700){
      //Serial.print("Locked");
      runStatus.publish("Locked");
      lockCounter = 0;
      SendOne++;
      checkOne = 0;
      unlockCounter = 0;
    }
   }
 } else if (lockState == HIGH){
    if (checkOne <= 2) {
      unlockCounter++;
      if (unlockCounter > 50){
        //Serial.print("Unlocked");
        runStatus.publish("Unlocked");
        //payMachine_NC(3);
        checkOne++;
        SendOne = 0;
        unlockCounter = 0;
        
      }
      lockCounter = 0;
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
  int ret;
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
  if (amt <= 9) {
    digitalWrite(LPSO_W_SEL,HIGH);
    for (int i=1; i <= amt; i++){
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(LPSO_W_CTRL, LOW);
      delay(50);
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LPSO_W_CTRL, HIGH); 
      delay(2000);
    }
    digitalWrite(LPSO_W_SEL,LOW);
  }
}

void payMachine_NO (int amt) {
  if (amt <= 9) {
    for (int i=1; i <= amt; i++){
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(DEX_D_CTRL, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(DEX_D_CTRL, LOW); 
      delay(2000);
    }
  }
}

void blink (int times) {
  for (int i=1; i <= times; i++){
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(2000); 
    }
}

boolean isValidNumber(String str){
  boolean isNum=false;
  for(byte i=0;i<str.length();i++) {
    isNum = isDigit(str.charAt(i));
  }
  return isNum;
} 

