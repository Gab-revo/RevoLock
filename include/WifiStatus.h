#ifndef WIFI_STATUS_H
#define WIFI_STATUS_H

#include <Arduino.h>

// Status update interval (milliseconds) - throttle requests to avoid overloading the cloud
#define STATUS_UPDATE_INTERVAL 30000 // 30 seconds

class WifiStatus {
public:
  /**
   * Initialize WiFi connection
   * @return true if successfully connected, false otherwise
   */
  static bool initWiFi();
  
  
  /**
   * Check if cloud/WiFi connection is active
   * @return true if connected, false otherwise
   */
  static bool isWifiConnected();
  
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

#endif // WIFI_STATUS_H
