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

/****************************** Feeds ***************************************/ 
 

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname> 
Adafruit_MQTT_Publish doorLock = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "Lock/6786f3831d7a750bac397d8967b81044");
Adafruit_MQTT_Publish coinIn = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "coinIn/6786f3831d7a750bac397d8967b81044");
// Setup a feed called 'esp8266_led' for subscribing to changes. 
Adafruit_MQTT_Subscribe receivedPayment = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "6786f3831d7a750bac397d8967b81044");
/*************************** Sketch Code ************************************/ 

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

 pinMode(DTG_CA1_IN, INPUT);
 pinMode(DTG_CA_CTRL, OUTPUT);
 pinMode(DTG_CA2_IN, INPUT);
 pinMode(DTG_MTR_SFT, INPUT);
 pinMode(DTG_MTR_DTG, INPUT);
 pinMode(DTG_MTR_BEG, INPUT);
 pinMode(LED_BUILTIN, OUTPUT);
 //blink(3);
 //Serial.begin(115200); 
 delay(1000);
 // always need to select low for the coin acceptor.
 //digitalWrite(SEL_PIN, LOW);
 //digitalWrite(COIN_OUTPIN, LOW); 
 digitalWrite(LED_BUILTIN, HIGH);
 
 
 //Serial.println();
 //Serial.print("Connecting to "); 
 //Serial.println(WLAN_SSID); 
 WiFi.begin(WLAN_SSID, WLAN_PASS); 
 while (WiFi.status() != WL_CONNECTED) { 
   delay(500); 
   //Serial.print("."); 
 } 
 //Serial.println(); 
 //Serial.println("WiFi connected"); 
 //Serial.println("IP address: "); Serial.println(WiFi.localIP());
 // Setup MQTT subscription for esp8266_led feed. 
 mqtt.subscribe(&receivedPayment);
 mqtt.will(MQTT_USERNAME "connectivity/6786f3831d7a750bac397d8967b81044", "OFF");
 timerInit();
}
int sftnrCounter = 0;
int sftnrDrop = 0;
int turn_sft = 0;
int dtgCounter = 0;
int dtgDrop = 0;
int turn_dtg = 0;
int begCounter = 0;
int begDrop = 0;
int turn_beg = 0;
int coin_input1 = 0;
int coin_input2 = 0;
int coinCounter1 = 0;
int coinCounter2 = 0;
int high1 = 0;
int high2 = 0;


///////////////////////////////////
/////// MAIN LOOP /////////////////
///////////////////////////////////

void loop() { 
 // Ensure the connection to the MQTT server is alive (this will make the first 
 // connection and automatically reconnect when disconnected).  See the MQTT_connect 
 
 MQTT_connect();
 sftnrDrop =  digitalRead(DTG_MTR_SFT);
 dtgDrop = digitalRead(DTG_MTR_DTG);
 begDrop = digitalRead(DTG_MTR_BEG);
 coin_input1 = digitalRead(DTG_CA1_IN);
 coin_input2 = digitalRead(DTG_CA2_IN);

///////////////////////////////////
////// Coin Acceptor 1 ////////////
///////////////////////////////////

  if (high1 == 0) {
    if(coin_input1 == LOW) {
      coinCounter1++;
      if (coinCounter1 > 5) {
        high1 = 1;
        coinCounter1 = 0;
        //Serial.print("One coin inserted\n");
        //coinIn.publish("1COIN");
      }
    } else {
      high1 = 0;
    }
  } else if (high1 == 1) {
    if (coin_input1 == HIGH) {
      coinCounter1++;
      if (coinCounter1 >20) {
        high1 = 0;
        coinIn.publish("1COIN");  
      } 
    } else {
      high1 = 1;
    }
  }

///////////////////////////////////
////// Coin Acceptor 2 ////////////
///////////////////////////////////


  if (high2 == 0) {
    if(coin_input2 == LOW) {
      coinCounter2++;
      if (coinCounter2 > 5) {
        high2 = 1;
        coinCounter2 = 0;
      //Serial.print("One coin inserted\n");
      //coinIn.publish("1COIN");
      }
    } else {
      high2 = 0;
    }
  } else if (high2 == 1) {
    if (coin_input2 == HIGH) {
      coinCounter2++;
      if (coinCounter2 >20) {
        high2 = 0;
        coinIn.publish("1COIN");  
      } 
    } else {
      high2 = 1;
    }
  }

////////////////////////////////
////// Softener motor turning //
////////////////////////////////

  if (turn_sft == 0) {
    if(sftnrDrop == HIGH) {
      sftnrCounter++;
      if (sftnrCounter > 8) {
        turn_sft = 1;
        sftnrCounter = 0;
      }
    } else {
      turn_sft = 0;
    }
  } else if (turn_sft == 1) {
    if (sftnrDrop == LOW) {
      sftnrCounter++;
      if (sftnrCounter >20) {
        turn_sft = 0;
        coinIn.publish("SFTNR_DROP");  
      } 
    } else {
      turn_sft = 1;
    }
  }

/////////////////////////////////
//// Detergent motor turning ////
/////////////////////////////////

  if (turn_dtg == 0) {
    if(dtgDrop == HIGH) {
      dtgCounter++;
      if (dtgCounter > 8) {
        turn_dtg = 1;
        dtgCounter = 0;
      }
    } else {
      turn_dtg = 0;
    }
  } else if (turn_dtg == 1) {
    if (dtgDrop == LOW) {
      dtgCounter++;
      if (dtgCounter >20) {
        turn_dtg = 0;
        coinIn.publish("DTG_DROP");  
      } 
    } else {
      turn_dtg = 1;
    }
  }

//////////////////////////////////
///// Beg Motor turning //////////
//////////////////////////////////

  if (turn_beg == 0) {
    if(begDrop == HIGH) {
      begCounter++;
      if (begCounter > 8) {
        turn_beg = 1;
        begCounter = 0;
      }
    } else {
      turn_beg = 0;
    }
  } else if (turn_beg == 1) {
    if (begDrop == LOW) {
      begCounter++;
      if (begCounter >20) {
        turn_beg = 0;
        coinIn.publish("BEG_DROP");  
      } 
    } else {
      turn_beg = 1;
    }
  }

//////////////////////////////////////
////// Receive E-payment /////////////
//////////////////////////////////////

 Adafruit_MQTT_Subscribe *subscription; 
 while ((subscription = mqtt.readSubscription())) { 
   if (subscription == &receivedPayment) {
     //Serial.println("test124");
     char *message = (char *)receivedPayment.lastread; 
     float to_float = atof(message);
     int amount = int(to_float);
     //Serial.print(F("Got: ")); 
     //Serial.println(message);
     
     // Check if the message was a number  
     if (isValidNumber(message) == true) { 
       if (amount <= 10) { 
         payMachine_NO(amount);
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
 //Serial.print("Connecting to MQTT... "); 
 uint8_t retries = 3; 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected 
    //Serial.println(mqtt.connectErrorString(ret)); 
    //Serial.println("Retrying MQTT connection in 5 seconds..."); 
    mqtt.disconnect(); 
    delay(5000);  // wait 5 seconds 
    retries--; 
     if (retries == 0) { 
        //basically die and wait for WDT to reset me 
        while (1); 
     } 
 }
 //Serial.println("MQTT Connected!");
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
    //Serial.print((int)moneyR);
    //Serial.print("\n");
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
    //Serial.print((int)moneyR);
    //Serial.print("\n");
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


