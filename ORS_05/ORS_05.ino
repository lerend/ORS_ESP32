
// =======================================================
// RiverSense_Deepsleep.ino
// Modulares ESP32/SIM7000G Deep Sleep Programm in einer Datei
// =======================================================
//
// Start of configuration after new start with power on
//
// -------------------------------------------------------
// lokale Anpassungen
// -------------------------------------------------------
// with BLE
// int sleep_time = 60; // Zeit in Minuten bis zum nächsten Wakeup
//
// 3. #define SENSOR_SERIAL           // Entfernen oder auskommentieren, wenn Sie den Puls-Sensor (TRIG/ECHO) verwenden
//
// 4. to test 
//      #define WITH_MQTT 
//      #define DEBUG_MODE_AKTIV
//      #define DUMP_AT_COMMANDS      setzen oder auskommentieren in Constants.h
//
//--------------------------------------------------------
// 0. Control of the program functions. now in Constants.h
// -------------------------------------------------------
// Distanzsensor: Seriell oder Puls
//#define SENSOR_SERIAL           // Entfernen Sie dies, wenn Sie den Puls-Sensor (TRIG/ECHO) verwenden

// Optional: Aktivieren Sie MQTT, wenn gesendet werden soll
//#define WITH_MQTT 

// Definitionen for Debugging
//#define DEBUG_MODE_AKTIV

//should the AT commands be debugged?
//#define DUMP_AT_COMMANDS

#define TINY_GSM_MODEM_SIM7000SSL     //must be the first statement for selction in TinyGsmClient

#include <Arduino.h>
#include <esp_sleep.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TinyGsmClient.h>
#define MQTT_MAX_PACKET_SIZE 512  // Erhöhen für korrekte Übertragung zu mqtt server
#include <PubSubClient.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "Constants.h"
#include "Globals.h"
// Hier werden die Klassen-Inhalte jetzt VOR dem Hauptcode geladen:
#include "DebugUtils.h"
#include "ModemService.h"
#include "04_SensorService.h"
#include "BLE_Service.h"

ModemService modemService;
SensorService sensorService;

// Tatsächliche Definition der Variablen
String sensor_id = "";
String mqtt_user = "";
String mqtt_pw = "";
String APN = ""; 
int sleep_time = 60;    // Zeit in Minuten bis zum nächsten Wakeup
// Hier wird der Speicher wirklich reserviert:
const char* MQTT_BROKER = "217.154.169.141";
const int   MQTT_PORT       = 1883;
//const char* TOPIC_PUBLISH   = "sensordaten/fluss/Loge004";
String mqttTopic = ""; // Hier wird der tatsächliche Speicher reserviert

bool ble_on = false;
bool deviceConnected = false;
bool oldDeviceConnected = false;
long bleStartTime = 0;    // Speichert die Zeit, zu der die BLE-Werbung gestartet wurde
Preferences preferences;

// BLE Objekte initialisieren
BLEServer* pServer = NULL;
BLECharacteristic* pSensorCharacteristic = NULL;
BLECharacteristic* pMQTTUSERCharacteristic = NULL;
BLECharacteristic* pMQTTPWCharacteristic = NULL;
BLECharacteristic* pSleepTimeCharacteristic = NULL;
BLECharacteristic* pAPNCharacteristic = NULL;
BLECharacteristic* pStatusCharacteristic = NULL;


// -------------------------------------------------------
// 4. HAUPTPROGRAMM (SETUP & LOOP)
// -------------------------------------------------------

// Globale Instanzen der Services

// Definiere den Button Pin (0 ist der BOOT Button auf dem Board)
// cb #define MAINTENANCE_PIN 0 
HardwareSerial SerialAT(1);

#ifdef SENSOR_SERIAL
  HardwareSerial SerialSensor(2);
#endif

void setup() {
    Serial.begin(115200); 
    SerialAT.begin(MODEM_BAUDRATE, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

    pinMode(BOARD_LED_PIN, OUTPUT); 
    
    // 1. Gespeicherte Config laden
    
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED) {
        digitalWrite(BOARD_LED_PIN, LOW);    // switch on the blue light >>> in Wartungsmode
        
        debugPrintln("--- ECHTER POWER-ON RESET ---");
        debugPrint("ESP32 Software Version; ");
        debugPrintln(FIRMWARE_VERSION);

        ble_setup();
        ble_on=true;
        getStoredValues();
        
        debugPrint("SensorID: "); debugPrintln(sensor_id.c_str());
        debugPrintln("\n!!! WARTUNGSMODUS AKTIV für 30 sec!!!");
        delay(1000);

        while (millis() - bleStartTime < ADVERTISING_DURATION_MS) {
          if (deviceConnected && !oldDeviceConnected) {         //nur beim ersten Durchlauf mit connected ausgeben
            debugBLEln("Service mode active for 30 s");
            debugBLE("ESP32 Software Version: ");
            debugBLEln(FIRMWARE_VERSION);

            pSensorCharacteristic->setValue(sensor_id); //String(value).c_str());
            //pSensorCharacteristic->notify();
            pMQTTUSERCharacteristic->setValue(mqtt_user);
            //pMQTTUSERCharacteristic->notify();
          }
          // disconnecting
          if (!deviceConnected && oldDeviceConnected) {
            debugPrintln("Device disconnected.");
            delay(500); // give the bluetooth stack the chance to get things ready
            pServer->startAdvertising(); // restart advertising
            debugPrintln("Start advertising");
            oldDeviceConnected = deviceConnected;
          }
          // connecting
          if (deviceConnected && !oldDeviceConnected) {
            // do stuff here on connecting
            oldDeviceConnected = deviceConnected;
            debugPrintln("Device Connected");
          }
          delay(2000);
          //pStatusCharacteristic->setValue("WARTUNGSMODUS still active");
          //pStatusCharacteristic->notify(); // Nachricht aktiv ans Tablet pushen
        }
        debugPrintln("30 s over. Start first measure");
        // Stoppe das Advertising
        //BLEDevice::stopAdvertising();
        //BLEDevice::deinit(true); // De-initialisiert den BLE-Stack vollständig
        //digitalWrite(BOARD_LED_PIN, HIGH);    // switch off the blue light
    }
    else {
      getStoredValues();
      digitalWrite(BOARD_LED_PIN, HIGH);    // switch off the blue light
    }
    // ----------------------------------------------------
    // NORMALER MODUS (Deep Sleep)
    // ----------------------------------------------------
    debugPrint("Start Normalbetrieb mit ID: "); debugPrintln(sensor_id.c_str());
    debugPrint("MQTT User: "); debugPrintln(mqtt_user.c_str());
    debugPrint("Sleep Time: "); debugPrintln(String(sleep_time).c_str());

    sensorService.initialize();
    SensorData data = sensorService.readSensors();
    debugPrint("Temp CPU: "); debugPrint(data.internalChipTemp_dC); debugPrintln("d°");

    #ifdef WITH_MQTT
      modemService.powerOn(); 
      if (modemService.connectGPRS()) {
          modemService.publishData(data.distance_mm, data.temperature_dC);
        //  modemService.publishConfig();
      }
      modemService.powerOff();
    #endif

    // Schlafen mit der konfigurierten Zeit
    debugPrint("CPU goes to sleep for "); debugPrint(sleep_time); debugPrintln(" minutes");
    esp_sleep_enable_timer_wakeup(sleep_time * uS_TO_M_FACTOR);
    esp_deep_sleep_start();
}

void loop()
{
    // loop() bleibt leer, da das Programm in setup() seine Arbeit erledigt und schlafen geht.
     // notify changed value
  
}
