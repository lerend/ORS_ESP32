// Bedienen der beiden Sensoren
#ifndef SENSOR_SERVICE_H    // Wenn "xxx_SERVICE_H" noch nicht definiert wurde...
#define SENSOR_SERVICE_H    // ...dann definiere es jetzt...

// #include "Constants.h"
// #include "Globals.h"

#include <DallasTemperature.h>
#include <OneWire.h>

class SensorService {
public:
    SensorService() : oneWire(ONE_WIRE_BUS), sensors(&oneWire) {}

    void initialize() {
        sensors.begin();
        #ifdef SENSOR_SERIAL
            SerialSensor.begin(SENSOR_BAUDRATE, SERIAL_8N1, SENSOR_RX_PIN, SENSOR_TX_PIN);
            pinMode(SENSOR_POWER, OUTPUT);
            digitalWrite(SENSOR_POWER, HIGH); // Sensor einschalten
        #else
            pinMode(ECHO_PIN, INPUT);
            // Wenn Trig-Pin genutzt wird, hier definieren
        #endif
        delay(1000); 
    }

    SensorData readSensors() {
        SensorData data;
        
        // 1. Interne Temperatur lesen (native ESP32 Funktion)
        data.internalChipTemp_dC = int(temperatureRead()*10.0);

        // 2. Temperatur lesen
        sensors.requestTemperatures();
        float temp=sensors.getTempCByIndex(0);
        if ((temp < -30.0) || (temp > 50.0)){          //dann  Messfehler > wiederholen der Messung
          sensors.requestTemperatures();
          float temp=sensors.getTempCByIndex(0);
        }
        data.temperature_dC = int(temp*10.0);
        
        // 3. Distanz lesen (Beispiel für Seriell)
        data.distance_mm = measureDistance();
        
        return data;
    }

private:
    OneWire oneWire;
    DallasTemperature sensors;

    long measureDistance() {
        #ifdef SENSOR_SERIAL
          digitalWrite(SENSOR_POWER, HIGH);
          delay(600); 
          while(SerialSensor.available()) SerialSensor.read(); // Puffer leeren
          
          unsigned char data[4] = {0};
          int distance = 0;
          long startTime = millis();
          
          while(millis() - startTime < 2000) { 
            if (SerialSensor.available() >= 4) {
              if (SerialSensor.read() == 0xFF) {
                data[0] = 0xFF;
                data[1] = SerialSensor.read(); 
                data[2] = SerialSensor.read(); 
                data[3] = SerialSensor.read(); 
  
                byte sum = (data[0] + data[1] + data[2]) & 0x00FF;
                
                if (sum == data[3]) {
                  distance = (data[1] << 8) + data[2];
                  if (distance > 200) { 
                    debugPrint("Entfernung (raw): "); debugPrint(distance); debugPrintln(" mm");
                    digitalWrite(SENSOR_POWER, LOW);
                    return distance;
                  }
                }
              }
            }
          }
          debugPrint("Fehler: Serielle Distanzmessung fehlgeschlagen.");
          digitalWrite(SENSOR_POWER, LOW);
          return 0;
        
        #else
          digitalWrite(TRIG_PIN, HIGH);
          delayMicroseconds(50);
          digitalWrite(TRIG_PIN, LOW);
          return pulseIn(ECHO_PIN, HIGH, 1000000);
        #endif
    }

};
#endif
