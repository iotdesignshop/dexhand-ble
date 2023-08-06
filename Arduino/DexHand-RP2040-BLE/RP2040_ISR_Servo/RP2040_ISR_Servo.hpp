/****************************************************************************************************************************
  RP2040_ISR_Servo.hpp
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

#ifndef RP2040_ISR_Servo_HPP
#define RP2040_ISR_Servo_HPP

#if ( defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_ADAFRUIT_FEATHER_RP2040) || \
      defined(ARDUINO_GENERIC_RP2040) ) && !defined(ARDUINO_ARCH_MBED)
      
  #define RP2040_ISR_SERVO_VERSION       "RP2040_ISR_Servo v1.1.2"
  
#elif ( defined(ARDUINO_NANO_RP2040_CONNECT) || defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_ADAFRUIT_FEATHER_RP2040) || \
      defined(ARDUINO_GENERIC_RP2040) ) && defined(ARDUINO_ARCH_MBED)
      
  #define RP2040_ISR_SERVO_VERSION       "Mbed RP2040_ISR_Servo v1.1.2"
  
#else      
  #error This code is intended to run on the mbed / non-mbed RP2040 platform! Please check your Tools->Board setting.
#endif

#define RP2040_ISR_SERVO_VERSION_MAJOR       1
#define RP2040_ISR_SERVO_VERSION_MINOR       1
#define RP2040_ISR_SERVO_VERSION_PATCH       2

#define RP2040_ISR_SERVO_VERSION_INT         1001002

#include <stddef.h>

#include <inttypes.h>

#if defined(ARDUINO)
  #if ARDUINO >= 100
    #include <Arduino.h>
  #else
    #include <WProgram.h>
  #endif
#endif

#include "RP2040_ISR_Servo_Debug.h"

#define RP2040_MAX_PIN           NUM_DIGITAL_PINS
#define RP2040_WRONG_PIN         255

// From Servo.h - Copyright (c) 2009 Michael Margolis.  All right reserved.

#define MIN_PULSE_WIDTH         800       // the shortest pulse sent to a servo  
#define MAX_PULSE_WIDTH         2450      // the longest pulse sent to a servo 
#define DEFAULT_PULSE_WIDTH     1500      // default pulse width when servo is attached
#define REFRESH_INTERVAL        20000     // minumim time to refresh servos in microseconds 

/////////////////////////////////////////////////////

#if defined(ARDUINO_ARCH_MBED)

/////////////////////////////////////////////////////

#include <mbed.h>

#if defined __has_include
#  if __has_include ("pinDefinitions.h")
#    include "pinDefinitions.h"
#  endif
#endif

/////////////////////////////////////////////////////

class ServoImpl 
{
  mbed::DigitalOut   *pin;
  mbed::Timeout      timeout;  // calls a callback once when a timeout expires
  mbed::Ticker       ticker;   // calls a callback repeatedly with a timeout

  public:
  
    ServoImpl(const PinName& _pin) 
    {
      pin = new mbed::DigitalOut(_pin);
    }

    ~ServoImpl() 
    {
      ticker.detach();
      timeout.detach();
      
      delete pin;
    }

    void start(const uint32_t& duration_us) 
    {
      duration = duration_us;
      
      // in microsecs
      ticker.attach(mbed::callback(this, &ServoImpl::call), (std::chrono::microseconds) 20000);
    }

    void call() 
    {
      timeout.attach(mbed::callback(this, &ServoImpl::toggle), (std::chrono::microseconds) duration);
      toggle();
    }

    void toggle() 
    {
      *pin = !*pin;
    }

    int32_t duration = -1;
};

/////////////////////////////////////////////////////

#else

/////////////////////////////////////////////////////

#include <hardware/pio.h>

/////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////

  // For mbed core
class RP2040_ISR_Servo
{

  public:
    // maximum number of servos
    const static int MAX_SERVOS = 18;

    // constructor
    RP2040_ISR_Servo();

    // destructor
    ~RP2040_ISR_Servo()
    {  
    }

    // Bind servo to the timer and pin, return servoIndex
    int8_t setupServo(const uint8_t& pin, const uint16_t& minPulseUs = MIN_PULSE_WIDTH, const uint16_t& maxPulseUs = MAX_PULSE_WIDTH, uint16_t value = 0);
    
    // if value is < MIN_PULSE_WIDTH its treated as an angle, otherwise as pulse width in microseconds
    void write(const uint8_t& servoIndex, uint16_t& value);
    
    // Write pulse width in microseconds
    void writeMicroseconds(const uint8_t& servoIndex, uint16_t value);

    // setPosition will set servo to position in degrees
    // by using PWM, turn HIGH 'duration' microseconds within REFRESH_INTERVAL (20000us)
    // returns true on success or -1 on wrong servoIndex
    bool setPosition(const uint8_t& servoIndex, uint16_t position);

    // returns last position in degrees if success, or -1 on wrong servoIndex
    int getPosition(const uint8_t& servoIndex);
    
    // setPulseWidth will set servo PWM Pulse Width in microseconds, correcponding to certain position in degrees
    // by using PWM, turn HIGH 'pulseWidth' microseconds within REFRESH_INTERVAL (20000us)
    // min and max for each individual servo are enforced
    // returns true on success or -1 on wrong servoIndex
    bool setPulseWidth(const uint8_t& servoIndex, uint16_t& pulseWidth);

    // returns pulseWidth in microsecs (within min/max range) if success, or 0 on wrong servoIndex
    uint16_t getPulseWidth(const uint8_t& servoIndex);

    // destroy the specified servo
    void deleteServo(const uint8_t& servoIndex);

    // returns true if the specified servo is enabled
    bool isEnabled(const uint8_t& servoIndex);

    // enables the specified servo
    bool enable(const uint8_t& servoIndex);

    // disables the specified servo
    bool disable(const uint8_t& servoIndex);

    // enables all servos
    void enableAll();

    // disables all servos
    void disableAll();

    // enables the specified servo if it's currently disabled,
    // and vice-versa
    bool toggle(const uint8_t& servoIndex);

    // returns the number of used servos
    int8_t getNumServos();

    // returns the number of available servos
    int8_t getNumAvailableServos() 
    {
      if (numServos <= 0)
        return MAX_SERVOS;
      else 
        return MAX_SERVOS - numServos;
    };

  private:

    void init()
    {
      for (int8_t servoIndex = 0; servoIndex < MAX_SERVOS; servoIndex++)
      {
        memset((void*) &servo[servoIndex], 0, sizeof (servo_t));
        servo[servoIndex].enabled  = false;
        // Intentional bad pin
        servo[servoIndex].pin      = RP2040_WRONG_PIN;
      }

      numServos   = 0;
    }

    // find the first available slot
    int8_t findFirstFreeSlot();

#if defined(ARDUINO_ARCH_MBED)

    typedef struct
    {
      uint8_t       pin;                  // pin servo connected to
      int           position;             // In degrees
      bool          enabled;              // true if enabled
      uint16_t      minPulseUs;           // The minimum pulse width the servo can handle
      uint16_t      maxPulseUs;           // The maximum pulse width the servo can handle
      ServoImpl*    servoImpl;
    } servo_t;
    
#else

    typedef struct
    {
      uint8_t       pin;        // pin servo connected to
      PIO           pio;
      int           smIdx;      // The state machine inside
      int           pgmOffset;  // Where in the SM the code lives
      int           position;   // In degrees
      bool          enabled;    // true if enabled
      uint16_t      minPulseUs; // The minimum pulse width the servo can handle
      uint16_t      maxPulseUs; // The maximum pulse width the servo can handle
    } servo_t;

#endif

    servo_t servo[MAX_SERVOS];

    // actual number of servos in use (-1 means uninitialized)
    int8_t numServos;
};

/////////////////////////////////////////////////////


#endif    // #ifndef RP2040_ISR_Servo_HPP
