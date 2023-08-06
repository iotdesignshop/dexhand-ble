/****************************************************************************************************************************
  RP2040_ISR_Servo_Debug.h
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

#ifndef RP2040_ISR_Servo_Debug_h
#define RP2040_ISR_Servo_Debug_h

//////////////////////////////////////////

#ifndef ISR_SERVO_DEBUG
  #define ISR_SERVO_DEBUG               1
#endif

//////////////////////////////////////////

#if !defined(ISR_SERVO_DEBUG_OUTPUT)
  #define ISR_SERVO_DEBUG_OUTPUT    Serial
#endif

//////////////////////////////////////////////////////

const char ISR_SERVO_MARK[]  = "[ISR_SERVO] ";
const char ISR_SERVO_SP[]    = " ";

#define ISR_SERVO_PRINT          ISR_SERVO_DEBUG_OUTPUT.print
#define ISR_SERVO_PRINTLN        ISR_SERVO_DEBUG_OUTPUT.println
#define ISR_SERVO_FLUSH          ISR_SERVO_DEBUG_OUTPUT.flush

#define ISR_SERVO_PRINT_MARK     ISR_SERVO_PRINT(ISR_SERVO_MARK)
#define ISR_SERVO_PRINT_SP       ISR_SERVO_PRINT(ISR_SERVO_SP)

//////////////////////////////////////////////////////

#define ISR_SERVO_LOGERROR(x)         if(ISR_SERVO_DEBUG>0) { ISR_SERVO_PRINT_MARK; ISR_SERVO_PRINTLN(x); }
#define ISR_SERVO_LOGERROR0(x)        if(ISR_SERVO_DEBUG>0) { ISR_SERVO_PRINT(x); }
#define ISR_SERVO_LOGERROR1(x,y)      if(ISR_SERVO_DEBUG>0) { ISR_SERVO_PRINT_MARK; ISR_SERVO_PRINT(x); ISR_SERVO_PRINT_SP; ISR_SERVO_PRINTLN(y); }
#define ISR_SERVO_LOGERROR2(x,y,z)    if(ISR_SERVO_DEBUG>0) { ISR_SERVO_PRINT_MARK; ISR_SERVO_PRINT(x); ISR_SERVO_PRINT_SP; ISR_SERVO_PRINT(y); ISR_SERVO_PRINT_SP; ISR_SERVO_PRINTLN(z); }
#define ISR_SERVO_LOGERROR3(x,y,z,w)  if(ISR_SERVO_DEBUG>0) { ISR_SERVO_PRINT_MARK; ISR_SERVO_PRINT(x); ISR_SERVO_PRINT_SP; ISR_SERVO_PRINT(y); ISR_SERVO_PRINT_SP; ISR_SERVO_PRINT(z); ISR_SERVO_PRINT_SP; ISR_SERVO_PRINTLN(w); }


//////////////////////////////////////////////////////

#define ISR_SERVO_LOGDEBUG(x)         if(ISR_SERVO_DEBUG>1) { ISR_SERVO_PRINT_MARK; ISR_SERVO_PRINTLN(x); }
#define ISR_SERVO_LOGDEBUG0(x)        if(ISR_SERVO_DEBUG>1) { ISR_SERVO_PRINT(x); }
#define ISR_SERVO_LOGDEBUG1(x,y)      if(ISR_SERVO_DEBUG>1) { ISR_SERVO_PRINT_MARK; ISR_SERVO_PRINT(x); ISR_SERVO_PRINT_SP; ISR_SERVO_PRINTLN(y); }
#define ISR_SERVO_LOGDEBUG2(x,y,z)    if(ISR_SERVO_DEBUG>1) { ISR_SERVO_PRINT_MARK; ISR_SERVO_PRINT(x); ISR_SERVO_PRINT_SP; ISR_SERVO_PRINT(y); ISR_SERVO_PRINT_SP; ISR_SERVO_PRINTLN(z); }
#define ISR_SERVO_LOGDEBUG3(x,y,z,w)  if(ISR_SERVO_DEBUG>1) { ISR_SERVO_PRINT_MARK; ISR_SERVO_PRINT(x); ISR_SERVO_PRINT_SP; ISR_SERVO_PRINT(y); ISR_SERVO_PRINT_SP; ISR_SERVO_PRINT(z); ISR_SERVO_PRINT_SP; ISR_SERVO_PRINTLN(w); }
//////////////////////////////////////////


#endif      // RP2040_ISR_Servo_Debug_h
