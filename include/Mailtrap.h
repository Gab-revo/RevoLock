#ifndef MAILTRAP_H
#define MAILTRAP_H

#include <Arduino.h>

class Mailtrap {
public:
    static bool sendEmail(const char* fromEmail, const char* fromName,
                          const char* toEmail, const char* toName,
                          const char* subject, const char* textBody, 
                          const char* htmlBody = nullptr);

    static bool sendSimpleEmail(const char* toEmail, const char* toName,
                                 const char* subject, const char* textBody);
};

#endif
