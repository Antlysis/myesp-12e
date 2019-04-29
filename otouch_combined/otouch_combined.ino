#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "ESP8266HTTPClient.h"
#include "ESP8266httpUpdate.h"
#include "BigNumber.h"
#include <QueueList.h>

extern "C" {
#include "user_interface.h"
}

/////////////////////////////////
////// WIFI SETTING /////////////
/////////////////////////////////

//#define WLAN_SSID       "Tenda_320CE0"
//#define WLAN_PASS       "0105654525afk" //
//#define WLAN_SSID       "antlysis_meadow_2.4G@unifi"
//#define WLAN_PASS       "!Ath4ht@w4dt!"
#define WLAN_SSID "MX PJ21"
#define WLAN_PASS "ssot1178"
//#define WLAN_SSID "MX SP"
//#define WLAN_PASS "ssot1178"
//#define WLAN_SSID "Dobidoo"
//#define WLAN_PASS "dobidoo123"
//#define WLAN_SSID "MX"
//#define WLAN_PASS "asdf7817"

//#define WLAN_SSID         "James's iPhone"
//#define WLAN_PASS         "ekufi3rewtbvh"
//#define WLAN_SSID         "HUAWEI P10 lite"
//#define WLAN_PASS         "c5943f26-c6f"

/////////////////////////////////
////// PIN SETUP ///////////////
/////////////////////////////////
#define BILL_IN 2
#define BILL_CTRL 10
#define IPSO_W_IN 10    //JP8
#define IPSO_W_STATUS 14   //JP1
#define IPSO_W_CTRL 12     //JP2
#define DTG_CA_CTRL 5     //JP6
#define DTG_CA1_IN 10       //JP8
#define DTG_CA2_IN 12      //JP2
#define DTG_MTR_BEG 14     //JP1 (first two)
#define DTG_MTR_SFT 16      //JP3 (second two)
#define DTG_MTR_DTG 4      //JP3 (third two)
#define DEX_D2_STATUS 16   //JP3
#define DEX_D_IN 5         //JP6
#define IPSO_D_IN 5        //JP6
#define DEX_W_IN 5         //JP6
#define DEX_D1_STATUS 4    //JP5
#define DEX_W_STATUS 4     //JP5
#define IPSO_D_STATUS 4    //JP5
#define IPSO_D_CTRL 2      //JP4
#define DEX_D_CTRL  2      //JP4
#define DEX_W_CTRL  2      //JP4     


/////////////////////////////////
////// MQTT SETUP ///////////////
/////////////////////////////////

#define MQTT_SERVER      "192.168.8.112" // give static address
#define MQTT_PORT         1883
#define MQTT_USERNAME    ""
#define MQTT_PASSWORD         ""

/////////////////////////////////
/////// OTA setting /////////////
/////////////////////////////////

const int FW_VERSION = 18;
const char* fwUrlBase = "http://192.168.8.112/fw/";

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
//Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
Adafruit_MQTT_Publish lastwill = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "connectivity/9b0d3c6dc579ed74ce4fd344d5d1e5fa");

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish runStatus = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "Lock/9b0d3c6dc579ed74ce4fd344d5d1e5fa");
Adafruit_MQTT_Publish detStatus = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "detDrop/9b0d3c6dc579ed74ce4fd344d5d1e5fa");
Adafruit_MQTT_Publish coinIn = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "coinIn/9b0d3c6dc579ed74ce4fd344d5d1e5fa");
Adafruit_MQTT_Publish versFeed = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "versionFeed/9b0d3c6dc579ed74ce4fd344d5d1e5fa");
// Setup a feed called 'esp8266_led' for subscribing to changes.
Adafruit_MQTT_Subscribe receivedPayment = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "9b0d3c6dc579ed74ce4fd344d5d1e5fa");
Adafruit_MQTT_Subscribe firmwareUpdate = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "firmwareUpdate");
Adafruit_MQTT_Subscribe timeUpdate = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "timeUpdate");
////////////////////////////////////
//////// SOME VARIABLE INIT ////////
////////////////////////////////////

unsigned int sftnrCounter = 0;
unsigned int sftnrDrop = 0;
unsigned int turn_sft = 0;
unsigned int dtgCounter = 0;
unsigned int dtgDrop = 0;
unsigned int turn_dtg = 0;
unsigned int begCounter = 0;
unsigned int begDrop = 0;
unsigned int turn_beg = 0;
unsigned int coin_input1 = 0;
unsigned int coin_input2 = 0;
unsigned int bill_input = 0;
unsigned int billCounter = 0;
unsigned int lockCounter1 = 0;
unsigned int lockCounter2 = 0;
unsigned int lockState1 = 0;
unsigned int lockState2 = 0;
unsigned int coinH = 0;
unsigned int coinL = 0;
unsigned int coinCounter1 = 0;
unsigned int coinRCV = 0;
unsigned int unlockCounter1 = 0;
unsigned int coinCounter2 = 0;
unsigned int unlockCounter2 = 0;
unsigned int high1 = 0;
unsigned int high2 = 0;
unsigned int billhigh1 = 0;
unsigned int checkOne1 = 0;
unsigned int SendOne1 = 0;
unsigned int checkOne2 = 0;
unsigned int SendOne2 = 0;
unsigned int noRun = 0;
BigNumber actTime = 0;
BigNumber startMillis = 0;
BigNumber currMillis = 0;
BigNumber actTime2 = 0;
BigNumber startMillis2 = 0;
BigNumber currMillis2 = 0;
boolean cneted = true;
unsigned int start = 0;
unsigned long previousMillis = 0;
const long interval = 5000;
// ipsoWasher | ipsoDryer | dexWasher | dexDryerSingle | dexDryerDouble | detergent | billAcceptor
String machineType = String("ipsoDryer");
QueueList <String> queue;
////////////////////////////////////
//////// TIME AND INTERRUPT ////////
////////////////////////////////////
// start the timer for the heartbeat
os_timer_t myHeartBeat;
os_timer_t IOReadout;

