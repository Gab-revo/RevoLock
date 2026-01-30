#include <Keypad.h>
#include "setup.h"
#include "CloudStatus.h"
#include "Mailtrap.h"
#include <driver/rtc_io.h> // Required for pin holding

#define TARGET_BOARD_ESP32

// --- SLEEP CONFIG ---
unsigned long lastActivityTime = 0;
const unsigned long SLEEP_TIMEOUT = 5000; //20seconds of inactivity

/* =========================================================
  FUNCTION DECLARATIONS
   ========================================================= */
void updateLEDs();
void handlePasswordToggle();
void enterDeepSleep();

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
byte colPins[COLS] = {26,25,33,32};

Keypad keypad = Keypad(makeKeymap(keymap), colPins, rowPins, ROWS, COLS);

/* =========================================================
   PASSWORD CONFIG
   ========================================================= */
String enteredPassword;
RTC_DATA_ATTR bool isLocked = false; // Persists in RTC memory during sleep

/* =========================================================
   LED STATE ENUM
   ========================================================= */
enum LEDState { LOCKED, ENTERING, UNLOCKED };

/* =========================================================
   SETUP
   ========================================================= */
void setup() {
  // Disable GPIO hold from previous deep sleep
  gpio_hold_dis(GPIO_NUM_32); 
  rtc_gpio_pulldown_dis(GPIO_NUM_32); // Disable the sleep pulldown

  Serial.begin(115200);
  delay(500); // Let serial stabilize
  Serial.println("\n\n=== System Waking Up ===");


  // Configure row pins as inputs with pull-ups (they become high when not pressed)
  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], INPUT_PULLUP);
  }
  
  // Configure column pins as outputs (low = scan, high = idle)
  for (int i = 0; i < COLS; i++) {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], HIGH);
  }

  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);

  //Turn on yellow LED during setup
  digitalWrite(YELLOW_PIN, HIGH);

  CloudStatus::initWiFi();
  enteredPassword.reserve(16);
  enteredPassword = ""; // Clear password on wake (start fresh)
  
  Serial.print("System initialized - Lock state: ");
  Serial.println(isLocked ? "LOCKED" : "UNLOCKED");
  Serial.println("System initialized");

  delay(2000); // Wait for WiFi to stabilize

 digitalWrite(YELLOW_PIN, LOW);
  if (!CloudStatus::isCloudConnected()) {
    //flasg red LED 3 times
    for (int i = 0; i < 3; i++) {
      digitalWrite(RED_PIN, HIGH);
      delay(500);
      digitalWrite(RED_PIN, LOW);
      delay(500);
    }
  }else{
    //flasg green LED 3 times
    for (int i = 0; i < 3; i++) {
      digitalWrite(GREEN_PIN, HIGH);
      delay(500);
      digitalWrite(GREEN_PIN, LOW);
      delay(500);
    }
  }
  
  lastActivityTime = millis(); // Reset timer on boot
  updateLEDs();
}

/* =========================================================
   LOOP
   ========================================================= */
void loop() {
  // Check for inactivity timeout (do this before returning)
  if (millis() - lastActivityTime > SLEEP_TIMEOUT) {
    Serial.println("Timeout - entering sleep");
    enterDeepSleep();
  }

  // Scan keypad - this must happen every loop
  char key = keypad.getKey();
  
  if (key) {
    // Key was pressed
    lastActivityTime = millis(); // Reset inactivity timer
    
    Serial.print("Key pressed: ");
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
        Serial.print("Entered password: ");
        Serial.println(enteredPassword);
        break;
    }
    
    // Update LEDs based on current state
    updateLEDs();

    // Send status to cloud
    CloudStatus::sendStatusToCloud(isLocked, enteredPassword);
  }
  
  delay(20); // Small delay to allow keypad scanning
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

/* =========================================================
   ENTER DEEP SLEEP ON INACTIVITY
   ========================================================= */
void enterDeepSleep() {
  Serial.println("Entering Deep Sleep...");
  Serial.flush();
  
  // 1. Turn off LEDs
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(RED_PIN, LOW);

  // 2. Configure GPIO 32 (Column 4 - wake-up pin) with pull-down
  rtc_gpio_pulldown_en(GPIO_NUM_32);
  rtc_gpio_pullup_dis(GPIO_NUM_32);

  // 3. Set Wake-up on GPIO 32 when HIGH (button press)
  esp_sleep_enable_ext1_wakeup((1ULL << 32), ESP_EXT1_WAKEUP_ANY_HIGH);

  esp_deep_sleep_start();
}
