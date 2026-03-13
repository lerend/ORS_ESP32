#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <Arduino.h>
#include "Globals.h"

// Deklarationen der Standard-Debug-Funktionen
void debugPrint(const char* message);
void debugPrintln(const char* message);
void debugPrint(int value);
void debugPrintln(int value);
void debugPrint(float value);
void debugPrint(const String& message);
void debugPrintln(const String& message); // NEU: Überladung für Strings mit Zeilenumbruch

// BLE Debugging
void debugBLE(const char* message);
void debugBLEln(const char* message);

#endif
