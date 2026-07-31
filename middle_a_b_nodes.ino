#include <WiFi.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>

const char* WIFI_SSID = "IndoorNav";
const char* WIFI_PASS = "12345678";
const char* MASTER_IP = "192.168.4.1";

String NODE_NAME = "B";

BLEScan* pBLEScan;

int minRSSI = -85;
bool foundStrong = false;

unsigned long lastReportTime = 0;
unsigned long reportCooldown = 5000;

BLEUUID targetUUID("12345678-1234-1234-1234-1234567890ab");

class MyCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    int rssi = advertisedDevice.getRSSI();

    if (advertisedDevice.haveServiceUUID()) {
      BLEUUID devUUID = advertisedDevice.getServiceUUID();

      if (devUUID.equals(targetUUID) && rssi >= minRSSI) {
        foundStrong = true;
        Serial.println("Target detected strongly at node B");
      }
    }
  }
};

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to hotspot");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected. IP = ");
  Serial.println(WiFi.localIP());
}

void sendReport() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastReportTime < reportCooldown) return;

  HTTPClient http;
  String url = "http://" + String(MASTER_IP) + "/report?node=" + NODE_NAME + "&name=NAV_USER_01";

  Serial.print("Sending report to: ");
  Serial.println(url);

  http.begin(url);
  int code = http.GET();

  Serial.print("HTTP code = ");
  Serial.println(code);
  Serial.println();

  http.end();
  lastReportTime = millis();
}

void setupBLE() {
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void setup() {
  Serial.begin(115200);
  connectWiFi();
  setupBLE();
}

void loop() {
  foundStrong = false;

  pBLEScan->start(3, false);
  pBLEScan->clearResults();

  if (foundStrong) {
    sendReport();
  }

  delay(500);
}