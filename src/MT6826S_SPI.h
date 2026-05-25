/*!
 * @file MT6826S_SPI.h
 * 
 * SPI driver for the MagnTek MT6826S magnetic angle encoder.
 *
 * This library provides a simple interface to configure and read angle data
 * from the MT6826S encoder over SPI. Supports ABZ, UVW, PWM output, 
 * zero-position configuration and EEPROM programming.
 *
 * Written by Bruno Cerna (BrozuTechworks)
 *
 * MIT License
 */
#ifndef _MT6826S_SPI_H
#define _MT6826S_SPI_H

#include <Arduino.h>
#include <SPI.h>

// ---------------------------------------------------------------------------
// SPI Command opcodes
// ---------------------------------------------------------------------------

#define MT6826S_READ_REG    0x3     ///< Opcode: read register
#define MT6826S_WRITE_REG   0x6     ///< Opcode: write register
#define MT6826S_EEPROM_PROG 0xC     ///< Opcode: erase and program EEPROM
#define MT6826S_READ_CONT   0xA     ///< Opcode: continuous angle register read

#define MT6826S_SUCCESS_ACK 0x55    ///< Acknowledge byte returned on successful EEPROM program

// ---------------------------------------------------------------------------
// Register map
// ---------------------------------------------------------------------------

#define MT6826S_ID_REG      0x001   ///< User ID register
#define MT6826S_READ_ONLY1  0x003   
#define MT6826S_READ_ONLY2  0x004   
#define MT6826S_READ_ONLY3  0x005   
#define MT6826S_READ_ONLY4  0x006   
#define MT6826S_ABZ_REG1    0x007   
#define MT6826S_ABZ_REG2    0x008   
#define MT6826S_ABZ_REG3    0x009   
#define MT6826S_ABZ_REG4    0x00A   
#define MT6826S_UVW_REG     0x00B   
#define MT6826S_PWM_REG     0x00C   
#define MT6826S_ROT_REG     0x00D   
#define MT6826S_EE_DONE_REG 0x112   

// ---------------------------------------------------------------------------
// Sensor constants
// ---------------------------------------------------------------------------

#define MT6826S_RESOLUTION 32768.0f ///< Angular resolution: 2^15

// ---------------------------------------------------------------------------

/**
 * @brief Warning status bits reported in MT6826S_READ_ONLY3.
 */
typedef enum {
    MT6826S_OVER_SPEED    = 0,  ///< Speed exceeded maximum rating
    MT6826S_MAGNET_WEAK   = 1,  ///< Magnetic field strength below threshold
    MT6826S_UNDER_VOLTAGE = 2,  ///< Supply voltage below minimum rating
} mt6826s_warnings_t;

/**
 * @brief ABZ incremental output enable/disable.
 */
typedef enum {
    MT6826S_ABZ_ON    = 0,  ///< ABZ output enabled
    MT6826S_ABZ_OFF   = 1,  ///< ABZ output disabled
} mt6826s_abz_output_t;

/**
 * @brief Swap A and B output channels.
 */
typedef enum {
    MT6826S_NO_SWAP = 0,    ///< A/B channels in default order
    MT6826S_SWAP    = 1,    ///< A and B channels swapped
} mt6826s_ab_swap_t;

/**
 * @brief Z index pulse width selection.
 *
 * LSB_n values define pulse width in encoder counts. 1 LSB = 360/(PPR*4) degrees.
 * ANGLE_n values define pulse width as a fixed angular span.
 */
