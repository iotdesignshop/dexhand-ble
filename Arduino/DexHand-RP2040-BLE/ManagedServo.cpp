#include "RP2040_ISR_Servo/RP2040_ISR_Servo.h"
#include "ManagedServo.h"

// Published values for ES3352 servos; adjust if you are using different servos
#define MIN_MICROS        700
#define MAX_MICROS        2300
#define DEFAULT_MICROS    MIN_MICROS+((MAX_MICROS-MIN_MICROS)/2)


ManagedServo::ManagedServo(uint8_t servoPin, uint8_t minPosition, uint8_t maxPosition, uint8_t defaultPosition, bool invertAngles)
: mServoPin(servoPin), mMinPosition(minPosition), mMaxPosition(maxPosition), 
    mDefaultPosition(defaultPosition), mCurrentPosition(defaultPosition), 
    mInvertAngles(invertAngles), mISRServoIndex(-1) {
}

ManagedServo::~ManagedServo() {
}

void ManagedServo::setupServo()
{
    mISRServoIndex = RP2040_ISR_Servos.setupServo(mServoPin, MIN_MICROS, MAX_MICROS);

    // Set default position
    if (mISRServoIndex != -1) {
        setServoPosition(mDefaultPosition);
    }
    else
    {
        Serial.print("Error setting up servo on pin ");
        Serial.println(mServoPin);
    }
}

void ManagedServo::setServoPosition(uint8_t position) {
    mCurrentPosition = position;

    if (mInvertAngles) {
        position = 180 - position;
    }
    
    if (position < mMinPosition) {
        position = mMinPosition;
    } else if (position > mMaxPosition) {
        position = mMaxPosition;
    }
    
    if (mISRServoIndex >= 0) {
        RP2040_ISR_Servos.setPosition(mISRServoIndex, position);
    } else {
        Serial.print("Error setting servo position on pin ");
        Serial.println(mServoPin);
    }
}

void ManagedServo::moveToMaxPosition() {
    setServoPosition(mMaxPosition);
}

void ManagedServo::moveToMinPosition() {
    setServoPosition(mMinPosition);
}