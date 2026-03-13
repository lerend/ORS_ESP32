#ifndef MODEM_SERVICE_H
#define MODEM_SERVICE_H

// Falls nicht schon in Globals.h definiert:
#ifndef TINY_GSM_MODEM_SIM7000SSL
  #define TINY_GSM_MODEM_SIM7000SSL
#endif
#include <Arduino.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include "Globals.h"

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
#endif

extern String mqttTopic;
class ModemService {
public:
    ModemService(); // Konstruktor

    void powerOn();
    void powerOff();
    bool connectGPRS();
    bool publishData(int distance_mm, int temperature_dC);

private:
    TinyGsm modem;
    TinyGsmClient client;
    PubSubClient mqttClient;

    void configureModemSettings();
    String setupAutoAPN(String op);
    bool connectMQTT();
};

#endif
