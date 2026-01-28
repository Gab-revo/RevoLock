#ifndef SETUP_H
#define SETUP_H

// ==========================================
// USER CONFIGURATION
// ==========================================

// Device Password (for lock/unlock functionality)
#define DEVICE_PASSWORD "1234"

// WiFi Configuration
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"

// Mailtrap API Credentials
#define MAILTRAP_TOKEN "your_mailtrap_api_token"
#define MAILTRAP_SANDBOX_ID 1234567
#define MAILTRAP_SENDER "noreply@revolock.com"
#define MAILTRAP_RECIPIENT "admin@revolock.com"
// ==========================================
// Optional: WiFi Connection Timeout (ms)
// ==========================================
#define WIFI_TIMEOUT 10000 // 10 seconds

#endif // SETUP_H
