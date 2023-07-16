#include <assert.h>

#include "ManagedServo.h"
#include "Finger.h"

#define CLAMP(value, min, max) (value < min ? min : (value > max ? max : value))



Finger::Finger(ManagedServo& leftPitchServo, ManagedServo& rightPitchServo, ManagedServo& flexionServo) 
: mLeftPitchServo(leftPitchServo), mRightPitchServo(rightPitchServo), mFlexionServo(flexionServo) {
    // Default ranges to something sane, but they can be overriden by a tuning
    // routine or by the user if desired.
    mPitchRange[0] = 0;
    mPitchRange[1] = 90;
    mYawRange[0] = -20;
    mYawRange[1] = 20;
    mFlexionRange[0] = 0;
    mFlexionRange[1] = 120;
    mYawBias = 40;

    // Targets to nominal
    mPitchTarget = 0;
    mYawTarget = 0;
    mFlexionTarget = 0;

}

Finger::~Finger() {
}

// At the moment, there is no acceleration or deceleration of the servos or
// any other movement where there could be a delta between the target and actual
// position of the servos. However, we don't want to eliminate the possibility
// from future implementations, so the idea is that the servos will get a call
// to update() every loop, and this could be used for dynamic computation.
//
// Specifically, we are going to mix in the yaw angle with flexion. As the
// finger flexes, the yaw angle is reduced to keep the finger from bending
// sideways as the fingers align into more of a fist.

void Finger::update() {

    // Update the pitch servos first
    updatePitchServos();

    // Scale the flexion angle to the range of the flexion servo
    int32_t position = mapInteger(mFlexionTarget, mFlexionRange[0], mFlexionRange[1], 
        mFlexionServo.getMinPosition(), mFlexionServo.getMaxPosition());
    
    #ifdef DEBUG    // Useful debug printing for tuning
    Serial.print("FlexTgt: ");
    Serial.println(mFlexionTarget);
    Serial.print("FlexRange:");
    Serial.print(mFlexionRange[0]);
    Serial.print(" - ");
    Serial.println(mFlexionRange[1]);
    Serial.print("ServoRange:");
    Serial.print(mFlexionServo.getMinPosition());
    Serial.print(" - ");
    Serial.println(mFlexionServo.getMaxPosition());
    Serial.print("FlexPos: ");
    Serial.println(position);
    #endif

    assert(position >= mFlexionServo.getMinPosition() && position <= mFlexionServo.getMaxPosition());
    mFlexionServo.setServoPosition(static_cast<uint8_t>(position));
}

float Finger::normalizedValue(int32_t value, int32_t min, int32_t max) const {
    float scaled =  static_cast<float>(value - min) / static_cast<float>(max - min);
    return CLAMP(scaled, 0.0f, 1.0f);   // Avoid epsilon errors
}

int32_t Finger::mapInteger(int32_t value, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax) const {
    // Get normalized value
    float scaled = normalizedValue(value, inMin, inMax);

    // Scale to output range
    int32_t val = static_cast<int32_t>(scaled * (outMax - outMin) + outMin);
    
    // Clamp
    return CLAMP(val, outMin, outMax);  // Avoid epsilon errors
    
}


void Finger::setFlexion(uint8_t flexion) {
    mFlexionTarget = CLAMP(flexion, getFlexionMin(), getFlexionMax());
}

void Finger::setPitch(uint8_t pitch) {
    mPitchTarget = CLAMP(pitch, getPitchMin(), getPitchMax());
}

void Finger::setYaw(int8_t yaw) {
    mYawTarget = CLAMP(yaw, getYawMin(), getYawMax());
}


// This is sort of the workhorse function - it does a couple of things:
// 1. It looks at the flexion angle and scales the yaw angle to keep the finger
//    from bending sideways as the finger flexes, but also computes the delta
//    between the two servos in order to try produce the requested yaw.
// 2. It then looks at the pitch angle and computes an overall position to 
//    send to the pitch servos to try produce the requested pitch.
//
// The yaw is computed and applied to a bias of the target positions on the left
// and right pitch servos. This pulls the finger to the left or right based
// on the value. Note - The bias value is kind of empiracal and something arrived
// at by tuning as opposed to a calculated value. This could potentially be calculated
// in a more accurate way down the road.

void Finger::updatePitchServos()
{
    float normalizedFlexion = normalizedValue(getFlexion(), mFlexionRange[0], mFlexionRange[1]);    
    
    // Normalize the yaw
    float normalizedYaw = normalizedValue(mYawTarget, mYawRange[0], mYawRange[1])-0.5f;
    
    // Scale the yaw based on the flexion angle, and apply the bias
    float scaledYaw = normalizedYaw * (1.0f - normalizedFlexion) * mYawBias;
    
    #ifdef DEBUG   // Useful debug printing for tuning
    Serial.print("NFlex: ");
    Serial.print(normalizedFlexion);
    Serial.print(" YawTgt: ");
    Serial.print(mYawTarget);
    Serial.print(" NYaw: ");
    Serial.print(normalizedYaw);
    Serial.print("SYaw: ");
    Serial.println(scaledYaw);
    #endif

    // Compute the pitch on the servos
    int32_t leftPitch = mapInteger(mPitchTarget, mPitchRange[0], mPitchRange[1], 
        mLeftPitchServo.getMinPosition(), mLeftPitchServo.getMaxPosition());

    int32_t rightPitch = mapInteger(mPitchTarget, mPitchRange[0], mPitchRange[1],
        mRightPitchServo.getMinPosition(), mRightPitchServo.getMaxPosition());

    // Mix in the yaw
    leftPitch = static_cast<int32_t>(leftPitch - scaledYaw);
    rightPitch = static_cast<int32_t>(rightPitch + scaledYaw);

    // Clamp
    leftPitch = CLAMP(leftPitch, mLeftPitchServo.getMinPosition(), mLeftPitchServo.getMaxPosition());
    rightPitch = CLAMP(rightPitch, mRightPitchServo.getMinPosition(), mRightPitchServo.getMaxPosition());

    // Send to servos
    mLeftPitchServo.setServoPosition(static_cast<uint8_t>(leftPitch));
    mRightPitchServo.setServoPosition(static_cast<uint8_t>(rightPitch));

}