typedef enum {
    MT6826S_LSB_1     = 0x0,    ///< Z pulse width = 1 LSB
    MT6826S_LSB_2     = 0x1,    ///< Z pulse width = 2 LSB
    MT6826S_LSB_4     = 0x2,    ///< Z pulse width = 4 LSB
    MT6826S_LSB_8     = 0x3,    ///< Z pulse width = 8 LSB
    MT6826S_LSB_16    = 0x4,    ///< Z pulse width = 16 LSB
    MT6826S_ANGLE_60  = 0x5,    ///< Z pulse width = 60 degrees
    MT6826S_ANGLE_120 = 0x6,    ///< Z pulse width = 120 degrees
    MT6826S_ANGLE_180 = 0x7,    ///< Z pulse width = 180 degrees
    MT6826S_LSB_32    = 0x8,    ///< Z pulse width = 32 LSB
    MT6826S_LSB_64    = 0x9,    ///< Z pulse width = 64 LSB
    MT6826S_LSB_128   = 0xA,    ///< Z pulse width = 128 LSB
    MT6826S_ANGLE_45  = 0xB,    ///< Z pulse width = 45 degrees
    MT6826S_ANGLE_90  = 0xC,    ///< Z pulse width = 90 degrees
    MT6826S_ANGLE_135 = 0xD,    ///< Z pulse width = 135 degrees
    MT6826S_ANGLE_240 = 0xE,    ///< Z pulse width = 240 degrees
} mt6826s_z_pulse_width_t;

/**
 * @brief UVW commutation output enable/disable.
 */
typedef enum {
    MT6826S_UVW_ON    = 0,  ///< UVW output enabled
    MT6826S_UVW_OFF   = 1,  ///< UVW output disabled
} mt6826s_uvw_output_t;

/**
 * @brief UVW pole pair count selection.
 *
 * Defines the number of cycles per mechanical revolution
 * for the UVW commutation output.
 */
typedef enum {
    MT6826S_POLE_PAIRS_1  = 0x0,    ///< 1 pole pair
    MT6826S_POLE_PAIRS_2  = 0x1,    ///< 2 pole pairs
    MT6826S_POLE_PAIRS_3  = 0x2,    ///< 3 pole pairs
    MT6826S_POLE_PAIRS_4  = 0x3,    ///< 4 pole pairs
    MT6826S_POLE_PAIRS_5  = 0x4,    ///< 5 pole pairs
    MT6826S_POLE_PAIRS_6  = 0x5,    ///< 6 pole pairs
    MT6826S_POLE_PAIRS_7  = 0x6,    ///< 7 pole pairs
    MT6826S_POLE_PAIRS_8  = 0x7,    ///< 8 pole pairs
    MT6826S_POLE_PAIRS_9  = 0x8,    ///< 9 pole pairs
    MT6826S_POLE_PAIRS_10 = 0x9,    ///< 10 pole pairs
    MT6826S_POLE_PAIRS_11 = 0xA,    ///< 11 pole pairs
    MT6826S_POLE_PAIRS_12 = 0xB,    ///< 12 pole pairs
    MT6826S_POLE_PAIRS_13 = 0xC,    ///< 13 pole pairs
    MT6826S_POLE_PAIRS_14 = 0xD,    ///< 14 pole pairs
    MT6826S_POLE_PAIRS_15 = 0xE,    ///< 15 pole pairs
    MT6826S_POLE_PAIRS_16 = 0xF,    ///< 16 pole pairs
} mt6826s_uvw_resolution_t;

/**
 * @brief PWM output frame frequency selection.
 */
typedef enum {
    MT6826S_PWM_994_HZ = 0, ///< PWM frame frequency = 994 Hz
    MT6826S_PWM_497_HZ = 1, ///< PWM frame frequency = 497 Hz
} mt6826s_pwm_frame_frequency_t;

/**
 * @brief PWM output active voltage level.
 */
typedef enum {
    MT6826S_HIGH_EFFECTIVE  = 0,    ///< PWM active high
    MT6826S_LOW_EFFECTIVE   = 1,    ///< PWM active low
} mt6826s_pwm_voltage_level_t;

/**
 * @brief PWM output data source selection.
 */
typedef enum {
    MT6826S_ANGLE_12BIT  = 0x0, ///< PWM encodes 12-bit angle
    MT6826S_SPEED_12BIT  = 0x2, ///< PWM encodes 12-bit speed
} mt6826s_pwm_output_source_t;

/**
 * @brief Shaft rotation direction convention.
 */
