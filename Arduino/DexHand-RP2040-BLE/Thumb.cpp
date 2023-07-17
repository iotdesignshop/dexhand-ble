#include <assert.h>

#include "ManagedServo.h"
#include "MathUtils.h"
#include "Thumb.h"




Thumb::Thumb(ManagedServo& leftPitchServo, ManagedServo& rightPitchServo, ManagedServo& flexionServo, ManagedServo& rollServo) 
: mLeftPitchServo(leftPitchServo), mRightPitchServo(rightPitchServo), mFlexionServo(flexionServo), mRollServo(rollServo) {
    // Default ranges to something sane, but they can be overriden by a tuning
    // routine or by the user if desired.
    mPitchRange[0] = 0;
    mPitchRange[1] = 90;
    mYawRange[0] = -40;
    mYawRange[1] = 40;
    mFlexionRange[0] = 0;
    mFlexionRange[1] = 120;
    mRollRange[0] = 0;
    mRollRange[1] = 20;
    mYawBias = 40;

    // Targets to nominal
    mPitchTarget = 0;
    mYawTarget = 0;
    mFlexionTarget = 0;
    mRollTarget = 0;

}

Thumb::~Thumb() {
}

// At the moment, there is no acceleration or deceleration of the servos or
// any other movement where there could be a delta between the target and actual
// position of the servos. However, we don't want to eliminate the possibility
// from future implementations, so the idea is that the servos will get a call
// to update() every loop, and this could be used for dynamic computation.
//
// Specifically, we are going to mix in the yaw angle and roll angle as a function
// of pitch and yaw. This is an approximation and not a perfect science, but it
// produces a reasonable aesthetic result for the thumb.

void Thumb::update() {

    // Update the pitch servos first
    updatePitchServos();

    // Scale the flexion angle to the range of the flexion servo
    int32_t position = mapInteger(mFlexionTarget, mFlexionRange[0], mFlexionRange[1], 
        mFlexionServo.getMinPosition(), mFlexionServo.getMaxPosition());

    assert(position >= mFlexionServo.getMinPosition() && position <= mFlexionServo.getMaxPosition());
    mFlexionServo.setServoPosition(static_cast<uint8_t>(position));

    // Scale the roll angle to the range of the roll servo
    position = mapInteger(mRollTarget, mRollRange[0], mRollRange[1], 
        mRollServo.getMinPosition(), mRollServo.getMaxPosition());

    assert(position >= mRollServo.getMinPosition() && position <= mRollServo.getMaxPosition());
    mRollServo.setServoPosition(static_cast<uint8_t>(position));
}



void Thumb::setFlexion(uint8_t flexion) {
    mFlexionTarget = CLAMP(flexion, getFlexionMin(), getFlexionMax());
}

void Thumb::setPitch(uint8_t pitch) {
    mPitchTarget = CLAMP(pitch, getPitchMin(), getPitchMax());
}

void Thumb::setYaw(int8_t yaw) {
    mYawTarget = CLAMP(yaw, getYawMin(), getYawMax());
}

void Thumb::setRoll(uint8_t roll) {
    mRollTarget = CLAMP(roll, getRollMin(), getRollMax());
}

// The yaw is computed and applied to a bias of the target positions on the left
// and right pitch servos. This pulls the Thumb to the left or right based
// on the value. Note - The bias value is kind of empiracal and something arrived
// at by tuning as opposed to a calculated value. This could potentially be calculated
// in a more accurate way down the road.

void Thumb::updatePitchServos()
{
    // Normalize the yaw
    float normalizedYaw = normalizedValue(mYawTarget, mYawRange[0], mYawRange[1])-0.5f;
    
    // Scale the yaw based on the flexion angle, and apply the bias
    float scaledYaw = normalizedYaw * mYawBias;
    
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
