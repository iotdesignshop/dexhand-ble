#ifndef MANAGED_SERVO_H
#define MANAGED_SERVO_H

#if ( defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_ADAFRUIT_FEATHER_RP2040) || \
      defined(ARDUINO_GENERIC_RP2040) ) && !defined(ARDUINO_ARCH_MBED)
#if !defined(RP2040_ISR_SERVO_USING_MBED)
  #define RP2040_ISR_SERVO_USING_MBED     false
#endif

#elif ( defined(ARDUINO_NANO_RP2040_CONNECT) || defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_ADAFRUIT_FEATHER_RP2040) || \
      defined(ARDUINO_GENERIC_RP2040) ) && defined(ARDUINO_ARCH_MBED)

#if !defined(RP2040_ISR_SERVO_USING_MBED)
  #define RP2040_ISR_SERVO_USING_MBED     true
#endif

#else
#error This code is intended to run on the mbed / non-mbed RP2040 platform! Please check your Tools->Board setting.
#endif

#define ISR_SERVO_DEBUG 0

#include "RP2040_ISR_Servo.hpp"



// The ManagedServo class is a wrapper around the Servo class that
// provides range checking and absolute limits on the servo position
// so that the model can't be asked to attain any position that would
// damage the model.

class ManagedServo {
    
    public:
        // Constructor
        ManagedServo(uint8_t servoPin, uint8_t minPosition, uint8_t maxPosition, uint8_t defaultPosition, bool invertAngles);
        ~ManagedServo();

        // Accessors
        inline uint8_t getServoPin() { return mServoPin; }
        inline uint8_t getMinPosition() { return mMinPosition; }
        inline uint8_t getMaxPosition() { return mMaxPosition; }
        inline void setMinPosition(uint8_t minPosition) { if(minPosition > 0 && minPosition < 180) mMinPosition = minPosition; }
        inline void setMaxPosition(uint8_t maxPosition) { if(maxPosition > 0 && maxPosition < 180) mMaxPosition = maxPosition; }

        inline uint8_t getDefaultPosition() { return mDefaultPosition; }

        void setupServo();
        void setServoPosition(uint8_t position);
        void moveToMaxPosition();
        void moveToMinPosition();

    private:
        uint8_t mServoPin;
        uint8_t mMinPosition;
        uint8_t mMaxPosition;
        uint8_t mDefaultPosition;
        bool mInvertAngles;
        int mISRServoIndex;

};

#endif