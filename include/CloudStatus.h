#ifndef CLOUD_STATUS_H
#define CLOUD_STATUS_H

#include <Arduino.h>

// Status update interval (milliseconds) - throttle requests to avoid overloading the cloud
#define STATUS_UPDATE_INTERVAL 30000 // 30 seconds

class CloudStatus {
public:
  /**
   * Initialize WiFi connection
   * @return true if successfully connected, false otherwise
   */
  static bool initWiFi();
  
  /**
   * Send device status to cloud
   * @param isLocked - Current lock state (true = locked, false = unlocked)
   * @param enteredPassword - Currently entered password for logging
   * @return true if successfully sent, false otherwise
   */
  static bool sendStatusToCloud(bool isLocked, String enteredPassword);
  
  /**
   * Check if cloud/WiFi connection is active
   * @return true if connected, false otherwise
   */
  static bool isCloudConnected();
  
  /**
   * Get WiFi signal strength in dBm
   * @return RSSI value (negative value, closer to 0 is stronger)
   */
  static int getSignalStrength();
  
  /**
   * Disconnect from WiFi
   */
  static void disconnect();
};

#endif // CLOUD_STATUS_H
