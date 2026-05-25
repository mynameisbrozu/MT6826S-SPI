/*!
 * @file mt6826s_zero_position.ino
 *
 * This example demonstrates the two zero position calibration functions
 * available on the MT6826S SPI library:
 *
 *  1. setZeroPosition(MT6826S_ANGLE_0)
 *     Resets the zero reference to absolute 0 (factory default).
 *
 *  2. setZeroPosition(MT6826S_CURRENT_ANGLE)
 *     Captures the current shaft position and sets it as the new zero.
 *     Use this to calibrate the encoder in-place without moving the shaft.
 *
 *  3. setSpecificZeroPosition(angle12Bit)
 *     Sets a precise 12-bit zero offset [0, 4095].
 *     Useful when the desired zero position is known in advance.
 *
 * The example uses Serial input to trigger each calibration interactively:
 *  - Send '1' to reset zero to absolute 0
 *  - Send '2' to set zero to current shaft angle
 *  - Send '3' to set a specific 12-bit zero value (defined below)
 *  - Send 'r' to read the current angle
 *
 * - All setZeroPosition calls write to RAM only.
 *   Send 'e' over Serial to persist the current zero to EEPROM.
 * - The EEPROM supports a limited number of write cycles.
 *   Do not program EEPROM repeatedly.
 *
 * Connect the MT6826S to the board as follows:
 * MT6826S uses VDD as logic voltage for SPI lines
 *
 *  MT6826S  |  Arduino Uno  |  ESP32
 *  ---------|---------------|----------------------
 *  VCC      |  5V           |  3.3V
 *  GND      |  GND          |  GND
 *  SCK      |  D13          |  GPIO 14
 *  MISO     |  D12          |  GPIO 12
 *  MOSI     |  D11          |  GPIO 13
 *  CSN      |  D10          |  GPIO 10
 *
 * Written by Bruno Cerna (BrozuTechworks).
 *
 * MIT License.
 */

#include <SPI.h>
#include "MT6826S_SPI.h"

#define CS_PIN            10      ///< Chip Select pin

// 12-bit angle value used by command '3'. Range [0, 4095].
// Formula: angle_deg = (angle12Bit / 4096.0) * 360.0
#define SPECIFIC_ZERO_12BIT   512     ///< Corresponds to 45.0 degrees

MT6826S_SPI mt6826s;  // Create mt6826s object

// Print current angle to Serial

void printAngle() {
  mt6826s.readAngle();
  Serial.print(F("  Current angle: "));
  Serial.print(mt6826s.getAngle(), 2);
  Serial.println(F(" degrees"));
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // Wait for Serial Monitor to open

  Serial.println(F("MT6826S — Zero Position Calibration Example"));
  Serial.println();
  Serial.println(F("Commands:"));
  Serial.println(F("  1 — Reset zero to absolute 0 (factory default)"));
  Serial.println(F("  2 — Set zero to current shaft angle"));
  Serial.println(F("  3 — Set specific 12-bit zero value"));
  Serial.println(F("  r — Read current angle"));
  Serial.println(F("  e — Program current zero to EEPROM"));
  Serial.println();

  // Initialize the sensor
  mt6826s.begin(CS_PIN);
  // For custom SPI pin (uncomment the line below and comment out the begin line above):
  //mt6826s.begin(sck_pin, miso_pin, mosi_pin, CS_PIN);

  Serial.println(F("Encoder initialized. Rotate the shaft and send a command."));
  Serial.println();
}

void loop() {
  if (!Serial.available()) return;

  char cmd = Serial.read();

  while (Serial.available()) Serial.read();

  switch (cmd) {
    // Ignore line ending characters
    case '\r':
    case '\n':
      break;

    // Reset zero to absolute 0
    case '1':
      Serial.println(F("Setting zero to absolute 0..."));
      if (mt6826s.setZeroPosition(MT6826S_ANGLE_0)) {
        Serial.println(F("  OK — zero position reset to absolute 0."));
      } else {
        Serial.println(F("  FAILED — register verification error."));
      }
      printAngle();
      break;

    // Set zero to current angle
    case '2':
      Serial.println(F("Setting zero to current shaft angle..."));
      printAngle();   // show angle before calibration
      if (mt6826s.setZeroPosition(MT6826S_CURRENT_ANGLE)) {
        Serial.println(F("  OK — current angle is now the new zero."));
      } else {
        Serial.println(F("  FAILED — register verification error."));
      }
      printAngle();   // should now read ~0.0 deg
      break;

    //  Set specific 12-bit zero value
    case '3': {
      Serial.print(F("Setting specific zero: "));
      Serial.print(SPECIFIC_ZERO_12BIT);
      Serial.println(F(" 12-bit angle..."));

      if (mt6826s.setSpecificZeroPosition(SPECIFIC_ZERO_12BIT)) {
        Serial.println(F("  OK — specific zero position applied."));
      } else {
        Serial.println(F("  FAILED — register verification error."));
      }
      printAngle();
      break;
    }

    // Read current angle
    case 'r':
      printAngle();
      break;

    // Program EEPROM
    case 'e':
      Serial.println(F("Programming EEPROM — this may take up to 10 seconds..."));
      if (mt6826s.programEEPROM()) {
        Serial.println(F("  OK — zero position saved to EEPROM."));
      } else {
        Serial.println(F("  FAILED — EEPROM programming error or timeout."));
      }
      break;

    default:
      Serial.println(F("Unknown command. Send 1, 2, 3, r, or e."));
      break;
  }

  Serial.println();
}