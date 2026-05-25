/**
 * @file MT6826S_SPI.cpp
 * Implementation of the MT6826S SPI driver.
 *
 * Written by Bruno Cerna (BrozuTechworks)
 *
 * MIT License
 */

#include "MT6826S_SPI.h"

MT6826S_SPI::MT6826S_SPI(uint32_t spiClock)
  : _spiClock(spiClock), 
    _spiMode(SPI_MODE3), 
    _bitOrder(MSBFIRST),
    _angleRegister{0,0,0,0} 
{}

void MT6826S_SPI::begin(uint8_t cs) {
  _csPin = cs;
  pinMode(_csPin, OUTPUT);
  digitalWrite(_csPin, HIGH);

  SPI.begin();
}

#if defined(ESP32) || defined(ESP8266) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_STM32)
void MT6826S_SPI::begin(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t cs) {
  _csPin = cs;
  pinMode(_csPin, OUTPUT);
  digitalWrite(_csPin, HIGH);
  SPI.begin(sck, miso, mosi, -1);
}
#endif

uint8_t MT6826S_SPI::readRegister(uint16_t address){
  digitalWrite(_csPin, LOW);
  SPI.transfer16((MT6826S_READ_REG << 12) | address);      
  uint8_t data = SPI.transfer(0x00);                     
  digitalWrite(_csPin, HIGH);

  return data;
}

void MT6826S_SPI::writeRegister(uint16_t address, uint8_t data){
  digitalWrite(_csPin, LOW);
  SPI.transfer16((MT6826S_WRITE_REG << 12) | address);      
  SPI.transfer(data);
  digitalWrite(_csPin, HIGH);
}

bool MT6826S_SPI::programEEPROM(){
  SPI.beginTransaction(SPISettings(_spiClock, _bitOrder, _spiMode));

  digitalWrite(_csPin, LOW);
  SPI.transfer16((MT6826S_EEPROM_PROG << 12) | 0x000);     
  uint8_t acknowledge = SPI.transfer(0x00);                   
  digitalWrite(_csPin, HIGH);

  SPI.endTransaction();

  if(acknowledge != MT6826S_SUCCESS_ACK){
    return false;
  }
  /* Poll EE_DONE bit (register 0x112, bit 5) up to 20 times (10 s total). */
  for(uint8_t i = 0; i < 20; i++){
    SPI.beginTransaction(SPISettings(_spiClock, _bitOrder, _spiMode));
    bool eeDone = (readRegister(MT6826S_EE_DONE_REG) >> 5) & 0x01;
    SPI.endTransaction();
    
    if(eeDone){
      return true;
    }
    delay(500);
  }

  return false;  
}

bool MT6826S_SPI::setZeroPosition(mt6826s_zero_pos_t ZERO_POS){
  /* Step 1: clear zero position registers to read absolute angle. */
  SPI.beginTransaction(SPISettings(_spiClock, _bitOrder, _spiMode));
  
  uint8_t abzReg4 = readRegister(MT6826S_ABZ_REG4) & 0xF;
  writeRegister(MT6826S_ABZ_REG3, 0x00);
  writeRegister(MT6826S_ABZ_REG4, abzReg4);
  SPI.endTransaction();

  /* Step 2: wait for the sensor to settle, then read current angle. */
  delay(120);
  readAngle();
  uint16_t posAngle12bit = getRawAngle() >> 3;
  if(!ZERO_POS){
    posAngle12bit = 0;
  }
  
  /* Step 3: write the new zero position. */
  uint8_t abzReg3 = posAngle12bit >> 4;
  abzReg4 = abzReg4 | ((posAngle12bit & 0xF) << 4);
  
  SPI.beginTransaction(SPISettings(_spiClock, _bitOrder, _spiMode));
  writeRegister(MT6826S_ABZ_REG3, abzReg3);
  writeRegister(MT6826S_ABZ_REG4, abzReg4);
  uint8_t check = (readRegister(MT6826S_ABZ_REG3) ^ abzReg3) | (readRegister(MT6826S_ABZ_REG4) ^ abzReg4);
  
  SPI.endTransaction();
  delay(120);
  return (check == 0);
}

bool MT6826S_SPI::setSpecificZeroPosition(uint16_t angle12Bit){
  SPI.beginTransaction(SPISettings(_spiClock, _bitOrder, _spiMode));

  uint8_t abzReg3 = (angle12Bit & 0xFF0) >> 4;
  uint8_t abzReg4 = (readRegister(MT6826S_ABZ_REG4) & 0xF) | ((angle12Bit & 0xF) << 4);
  writeRegister(MT6826S_ABZ_REG3, abzReg3);
  writeRegister(MT6826S_ABZ_REG4, abzReg4);
  uint8_t check = (readRegister(MT6826S_ABZ_REG3) ^ abzReg3) | (readRegister(MT6826S_ABZ_REG4) ^ abzReg4);
  
  SPI.endTransaction();
  delay(120);
  return (check == 0);
}

