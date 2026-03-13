#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Preferences.h>
#include "Globals.h" // WICHTIG: Hier müssen sensor_id, APN etc. als extern drin stehen

// UUIDs
#define SERVICE_UUID               "19b10000-e8f2-537e-4f6c-d104768a1214"
#define SENSOR_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define SLEEP_CHARACTERISTIC_UUID  "2efb949c-1707-4c22-b0e0-8112efe7ba68"
#define MQTTUSER_CHARACTERISTIC_UUID "149df1e1-4d59-4529-befb-b6d5a2c0fe33"
#define APN_CHARACTERISTIC_UUID    "3a14b289-1f67-4084-8a5e-c4a1e22d3388"
#define MQTTPW_CHARACTERISTIC_UUID "fafc1daf-dff4-4254-854b-c0b0a6e5ba31"
#define STATUS_CHARACTERISTIC_UUID "62f1437c-3965-430c-9742-1277a06d9a91"

// Funktionen ankündigen
void ble_setup();
void getStoredValues();

#endif
