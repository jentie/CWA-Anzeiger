/*
   Corona-Warn-App Anzeiger - zeigt Corona-Warn-Apps in der Umgebung an

   Bluetooth Low Energy (BLE) Scanner mit Filterung nach der Identifikation der
   Corona-Warn-App (CWA), Anzeige einer oder mehrerer Corona-Warn-Apps in der
   Umgebung, ohne Speicherung von Adressen oder IDs.

   basiert auf dem Beispiel "BLE_scan" aus der Bibliothek "ESP32 BLE Arduino"

   Corona-Warn-App: https://www.bundesregierung.de/breg-de/themen/corona-warn-app
   
   TODO: board: ESP32 DevKit v1 / Joy-it NodeMCU ESP32 --> "ESP32 Dev Module"

   TODO: Herkunft, Lizenz

*/

#include <BLEDevice.h>            // Teil des "esp32"-Pakets von Espressif Systems
                                  // siehe auch https://github.com/espressif/arduino-esp32
#include <TM1637Display.h>        // Arduino-Bibliothek "TM1637" (von Avishay)
                                  // siehe auch https://github.com/avishorp/TM1637

#define LED 2                     // LED auf dem ESP-Controller: CWA-Indikator

// Anbindung des LED-Display (plus GND & 3.3V)
#define CLK 15
#define DIO 4

int scanTime = 5;                 // in Sekunden
int idleTime = 2;                 // ... Pause zwischen Scans

int numCWA = 0;                   // Zähler für gefundene Corona-Warn-Apps
int maxCWA = 0;                   // Höchstwert der Anzahl während des Betriebs
const int thrCWA = 1;             // Schwelle: CWA-Indikator erst darüber aktiv

BLEScan* pBLEScan;            

TM1637Display display(CLK, DIO);

/*
  Auswertung für jedes einzelne, gefundene BLE-Device 
*/

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {       // BLE Device gefunden
      if (advertisedDevice.haveServiceUUID()) {
        BLEUUID service = advertisedDevice.getServiceUUID();    // BLE Device bietet Dienste
        if (service.equals(BLEUUID((uint16_t)0xFD6F))) {        // UUID der Corona-Warn-App

          Serial.printf("MAC: %s ", advertisedDevice.getAddress().toString().c_str());
          // Ausgabe eindeutiger Adresse ...
          if (advertisedDevice.haveRSSI()) {                    // ... und Signalstärke
            Serial.print("CWA RSSI: ");
            Serial.println(advertisedDevice.getRSSI());
          } else
            Serial.println("CWA unknown RSSI");

          numCWA++;                                             // Anzahl der gefunden CWAs
          if (numCWA > thrCWA)                                  // Indikator an, wenn Schwelle überschritten
            digitalWrite(LED, HIGH);
        }
      }
    }
};


void setup() {
  Serial.begin(115200);                             // Infos auch über USB-Schnitstelle
  Serial.println("\nCorona-Warn-App Anzeiger\n");

  pinMode(LED, OUTPUT);

  BLEDevice::init("");                              // BLE-Funktion des ESP32 vorbereiten
  pBLEScan = BLEDevice::getScan();                  // Scan wird vorbereitet
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);                    // Aktiver Scan, ESP32 sendet Aufforderung
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  display.clear();
  display.setBrightness(3, true);                   // Helligkeit 0...7, Display ein

  for (int i = 0; i < 3; i++) {                     // bei Start blinkt LED und Anzeige
    digitalWrite(LED, HIGH);
    display.showNumberDec(8888);
    delay(100);
    digitalWrite(LED, LOW);
    display.clear();
    delay(100);
  }
}


void loop() {                                       // ewige Wiederholung des Scan-Zyklus

  numCWA = 0;                                       // Zähler für CWAs zurücksetzen
  //  Serial.println("scanning...");
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);       // BLE Scan starten
  //  Serial.print("devices found: ");
  //  Serial.println(foundDevices.getCount());
  //  Serial.println("scan done.");
  pBLEScan->clearResults();                         // erfasste BLE-Daten aus dem Scan löschen
                                                    // nur die Anzahl gefundener CWA kurz behalten
  if (numCWA > maxCWA)                              // ggf. als Höchstwert speichern  
    maxCWA = numCWA;

  Serial.print("Corona-Warn-Apps: ");               // Ausgaben über USB
  Serial.print(numCWA);
  Serial.print(" | ");
  Serial.println(maxCWA);
                                                  
  display.showNumberDecEx(numCWA * 100 + maxCWA, 0b01000000, false, 4, 0);  // Ausgabe auf Display
                                                                            // "numCWA:maxCWA"
  delay(idleTime * 1000);                           // kurze Pause
  digitalWrite(LED, LOW);                           // CWA-Indikator vor nächstem Scan ausschalten
}