bool MT6826S_SPI::configMT6826S(uint8_t userID, mt6826s_rotation_direction_t ROTATION_DIRECTION){
  SPI.beginTransaction(SPISettings(_spiClock, _bitOrder, _spiMode));

  uint8_t rotRegister = (readRegister(MT6826S_ROT_REG) & 0b11110111) | (ROTATION_DIRECTION << 3);
  writeRegister(MT6826S_ID_REG, userID);
  writeRegister(MT6826S_ROT_REG, rotRegister);
  uint8_t check = (readRegister(MT6826S_ID_REG) ^ userID) | (readRegister(MT6826S_ROT_REG) ^ rotRegister);
  
  SPI.endTransaction();
  return (check == 0);
}

bool MT6826S_SPI::configABZ(uint16_t PPR, mt6826s_abz_output_t ABZ_OUTPUT, mt6826s_ab_swap_t AB_SWAP, mt6826s_z_pulse_width_t Z_WIDTH){
  SPI.beginTransaction(SPISettings(_spiClock, _bitOrder, _spiMode));

  uint8_t abzReg1 = ((PPR - 1) & 0x0FFF) >> 4;
  uint8_t abzReg2 = (readRegister(MT6826S_ABZ_REG2) & 0b00001100) | (((PPR-1) & 0xF) << 4) | (ABZ_OUTPUT << 1) | (AB_SWAP);
  uint8_t abzReg4 = (readRegister(MT6826S_ABZ_REG4) & 0xF0) | Z_WIDTH;
  writeRegister(MT6826S_ABZ_REG1, abzReg1);
  writeRegister(MT6826S_ABZ_REG2, abzReg2);
  writeRegister(MT6826S_ABZ_REG4, abzReg4);
  uint8_t check = (readRegister(MT6826S_ABZ_REG1) ^ abzReg1) | ((readRegister(MT6826S_ABZ_REG2) ^ abzReg2) & 0b11110011) | (readRegister(MT6826S_ABZ_REG4) ^ abzReg4);

  SPI.endTransaction();
  return (check == 0);
}

bool MT6826S_SPI::configUVW(mt6826s_uvw_output_t UVW_OUTPUT, mt6826s_uvw_resolution_t UVW_RESOLUTION){
  SPI.beginTransaction(SPISettings(_spiClock, _bitOrder, _spiMode));

  uint8_t uvwReg = (readRegister(MT6826S_UVW_REG) & 0b11100000) | (UVW_OUTPUT << 4) | UVW_RESOLUTION;
  writeRegister(MT6826S_UVW_REG, uvwReg);
  uint8_t check = readRegister(MT6826S_UVW_REG) ^ uvwReg;

  SPI.endTransaction();
  return (check == 0);
}

bool MT6826S_SPI::configPWM(mt6826s_pwm_frame_frequency_t PWM_FRQ, mt6826s_pwm_voltage_level_t PWM_VOLTAGE, mt6826s_pwm_output_source_t PWM_SOURCE){
  SPI.beginTransaction(SPISettings(_spiClock, _bitOrder, _spiMode));

  uint8_t pwmReg = (readRegister(MT6826S_PWM_REG) & 0b11100000) | (PWM_FRQ << 4) | (PWM_VOLTAGE << 3) | PWM_SOURCE;
  writeRegister(MT6826S_PWM_REG, pwmReg);
  uint8_t check = (readRegister(MT6826S_PWM_REG) ^ pwmReg) & 0b00011111;

  SPI.endTransaction();
  return (check == 0);
}

bool MT6826S_SPI::readAngle() {
  SPI.beginTransaction(SPISettings(_spiClock, _bitOrder, _spiMode));

  uint16_t addresses[4] = {MT6826S_READ_ONLY1, MT6826S_READ_ONLY2, MT6826S_READ_ONLY3, MT6826S_READ_ONLY4};

  for (uint8_t i = 0; i < 4; i++){
    _angleRegister[i] = readRegister(addresses[i]);
  }

  SPI.endTransaction();

  return true;
}

uint16_t MT6826S_SPI::getRawAngle(){ 
  uint16_t rawAngle = (_angleRegister[0] << 7) | (_angleRegister[1] >> 1);

  return rawAngle;
}

float MT6826S_SPI::getAngle(){
  uint16_t rawAngle = (_angleRegister[0] << 7) | (_angleRegister[1] >> 1);
  float angle = (rawAngle * 360.0f) / MT6826S_RESOLUTION;

  return angle;
}

bool MT6826S_SPI::isWarning(mt6826s_warnings_t STATUS_BIT){
  bool isWarning = (_angleRegister[2] >> STATUS_BIT) & 0x01;

  return isWarning;
}