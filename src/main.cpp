#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Arduino.h>
#include <ESP32Servo.h>
#include "servowrapper.h"

// UART service UUID data
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

bool isPasswordCorrect();
void normalOperation();
void setupServos();
void setupUART();
void debugMode();

bool isDebugMode = false;

//Servo ports
const int LEFT_LIGHT_PORT = 15;
const int RIGHT_LIGHT_PORT = 16;
const int AC_FAN_PORT = 12;
const int AC_ON_OFF_PORT = 13;

const int LEFT_LIGHT_DEFAULT_POS = 0;
const int RIGHT_LIGHT_DEFAULT_POS = 0;
const int AC_FAN_DEFAULT_POS = 0; //Actual fan = 0
const int AC_ON_OFF_DEFAULT_POS = 120;

const int LEFT_LIGHT_UP = 180;
const int LEFT_LIGHT_DOWN = 0;
const int RIGHT_LIGHT_UP = 180; 
const int RIGHT_LIGHT_DOWN = 0;
const int AC_FAN_UP = 0; //Actual fan = 0
const int AC_FAN_DOWN = 23; //Actual fan down = 23
const int AC_ON_OFF_UP = 120;
const int AC_ON_OFF_DOWN = 120-23;


BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
float txValue = 0;
const int button = 0;      // button on PIN G0
const int readPin = 32;    // analog pin G32
const int LEDpin = 2;      // LED on pin G2
bool convert = false;
String rxString = "";
std::string rxValue;       // rxValue gathers input data
BLEServer *pServer;

const String PASSWORD = "fnaf";
bool hasEnterCorrectPsword = false;

ServoWrapper* leftLight;
ServoWrapper* rightLight;
ServoWrapper* acFan;
ServoWrapper* acOnOff;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0)  {
      convert = true;      // flag to invoke convertControlpad routine
      Serial.println(" ");
      Serial.print("Received data: ");
      for (int i = 0; i < rxValue.length(); i++) { 
        Serial.print(rxValue[i]);
        rxString = rxString + rxValue[i]; // build string from received data 
      } 
    } 
  } 
}; 

// ***************************** SETUP *******************************
void setup() { 
  Serial.begin(115200); 
  setupServos();
  setupUART();
}

// *************************** MAIN PROGRAM *********************************
void loop() {
  if (!deviceConnected) {
    pServer->getAdvertising()->start(); // start advertising
    Serial.println("Waiting a client connection to notify...");
    Serial.println(" ");
    hasEnterCorrectPsword = false;
  } else {
    if (!hasEnterCorrectPsword) {
      hasEnterCorrectPsword = isPasswordCorrect();
    } else {
        if (!isDebugMode) {
          normalOperation();
        } else {
            debugMode();
        }
    }

  }
  delay(50);
}

void setupServos() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  leftLight = new ServoWrapper(LEFT_LIGHT_PORT, LEFT_LIGHT_UP, LEFT_LIGHT_DOWN, LEFT_LIGHT_DEFAULT_POS);
  rightLight = new ServoWrapper(RIGHT_LIGHT_PORT, RIGHT_LIGHT_UP, RIGHT_LIGHT_DOWN, RIGHT_LIGHT_DEFAULT_POS);
  acFan = new ServoWrapper(AC_FAN_PORT, AC_FAN_UP, AC_FAN_DOWN, AC_FAN_DEFAULT_POS);
  acOnOff = new ServoWrapper(AC_ON_OFF_PORT, AC_ON_OFF_UP, AC_ON_OFF_DOWN, AC_ON_OFF_DEFAULT_POS);

}

void setupUART() {
  BLEDevice::init("Unknown Devices"); // give the BLE device a name
  
  BLEServer *pServer = BLEDevice::createServer(); // create BLE server
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY);                    
  pCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  
  pService->start(); // start the service

  pServer->getAdvertising()->start(); // start advertising
  Serial.println("Waiting a client connection to notify...");
  Serial.println(" ");
}

bool isPasswordCorrect() {
  convert = false;
  rxString.trim();
  if (String(rxString) == PASSWORD) { 
    return true;
  }

  rxString = "";
  return false;
}

void normalOperation() {
  convert = false;
  rxString.trim();
  if (rxString == "!B11:") leftLight->invert();
  else if (rxString == "!B219") rightLight->invert();
  else if (rxString == "!B318")  {
    acFan->setToDown();
    delay(400);
    acFan->setToUp(); 
  }
  else if (rxString == "!B417") {
    acOnOff->setToDown();
    delay(400);
    acOnOff->setToUp(); 
  } 
  rxString = "";
}

void debugMode() {
  convert = false;
  rxString.trim();
  String firstComponent = rxString.substring(0, rxString.indexOf("-"));
  String secondComponent = rxString.substring(rxString.indexOf("-") + 1);
  Serial.println(firstComponent);
  Serial.println(secondComponent);
  if (firstComponent == "leftLight") leftLight->write(secondComponent.toInt());
  else if (firstComponent == "rightLight") rightLight->write(secondComponent.toInt());
  else if (firstComponent == "acFan") acFan->write(secondComponent.toInt());
  else if (firstComponent == "acOnOff") acOnOff->write(secondComponent.toInt());
  rxString = "";
}
