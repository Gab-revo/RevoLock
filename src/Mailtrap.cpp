#include "Mailtrap.h"
#include "setup.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Mailtrap Sandbox API configuration (for testing)
#define MAILTRAP_API_URL "https://sandbox.api.mailtrap.io/api/send/" + String(MAILTRAP_SANDBOX_ID)

/**
 * Send email via Mailtrap
 * @param fromEmail - Sender email address
 * @param fromName - Sender name
 * @param toEmail - Recipient email address
 * @param toName - Recipient name
 * @param subject - Email subject
 * @param textBody - Plain text body
 * @param htmlBody - HTML body (optional)
 * @return true if email sent successfully, false otherwise
 */
bool Mailtrap::sendEmail(const char* fromEmail, const char* fromName,
                         const char* toEmail, const char* toName,
                         const char* subject, const char* textBody, 
                         const char* htmlBody) {
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Mailtrap] WiFi not connected");
    return false;
  }

  // Build JSON payload for Mailtrap API
  String payload = "{";
  payload += "\"from\":{\"email\":\"" + String(fromEmail) + "\",\"name\":\"" + String(fromName) + "\"},";
  payload += "\"to\":[{\"email\":\"" + String(toEmail) + "\",\"name\":\"" + String(toName) + "\"}],";
  payload += "\"subject\":\"" + String(subject) + "\",";
  payload += "\"text\":\"" + String(textBody) + "\"";
  
  if (htmlBody != nullptr && strlen(htmlBody) > 0) {
    payload += ",\"html\":\"" + String(htmlBody) + "\"";
  }
  
  payload += "}";

  Serial.println("[Mailtrap] Sending email...");
  Serial.print("[Mailtrap] Payload: ");
  Serial.println(payload);

  // Send HTTP POST request to Mailtrap Sandbox
  HTTPClient http;
  String apiUrl = "https://sandbox.api.mailtrap.io/api/send/" + String(MAILTRAP_SANDBOX_ID);
  http.begin(apiUrl);
  
  // Set required headers
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Api-Token", MAILTRAP_TOKEN);

  int httpResponseCode = http.POST(payload);

  Serial.print("[Mailtrap] HTTP Response Code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode == 200 || httpResponseCode == 201) {
    Serial.println("[Mailtrap] Email sent successfully!");
    http.end();
    return true;
  } else {
    Serial.print("[Mailtrap] Failed to send email. Response: ");
    Serial.println(http.getString());
    http.end();
    return false;
  }
}

/**
 * Send a simple text email
 */
bool Mailtrap::sendSimpleEmail(const char* toEmail, const char* toName,
                               const char* subject, const char* textBody) {
  return sendEmail(MAILTRAP_SENDER, "Smart Lock System", 
                   toEmail, toName, subject, textBody, nullptr);
}