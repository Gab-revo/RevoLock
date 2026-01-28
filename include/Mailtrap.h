#ifndef MAILTRAP_H
#define MAILTRAP_H

#include <Arduino.h>

class Mailtrap {
public:
  /**
   * Send email via Mailtrap API
   * @param fromEmail - Sender email address
   * @param fromName - Sender name
   * @param toEmail - Recipient email address
   * @param toName - Recipient name
   * @param subject - Email subject
   * @param textBody - Plain text body
   * @param htmlBody - HTML body (optional, pass nullptr for text-only)
   * @return true if email sent successfully, false otherwise
   */
  static bool sendEmail(const char* fromEmail, const char* fromName,
                        const char* toEmail, const char* toName,
                        const char* subject, const char* textBody,
                        const char* htmlBody = nullptr);

  /**
   * Send a simple text email
   * @param toEmail - Recipient email address
   * @param toName - Recipient name
   * @param subject - Email subject
   * @param textBody - Plain text body
   * @return true if email sent successfully, false otherwise
   */
  static bool sendSimpleEmail(const char* toEmail, const char* toName,
                              const char* subject, const char* textBody);
};

#endif // MAILTRAP_H
