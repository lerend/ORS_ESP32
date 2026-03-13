#include "BLE_Service.h"
#include "Constants.h"

// Callback: Server Status
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    }
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

// Callback: Sensor ID
class MySensorCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pSensorCharacteristic) {
        String value = pSensorCharacteristic->getValue().c_str();
        if (value.length() > 0) {
            sensor_id = value;
            preferences.putString("sensor_id", sensor_id);
            debugPrint("BLE: SensorID gespeichert: "); debugPrintln(sensor_id.c_str());
        }
    }
};

// Callback: MQTT User
class MyMQTTUSERCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        String value = pCharacteristic->getValue().c_str();
        if (value.length() > 0) {
            mqtt_user = value;
            bleStartTime = millis(); 
            preferences.putString("mqtt_user", mqtt_user);
            debugPrintln("BLE: MQTT User gespeichert.");
        }
    }
    void onRead(BLECharacteristic* pCharacteristic) {
        pCharacteristic->setValue(mqtt_user.c_str());
    }
};

// Callback: MQTT PW
class MyMQTTPWCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        String value = pCharacteristic->getValue().c_str();
        if (value.length() > 0) {
            mqtt_pw = value;
            bleStartTime = millis();
            preferences.putString("mqtt_pw", mqtt_pw);
            debugPrintln("BLE: MQTT PW gespeichert.");
        }
    }
    void onRead(BLECharacteristic* pCharacteristic) {
        pCharacteristic->setValue(mqtt_pw.c_str());
    }
};

// Callback: APN
class MyAPNCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        String value = pCharacteristic->getValue().c_str();
        if (value.length() > 0) {
            APN = value;
            bleStartTime = millis();
            preferences.putString("apn", APN);
            debugPrint("BLE: APN gespeichert: "); debugPrintln(APN.c_str());
        }
    }
    void onRead(BLECharacteristic* pCharacteristic) {
        pCharacteristic->setValue(APN.c_str());
    }
};

// Callback: Sleep Time
class MySleepTimeCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        String value = pCharacteristic->getValue().c_str();
        if (value.length() > 0) {
            sleep_time = atoi(value.c_str());
            if (sleep_time > 0) {
                bleStartTime = millis();
                preferences.putInt("sleep_time", sleep_time);
                debugPrintln("BLE: SleepTime gespeichert.");
            }
        }
    }
    void onRead(BLECharacteristic* pCharacteristic) {
        String sleepStr = String(sleep_time);
        pCharacteristic->setValue(sleepStr.c_str());
    }
};

void getStoredValues() {
    preferences.begin("river-config", false);
    sensor_id = preferences.getString("sensor_id", "sensor_default");
    mqtt_user = preferences.getString("mqtt_user", "ors-client-test1");
    mqtt_pw = preferences.getString("mqtt_pw", "test1");
    APN = preferences.getString("apn","null");
    sleep_time = preferences.getInt("sleep_time", 60);
    
    #ifdef DEBUG_MODE_AKTIV
        debugPrintln("Ini: hole Standardwerte aus Flash");
    #endif
}

void ble_setup() {
    BLEDevice::init(BT_NAME);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Charakteristiken erstellen & Callbacks zuweisen
    pSensorCharacteristic = pService->createCharacteristic(SENSOR_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    pSensorCharacteristic->setCallbacks(new MySensorCharacteristicCallbacks());

    pMQTTUSERCharacteristic = pService->createCharacteristic(MQTTUSER_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    pMQTTUSERCharacteristic->setCallbacks(new MyMQTTUSERCharacteristicCallbacks());

    pMQTTPWCharacteristic = pService->createCharacteristic(MQTTPW_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    pMQTTPWCharacteristic->setCallbacks(new MyMQTTPWCharacteristicCallbacks());

    pAPNCharacteristic = pService->createCharacteristic(APN_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    pAPNCharacteristic->setCallbacks(new MyAPNCharacteristicCallbacks());

    pSleepTimeCharacteristic = pService->createCharacteristic(SLEEP_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    pSleepTimeCharacteristic->setCallbacks(new MySleepTimeCharacteristicCallbacks());

    pStatusCharacteristic = pService->createCharacteristic(STATUS_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);
    pStatusCharacteristic->addDescriptor(new BLE2902());

    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    bleStartTime = millis();
    debugPrintln("BLE aktiv & Advertising gestartet.");
}
