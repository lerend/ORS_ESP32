#include "ModemService.h"
#include "DebugUtils.h"

#ifdef DUMP_AT_COMMANDS
  static StreamDebugger debugger(SerialAT, Serial);
#endif

// Der Konstruktor mit Initialisierungsliste
ModemService::ModemService() :
#ifdef DUMP_AT_COMMANDS
    modem(debugger),
#else
    modem(SerialAT),
#endif
    client(modem),
    mqttClient(client)
{}

void ModemService::powerOn() {
    debugPrintln("Modem einschalten...");
    pinMode(MODEM_DTR_PIN, OUTPUT); digitalWrite(MODEM_DTR_PIN, LOW);
    pinMode(BOARD_PWRKEY_PIN, OUTPUT); digitalWrite(BOARD_PWRKEY_PIN, LOW);
    delay(100); digitalWrite(BOARD_PWRKEY_PIN, HIGH);
    delay(MODEM_POWERON_PULSE_WIDTH_MS); digitalWrite(BOARD_PWRKEY_PIN, LOW);
    delay(10000);
    while (!modem.testAT()) { debugPrint("."); delay(500); }
    debugPrintln("Modem online.");
    delay(5000);
    configureModemSettings();
}

void ModemService::powerOff() {
    debugPrintln("Modem ausschalten...");
    debugBLEln("Modem ausschalten");
    modem.gprsDisconnect();
    mqttClient.disconnect();

    if (modem.poweroff()) {
        debugPrintln("Modem enter power off modem!");
    } else {
        debugPrintln("modem power off failed!");
    }
    delay(5000);
    debugPrintln("Check modem response .");
    while (modem.testAT()) {
        debugPrint("."); delay(500);
    }
    debugPrintln("Modem is not response, power off success!");
    delay(5000);
}

bool ModemService::connectGPRS() {
    debugPrintln("Verbinde GPRS...");
    if (!modem.waitForNetwork()){
        #ifdef DEBUG_MODE_AKTIV
            debugBLEln("Network connection failed");
            debugPrintln("Network connection failed");
        #endif
        return false;
    }
    String op = modem.getOperator();
    #ifdef DEBUG_MODE_AKTIV
        String message = "Operator: " + op;
        debugBLEln(message.c_str());
        debugPrintln(message.c_str());
    #endif
    if(APN == "null"){
        APN = setupAutoAPN(op);
    }

    if (!modem.gprsConnect(APN.c_str(), GPRS_USER, GPRS_PASS)) {
        #ifdef DEBUG_MODE_AKTIV
            debugPrintln("gprs failed, APN überprüfen");
            debugBLEln("gprs failed, APN überprüfen");
        #endif
      return false;
    }

    #ifdef DEBUG_MODE_AKTIV
        int csq = modem.getSignalQuality();
        message = "Signal quality: " + String(csq);
        debugPrintln(message.c_str());
        debugBLEln(message.c_str());
        modem.sendAT(GF("+CPSI?"));
        if (modem.waitResponse(2000L, GF("+CPSI:")) == 1) {
            String resp = modem.stream.readStringUntil('\n');
            resp.trim();
            if (resp.length() > 0) {
                debugPrint("System Info: "); debugPrintln(resp.c_str());
                debugBLE("Verbindung: "); debugBLEln(resp.c_str());
            }
        } else {
            debugPrintln("Modem hat nicht auf CPSI reagiert.");
        }
    #endif
    return modem.isGprsConnected();
}

bool ModemService::publishData(int distance_mm, int temperature_dC) {
    if (!connectMQTT()) {
          debugPrintln("not connected with MQTT");
          return false;
    }
    
    mqttTopic = String("ors/") + sensor_id;
    int battVoltage = int(modem.getBattVoltage());
    String payload = "{\"waterdist_mm\":" + String(distance_mm) +
                     ", \"watertemp_dc\":" + String(temperature_dC) +
                     ", \"Batt_mV\":" + String(battVoltage) +
                     ", \"version\": \"" + FIRMWARE_VERSION + "\"" +
                     ", \"sensor_id\": \"" + sensor_id + "\"}";
    debugBLEln(payload.c_str());
    debugPrint("MQTT_Topic: " ); debugPrintln(mqttTopic.c_str());
    debugPrint("Payload: " ); debugPrintln(payload.c_str());

    bool success= (mqttClient.publish(mqttTopic.c_str(), payload.c_str())); 
    // Wenn success false ist, aber der state 0 UND wir noch verbunden sind,
    // dann war es ein reiner Timing-Fehler des Modems.
    if (success || (mqttClient.state() == 0 && mqttClient.connected())) {
      debugPrintln("MQTT Publish erfolgreich (Bestätigung ignoriert, da State 0)");
      return true;
    }
    //debugPrintln("Publizieren -> Fehlgeschlagen!");
    int rc = mqttClient.state();
    debugPrint(" -> Fehlgeschlagen! Fehlercode: ");
    debugPrintln(String(rc).c_str());
    debugBLEln("Error in MQTT Upload");
    return false;
}

void ModemService::configureModemSettings() {
    modem.sendAT("+CFUN=0"); modem.waitResponse(10000L);
    modem.setNetworkMode(2); modem.setPreferredMode(3);
    modem.sendAT("+CFUN=1"); modem.waitResponse(10000L);
}

String ModemService::setupAutoAPN(String op) {
    String targetAPN = "";
    if (op.indexOf("Telekom") >= 0 || op.indexOf("T-Mobile") >= 0) targetAPN = "internet.telekom";
    else if (op.indexOf("Vodafone") >= 0) targetAPN = "web.vodafone.de";
    else if (op.indexOf("O2") >= 0 || op.indexOf("Telefonica") >= 0) targetAPN = "internet";
    else if (op.indexOf("MEDIONmobil") >= 0) targetAPN = "internet.eplus.de";
    else if (op.indexOf("Congstar") >= 0) targetAPN = "internet.t-mobile";
    else {
        debugPrintln("Provider unbekannt");
        debugBLEln("APN unbekannt, bitte eingeben");
        return "null";
    }
    preferences.putString("apn", targetAPN);
    return targetAPN;
}

bool ModemService::connectMQTT() {
    if (mqttClient.connected()) return true;

    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    String clientId = "ESP32-" + String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_pw.c_str())) {
        debugPrintln("MQTT verbunden!");
        return true;
    }
    debugPrintln(">>Error at MQTT connecting");
    return false;
}
