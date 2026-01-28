#include "CloudStatus.h"
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
bool CloudStatus::initWiFi() {
  Serial.println("\n[CloudStatus] Connecting to WiFi...");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < WIFI_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[CloudStatus] WiFi Connected!");
    Serial.print("[CloudStatus] IP Address: ");
    Serial.println(WiFi.localIP());
    cloudConnected = true;
    return true;
  } else {
    Serial.println("\n[CloudStatus] WiFi Connection Failed");
    cloudConnected = false;
    return false;
  }
}

/**
 * Send device status to cloud
 * @param isLocked - Current lock state
 * @param enteredPassword - Currently entered password
 * @return true if successfully sent, false otherwise
 */
bool CloudStatus::sendStatusToCloud(bool isLocked, String enteredPassword) {
  // Skip if not enough time has passed (throttle requests)
  if (millis() - lastStatusUpdate < STATUS_UPDATE_INTERVAL) {
    return false;
  }
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[CloudStatus] WiFi disconnected, skipping cloud update");
    return false;
  }
  
  // Create JSON payload
  String jsonPayload = "{";
  jsonPayload += "\"deviceId\":\"" + String(ESP.getEfuseMac()) + "\",";
  jsonPayload += "\"isLocked\":" + String(isLocked ? "true" : "false") + ",";
  jsonPayload += "\"timestamp\":" + String(millis()) + ",";
  jsonPayload += "\"ipAddress\":\"" + WiFi.localIP().toString() + "\"";
  jsonPayload += "}";
  
  Serial.print("[CloudStatus] Payload: ");
  Serial.println(jsonPayload);
  
  // Send HTTP POST request
  HTTPClient http;
  http.begin(CLOUD_ENDPOINT);
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.POST(jsonPayload);
  
  Serial.print("[CloudStatus] HTTP Response Code: ");
  Serial.println(httpResponseCode);
  
  if (httpResponseCode == 200) {
    Serial.println("[CloudStatus] Status sent successfully to cloud");
    lastStatusUpdate = millis();
    http.end();
    return true;
  } else {
    Serial.print("[CloudStatus] Failed to send status. Response: ");
    Serial.println(http.getString());
    http.end();
    return false;
  }
}

/**
 * Check if cloud connection is active
 */
bool CloudStatus::isCloudConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

/**
 * Get WiFi signal strength
 */
int CloudStatus::getSignalStrength() {
  return WiFi.RSSI();
}

/**
 * Disconnect from cloud/WiFi
 */
void CloudStatus::disconnect() {
  WiFi.disconnect(true); // true to turn off WiFi radio
  cloudConnected = false;
  Serial.println("[CloudStatus] Disconnected from WiFi");
}