void timerCallback(void *pArg) {
  //////Serial.print("timer is functioning\b");
  lastwill.publish("ON");
}
void IOtimerISR(void *pArg) {
  //////Serial.print("timer is functioning\b");
  if (machineType == "ipsoWasher") {
    lockState1 =  digitalRead(IPSO_W_STATUS);
    coin_input1 = digitalRead(IPSO_W_IN);
    coinCount(coin_input1, &coinCounter1, &high1, &start, &coinRCV);
  } else if (machineType == "ipsoDryer") {
    lockState1 =  digitalRead(IPSO_D_STATUS);
    coin_input1 = digitalRead(IPSO_D_IN);
    coinCount(coin_input1, &coinCounter1, &high1, &start, &coinRCV);
  } else if (machineType == "dexWasher") {
    lockState1 =  digitalRead(DEX_W_STATUS);
    coin_input1 = digitalRead(DEX_W_IN);
    coinCountDex(coin_input1, &coinCounter1, &high1, &start, &coinRCV);
  } else if (machineType == "dexDryerSingle") {
    lockState1 =  digitalRead(DEX_D1_STATUS);
    coin_input1 = digitalRead(DEX_D_IN);
    coinCountDex(coin_input1, &coinCounter1, &high1, &start, &coinRCV);
  } else if (machineType == "dexDryerDouble") {
    lockState1 =  digitalRead(DEX_D1_STATUS);
    lockState2 =  digitalRead(DEX_D2_STATUS);
    coin_input1 = digitalRead(DEX_D_IN);
    coinCountDex(coin_input1, &coinCounter1, &high1, &start, &coinRCV);
  }

}

//void ShiftRegisterISR(void *pArg) {
//   /* Read in first 74HC165 data. */
//   register_value = shift_in();
//   Serial.print(register_value, BIN);
//   Serial.println(" ");
//}

void timerInit(void) {
  os_timer_setfn(&myHeartBeat, timerCallback, NULL);
  os_timer_arm(&myHeartBeat, 30000, true);

  os_timer_setfn(&IOReadout, IOtimerISR, NULL);
  os_timer_arm(&IOReadout, 5, true);

  //os_timer_setfn(&ShiftRegister, ShiftRegisterISR, NULL);
  //os_timer_arm(&ShiftRegister, 15, true);
}



///////////////////////////////////
///////// INITIALISATION //////////
///////////////////////////////////


void setup() {
  String mac;
  if (machineType == "ipsoWasher") {
    pinMode(IPSO_W_IN, INPUT);
    pinMode(IPSO_W_STATUS, INPUT);
    pinMode(IPSO_W_CTRL, OUTPUT);
    digitalWrite(IPSO_W_CTRL, LOW);
    //digitalWrite(IPSO_W_IN, HIGH);
    delay(1000);
    //payIpsoW_NO(1);
  } else if (machineType == "ipsoDryer") {
    pinMode(IPSO_D_IN, INPUT);
    pinMode(IPSO_D_CTRL, OUTPUT);
    pinMode(IPSO_D_STATUS, INPUT);
    digitalWrite(IPSO_D_CTRL, HIGH);
    delay(1000);
    digitalWrite(IPSO_D_CTRL, LOW);
  } else if (machineType == "dexWasher") {
    pinMode(DEX_W_IN, INPUT);
    pinMode(DEX_W_CTRL, OUTPUT);
    pinMode(DEX_W_STATUS, INPUT);
    /// this is to avoid this small toggle create a credit to the machine
    digitalWrite(DEX_W_CTRL, HIGH);
    delay(1000);
    digitalWrite(DEX_W_CTRL, LOW);
  } else if (machineType == "dexDryerDouble") {
    pinMode(DEX_D_IN, INPUT);
    pinMode(DEX_D_CTRL, OUTPUT);
    pinMode(DEX_D1_STATUS, INPUT);
    pinMode(DEX_D2_STATUS, INPUT);
    /// this is to avoid this small toggle create a credit to the machine
    digitalWrite(DEX_D_CTRL, HIGH);
    delay(1000);
    digitalWrite(DEX_D_CTRL, LOW);
  } else if (machineType == "dexDryerSingle") {
    pinMode(DEX_D_IN, INPUT);
    pinMode(DEX_D_CTRL, OUTPUT);
    pinMode(DEX_D1_STATUS, INPUT);
    /// this is to avoid this small toggle create a credit to the machine
    digitalWrite(DEX_D_CTRL, HIGH);
    delay(1000);
    digitalWrite(DEX_D_CTRL, LOW);
  } else if (machineType == "detergent") {
    pinMode(DTG_CA1_IN, INPUT);
    pinMode(DTG_CA_CTRL, OUTPUT);
    pinMode(DTG_CA2_IN, INPUT);
    pinMode(DTG_MTR_SFT, INPUT);
    pinMode(DTG_MTR_DTG, INPUT);
    pinMode(DTG_MTR_BEG, INPUT);
    digitalWrite(DTG_CA_CTRL, LOW);
    //delay(2000);
    //digitalWrite(DTG_CA_CTRL,LOW);
  } else if (machineType == "billAcceptor") {
    pinMode(BILL_IN, INPUT);
    pinMode(BILL_CTRL, OUTPUT);
    digitalWrite(BILL_CTRL, LOW);
  }
  Serial.begin(115200);
  //digitalWrite(LED_BUILTIN, HIGH);
  //////Serial.println();
  //////Serial.print("Connecting to ");
  //////Serial.println(WLAN_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //////Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); ////Serial.println(WiFi.localIP());
  mqtt.subscribe(&receivedPayment);
  mqtt.subscribe(&firmwareUpdate);
  mqtt.subscribe(&timeUpdate);
  mqtt.will(MQTT_USERNAME "connectivity/9b0d3c6dc579ed74ce4fd344d5d1e5fa", "OFF");
  timerInit();
  mac = getMAC();
  Serial.println (mac);
}



