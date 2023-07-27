#include <assert.h>

#include "ManagedServo.h"
#include "MathUtils.h"
#include "Finger.h"




Finger::Finger(ManagedServo& leftPitchServo, ManagedServo& rightPitchServo, ManagedServo& flexionServo) 
: mLeftPitchServo(leftPitchServo), mRightPitchServo(rightPitchServo), mFlexionServo(flexionServo) {
    // Default ranges to something sane, but they can be overriden by a tuning
    // routine or by the user if desired.
    mPitchRange[0] = 0;
    mPitchRange[1] = 45;
    mYawRange[0] = -20;
    mYawRange[1] = 20;
    mFlexionRange[0] = 0;
    mFlexionRange[1] = 120;
    mYawBias = 60;

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



void Finger::setFlexion(int16_t flexion) {
    mFlexionTarget = CLAMP(flexion, getFlexionMin(), getFlexionMax());
}

void Finger::setPitch(int16_t pitch) {
    mPitchTarget = CLAMP(pitch, getPitchMin(), getPitchMax());
}

void Finger::setYaw(int16_t yaw) {
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
    leftPitch = static_cast<int32_t>(leftPitch + scaledYaw);
    rightPitch = static_cast<int32_t>(rightPitch - scaledYaw);

    // If flexion is > 50%, mix in additional pitch
    if (normalizedFlexion > 0.5f) {
        int flexionGain = static_cast<int32_t>((normalizedFlexion - 0.5f) * 30.0f);
        
        #ifdef DEBUG
        Serial.print("FlexionGain: ");
        Serial.println(flexionGain);
        #endif
        
        leftPitch += flexionGain;
        rightPitch += flexionGain;
    }

    // Clamp
    leftPitch = CLAMP(leftPitch, mLeftPitchServo.getMinPosition(), mLeftPitchServo.getMaxPosition());
    rightPitch = CLAMP(rightPitch, mRightPitchServo.getMinPosition(), mRightPitchServo.getMaxPosition());

    // Send to servos
    mLeftPitchServo.setServoPosition(static_cast<uint8_t>(leftPitch));
    mRightPitchServo.setServoPosition(static_cast<uint8_t>(rightPitch));

}
