# Smart Attendance System

A fingerprint-based attendance system built around an ESP8266, an Adafruit-compatible fingerprint sensor, and an OLED display. The project includes two Arduino sketches:

- **Enrollment sketch** to register fingerprints into the sensor template memory.
- **Attendance sketch** to authenticate users and push attendance events to Google Sheets through Google Apps Script.

## Repository Contents

- `project_FingerprintEnrollment.ino` – Captures and stores fingerprint templates with user-selected IDs (1–127).
- `project_FingerprintBasedAttendance.ino` – Main runtime sketch for scan/identify + upload to Google Sheets over HTTPS.
- `GAS.txt` – Google Apps Script `doGet(e)` endpoint used by the device to append attendance rows to a sheet.
- `Deploy.txt` – Deployed Apps Script ID and web app URL.

## System Workflow

1. Enroll users in the fingerprint sensor using `project_FingerprintEnrollment.ino`.
2. Run `project_FingerprintBasedAttendance.ino` on the same hardware.
3. User scans fingerprint.
4. ESP8266 matches the fingerprint template locally.
5. Matched `fingerID` is sent to Google Apps Script as `Value1`.
6. Apps Script appends timestamp + ID into the configured Google Sheet.

## Attendance Logic in the Current Sketch

The current `project_FingerprintBasedAttendance.ino` encodes a simple session flow using `isFaculty`:

- `fingerID == 1` is treated as **faculty**.
- First faculty scan toggles **Faculty Sign-In**.
- Second faculty scan toggles **Faculty Sign-Out**.
- Student fingerprints (`fingerID != 1`) are accepted only while faculty is signed in.

This allows attendance capture only during an active faculty session.

## Hardware Requirements

- ESP8266 development board
- UART fingerprint sensor module (Adafruit-compatible protocol)
- 128x64 I2C OLED display (SSD1306)
- Jumper wires and stable power supply

## Pin Mapping (as coded)

From both sketches:

- Fingerprint sensor **TX -> ESP8266 D5 (GPIO14)** (`FP_Rx_Pin 14`)
- Fingerprint sensor **RX -> ESP8266 D6 (GPIO12)** (`FP_Tx_Pin 12`)
- OLED via I2C at address `0x3C`

> Verify your board's D-pin to GPIO mapping before wiring.

## Software / Library Requirements

Install in Arduino IDE:

- **ESP8266 by ESP8266 Community** (board package)
- **Adafruit GFX Library**
- **Adafruit SSD1306**
- **Adafruit Fingerprint Sensor Library**

Used headers include:

- `ESP8266WiFi.h`
- `WiFiClientSecure.h`
- `Wire.h`
- `Adafruit_GFX.h`
- `Adafruit_SSD1306.h`
- `Adafruit_Fingerprint.h`

## Setup Guide

### 1) Enroll Fingerprints

1. Open `project_FingerprintEnrollment.ino`.
2. Select ESP8266 board and correct COM port.
3. Upload sketch.
4. Open Serial Monitor at **9600 baud**.
5. Enter an ID from **1 to 127** and follow prompts to place/remove/place the same finger.
6. Repeat for all users.

### 2) Configure Attendance Sketch

In `project_FingerprintBasedAttendance.ino`, update:

- `ssid` and `pass`
- `GAS_ID` (Apps Script deployment ID)
- (Optional) TLS handling strategy

Then upload the sketch and monitor serial output.

### 3) Configure Google Apps Script

1. Create a Google Sheet for attendance.
2. Create Apps Script project and paste code from `GAS.txt`.
3. Replace `sheet_id` in script with your Sheet ID.
4. Deploy as Web App (execute as you, accessible to required users/devices).
5. Copy deployment ID/URL into `GAS_ID` and/or `Deploy.txt`.

## Google Sheet Data Format

The current script writes:

- **Column A**: timestamp (`new Date()`)
- **Column B**: `Value1` (fingerprint ID)

## Security Notes (Important)

The current repository stores sensitive values in plain text (Wi-Fi credentials, script IDs, deployment URL). Before production use:

- Rotate exposed Wi-Fi credentials.
- Avoid committing credentials and IDs to source control.
- Prefer compile-time config headers excluded by `.gitignore`.
- Restrict Apps Script web app access.
- Validate incoming parameters more strictly server-side.

## Troubleshooting

- **"Did not find fingerprint sensor"**: check UART wiring, power, and baud expectations.
- **OLED not starting (`SSD1306 allocation failed`)**: verify I2C wiring/address (`0x3C` vs `0x3D`).
- **No uploads to Sheet**: verify Wi-Fi, Apps Script deployment permissions, and correct `GAS_ID`.
- **Frequent HTTPS failures**: the sketch uses `client.setInsecure()`; network filtering/certs may still interfere.

## Suggested Improvements

- Add user mapping table (ID -> name/role) in Google Sheet.
- Send structured parameters (`id`, `role`, `event`, `device`).
- Add duplicate-scan debouncing and retry logic.
- Replace insecure TLS mode with certificate pinning/validation strategy.
- Move secrets to a local, ignored configuration file.

## License

No explicit license is currently provided in this repository. Add a `LICENSE` file to define usage terms.
