#include "DebugUtils.h"

// 1. Text-Ausgabe
void debugPrintln(const char* message) {
#ifdef DEBUG_MODE_AKTIV
  Serial.println(message);
#endif
}

void debugPrint(const char* message) {
#ifdef DEBUG_MODE_AKTIV
  Serial.print(message);
#endif
}

// 2. Ganzzahl
void debugPrint(int value) {
#ifdef DEBUG_MODE_AKTIV
  Serial.print(value);
#endif
}

void debugPrintln(int value) {
#ifdef DEBUG_MODE_AKTIV
  Serial.println(value);
#endif
}

// 3. Gleitkommazahl
void debugPrint(float value) {
#ifdef DEBUG_MODE_AKTIV
  Serial.print(value, 2); 
#endif
}

// 4. Strings
void debugPrint(const String& message) {
#ifdef DEBUG_MODE_AKTIV
    Serial.print(message);
#endif
}

// NEU: Diese Funktion fängt den "StringSumHelper" Fehler ab
void debugPrintln(const String& message) {
#ifdef DEBUG_MODE_AKTIV
  Serial.println(message);
#endif
}

// 10. BLE ohne Zeilenumbruch
void debugBLE(const char* message) {
    // Diese Variablen müssen global oder extern verfügbar sein
    if (ble_on && pStatusCharacteristic != nullptr) {
      pStatusCharacteristic->setValue(message);
      pStatusCharacteristic->notify();
    }
}

// 11. BLE mit manuellem Zeilenumbruch
void debugBLEln(const char* message) {
    if (ble_on && pStatusCharacteristic != nullptr) {
      pStatusCharacteristic->setValue(message);
      pStatusCharacteristic->notify();
      // Optional: Ein echtes "\n" senden, wenn die Web-Konsole das braucht
      pStatusCharacteristic->setValue("\n");
      pStatusCharacteristic->notify();
    }
}
