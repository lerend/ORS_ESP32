#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Arduino.h>
#define FIRMWARE_VERSION "0.9.1"  // diese Version
// =======================================================
// 1. PROGRAMM-STEUERUNG (SCHALTER)
// =======================================================
#define DEBUG_MODE_AKTIV        // Debug-Ausgaben auf Serial
//#define DUMP_AT_COMMANDS      // AT-Kommunikation des Modems loggen
#define WITH_MQTT               // Datentransfer via MQTT aktivieren
#define SENSOR_SERIAL           // Distanzsensor via Serial (RX/TX). Auskommentieren für Puls.

// =======================================================
// 2. HARDWARE PINS (LILYGO SIM7000G / ESP32)
// =======================================================
#define MODEM_RX_PIN              (26)
#define MODEM_TX_PIN              (27)
#define MODEM_DTR_PIN             (25)
#define BOARD_PWRKEY_PIN          (4)
#define MODEM_BAUDRATE            (115200)
#define MODEM_POWERON_PULSE_WIDTH_MS (1000)

#define BOARD_LED_PIN             (12)
#define LED_ON                    (LOW)
#define LED_OFF                   (HIGH)

#define ONE_WIRE_BUS              (21)

#ifdef SENSOR_SERIAL
  #define SENSOR_RX_PIN           (32)
  #define SENSOR_TX_PIN           (33)
  #define SENSOR_POWER            (15) 
  #define SENSOR_BAUDRATE         (9600)
#else
  #define ECHO_PIN                (33)
  #define TRIG_PIN                (32) // Falls benötigt ergänzen
#endif

// =======================================================
// 3. NETZWERK & MQTT KONFIGURATION
// =======================================================

const char GPRS_USER[]      = "";
const char GPRS_PASS[]      = "";

extern const char* MQTT_BROKER;
extern const int   MQTT_PORT;
//extern char* TOPIC_PUBLISH;

// =======================================================
// 4. SYSTEM-KONSTANTEN
// =======================================================
const char BT_NAME[]        = "ORS_ESP32_Sensor";
#define uS_TO_M_FACTOR      (60000000ULL)
const long ADVERTISING_DURATION_MS = 60000; // 60 Sekunden Wartungsmodus
// // Falls die Klassen in setup() noch nicht bekannt sind:
// class ModemService;
// class SensorService;
// extern ModemService modemService;
// extern SensorService sensorService;

#endif
