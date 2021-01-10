/*
   Corona-Warn-App Detektor - zeigt Nutzung von Corona-Warn-Apps in der Umgebung an

   Copyright 2021 Jens

   License: MIT

  
   Kurzbeschreibung:
   Bluetooth Low Energy (BLE) Scanner mit Filterung auf Identifikation der
   Corona-Warn-App (CWA), Anzeige mehrerer Corona-Warn-Apps in der Umgebung,
   ohne Speicherung von Adressen oder IDs.

   basiert auf dem Beispiel "BLE_scan" der Bibliothek "ESP32 BLE Arduino",
   siehe auch https://github.com/espressif/arduino-esp32
   nutzt Arduino-Bibliothek "TM1637" (von Avishay) zur Ansteuerung der Anzeige,
   siehe auch https://github.com/avishorp/TM1637
   
   Corona-Warn-App: https://www.bundesregierung.de/breg-de/themen/corona-warn-app

   Hardware: ESP32, bspw. NodeMCU ESP32 --> Board "ESP32 Dev Module"
             TM1637 4-Digit 7-Segment Display

*/

#include <BLEDevice.h>            // Teil des "esp32"-Pakets von Espressif Systems
#include <TM1637Display.h>        // Arduino-Bibliothek "TM1637"

#define LEDint 2                  // LED auf dem ESP-Controller: Bluetooth-Scanindikator

// Verbindung Controller <-> LED-Display (plus GND & 3.3V)
#define CLK 15
#define DIO 4

int scanTime = 5;                 // in Sekunden
int idleTime = 5;                 // ... Pause zwischen Scans

int numCWA = 0;                   // Zähler für gefundene Corona-Warn-Apps
const int thrCWA = 5;             // Schwelle: CWA-Anzeige erst darüber aktiv

BLEScan* pBLEScan;

TM1637Display display(CLK, DIO);


const byte seg_alle[] = {0xff, 0xff, 0xff, 0xff};   // alle Segmente  --> Muster '8888'
const byte seg_kein[] = {0x00, 0x00, 0x00, 0x00};   // keine Segmente --> Muster '    '
const byte seg_leer[] = {0x40, 0x40, 0x40, 0x40};   // nur Segmente G --> Muster '----'


/*
  Auswertung für jedes gefundene BLE-Device
*/

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {       // BLE Device gefunden
      if (advertisedDevice.haveServiceUUID()) {
        BLEUUID service = advertisedDevice.getServiceUUID();    // BLE Device bietet Dienste
        if (service.equals(BLEUUID((uint16_t)0xFD6F))) {        // UUID der Corona-Warn-App

//          if (advertisedDevice.haveRSSI()) {                    // Ausgabe der Signalstärke
//            Serial.print("CWA RSSI: ");
//            Serial.println(advertisedDevice.getRSSI());
//          } else
//            Serial.println("CWA unknown RSSI");

          numCWA++;                                             // für gefundene CWAs hochzählen
        }
      }
    }
};


void setup() {
  Serial.begin(115200);                             // Infos auch über USB-Schnitstelle
  Serial.println("\n\nCorona-Warn-App Detektor");

  pinMode(LEDint, OUTPUT);                          // LED-Pins als Ausgang

  BLEDevice::init("");                              // BLE-Funktion des ESP32 vorbereiten
  pBLEScan = BLEDevice::getScan();                  // Scan wird vorbereitet
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);                    // aktiver Scan: ESP32 sendet Aufforderung
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  display.clear();
  display.setBrightness(3, true);                   // Helligkeit 0...7, Display ein

  for (int i = 0; i < 3; i++) {                     // bei Start blinkt LED und Anzeige
    digitalWrite(LEDint, HIGH);
    display.setSegments(seg_alle);                  // zeige '8888'
    delay(100);
    digitalWrite(LEDint, LOW);
    display.setSegments(seg_kein);                  // zeige '    '
    delay(100);
  }
}


void loop() {                                       // ewige Wiederholung des Scan-Zyklus

  numCWA = 0;                                       // Zähler für aktuelle CWAs zurücksetzen
  //  Serial.println("scanning...");

  digitalWrite(LEDint, HIGH);                       // Scan-Indikator einschalten
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);       // BLE Scan starten
  digitalWrite(LEDint, LOW);                        // Scan-Indikator ausschalten

  //  Serial.print("devices found: ");
  //  Serial.println(foundDevices.getCount());
  //  Serial.println("scan done.");
  
  pBLEScan->clearResults();                         // erfasste BLE-Daten aus dem Scan löschen

  Serial.print("CWAs: ");                           // Ausgaben über USB
  Serial.println(numCWA);

  if (numCWA < thrCWA)
    display.setSegments(seg_leer);                  // keine Ausgabe, zeige '----'
  else
    display.showNumberDec(numCWA, false);           // Ausgabe der Anzahl

  delay(idleTime * 1000);                           // Pause in Sekunden
}
