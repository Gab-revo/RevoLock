#include <Keypad.h>
#include "setup.h"
#include "CloudStatus.h"
#include "Mailtrap.h"

#define TARGET_BOARD_ESP32

/* =========================================================
  FUNCTION DECLARATIONS
   ========================================================= */
void updateLEDs();
void handlePasswordToggle();

/* =========================================================
   PIN CONFIG
   ========================================================= */
#define GREEN_PIN 27
#define YELLOW_PIN 14
#define RED_PIN 12

/* =========================================================
   KEYPAD CONFIG
   ========================================================= */
#define ROWS 4
#define COLS 4

char keymap[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {16,17,18,19};
//byte colPins[COLS] = {14,27,26,25};
byte colPins[COLS] = {26,25,33,32};

Keypad keypad = Keypad(makeKeymap(keymap), colPins, rowPins, COLS, ROWS);

/* =========================================================
   PASSWORD CONFIG
   ========================================================= */
String enteredPassword;
bool isLocked = false;

/* =========================================================
   LED STATE ENUM
   ========================================================= */
enum LEDState { LOCKED, ENTERING, UNLOCKED };

/* =========================================================
   SETUP
   ========================================================= */
void setup() {
  Serial.begin(115200);

  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);

  //Turn on yellow LED during setup
  digitalWrite(YELLOW_PIN, HIGH);

  CloudStatus::initWiFi();
  enteredPassword.reserve(16);
  Serial.println("System initialized");

  // Test email on startup
  delay(2000); // Wait for WiFi to stabilize

  updateLEDs();
  // if (CloudStatus::isCloudConnected()) {
  //   Serial.println("Sending test email...");
  //   bool emailSent = Mailtrap::sendSimpleEmail(
  //     MAILTRAP_RECIPIENT, 
  //     "Admin",
  //     "RevoLock System Started",
  //     "Your RevoLock smart lock system has successfully started and connected to WiFi."
  //   );
  //   if (emailSent) {
  //     Serial.println("Test email sent successfully!");
  //   } else {
  //     Serial.println("Failed to send test email");
  //   }
  // }
}

/* =========================================================
   LOOP
   ========================================================= */
void loop() {

  char key = keypad.getKey();
  if (!key) return; // no key pressed

  Serial.println(key);

  // Handle special keys
  switch (key) {
    case '*': // clear input
      enteredPassword = "";
      Serial.println("Input cleared");
      break;

    case '#': // submit password to toggle lock/unlock
      handlePasswordToggle();
      enteredPassword = ""; // always clear after #
      break;

    default: // regular key
      enteredPassword += key;
      Serial.print("Entered: ");
      Serial.println(enteredPassword);
      break;
  }
  
  // Update LEDs based on current state
  updateLEDs();

  // Send status to cloud
  CloudStatus::sendStatusToCloud(isLocked, enteredPassword);

  delay(100); // simple debounce
}

/* =========================================================
   HANDLE PASSWORD TOGGLE
   ========================================================= */
void handlePasswordToggle() {
  if (enteredPassword != DEVICE_PASSWORD) {
    Serial.print("ACCESS DENIED, entered: ");
    Serial.println(enteredPassword);
    return; // do nothing if password is wrong
  }

  // correct password → toggle lock
  isLocked = !isLocked;

  if (isLocked){
    Serial.println("SITE LOCKED");
    Mailtrap::sendLockStatusEmail(
      MAILTRAP_RECIPIENT, 
      "Admin",
      true
    );
  } else {
    Serial.println("SITE UNLOCKED");
    Mailtrap::sendLockStatusEmail(
      MAILTRAP_RECIPIENT, 
      "Admin",
      false
    );
  }
}

/* =========================================================
   UPDATE LEDS BASED ON STATE
   ========================================================= */
void updateLEDs() {
  LEDState state;

  if (!isLocked && enteredPassword == "") state = UNLOCKED;
  else if (enteredPassword == "") state = LOCKED;
  else state = ENTERING;

  digitalWrite(GREEN_PIN, state == UNLOCKED);
  digitalWrite(YELLOW_PIN, state == ENTERING);
  digitalWrite(RED_PIN, state == LOCKED);
}
