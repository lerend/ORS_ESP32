#ifndef GLOBALS_H
#define GLOBALS_H

#define TINY_GSM_MODEM_SIM7000SSL     //must be the first statement for selction in TinyGsmClient
#define TINY_GSM_RX_BUFFER 1024

#include <Arduino.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include "Constants.h"

// 1. Datenstrukturen
struct SensorData {
    int distance_mm;
    int temperature_dC;
    int internalChipTemp_dC; // Interne ESP32 Temperatur
};

// 2. Hardware-Objekte bekannt machen
extern HardwareSerial SerialAT;
#ifdef SENSOR_SERIAL
  extern HardwareSerial SerialSensor;
#endif
#define MODEM_STREAM SerialAT

// 3. Globale Variablen (Deklaration)
extern String sensor_id;
extern String mqtt_user;
extern String mqtt_pw;
extern String APN;
extern int sleep_time;
extern bool ble_on;
extern bool deviceConnected;
extern bool oldDeviceConnected;
extern long bleStartTime;
extern Preferences preferences;

// 4. BLE Characteristic Objekte
extern BLECharacteristic* pSensorCharacteristic;
extern BLECharacteristic* pMQTTUSERCharacteristic;
extern BLECharacteristic* pMQTTPWCharacteristic;
extern BLECharacteristic* pSleepTimeCharacteristic;
extern BLECharacteristic* pAPNCharacteristic;
extern BLECharacteristic* pStatusCharacteristic;
extern BLEServer* pServer;

// 5. Funktions-Prototypen (Damit Tabs sich gegenseitig finden)
void debugPrintln(const char* message);
void debugPrint(const char* message);
void debugPrint(int value);
void debugPrint(float value);
void debugPrint(const String& message);
void debugBLEln(const char* message);
void ble_setup();
void getStoredValues();

#endif