typedef enum {
    MT6826S_COUNTER_CLOCKWISE = 0,  ///< Angle increases counter-clockwise
    MT6826S_CLOCKWISE         = 1,  ///< Angle increases clockwise
} mt6826s_rotation_direction_t;

/**
 * @brief Zero position reference for setZeroPosition().
 */
typedef enum {
    MT6826S_ANGLE_0         = 0,    ///< Set zero position to absolute 0 degrees
    MT6826S_CURRENT_ANGLE   = 1,    ///< Set zero position to current shaft angle
} mt6826s_zero_pos_t;

/**
 * @brief SPI driver for the MagnTek MT6826S 15-bit magnetic angle encoder.
 *
 * Communicates with the sensor using Arduino's SPI library (SPI_MODE3, MSBFIRST).
 */
class MT6826S_SPI {
  public:
    /**
     * @brief Construct a new MT6826S_SPI object.
     *
     * @param spiClock SPI clock frequency in Hz. Maximum is 15,625,000 Hz (default).
     */          
    MT6826S_SPI(uint32_t spiClock = 15000000);

    /**
     * @brief Initialize the sensor using default SPI bus pins.
     *
     * Configures the CS pin and calls SPI.begin() using the board's default
     * SPI pins.
     *
     * @param cs Chip Select pin number. Default is 10
     */
    void begin(uint8_t cs = 10);

    #if defined(ESP32) || defined(ESP8266) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_STM32)
    /**
     * @brief Initialize the sensor using custom SPI bus pins.
     *
     * @param sck  SPI clock pin.
     * @param miso SPI MISO pin.
     * @param mosi SPI MOSI pin.
     * @param cs   Chip Select pin.
     */
    void begin(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t cs);
    #endif

    /**
     * @brief Read raw angle and status registers from the sensor.
     *
     * Must be called before getAngle(), getRawAngle(), or isWarning().
     *
     * @return true  Always returns true (reserved for future error detection).
     */
    bool readAngle(); 

    /**
     * @brief Get the raw 15-bit angle count from the last readAngle() call.
     *
     * @return uint16_t Raw angle value in the range [0, 32767].
     * @note Call readAngle() first to update the internal register cache.
     */                                                   
    uint16_t getRawAngle();

    /**
     * @brief Get the angle in degrees from the last readAngle() call.
     *
     * @return float Angle in degrees in the range [0.0, 360.0).
     * @note Call readAngle() first to update the internal register cache.
     */
    float getAngle();

    /**
     * @brief Check a warning flag from the last readAngle() call.
     *
     * @param STATUS_BIT The warning bit to check (see mt6826s_warnings_t).
     * @return true  The specified warning condition is active.
     * @return false The specified warning condition is not active.
     * @note Call readAngle() first to update the internal register cache.
     */
    bool isWarning(mt6826s_warnings_t STATUS_BIT);

    /**
     * @brief Erase and program the sensor's EEPROM with current register values.
     *
     * Sends the EEPROM program command and polls the EE_DONE flag for up to
     * 10 seconds (20 attempts x 500 ms). This is a blocking operation.
     *
     * @return true  EEPROM programmed successfully.
     * @return false Programming failed or timed out.
     *
     * @warning This operation writes to non-volatile memory. The sensor supports
     *          a limited number of write cycles. Use preferably on setup.
     */
    bool programEEPROM();

    /**
     * @brief Set the encoder zero position.
     *
     * Writes the zero position registers based on the selected mode.
     * If MT6826S_CURRENT_ANGLE is selected, the current shaft angle is
     * captured and stored as the new zero reference.
     *
     * @param ZERO_POS Zero position mode (default: MT6826S_ANGLE_0).
     * @return true  Zero position written and verified successfully.
     * @return false Register verification failed.
     *
     * @note Changes are stored in RAM only. Call programEEPROM() after
     * to persist.
     */
    bool setZeroPosition(mt6826s_zero_pos_t ZERO_POS = MT6826S_ANGLE_0);
    /**
     * @brief Set a specific 12-bit zero position value.
     *
     * Directly writes a 12-bit angle value [0, 4095] as the zero reference.
     * Angle = ((12-bit angle)/4096)*360
     *
     * @param angle12Bit Desired zero position as a 12-bit value.
     * @return true  Zero position written and verified successfully.
     * @return false Register verification failed.
     *
     * @note Changes are stored in RAM only. Call programEEPROM() to persist.
     */
    bool setSpecificZeroPosition(uint16_t angle12Bit = 0);

