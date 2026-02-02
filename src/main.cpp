#include <Keypad.h>
#include <driver/rtc_io.h> // Required for pin holding
#include "setup.h"
#include "WifiStatus.h"
#include "Mailtrap.h"
#include "Dolynk.h"

#define TARGET_BOARD_ESP32

// --- SLEEP CONFIG ---
unsigned long lastActivityTime = 0;
const unsigned long SLEEP_TIMEOUT = 60000; //60 seconds of inactivity

/* =========================================================
  FUNCTION DECLARATIONS
   ========================================================= */
void updateLEDs();
void handlePasswordToggle();
void enterDeepSleep();
void flashLED(int pin, int times);

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

byte rowPins[ROWS] = {16,17,18,13};
byte colPins[COLS] = {26,25,33,32}; // Column 4 (GPIO32) is the wake-up pin

Keypad keypad = Keypad(makeKeymap(keymap), rowPins, colPins, ROWS, COLS);

/* =========================================================
   PASSWORD CONFIG
   ========================================================= */
String enteredPassword;
RTC_DATA_ATTR bool isLocked = false; // Persists in RTC memory during sleep
unsigned long lastPasswordInputTime = 0;
const unsigned long PASSWORD_TIMEOUT = 30000; // 30 seconds

/* =========================================================
   LED STATE ENUM
   ========================================================= */
enum LEDState { LOCKED, ENTERING, UNLOCKED };

/* =========================================================
   SETUP
   ========================================================= */
void setup() {
  // Disable GPIO hold from previous deep sleep
  gpio_hold_dis(GPIO_NUM_13); 
  rtc_gpio_pulldown_dis(GPIO_NUM_26); // Disable the sleep pulldown

  Serial.begin(115200);
  delay(500); // Let serial stabilize
  Serial.println("\n\n=== System Waking Up ===");


  // Configure col pins as inputs with pull-ups (they become high when not pressed)
  for (int i = 0; i < COLS; i++) {
    pinMode(colPins[i], INPUT_PULLUP);
  }
  
  // Configure row pins as outputs (low = scan, high = idle)
  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);
  }

  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);

  //Turn on yellow LED during setup
  digitalWrite(YELLOW_PIN, HIGH);

  WifiStatus::initWiFi();
  enteredPassword.reserve(16);
  enteredPassword = ""; // Clear password on wake (start fresh)
  
  Serial.print("System initialized - Lock state: ");
  Serial.println(isLocked ? "LOCKED" : "UNLOCKED");

    configTime(0, 0, "pool.ntp.org");
    while (time(nullptr) < 1000000000) delay(500);

  delay(1000); // Wait for initialization to complete

  // Turn off yellow LED after setup
  digitalWrite(YELLOW_PIN, LOW);
  
  if (!WifiStatus::isWifiConnected()) {
    //flash red LED 3 times
    flashLED(RED_PIN, 3);
  }else{
    //flash green LED 3 times
    flashLED(GREEN_PIN, 3);
  }
  
  // === TOGGLE HERE ===
  toggle_alarms("off");  // Change to "on" or "off"

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
    
    flashLED(YELLOW_PIN, 1);
    
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
        break;
    }
  }
    
  // Update LEDs based on current state
  updateLEDs();
  
  // Track last password input time
  if (key) {
    lastPasswordInputTime = millis();
  }
  
  // Clear password if timeout exceeded
  if (enteredPassword != "" && millis() - lastPasswordInputTime > PASSWORD_TIMEOUT) {
    Serial.println("Password entry timeout - clearing");
    enteredPassword = "";
  }

  delay(100); //Simple debounce for keypad
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
    toggle_alarms("on");
  //   Serial.println("SITE LOCKED");
  //   Mailtrap::sendLockStatusEmail(
  //     MAILTRAP_RECIPIENT, 
  //     "Admin",
  //     true
  //   );
  } else {
    toggle_alarms("off");
  //   Serial.println("SITE UNLOCKED");
  //   Mailtrap::sendLockStatusEmail(
  //     MAILTRAP_RECIPIENT, 
  //     "Admin",
  //     false
  //   );
  }
}

/* =========================================================
   UPDATE LEDS BASED ON STATE
   ========================================================= */
void updateLEDs() {
  LEDState state;
  
  if (enteredPassword != "" ) state = ENTERING;
  else if (isLocked) state = LOCKED;
  else state = UNLOCKED;

  digitalWrite(GREEN_PIN, state == UNLOCKED);
  digitalWrite(YELLOW_PIN, state == ENTERING);
  digitalWrite(RED_PIN, state == LOCKED);
}

void flashLED(int pin, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(100);
    digitalWrite(pin, LOW);
    delay(100);
  }
}

/* =========================================================
   ENTER DEEP SLEEP ON INACTIVITY
   ========================================================= */
void enterDeepSleep() {
  Serial.println("Entering Sleep (Key-Intersection Mode)...");

  // 1. Prepare the 'Source' Row
  rtc_gpio_init(GPIO_NUM_13);
  rtc_gpio_set_direction(GPIO_NUM_13, RTC_GPIO_MODE_OUTPUT_ONLY);
  rtc_gpio_set_level(GPIO_NUM_13, 1); // Set HIGH
  gpio_hold_en(GPIO_NUM_13);         // Lock it HIGH during sleep

  // 2. Prepare the 'Trigger' Column
  rtc_gpio_init(GPIO_NUM_26);
  rtc_gpio_set_direction(GPIO_NUM_26, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pulldown_en(GPIO_NUM_26); // Keep it LOW
  rtc_gpio_pullup_dis(GPIO_NUM_26);

  // 3. Enable wake-up when Column goes HIGH
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_26, 1);
  // 4. Global hold enable
  gpio_deep_sleep_hold_en();
  
  Serial.flush();
  esp_deep_sleep_start();
}
