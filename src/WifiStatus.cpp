#include "WifiStatus.h"
#include "setup.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Cloud service endpoint (update with your actual cloud service URL)
#define CLOUD_ENDPOINT "https://your-cloud-service.com/api/device/status"

// Initialize WiFi connection status
bool cloudConnected = false;
unsigned long lastStatusUpdate = 0;

/**
 * Initialize WiFi connection for cloud communication
 */
bool WifiStatus::initWiFi() {
  Serial.println("\n[WifiStatus] Connecting to WiFi...");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < WIFI_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WifiStatus] WiFi Connected!");
    Serial.print("[WifiStatus] IP Address: ");
    Serial.println(WiFi.localIP());
    cloudConnected = true;
    return true;
  } else {
    Serial.println("\n[WifiStatus] WiFi Connection Failed");
    cloudConnected = false;
    return false;
  }
}

/**
 * Check if cloud connection is active
 */
bool WifiStatus::isWifiConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

/**
 * Get WiFi signal strength
 */
int WifiStatus::getSignalStrength() {
  return WiFi.RSSI();
}

/**
 * Disconnect from cloud/WiFi
 */
void WifiStatus::disconnect() {
  WiFi.disconnect(true); // true to turn off WiFi radio
  cloudConnected = false;
  Serial.println("[WifiStatus] Disconnected from WiFi");
}
