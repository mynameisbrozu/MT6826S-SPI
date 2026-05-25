/*!
 * @file mt6826s_configuration.ino
 *
 * This example demonstrates how to configure the MT6826S encoder outputs
 * and persist the settings to EEPROM.
 *
 * The following are configured and then programmed to EEPROM :
 *  - General settings: user ID and rotation direction
 *  - ABZ incremental output: PPR, output enable, A/B swap, Z pulse width
 *  - UVW commutation output: enable and pole pair count
 *  - PWM analog output: frequency, active level, and data source
 *
 * Important
 * - All config functions write to RAM registers only.
 *   Call programEEPROM() to persist changes across power cycles.
 * - The EEPROM supports a limited number of write cycles.
 *   Avoid calling programEEPROM() in a loop.
 * - programEEPROM() is blocking and may take up to 10 seconds.
 *
 * Connect the MT6826S to the board as follows:
 * MT6826S uses VDD as logic voltage for SPI lines.
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

#define CS_PIN   10       ///< Chip Select pin

// General
#define USER_ID             0x01
#define ROTATION_DIR        MT6826S_COUNTER_CLOCKWISE

// ABZ output
#define ABZ_PPR             1000
#define ABZ_OUTPUT          MT6826S_ABZ_ON
#define ABZ_SWAP            MT6826S_NO_SWAP
#define ABZ_Z_WIDTH         MT6826S_LSB_1

// UVW output
#define UVW_OUTPUT          MT6826S_UVW_ON
#define UVW_POLE_PAIRS      MT6826S_POLE_PAIRS_4

// PWM output
#define PWM_FREQUENCY       MT6826S_PWM_994_HZ
#define PWM_VOLTAGE         MT6826S_HIGH_EFFECTIVE
#define PWM_SOURCE          MT6826S_ANGLE_12BIT

MT6826S_SPI mt6826s; // Create mt6826s object

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // Wait for Serial Monitor to open

  Serial.println(F("MT6826S — Configuration and EEPROM Example"));
  Serial.println();

  // Initialize the sensor
  mt6826s.begin(CS_PIN);
  // For custom SPI pin (uncomment the line below and comment out the begin line above):
  //mt6826s.begin(sck_pin, miso_pin, mosi_pin, CS_PIN);

  // Step 1: Apply configuration to registers
  Serial.println(F("Applying configuration..."));

  if (mt6826s.configMT6826S(USER_ID, ROTATION_DIR)) {
    Serial.println(F("  General settings (ID + direction) ... OK"));
  } else {
    Serial.println(F("  General settings (ID + direction) ... FAILED"));
  }

  if (mt6826s.configABZ(ABZ_PPR, ABZ_OUTPUT, ABZ_SWAP, ABZ_Z_WIDTH)) {
    Serial.println(F("  ABZ output ... OK"));
  } else {
    Serial.println(F("  ABZ output ... FAILED"));
  }

  if (mt6826s.configUVW(UVW_OUTPUT, UVW_POLE_PAIRS)) {
    Serial.println(F("  UVW output ... OK"));
  } else {
    Serial.println(F("  UVW output ... FAILED"));
  }

  if (mt6826s.configPWM(PWM_FREQUENCY, PWM_VOLTAGE, PWM_SOURCE)) {
    Serial.println(F("  PWM output ... OK"));
  } else {
    Serial.println(F("  PWM output ... FAILED"));
  }

  Serial.println();

  // Step 2: Program EEPROM
  Serial.println(F("Programming EEPROM — this may take up to 10 seconds..."));

  if (mt6826s.programEEPROM()) {
    Serial.println(F("EEPROM programmed successfully."));
  } else {
    Serial.println(F("EEPROM programming failed or timed out."));
  }

  Serial.println();
  Serial.println(F("You may now power cycle the sensor."));
}

void loop() {
  delay(1000);
}