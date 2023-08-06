/****************************************************************************************************************************
  RP2040_ISR_Servo.h
  For :
  - MBED RP2040-based boards such as Nano_RP2040_Connect, RASPBERRY_PI_PICO, ADAFRUIT_FEATHER_RP2040 and GENERIC_RP2040.
  - RP2040-based boards such as RASPBERRY_PI_PICO, ADAFRUIT_FEATHER_RP2040 and GENERIC_RP2040 using arduino_pico core

  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/RP2040_ISR_Servo
  Licensed under MIT license

  Version: 1.1.2

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K Hoang      21/08/2021 Initial coding for RP2040 boards using ArduinoCore-mbed or arduino-pico core
  1.0.1   K Hoang      22/10/2021 Fix platform in library.json for PIO
  1.1.0   K Hoang      27/02/2022 Fix setPulseWidth() bug. Convert to h-only style
  1.1.1   K Hoang      08/03/2022 Delete redundant `.cpp` file causing compile error
  1.1.2   K Hoang      08/03/2022 Permit using servos with different pulse ranges simultaneously
 *****************************************************************************************************************************/

#pragma once

#ifndef RP2040_ISR_Servo_H
#define RP2040_ISR_Servo_H

#include "RP2040_ISR_Servo.hpp"
#include "RP2040_ISR_Servo_Impl.h"


#endif