    /**
     * @brief Configure general sensor settings.
     *
     * Sets the user ID byte and rotation direction convention.
     *
     * @param userID             User-defined ID byte stored in the sensor. Default is 0x00.
     * @param ROTATION_DIRECTION Rotation direction for increasing angle counts
     *                           Default is MT6826S_COUNTER_CLOCKWISE.
     * @return true  Configuration written and verified successfully.
     * @return false Register verification failed.
     *
     * @note Changes are stored in RAM only. Call programEEPROM() to persist.
     */
    bool configMT6826S(uint8_t userID = 0x0, mt6826s_rotation_direction_t ROTATION_DIRECTION = MT6826S_COUNTER_CLOCKWISE);
    
    /**
     * @brief Configure ABZ incremental output.
     *
     * @param PPR        Pulses per revolution [1, 4096]. Default is 1.
     * @param ABZ_OUTPUT Enable or disable ABZ output. Default is MT6826S_ABZ_ON.
     * @param AB_SWAP    Swap A and B channels. Default is MT6826S_NO_SWAP.
     * @param Z_WIDTH    Z index pulse width. Default is MT6826S_LSB_1.
     * @return true  Configuration written and verified successfully.
     * @return false Register verification failed.
     *
     * @note Changes are stored in RAM only. Call programEEPROM() to persist.
     */
    bool configABZ(uint16_t PPR = 1, mt6826s_abz_output_t ABZ_OUTPUT = MT6826S_ABZ_ON, mt6826s_ab_swap_t AB_SWAP = MT6826S_NO_SWAP, mt6826s_z_pulse_width_t Z_WIDTH = MT6826S_LSB_1);
    
    /**
     * @brief Configure UVW commutation output.
     *
     * @param UVW_OUTPUT     Enable or disable UVW output. Default is MT6826S_UVW_ON.
     * @param UVW_RESOLUTION Number of pole pairs for UVW commutation. Default is MT6826S_POLE_PAIRS_1.
     * @return true  Configuration written and verified successfully.
     * @return false Register verification failed.
     *
     * @note Changes are stored in RAM only. Call programEEPROM() to persist.
     */
    bool configUVW(mt6826s_uvw_output_t UVW_OUTPUT = MT6826S_UVW_ON, mt6826s_uvw_resolution_t UVW_RESOLUTION = MT6826S_POLE_PAIRS_1);
    
    /**
     * @brief Configure PWM analog output.
     *
     * @param PWM_FRQ     PWM frame frequency. Default is MT6826S_PWM_994_HZ.
     * @param PWM_VOLTAGE Active voltage level. Default is MT6826S_HIGH_EFFECTIVE.
     * @param PWM_SOURCE  Data source encoded in the PWM signal. Default is MT6826S_ANGLE_12BIT.
     * @return true  Configuration written and verified successfully.
     * @return false Register verification failed.
     *
     * @note Changes are stored in RAM only. Call programEEPROM() to persist.
     */
    bool configPWM(mt6826s_pwm_frame_frequency_t PWM_FRQ = MT6826S_PWM_994_HZ, mt6826s_pwm_voltage_level_t PWM_VOLTAGE = MT6826S_HIGH_EFFECTIVE, mt6826s_pwm_output_source_t PWM_SOURCE = MT6826S_ANGLE_12BIT);

  private:
    uint8_t _csPin;
    uint32_t _spiClock;
    uint8_t _spiMode;
    uint8_t _bitOrder;

    uint8_t _angleRegister[4];
    uint8_t readRegister(uint16_t address);
    void writeRegister(uint16_t address, uint8_t data);
};

#endif // MT6826S_SPI_H