///////////////////////////////////
/////// MAIN LOOP /////////////////
///////////////////////////////////
int counter = 0;
boolean donefw = false;
String mac;
void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  MQTT_connect(&cneted);
  if (!donefw && cneted) {
    versFeed.publish(FW_VERSION);
    donefw = true;
    mac = getMAC();
    Serial.println (mac);
  }
  if (queue.isEmpty()) {
    //startMillis = millis();
    //Serial.println("queue is empty");
  } else {
    //Serial.println("queue is not empty");
    if (cneted) {
      char send[30];
      queue.pop().toCharArray(send, 30);
      if (machineType == "detergent") {
        detStatus.publish(send);
      } else {
        runStatus.publish(send);
      }
    } else {
      //Serial.println("mqtt_disconnected");
    }
  }
  Adafruit_MQTT_Subscribe *subscription;
  if (machineType == "ipsoWasher") {
    washerStatusRead(lockState1, &SendOne1, &checkOne1, &lockCounter1, &unlockCounter1, &coinRCV);
    while ((subscription = mqtt.readSubscription())) {
      if (subscription == &receivedPayment) {
        char *message = (char *)receivedPayment.lastread;
        float to_float = atof(message);
        int amount = int(to_float);
        if (isValidNumber(message) == true) {
          payIpsoW_NO(amount);
        }
      } else if (subscription == &firmwareUpdate) {
        //////Serial.println("receive firmware update request");
        char *message = (char *)firmwareUpdate.lastread;
        //////Serial.print(F("Got: "));
        ////Serial.println(message);
        // Check if the message was ON, OFF, or TOGGLE.
        if (strncmp(message, "update", 6) == 0) {
          ////Serial.println("ya, it is confirmed to update the firmware");
          checkForUpdates();
        }
      } else if (subscription == &timeUpdate) {
        //Serial.println("receive time update request");
        char *message = (char *)timeUpdate.lastread;
        actTime = message;
        startMillis = millis();
      }
    }
  } else if (machineType == "ipsoDryer") {
    ipsoDryerStatusRead(lockState1, &SendOne1, &checkOne1, &lockCounter1, &unlockCounter1, &coinRCV);
    while ((subscription = mqtt.readSubscription())) {
      if (subscription == &receivedPayment) {
        char *message = (char *)receivedPayment.lastread;
        float to_float = atof(message);
        int amount = int(to_float);
        if (isValidNumber(message) == true) {
          payIpsoD_NO(amount);
        }
      } else if (subscription == &firmwareUpdate) {
        ////Serial.println("receive firmware update request");
        char *message = (char *)firmwareUpdate.lastread;
        ////Serial.print(F("Got: "));
        ////Serial.println(message);
        // Check if the message was ON, OFF, or TOGGLE.
        if (strncmp(message, "update", 6) == 0) {
          ////Serial.println("ya, it is confirmed to update the firmware");
          checkForUpdates();
        }
      } else if (subscription == &timeUpdate) {
        //Serial.println("receive time update request");
        char *message = (char *)timeUpdate.lastread;
        actTime = message;
        startMillis = millis();
      }
    }
  } else if (machineType == "dexWasher") {
    dexwasherStatusRead(lockState1, &SendOne1, &checkOne1, &lockCounter1, &unlockCounter1, &coinRCV);
    while ((subscription = mqtt.readSubscription())) {
      if (subscription == &receivedPayment) {
        char *message = (char *)receivedPayment.lastread;
        float to_float = atof(message);
        int amount = int(to_float);
        if (isValidNumber(message) == true) {
          payDexW_NO(amount);
        }
      } else if (subscription == &firmwareUpdate) {
        ////Serial.println("receive firmware update request");
        char *message = (char *)firmwareUpdate.lastread;
        ////Serial.print(F("Got: "));
        ////Serial.println(message);
        // Check if the message was ON, OFF, or TOGGLE.
        if (strncmp(message, "update", 6) == 0) {
          ////Serial.println("ya, it is confirmed to update the firmware");
          checkForUpdates();
        }
      } else if (subscription == &timeUpdate) {
        //Serial.println("receive time update request");
        char *message = (char *)timeUpdate.lastread;
        actTime = message;
        startMillis = millis();
      }
    }
  } else if (machineType == "dexDryerSingle") {
    dexdryerStatusReadSingle(lockState1, &SendOne1, &checkOne1, &lockCounter1, &unlockCounter1, &coinRCV);
    while ((subscription = mqtt.readSubscription())) {
      if (subscription == &receivedPayment) {
        ////Serial.println("received payment");
        char *message = (char *)receivedPayment.lastread;
        float to_float = atof(message);
        int amount = int(to_float);
        if (isValidNumber(message) == true) {
          payDexD_NO(amount);
        }
      } else if (subscription == &firmwareUpdate) {
        ////Serial.println("receive firmware update request");
        char *message = (char *)firmwareUpdate.lastread;
        ////Serial.print(F("Got: "));
        ////Serial.println(message);
        // Check if the message was ON, OFF, or TOGGLE.
        if (strncmp(message, "update", 6) == 0) {
          ////Serial.println("ya, it is confirmed to update the firmware");
          checkForUpdates();
        }
      } else if (subscription == &timeUpdate) {
        //Serial.println("receive time update request");
        char *message = (char *)timeUpdate.lastread;
        actTime = message;
        startMillis = millis();
      }
    }
  } else if (machineType == "dexDryerDouble") {
    dexdryerStatusReadDouble(lockState1, lockState2, &SendOne1, &SendOne2, &checkOne1, &checkOne2, &lockCounter1, &lockCounter2, &unlockCounter1, &unlockCounter2, &coinRCV);
    while ((subscription = mqtt.readSubscription())) {
      if (subscription == &receivedPayment) {
        char *message = (char *)receivedPayment.lastread;
        float to_float = atof(message);
        int amount = int(to_float);
        if (isValidNumber(message) == true) {
          payDexD_NO(amount);
        }
      } else if (subscription == &firmwareUpdate) {
        //Serial.println("receive firmware update request");
        char *message = (char *)firmwareUpdate.lastread;
        //Serial.print(F("Got: "));
        //Serial.println(message);
        // Check if the message was ON, OFF, or TOGGLE.
        if (strncmp(message, "update", 6) == 0) {
          //Serial.println("ya, it is confirmed to update the firmware");
          checkForUpdates();
        }
      } else if (subscription == &timeUpdate) {
        //Serial.println("receive time update request");
        char *message = (char *)timeUpdate.lastread;
        actTime = message;
        startMillis = millis();
        actTime2 = message;
        startMillis2 = millis();
      }
    }
  } else if (machineType == "detergent") {
    sftnrDrop =  digitalRead(DTG_MTR_SFT);
    dtgDrop = digitalRead(DTG_MTR_DTG);
    begDrop = digitalRead(DTG_MTR_BEG);
    coin_input1 = digitalRead(DTG_CA1_IN);
    coin_input2 = digitalRead(DTG_CA2_IN);
    //bill_input = digitalRead(BILL_IN);
    detergentCoinCount(coin_input1, coin_input2, &high1, &high2, &coinCounter1, &coinCounter2, &coinRCV);
    detergentReadStatus(&turn_sft, &turn_beg, &turn_dtg, sftnrDrop, dtgDrop, begDrop, &sftnrCounter, &begCounter, &dtgCounter, &coinRCV);
    //billInRead(bill_input, &billhigh1, &billCounter);
    while ((subscription = mqtt.readSubscription())) {
      if (subscription == &receivedPayment) {
        char *message = (char *)receivedPayment.lastread;
        float to_float = atof(message);
        int amount = int(to_float);
        if (isValidNumber(message) == true) {
          payDet_NO(amount);
        }
      } else if (subscription == &firmwareUpdate) {
        //Serial.println("receive firmware update request");
        char *message = (char *)firmwareUpdate.lastread;
        //Serial.print(F("Got: "));
        //Serial.println(message);
        // Check if the message was ON, OFF, or TOGGLE.
        if (strncmp(message, "update", 6) == 0) {
          //Serial.println("ya, it is confirmed to update the firmware");
          checkForUpdates();
        }
      } else if (subscription == &timeUpdate) {
        //Serial.println("receive time update request");
        char *message = (char *)timeUpdate.lastread;
        actTime = message;
        startMillis = millis();
      }
    }
  }
}

