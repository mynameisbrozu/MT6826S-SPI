/*!
 * @file mt6826s_read_angle.ino
 *
 * This example reads the shaft angle and checks for warning conditions
 * using the MT6826S magnetic encoder over SPI.
 *
 * The sensor reports:
 *  - A 15-bit raw angle count [0, 32767]
 *  - A floating-point angle in degrees [0.0, 360.0)
 *  - Warning flags: over-speed, weak magnet, and under-voltage
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

#define CS_PIN  10    ///< Chip Select pin — change to match your wiring

MT6826S_SPI mt6826s;  // Create mt6826s object

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // Wait for Serial Monitor to open

  Serial.println(F("MT6826S — Angle Reading Example"));

  // Initialize the sensor
  mt6826s.begin(CS_PIN);
  // For custom SPI pin (uncomment the line below and comment out the begin line above):
  //mt6826s.begin(sck_pin, miso_pin, mosi_pin, CS_PIN);

  Serial.println(F("Encoder initialized."));
  Serial.println();
}

void loop() {
  // Read all angle and status registers in a single SPI burst.
  mt6826s.readAngle();

  // Angle data
  uint16_t rawAngle = mt6826s.getRawAngle();
  float    angle    = mt6826s.getAngle();

  Serial.print(F("Raw: "));
  Serial.print(rawAngle);
  Serial.print(F("  |  Angle: "));
  Serial.print(angle, 2);
  Serial.print(F(" deg"));

  // Warning flags
  bool overSpeed    = mt6826s.isWarning(MT6826S_OVER_SPEED);
  bool magnetWeak   = mt6826s.isWarning(MT6826S_MAGNET_WEAK);
  bool underVoltage = mt6826s.isWarning(MT6826S_UNDER_VOLTAGE);

  if (overSpeed || magnetWeak || underVoltage) {
    Serial.print(F("  |  WARNINGS:"));
    if (overSpeed)    Serial.print(F(" [OVER_SPEED]"));
    if (magnetWeak)   Serial.print(F(" [MAGNET_WEAK]"));
    if (underVoltage) Serial.print(F(" [UNDER_VOLTAGE]"));
  }

  Serial.println();

  delay(100);
}