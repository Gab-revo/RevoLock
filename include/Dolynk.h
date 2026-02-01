#ifndef DOLYNK_H
#define DOLYNK_H

#include <Arduino.h>

String generate_uuid();
String get_timestamp_ms();
String hmac_sha512(const String& key, const String& data);
String sha512_hash(const String& data);
bool getAccessToken();
bool callApi(const char* abilityType, const char* status);
bool toggle_alarms(const char* state);

#endif // DOLYNK_H