//////////////////////////////////
////// FUNCTIONS /////////////////
//////////////////////////////////

void MQTT_connect(boolean *cneted) {
  int8_t ret;
  int mycount;
  mycount = 0;
  // Stop if already connected.
  if (mqtt.connected()) {
    *cneted = true;
    ////Serial.println("MQTT Connected!");
    return;
  }
  *cneted = false;
  unsigned long currentMillis = millis();
  // Serial.print("Connecting to MQTT... ");
  if (currentMillis - previousMillis >= interval) {
    if ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
      //    Serial.println(mqtt.connectErrorString(ret));
      //    Serial.println("Retrying MQTT connection in 5 seconds...");
      mqtt.disconnect();
      previousMillis = currentMillis;
    }
  }
  //Serial.println("MQTT Connected!");
}

void payIpsoW_NO(int amt) {
  for (int i = 1; i <= amt; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(IPSO_W_CTRL, HIGH);
    delay(50);//50
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(IPSO_W_CTRL, LOW);
    delay(800);
  }
}

void payDexW_NO (int amt) {
  for (int i = 1; i <= amt; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(DEX_W_CTRL, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(DEX_W_CTRL, LOW);
    delay(800);
  }
}

void payDexD_NO (int amt) {
  for (int i = 1; i <= amt; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(DEX_D_CTRL, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(DEX_D_CTRL, LOW);
    delay(800);
  }
}

void payIpsoD_NO (int amt) {
  for (int i = 1; i <= amt; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(IPSO_D_CTRL, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(IPSO_D_CTRL, LOW);
    delay(800);
  }
}

void payDet_NO (int amt) {
  for (int i = 1; i <= amt; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(DTG_CA_CTRL, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(DTG_CA_CTRL, LOW);
    delay(800);
  }
}

void payBill_NO (int amt) {
  for (int i = 1; i <= amt; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(BILL_CTRL, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(BILL_CTRL, LOW);
    delay(800);
  }
}

void blink (int times) {
  for (int i = 1; i <= times; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(800);
  }
}

///////////////////////////////////
////// Coin Acceptor //////////////
///////////////////////////////////
void coinCountDex(unsigned int coin_input, unsigned int *coinHigh, unsigned int *high, unsigned int *coinLow, unsigned int *coin_rcv) {
  if (*high == 0) {
    if (coin_input == HIGH) {
      //Serial.print("coin high");
      (*coinHigh)++;
      //Serial.println(*coinHigh);
      if (*coinHigh > 3) {
        *coinHigh = 0;
        *high = 1;
      }
    } else {
      if (*coinLow > 40) {
        *coinHigh = 0;
      }
      *high = 0;
    }
  } else if (*high == 1) {
    if (coin_input == LOW) {
      //Serial.print("coin low");
      (*coinLow)++;
      //Serial.println(*coinLow);
      if (*coinLow > 40) {
        *coinHigh = 0;
        *coinLow = 0;
        *high = 0;
        (*coin_rcv)++;
      }
    } else {
      *high = 1;
    }
  }
}


void coinCount(unsigned int coin_input, unsigned int *coinHigh, unsigned int *high, unsigned int *coinLow, unsigned int *coin_rcv) {
  if (*high == 0) {
    if (coin_input == HIGH) {
      //Serial.print("coin high");
      (*coinHigh)++;
      //Serial.println(*coinHigh);
      if (*coinHigh > 3) {
        *coinHigh = 0;
        *high = 1;
      }
    } else {
      if (*coinLow > 40) {
        *coinHigh = 0;
      }
      *high = 0;
    }
  } else if (*high == 1) {
    if (coin_input == LOW) {
      //Serial.print("coin low");
      (*coinLow)++;
      //Serial.println(*coinLow);
      if (*coinLow > 40) {
        *coinHigh = 0;
        *coinLow = 0;
        *high = 0;
        (*coin_rcv)++;
      }
    } else {
      *high = 1;
    }
  }
}

void washerStatusRead(unsigned int lockState1, unsigned int *SendOne1, unsigned int *checkOne1, unsigned int *lockCounter1, unsigned int *unlockCounter1, unsigned int *coinRcv) {
  if (lockState1 == LOW) {
    //Serial.println("low");
    //Serial.println(*SendOne1);
    if (*SendOne1 <= 5) {
      (*lockCounter1)++;
      //Serial.println(*lockCounter1);
      if (*lockCounter1 > 700) {
        //Serial.println("Locked");
        //runStatus.publish("Locked");
        if (*SendOne1 == 3) {
          currMillis = millis();
          BigNumber diff = currMillis - startMillis;
          actTime = actTime + diff;
          startMillis = millis();
          String lock = "Locked_";
          String moneyrcv = String(*coinRcv);
          *coinRcv = 0;
          String dash = "_";
          lock += actTime.toString();
          lock += dash;
          lock += moneyrcv;
          queue.push(lock);
        }
        *lockCounter1 = 0;
        (*SendOne1)++;
        *checkOne1 = 0;
        *unlockCounter1 = 0;
      }
    }
  } else if (lockState1 == HIGH) {
    if (*checkOne1 <= 2) {
      (*unlockCounter1)++;
      if (*unlockCounter1 > 350) {
        //Serial.println("Unlocked");
        //runStatus.publish("Unlocked");
        if (*checkOne1 == 2) {
          currMillis = millis();
          BigNumber diff = currMillis - startMillis;
          actTime = actTime + diff;
          startMillis = millis();
          String lock = "Unlocked_";
          String moneyrcv = String(*coinRcv);
          *coinRcv = 0;
          String dash = "_";
          lock += actTime.toString();
          lock += dash;
          lock += moneyrcv;
          queue.push(lock);
          *SendOne1 = 0;
        }
        (*checkOne1)++;
        *unlockCounter1 = 0;
      }
      *lockCounter1 = 0;
    }
  }
}

void dexwasherStatusRead(unsigned int lockState1, unsigned int *SendOne1, unsigned int *checkOne1, unsigned int *lockCounter1, unsigned int *unlockCounter1, unsigned int *coinRcv) {
  if (lockState1 == HIGH) {
    //Serial.println("low");
    //Serial.println(*SendOne1);
    if (*SendOne1 <= 5) {
      (*lockCounter1)++;
      //Serial.println(*lockCounter1);
      if (*lockCounter1 > 700) {
        //Serial.println("Locked");
        //runStatus.publish("Locked");
        if (*SendOne1 == 3) {
          currMillis = millis();
          BigNumber diff = currMillis - startMillis;
          actTime = actTime + diff;
          startMillis = millis();
          String lock = "Locked_";
          String moneyrcv = String(*coinRcv);
          *coinRcv = 0;
          String dash = "_";
          lock += actTime.toString();
          lock += dash;
          lock += moneyrcv;
          queue.push(lock);
        }
        *lockCounter1 = 0;
        (*SendOne1)++;
        *checkOne1 = 0;
        *unlockCounter1 = 0;
      }
    }
  } else if (lockState1 == LOW) {
    if (*checkOne1 <= 2) {
      (*unlockCounter1)++;
      if (*unlockCounter1 > 350) {
        //Serial.println("Unlocked");
        //runStatus.publish("Unlocked");
        if (*checkOne1 == 2) {
          currMillis = millis();
          BigNumber diff = currMillis - startMillis;
          actTime = actTime + diff;
          startMillis = millis();
          String lock = "Unlocked_";
          String moneyrcv = String(*coinRcv);
          *coinRcv = 0;
          String dash = "_";
          lock += actTime.toString();
          lock += dash;
          lock += moneyrcv;
          queue.push(lock);
          *SendOne1 = 0;
        }
        (*checkOne1)++;
        *unlockCounter1 = 0;
      }
      *lockCounter1 = 0;
    }
  }
}

void ipsoDryerStatusRead(unsigned int lockState1,  unsigned int *SendOne1, unsigned int *checkOne1, unsigned int *lockCounter1, unsigned int *unlockCounter1, unsigned int *coinRcv) {
  if (lockState1 == HIGH) {
    if (*SendOne1 <= 5) {
      (*lockCounter1)++;
      if (*lockCounter1 > 700) {
        //Serial.println("Locked");
        //runStatus.publish("Locked");
        if (*SendOne1 == 3) {
          currMillis = millis();
          BigNumber diff = currMillis - startMillis;
          actTime = actTime + diff;
          startMillis = millis();
          String lock = "Locked_";
          String moneyrcv = String(*coinRcv);
          *coinRcv = 0;
          String dash = "_";
          lock += actTime.toString();
          lock += dash;
          lock += moneyrcv;
          queue.push(lock);
        }
        *lockCounter1 = 0;
        (*SendOne1)++;
        *checkOne1 = 0;
        *unlockCounter1 = 0;
      }
    }
  } else if (lockState1 == LOW) {
    if (*checkOne1 <= 2) {
      (*unlockCounter1)++;
      if (*unlockCounter1 > 350) {
        Serial.println("Unlocked");
        //runStatus.publish("Unlocked");
        if (*checkOne1 == 2) {
          currMillis = millis();
          BigNumber diff = currMillis - startMillis;
          actTime = actTime + diff;
          startMillis = millis();
          String lock = "Unlocked_";
          String moneyrcv = String(*coinRcv);
          *coinRcv = 0;
          String dash = "_";
          lock += actTime.toString();
          lock += dash;
          lock += moneyrcv;
          queue.push(lock);
          *SendOne1 = 0;
        }
        (*checkOne1)++;
        *unlockCounter1 = 0;
      }
      *lockCounter1 = 0;
    }
  }
}

void dexdryerStatusReadDouble(unsigned int lockState1, unsigned int lockState2, unsigned int *SendOne1, unsigned int *SendOne2, unsigned int *checkOne1, unsigned int *checkOne2, unsigned int *lockCounter1, unsigned int *lockCounter2, unsigned int *unlockCounter1, unsigned int *unlockCounter2, unsigned int *coinRcv) {
  if (lockState1 == LOW) {
    if (*SendOne1 <= 5) {
      (*lockCounter1)++;
      if (*lockCounter1 > 700) {
        //Serial.println("Locked");
        //runStatus.publish("Locked");
        if (*SendOne1 == 3) {
          currMillis = millis();
          BigNumber diff = currMillis - startMillis;
          actTime = actTime + diff;
          startMillis = millis();
          String lock = "Locked1_";
          String moneyrcv = String(*coinRcv);
          *coinRcv = 0;
          String dash = "_";
          lock += actTime.toString();
          lock += dash;
          lock += moneyrcv;
          queue.push(lock);
        }
        *lockCounter1 = 0;
        (*SendOne1)++;
        *checkOne1 = 0;
        *unlockCounter1 = 0;
      }
    }
  } else if (lockState1 == HIGH) {
    if (*checkOne1 <= 2) {
      (*unlockCounter1)++;
      if (*unlockCounter1 > 350) {
        //Serial.println("Unlocked");
        //runStatus.publish("Unlocked");
        if (*checkOne1 == 2) {
          currMillis = millis();
          BigNumber diff = currMillis - startMillis;
          actTime = actTime + diff;
          startMillis = millis();
          String lock = "Unlocked1_";
          String moneyrcv = String(*coinRcv);
          *coinRcv = 0;
          String dash = "_";
          lock += actTime.toString();
          lock += dash;
          lock += moneyrcv;
          queue.push(lock);
          *SendOne1 = 0;
        }
        (*checkOne1)++;
        *unlockCounter1 = 0;
      }
      *lockCounter1 = 0;
    }
  }
  if (lockState2 == LOW) {
    if (*SendOne2 <= 5) {
      (*lockCounter2)++;
      if (*lockCounter2 > 700) {
        ////Serial.print("Locked");
        if (*SendOne2 == 3) {
          currMillis2 = millis();
          BigNumber diff2 = currMillis2 - startMillis2;
          actTime2 = actTime2 + diff2;
          startMillis2 = millis();
          String lock = "Locked2_";
          String moneyrcv = String(*coinRcv);
          *coinRcv = 0;
          String dash = "_";
          lock += actTime.toString();
          lock += dash;
          lock += moneyrcv;
          queue.push(lock);
        }
        *lockCounter2 = 0;
        (*SendOne2)++;
        *checkOne2 = 0;
        *unlockCounter2 = 0;
      }
    }
  } else if (lockState2 == HIGH) {
    if (*checkOne2 <= 2) {
      (*unlockCounter2)++;
      if (*unlockCounter2 > 350) {
        ////Serial.print("Unlocked");
        if (*checkOne2 == 2) {
          currMillis2 = millis();
          BigNumber diff2 = currMillis2 - startMillis2;
          actTime2 = actTime2 + diff2;
          startMillis2 = millis();
          String lock = "Unlocked2_";
          String moneyrcv = String(*coinRcv);
          *coinRcv = 0;
          String dash = "_";
          lock += actTime.toString();
          lock += dash;
          lock += moneyrcv;
          queue.push(lock);
          *SendOne2 = 0;
        }
        (*checkOne2)++;
        *unlockCounter2 = 0;
      }
      *lockCounter2 = 0;
    }
  }
}

void dexdryerStatusReadSingle(unsigned int lockState1, unsigned int *SendOne1, unsigned int *checkOne1, unsigned int *lockCounter1, unsigned int *unlockCounter1, unsigned int *coinRcv) {
  if (lockState1 == LOW) {
    if (*SendOne1 <= 5) {
      (*lockCounter1)++;
      if (*lockCounter1 > 700) {
        ////Serial.print("Locked");
        if (*SendOne1 == 3) {
          currMillis = millis();
          BigNumber diff = currMillis - startMillis;
          actTime = actTime + diff;
          startMillis = millis();
          String lock = "Locked_";
          String moneyrcv = String(*coinRcv);
          *coinRcv = 0;
          String dash = "_";
          lock += actTime.toString();
          lock += dash;
          lock += moneyrcv;
          queue.push(lock);
        }
        *lockCounter1 = 0;
        (*SendOne1)++;
        *checkOne1 = 0;
        *unlockCounter1 = 0;
      }
    }
  } else if (lockState1 == HIGH) {
    if (*checkOne1 <= 2) {
      (*unlockCounter1)++;
      if (*unlockCounter1 > 350) {
        ////Serial.print("Unlocked");
        if (*checkOne1 == 2) {
          currMillis = millis();
          BigNumber diff = currMillis - startMillis;
          actTime = actTime + diff;
          startMillis = millis();
          String lock = "Unlocked_";
          String moneyrcv = String(*coinRcv);
          *coinRcv = 0;
          String dash = "_";
          lock += actTime.toString();
          lock += dash;
          lock += moneyrcv;
          queue.push(lock);
          *SendOne1 = 0;
        }
        (*checkOne1)++;
        *unlockCounter1 = 0;
      }
      *lockCounter1 = 0;
    }
  }
}


void detergentReadStatus(unsigned int *turn_sft, unsigned int *turn_beg, unsigned int *turn_dtg, unsigned int sftnrDrop, unsigned int dtgDrop, unsigned int begDrop, unsigned int *sftnrCounter, unsigned int *begCounter, unsigned int *dtgCounter, unsigned int *coinRcv) {
  ////////////////////////////////
  ////// Softener motor turning //
  ////////////////////////////////

  if (*turn_sft == 0) {
    if (sftnrDrop == HIGH) {
      (*sftnrCounter)++;
      if (*sftnrCounter > 3) {
        *turn_sft = 1;
        *sftnrCounter = 0;
      }
    } else {
      *turn_sft = 0;
    }
  } else if (*turn_sft == 1) {
    if (sftnrDrop == LOW) {
      (*sftnrCounter)++;
      if (*sftnrCounter > 20) {
        *turn_sft = 0;
        currMillis = millis();
        BigNumber diff = currMillis - startMillis;
        actTime = actTime + diff;
        startMillis = millis();
        String lock = "SFTNR_DROP_";
        String moneyrcv = String(*coinRcv);
        *coinRcv = 0;
        String dash = "_";
        lock += actTime.toString();
        lock += dash;
        lock += moneyrcv;
        queue.push(lock);

      }
    } else {
      *turn_sft = 1;
    }
  }

  /////////////////////////////////
  //// Detergent motor turning ////
  /////////////////////////////////

  if (*turn_dtg == 0) {
    if (dtgDrop == HIGH) {
      (*dtgCounter)++;
      if (*dtgCounter > 3) {
        *turn_dtg = 1;
        *dtgCounter = 0;
      }
    } else {
      *turn_dtg = 0;
    }
  } else if (*turn_dtg == 1) {
    if (dtgDrop == LOW) {
      (*dtgCounter)++;
      if (*dtgCounter > 20) {
        *turn_dtg = 0;
        currMillis = millis();
        BigNumber diff = currMillis - startMillis;
        actTime = actTime + diff;
        startMillis = millis();
        String lock = "DTG_DROP_";
        String moneyrcv = String(*coinRcv);
        *coinRcv = 0;
        String dash = "_";
        lock += actTime.toString();
        lock += dash;
        lock += moneyrcv;
        queue.push(lock);
      }
    } else {
      *turn_dtg = 1;
    }
  }

  //////////////////////////////////
  ///// Beg Motor turning //////////
  //////////////////////////////////

  if (*turn_beg == 0) {
    if (begDrop == HIGH) {
      (*begCounter)++;
      if (*begCounter > 3) {
        *turn_beg = 1;
        *begCounter = 0;
      }
    } else {
      *turn_beg = 0;
    }
  } else if (*turn_beg == 1) {
    if (begDrop == LOW) {
      //Serial.println("Got Beg");
      (*begCounter)++;
      if (*begCounter > 20) {
        *turn_beg = 0;
        currMillis = millis();
        BigNumber diff = currMillis - startMillis;
        actTime = actTime + diff;
        startMillis = millis();
        String lock = "BEG_DROP_";
        String moneyrcv = String(*coinRcv);
        *coinRcv = 0;
        String dash = "_";
        lock += actTime.toString();
        lock += dash;
        lock += moneyrcv;
        queue.push(lock);
        //detStatus.publish("BEG_DROP");

      }
    } else {
      *turn_beg = 1;
    }
  }

}

void detergentCoinCount(unsigned int coin_input1, unsigned int coin_input2, unsigned int *high1, unsigned int *high2, unsigned int *coinCounter1, unsigned int *coinCounter2, unsigned int *coin_rcv) {
  ///////////////////////////////////
  ////// Coin Acceptor 1 ////////////
  ///////////////////////////////////

  if (*high1 == 0) {
    if (coin_input1 == HIGH) {
      (*coinCounter1)++;
      if (*coinCounter1 > 3) {
        *high1 = 1;
        *coinCounter1 = 0;
        ////Serial.print("One coin inserted\n");
        //coinIn.publish("1COIN");
      }
    } else {
      *high1 = 0;
    }
  } else if (*high1 == 1) {
    if (coin_input1 == LOW) {
      (*coinCounter1)++;
      if (*coinCounter1 > 10) {
        *high1 = 0;
        ////Serial.print("One coin inserted CA1\n");
        //coinIn.publish("coin1");
        (*coin_rcv)++;
      }
    } else {
      *high1 = 1;
    }
  }

  ///////////////////////////////////
  ////// Coin Acceptor 2 ////////////
  ///////////////////////////////////


  if (*high2 == 0) {
    if (coin_input2 == HIGH) {
      (*coinCounter2)++;
      if (*coinCounter2 > 3) {
        *high2 = 1;
        *coinCounter2 = 0;
        ////Serial.print("One coin inserted\n");
        //coinIn.publish("1COIN");
      }
    } else {
      *high2 = 0;
    }
  } else if (*high2 == 1) {
    if (coin_input2 == LOW) {
      (*coinCounter2)++;
      if (*coinCounter2 > 10) {
        *high2 = 0;
        ////Serial.print("One coin inserted CA2\n");
        //coinIn.publish("coin2");
        (*coin_rcv)++;
      }
    } else {
      *high2 = 1;
    }
  }
}

//void logData(String newData){
//    Serial.print("Logging: ");
//    Serial.println(newData);
//    if (SPIFFS.exists("/data.txt")) {
//      //open file for appending new blank line to EOF.
//      File f = SPIFFS.open("/data.txt", "a");
//      f.println(newData);
//      f.close();
//    } else {
//      //open file for appending new blank line to EOF.
//      File f = SPIFFS.open("/data.txt", "w");
//      f.println(newData);
//      f.close();
//    }
//
//}
//
//void readData(){
//  int xCnt = 0;
//  File f = SPIFFS.open("/data.txt", "r");
//
//  if (!f) {
//      Serial.println("file open failed");
//  } else {
//    Serial.println("====== Reading from SPIFFS file =======");
//  }
//
//  while(f.available()) {
//    //Lets read line by line from the file
//    String line = f.readStringUntil('\n');
//    Serial.print(xCnt);
//    Serial.print("  ");
//    Serial.println(line);
//    xCnt ++;
//    queue.push(line);
//  }
//  f.close();
//}
//
//void removeData() {
//  bool f = SPIFFS.remove("/data.txt");
//
//  if (f) {
//    Serial.println("file is been removed");
//  } else {
//    Serial.println("file is unable to remove");
//  }
//}

void checkForUpdates() {
  String mac = getMAC();
  String fwURL = String( fwUrlBase );
  fwURL.concat( mac );
  String fwVersionURL = fwURL;
  fwVersionURL.concat( ".version" );

  //Serial.println( "Checking for firmware updates." );
  //Serial.print( "MAC address: " );
  //Serial.println( mac );
  //Serial.print( "Firmware version URL: " );
  //Serial.println( fwVersionURL );

  HTTPClient httpClient;
  httpClient.begin( fwVersionURL );
  int httpCode = httpClient.GET();
  if ( httpCode == 200 ) {
    String newFWVersion = httpClient.getString();

    //Serial.print( "Current firmware version: " );
    //Serial.println( FW_VERSION );
    //Serial.print( "Available firmware version: " );
    //Serial.println( newFWVersion );

    int newVersion = newFWVersion.toInt();

    if ( newVersion > FW_VERSION ) {
      //Serial.println( "Preparing to update" );

      String fwImageURL = fwURL;
      fwImageURL.concat( ".bin" );
      t_httpUpdate_return ret = ESPhttpUpdate.update( fwImageURL );

      switch (ret) {
        case HTTP_UPDATE_FAILED:
          //Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;

        case HTTP_UPDATE_NO_UPDATES:
          //Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;
      }
    }
    else {
      //Serial.println( "Already on latest version" );
    }
  }
  else {
    //Serial.print( "Firmware version check failed, got HTTP response code " );
    //Serial.println( httpCode );
  }
  httpClient.end();
}

String getMAC()
{
  uint8_t mac[6];
  char result[14];
  WiFi.macAddress(mac);
  sprintf(result, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String( result );
}

boolean isValidNumber(String str) {
  boolean isNum = false;
  for (byte i = 0; i < str.length(); i++) {
    isNum = isDigit(str.charAt(i));
  }
  return isNum;
}




