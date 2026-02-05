# RevoLock

An ESP32-based smart lock system with keypad authentication, WiFi connectivity, and IoT integration. RevoLock provides a secure access control solution with deep sleep power management, DoLynk cloud integration, and optional email notifications via Mailtrap.

## Features

- **Keypad Authentication**: 4x4 matrix keypad for password entry
- **Visual Feedback**: LED indicators for lock status
  - 🔴 Red: System locked
  - 🟢 Green: System unlocked
  - 🟡 Yellow: Password entry in progress
- **Deep Sleep Mode**: Automatic sleep after 60 seconds of inactivity for power conservation
- **Wake-on-Key**: ESP32 wakes from deep sleep when '*'
- **IoT Integration**: DoLynk cloud platform integration for remote alarm control
- **Email Notifications**: Optional Mailtrap integration for lock status updates
- **WiFi Connectivity**: Automatic WiFi connection on startup
- **Persistent State**: Lock state is retained through deep sleep using RTC memory

## Hardware Requirements

- ESP32 Development Board
- 4x4 Matrix Keypad
- 3x LEDs (Red, Yellow, Green) with appropriate resistors
- Jumper wires and breadboard

## Pin Configuration

### LEDs
- Green LED: GPIO 27 (Unlocked state)
- Yellow LED: GPIO 14 (Entering password)
- Red LED: GPIO 12 (Locked state)

### Keypad
**Rows:**
- Row 1: GPIO 16
- Row 2: GPIO 17
- Row 3: GPIO 18
- Row 4: GPIO 13

**Columns:**
- Column 1: GPIO 26 (Wake-up pin)
- Column 2: GPIO 25
- Column 3: GPIO 33
- Column 4: GPIO 32

## Software Setup

### Prerequisites

- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- ESP32 board support

### Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/RevoLock.git
   cd RevoLock
   ```

2. Create your configuration file:
   ```bash
   cp include/setup.h.example include/setup.h
   ```

3. Edit `include/setup.h` with your credentials:
   ```cpp
   // Device Password
   #define DEVICE_PASSWORD "your_password"
   
   // WiFi Configuration
   #define WIFI_SSID "your_wifi_ssid"
   #define WIFI_PASSWORD "your_wifi_password"
   
   // DoLynk API Credentials
   #define ACCESS_KEY "your_access_key"
   #define SECRET_ACCESS_KEY "your_secret_access_key"
   #define PRODUCT_ID "your_product_id"
   #define DEVICE_ID "your_device_id"
   
   // Mailtrap API Credentials (Optional)
   #define MAILTRAP_TOKEN "your_mailtrap_token"
   #define MAILTRAP_SANDBOX_ID "your_sandbox_id"
   #define MAILTRAP_SENDER "noreply@revolock.com"
   #define MAILTRAP_RECIPIENT "admin@revolock.com"
   ```

4. Build and upload:
   ```bash
   pio run --target upload
   ```

5. Monitor serial output:
   ```bash
   pio device monitor
   ```

## Usage

### Basic Operation

1. **Enter Password**: Type your password on the keypad
   - Yellow LED illuminates during password entry
   - Entered digits are masked in serial output as `###`

2. **Clear Input**: Press `*` to clear the current password entry

3. **Submit Password**: Press `#` to submit and toggle lock state
   - If password is correct, lock state toggles
   - If password is incorrect, "ACCESS DENIED" message appears

4. **Lock States**:
   - **Locked** (Red LED): System is secured, DoLynk alarms enabled
   - **Unlocked** (Green LED): System is open, DoLynk alarms disabled

### Power Management

- System automatically enters deep sleep after 60 seconds of inactivity
- Press any key on the keypad to wake the system
- Lock state persists through sleep cycles using RTC memory

### Timeouts

- **Password Entry**: 30 seconds to complete password entry
- **Sleep Timer**: 60 seconds of inactivity before deep sleep

## Project Structure

```
RevoLock/
├── platformio.ini          # PlatformIO configuration
├── include/
│   ├── setup.h.example     # Configuration template
│   ├── setup.h             # Your credentials (gitignored)
│   ├── Dolynk.h            # DoLynk API declarations
│   ├── Mailtrap.h          # Mailtrap email declarations
│   └── WifiStatus.h        # WiFi management declarations
├── lib/
│   └── Keypad/             # Keypad library
├── src/
│   ├── main.cpp            # Main application logic
│   ├── Dolynk.cpp          # DoLynk API implementation
│   ├── Mailtrap.cpp        # Mailtrap email implementation
│   └── WifiStatus.cpp      # WiFi management implementation
└── test/
```

## Dependencies

- **ArduinoJson** (^7.0.0): JSON parsing for API communication
- **Keypad Library**: Matrix keypad handling (included in `lib/`)

## DoLynk Integration

RevoLock integrates with the DoLynk IoT platform to control alarm systems:

- When locked: Sends "on" command to DoLynk alarm API
- When unlocked: Sends "off" command to DoLynk alarm API
- Uses HMAC-SHA512 authentication for secure API access
- Supports automatic token refresh and request signing

## Security Considerations

⚠️ **Important Security Notes:**

1. **Never commit `setup.h`** - Contains sensitive credentials
2. Change the default password from `1234` to a secure code
3. Use strong WiFi passwords
4. Keep API keys and tokens confidential
5. Consider enabling HTTPS for production deployments
6. Review and update credentials regularly

## Troubleshooting

### WiFi Connection Issues
- Red LED flashes 3 times on startup if WiFi fails to connect
- Check SSID and password in `setup.h`
- Ensure WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)

### Keypad Not Responding
- Verify pin connections match the configuration
- Check keypad library is properly installed in `lib/`
- Use serial monitor to debug key press detection

### Deep Sleep Issues
- Ensure GPIO 13 and GPIO 26 are correctly configured
- Check that the wake-up key is being properly detected
- Review serial output before sleep for debugging

### DoLynk API Errors
- Verify all DoLynk credentials in `setup.h`
- Check internet connectivity
- Review serial output for API response codes

## Development

### Building
```bash
pio run
```

### Uploading
```bash
pio run --target upload
```

### Serial Monitor
```bash
pio device monitor -b 115200
```

### Clean Build
```bash
pio run --target clean
```

## License

This project is provided as-is for educational and personal use.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Author

Created for secure access control applications using ESP32 microcontrollers.

## Acknowledgments

- [Keypad Library](https://github.com/Chris--A/Keypad) by Chris--A
- DoLynk IoT Platform
- Mailtrap Email Service
