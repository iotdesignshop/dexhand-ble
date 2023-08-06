/****************************************************************************************************************************
  RP2040_ISR_Servo_Impl_H.h
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

#ifndef RP2040_ISR_Servo_Impl_H
#define RP2040_ISR_Servo_Impl_H

#include "RP2040_ISR_Servo.h"
#include <string.h>

#ifndef ISR_SERVO_DEBUG
  #define ISR_SERVO_DEBUG               1
#endif

static RP2040_ISR_Servo RP2040_ISR_Servos;   // create servo object to control up to 16 servos

#if defined(ARDUINO_ARCH_MBED)

  #define TRIM_DURATION   15          //callback overhead (35 us) -> 15 us if toggle() is called after starting the timeout

#else

  #include "servo.pio.h"
  static PIOProgram _servoPgm(&servo_program);

#endif

/////////////////////////////////////////////////////

RP2040_ISR_Servo::RP2040_ISR_Servo()
  : numServos (-1)
{
}

/////////////////////////////////////////////////////

// find the first available slot
// return -1 if none found
int8_t RP2040_ISR_Servo::findFirstFreeSlot()
{
  // all slots are used
  if (numServos >= MAX_SERVOS)
    return -1;

  // return the first slot with no count (i.e. free)
  for (int8_t servoIndex = 0; servoIndex < MAX_SERVOS; servoIndex++)
  {
    if (servo[servoIndex].enabled == false)
    {
      ISR_SERVO_LOGDEBUG1("Index =", servoIndex);

      return servoIndex;
    }
  }

  // no free slots found
  return -1;
}

/////////////////////////////////////////////////////

int8_t RP2040_ISR_Servo::setupServo(const uint8_t& pin, const uint16_t& minPulseUs, const uint16_t& maxPulseUs,
                                    uint16_t value)
{
  int8_t servoIndex;

  if (pin > RP2040_MAX_PIN)
    return -1;

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);

  if (numServos < 0)
    init();

  servoIndex = findFirstFreeSlot();

  if (servoIndex < 0)
    return -1;

  servo[servoIndex].pin       = pin;
  servo[servoIndex].maxPulseUs = maxPulseUs;
  servo[servoIndex].minPulseUs = minPulseUs;
  servo[servoIndex].position  = 0;
  servo[servoIndex].enabled   = true;

  numServos++;

#if defined(ARDUINO_ARCH_MBED)

  // Add code here for mbed
  servo[servoIndex].servoImpl = new ServoImpl(digitalPinToPinName(pin));

#else

  if (!_servoPgm.prepare(&servo[servoIndex].pio, &servo[servoIndex].smIdx, &servo[servoIndex].pgmOffset))
  {
    // ERROR, no free slots
    ISR_SERVO_LOGERROR("Error no free slot");
    return -1;
  }

  servo[servoIndex].enabled = true;

  servo_program_init(servo[servoIndex].pio, servo[servoIndex].smIdx, servo[servoIndex].pgmOffset, pin);
  pio_sm_set_enabled(servo[servoIndex].pio, servo[servoIndex].smIdx, false);
  pio_sm_put_blocking(servo[servoIndex].pio, servo[servoIndex].smIdx, RP2040::usToPIOCycles(REFRESH_INTERVAL) / 3);
  pio_sm_exec(servo[servoIndex].pio, servo[servoIndex].smIdx, pio_encode_pull(false, false));
  pio_sm_exec(servo[servoIndex].pio, servo[servoIndex].smIdx, pio_encode_out(pio_isr, 32));

  write(servoIndex, value);

  pio_sm_exec(servo[servoIndex].pio, servo[servoIndex].smIdx, pio_encode_pull(false, false));
  pio_sm_exec(servo[servoIndex].pio, servo[servoIndex].smIdx, pio_encode_mov(pio_x, pio_osr));
  pio_sm_set_enabled(servo[servoIndex].pio, servo[servoIndex].smIdx, true);

  /////////////////////////////////////////

  write(servoIndex, value);

#endif

  ISR_SERVO_LOGDEBUG1("Index =", servoIndex);
  ISR_SERVO_LOGDEBUG3("min =", servo[servoIndex].minPulseUs, ", max =", servo[servoIndex].maxPulseUs);

  return servoIndex;
}

/////////////////////////////////////////////////////

void RP2040_ISR_Servo::write(const uint8_t& servoIndex, uint16_t& value)
{
  // treat any value less than MIN_PULSE_WIDTH as angle in degrees (values equal or larger are handled as microseconds)
  if (value < MIN_PULSE_WIDTH)
  {
    // assumed to be 0-180 degrees servo
    value = constrain(value, 0, 180);
    value = map(value, 0, 180, servo[servoIndex].minPulseUs, servo[servoIndex].maxPulseUs);
  }

  writeMicroseconds(servoIndex, value);
}

/////////////////////////////////////////////////////

void RP2040_ISR_Servo::writeMicroseconds(const uint8_t& servoIndex, uint16_t value)
{
  value = constrain(value, servo[servoIndex].minPulseUs, servo[servoIndex].maxPulseUs);
  servo[servoIndex].position = value;

  if (servo[servoIndex].enabled)
  {
#if defined(ARDUINO_ARCH_MBED)

    value = value - TRIM_DURATION;

    if (servo[servoIndex].servoImpl->duration == -1)
    {
      servo[servoIndex].servoImpl->start(value);
    }

    servo[servoIndex].servoImpl->duration = value;
#else

    // Remove any old updates that haven't yet taken effect
    pio_sm_clear_fifos(servo[servoIndex].pio, servo[servoIndex].smIdx);
    pio_sm_put_blocking(servo[servoIndex].pio, servo[servoIndex].smIdx, RP2040::usToPIOCycles(value) / 3);

#endif
  }
}

/////////////////////////////////////////////////////

bool RP2040_ISR_Servo::setPosition(const uint8_t& servoIndex, uint16_t position)
{
  if (servoIndex >= MAX_SERVOS)
    return false;

  // Updates interval of existing specified servo
  if ( servo[servoIndex].enabled && (servo[servoIndex].pin <= RP2040_MAX_PIN) )
  {
    // treat any value less than MIN_PULSE_WIDTH as angle in degrees (values equal or larger are handled as microseconds)
    if (position < MIN_PULSE_WIDTH)
    {
      // assumed to be 0-180 degrees servo
      position = constrain(position, 0, 180);
      position = map(position, 0, 180, servo[servoIndex].minPulseUs, servo[servoIndex].maxPulseUs);
    }

    servo[servoIndex].position = position;

    writeMicroseconds(servoIndex, position);

    ISR_SERVO_LOGDEBUG3("Idx =", servoIndex, ", pos =", servo[servoIndex].position);

    return true;
  }

  // false return for non-used numServo or bad pin
  return false;
}

/////////////////////////////////////////////////////

// returns last position in degrees if success, or -1 on wrong servoIndex
int RP2040_ISR_Servo::getPosition(const uint8_t& servoIndex)
{
  if (servoIndex >= MAX_SERVOS)
    return -1;

  // Updates interval of existing specified servo
  if ( servo[servoIndex].enabled && (servo[servoIndex].pin <= RP2040_MAX_PIN) )
  {
    ISR_SERVO_LOGDEBUG3("Idx =", servoIndex, ", pos =", servo[servoIndex].position);

    return (servo[servoIndex].position);
  }

  // return 0 for non-used numServo or bad pin
  return -1;
}

/////////////////////////////////////////////////////

// setPulseWidth will set servo PWM Pulse Width in microseconds, correcponding to certain position in degrees
// by using PWM, turn HIGH 'pulseWidth' microseconds within REFRESH_INTERVAL (20000us)
// min and max for each individual servo are enforced
// returns true on success or -1 on wrong servoIndex
bool RP2040_ISR_Servo::setPulseWidth(const uint8_t& servoIndex, uint16_t& pulseWidth)
{
  if (servoIndex >= MAX_SERVOS)
    return false;

  // Updates interval of existing specified servo
  if ( servo[servoIndex].enabled && (servo[servoIndex].pin <= RP2040_MAX_PIN) )
  {
    if (pulseWidth < servo[servoIndex].minPulseUs)
      pulseWidth = servo[servoIndex].minPulseUs;
    else if (pulseWidth > servo[servoIndex].maxPulseUs)
      pulseWidth = servo[servoIndex].maxPulseUs;

    writeMicroseconds(servoIndex, pulseWidth);

    ISR_SERVO_LOGDEBUG3("Idx =", servoIndex, ", pos =", servo[servoIndex].position);


    return true;
  }

  // false return for non-used numServo or bad pin
  return false;
}

/////////////////////////////////////////////////////

// returns pulseWidth in microsecs (within min/max range) if success, or 0 on wrong servoIndex
uint16_t RP2040_ISR_Servo::getPulseWidth(const uint8_t& servoIndex)
{
  if (servoIndex >= MAX_SERVOS)
    return 0;

  // Updates interval of existing specified servo
  if ( servo[servoIndex].enabled && (servo[servoIndex].pin <= RP2040_MAX_PIN) )
  {
    ISR_SERVO_LOGDEBUG3("Idx =", servoIndex, ", pos =", servo[servoIndex].position);

    return (servo[servoIndex].position);
  }

  // return 0 for non-used numServo or bad pin
  return 0;
}

/////////////////////////////////////////////////////

void RP2040_ISR_Servo::deleteServo(const uint8_t& servoIndex)
{
  if ( (numServos == 0) || (servoIndex >= MAX_SERVOS) )
  {
    return;
  }

  // don't decrease the number of servos if the specified slot is already empty
  if (servo[servoIndex].enabled)
  {
#if defined(ARDUINO_ARCH_MBED)

    //Must be before memset
    if (servo[servoIndex].servoImpl)
      delete servo[servoIndex].servoImpl;

#endif

    memset((void*) &servo[servoIndex], 0, sizeof (servo_t));

    servo[servoIndex].enabled   = false;
    servo[servoIndex].position  = 0;
    // Intentional bad pin, good only from 0-16 for Digital, A0=17
    servo[servoIndex].pin       = RP2040_WRONG_PIN;

    // update number of servos
    numServos--;
  }
}

/////////////////////////////////////////////////////

bool RP2040_ISR_Servo::isEnabled(const uint8_t& servoIndex)
{
  if (servoIndex >= MAX_SERVOS)
    return false;

  if (servo[servoIndex].pin > RP2040_MAX_PIN)
  {
    // Disable if something wrong
    servo[servoIndex].pin     = RP2040_WRONG_PIN;
    servo[servoIndex].enabled = false;
    return false;
  }

  return servo[servoIndex].enabled;
}

/////////////////////////////////////////////////////

bool RP2040_ISR_Servo::enable(const uint8_t& servoIndex)
{
  if (servoIndex >= MAX_SERVOS)
    return false;

  if (servo[servoIndex].pin > RP2040_MAX_PIN)
  {
    // Disable if something wrong
    servo[servoIndex].pin     = RP2040_WRONG_PIN;
    servo[servoIndex].enabled = false;
    return false;
  }

  if ( servo[servoIndex].position >= servo[servoIndex].minPulseUs )
    servo[servoIndex].enabled = true;

  return true;
}

/////////////////////////////////////////////////////

bool RP2040_ISR_Servo::disable(const uint8_t& servoIndex)
{
  if (servoIndex >= MAX_SERVOS)
    return false;

  if (servo[servoIndex].pin > RP2040_MAX_PIN)
    servo[servoIndex].pin     = RP2040_WRONG_PIN;

  servo[servoIndex].enabled = false;

  return true;
}

void RP2040_ISR_Servo::enableAll()
{
  // Enable all servos with a enabled and count != 0 (has PWM) and good pin
  for (int8_t servoIndex = 0; servoIndex < MAX_SERVOS; servoIndex++)
  {
    if ( (servo[servoIndex].position >= servo[servoIndex].minPulseUs) && !servo[servoIndex].enabled
         && (servo[servoIndex].pin <= RP2040_MAX_PIN) )
    {
      servo[servoIndex].enabled = true;
    }
  }
}

/////////////////////////////////////////////////////

void RP2040_ISR_Servo::disableAll()
{
  // Disable all servos
  for (int8_t servoIndex = 0; servoIndex < MAX_SERVOS; servoIndex++)
  {
    servo[servoIndex].enabled = false;
  }
}

/////////////////////////////////////////////////////

bool RP2040_ISR_Servo::toggle(const uint8_t& servoIndex)
{
  if (servoIndex >= MAX_SERVOS)
    return false;

  servo[servoIndex].enabled = !servo[servoIndex].enabled;

  return true;
}

/////////////////////////////////////////////////////

int8_t RP2040_ISR_Servo::getNumServos()
{
  return numServos;
}

/////////////////////////////////////////////////////

#endif    // RP2040_ISR_Servo_Impl_H
