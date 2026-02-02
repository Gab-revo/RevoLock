#include <WiFi.h>
#include <HTTPClient.h>
#include <mbedtls/md.h>
#include <ArduinoJson.h>
#include "setup.h"

String app_access_token = "";

String generate_uuid() {
    char uuid[37];
    snprintf(uuid, sizeof(uuid), "%08x-%04x-4%03x-%04x-%04x%08x",
             esp_random(), (esp_random() >> 16) & 0xFFFF, esp_random() & 0x0FFF,
             (esp_random() >> 16 & 0x3FFF) | 0x8000, esp_random() & 0xFFFF, esp_random());
    return String(uuid);
}

String get_timestamp_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return String((uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

String hmac_sha512(const String& key, const String& data) {
    unsigned char result[64];
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA512), 1);
    mbedtls_md_hmac_starts(&ctx, (unsigned char*)key.c_str(), key.length());
    mbedtls_md_hmac_update(&ctx, (unsigned char*)data.c_str(), data.length());
    mbedtls_md_hmac_finish(&ctx, result);
    mbedtls_md_free(&ctx);
    
    String hex = "";
    for (int i = 0; i < 64; i++) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02X", result[i]);
        hex += buf;
    }
    return hex;
}

String sha512_hash(const String& data) {
    unsigned char result[64];
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA512), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (unsigned char*)data.c_str(), data.length());
    mbedtls_md_finish(&ctx, result);
    mbedtls_md_free(&ctx);
    
    String hex = "";
    for (int i = 0; i < 64; i++) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", result[i]);
        hex += buf;
    }
    return hex;
}

bool getAccessToken() {
    String timestamp = get_timestamp_ms();
    String nonce = "web-" + generate_uuid() + "-" + timestamp;
    String signature = hmac_sha512(SECRET_ACCESS_KEY, String(ACCESS_KEY) + timestamp + nonce + "POST");
    
    HTTPClient http;
    http.begin(String(BASE_URL) + "/api-base/auth/getAppAccessToken");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Version", "v1");
    http.addHeader("AccessKey", ACCESS_KEY);
    http.addHeader("Timestamp", timestamp);
    http.addHeader("Nonce", nonce);
    http.addHeader("X-TraceId-Header", generate_uuid());
    http.addHeader("ProductId", PRODUCT_ID);
    http.addHeader("Sign", signature);
    
    int httpCode = http.POST("{}");
    String response = http.getString();
    
    Serial.print("[Dolynk] getAccessToken HTTP Code: ");
    Serial.println(httpCode);
    Serial.print("[Dolynk] Response: ");
    Serial.println(response);
    
    if (httpCode == 200) {
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, response);
        if (error) {
            Serial.print("[Dolynk] JSON Parse Error: ");
            Serial.println(error.c_str());
            http.end();
            return false;
        }
        if (doc["code"].as<String>() == "200") {
            app_access_token = doc["data"]["appAccessToken"].as<String>();
            Serial.print("[Dolynk] Token obtained: ");
            Serial.println(app_access_token);
            http.end();
            return true;
        } else {
            Serial.print("[Dolynk] API returned code: ");
            Serial.println(doc["code"].as<String>());
        }
    }
    http.end();
    return false;
}

bool callApi(const char* abilityType, const char* status) {
    if (app_access_token.isEmpty() && !getAccessToken()) return false;
    
    String timestamp = get_timestamp_ms();
    String nonce = "web-" + generate_uuid() + "-" + timestamp;
    String body = "{\"deviceId\":\"" + String(DEVICE_ID) + "\",\"channelId\":\"0\",\"abilityType\":\"" + abilityType + "\",\"status\":\"" + status + "\"}";
    String signature = hmac_sha512(SECRET_ACCESS_KEY, String(ACCESS_KEY) + app_access_token + timestamp + nonce + "POST\n" + sha512_hash(body));
    
    HTTPClient http;
    http.begin(String(BASE_URL) + "/api-iot/device/setAbilityStatus");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Version", "v1");
    http.addHeader("AccessKey", ACCESS_KEY);
    http.addHeader("AppAccessToken", app_access_token);
    http.addHeader("Timestamp", timestamp);
    http.addHeader("Nonce", nonce);
    http.addHeader("X-TraceId-Header", generate_uuid());
    http.addHeader("ProductId", PRODUCT_ID);
    http.addHeader("Sign", signature);
    
    int httpCode = http.POST(body);
    String response = http.getString();
    http.end();
    
    StaticJsonDocument<512> doc;
    deserializeJson(doc, response);
    return doc["code"].as<String>() == "200";
}

bool toggle_alarms(const char* state) {
    String status = String(state);
    status.toLowerCase();
    
    if (status == "on") callApi("motionDetect", "off");
    
    bool siren = callApi("linkDevAlarm", status.c_str());
    bool strobe = callApi("linkageWhiteLight", status.c_str());
    
    Serial.printf("Alarms %s: Siren=%s, Strobe=%s\n", 
                  status == "on" ? "ON" : "OFF",
                  siren ? "OK" : "FAIL", 
                  strobe ? "OK" : "FAIL");
    return siren && strobe;
}

// void setup() {
//     Serial.begin(115200);
//     WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//     while (WiFi.status() != WL_CONNECTED) delay(500);
//     configTime(0, 0, "pool.ntp.org");
//     while (time(nullptr) < 1000000000) delay(500);
    
//     // === TOGGLE HERE ===
//     toggle_alarms("off");  // Change to "on" or "off"
// }

// void loop() {}
