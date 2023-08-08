#include <assert.h>

#include "ManagedServo.h"
#include "MathUtils.h"
#include "Thumb.h"




Thumb::Thumb(ManagedServo& leftPitchServo, ManagedServo& rightPitchServo, ManagedServo& flexionServo, ManagedServo& rollServo) 
: mLeftPitchServo(leftPitchServo), mRightPitchServo(rightPitchServo), mFlexionServo(flexionServo), mRollServo(rollServo) {
    // Default ranges to something sane, but they can be overriden by a tuning
    // routine or by the user if desired.
    mPitchRange[0] = 30;
    mPitchRange[1] = 60;
    mYawRange[0] = 0;
    mYawRange[1] = 45;
    mFlexionRange[0] = 0;
    mFlexionRange[1] = 45;
    mRollRange[0] = 0;
    mRollRange[1] = 20;
    
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



void Thumb::setFlexion(int16_t flexion) {
    mFlexionTarget = CLAMP(flexion, getFlexionMin(), getFlexionMax());
}

void Thumb::setPitch(int16_t pitch) {
    mPitchTarget = CLAMP(pitch, getPitchMin(), getPitchMax());
}

void Thumb::setYaw(int16_t yaw) {
    mYawTarget = CLAMP(yaw, getYawMin(), getYawMax());
}

void Thumb::setRoll(int16_t roll) {
    mRollTarget = CLAMP(roll, getRollMin(), getRollMax());
}

void Thumb::setMaxPosition()
{
    setPosition(getPitchMax(), getYawMax(), getFlexionMax());
}

void Thumb::setMinPosition()
{
    setPosition(getPitchMin(), getYawMin(), getFlexionMin());
}


/* This function is a little bit tricky and worthy of explanation:

  The Yaw angle coming through from the MediaPipe tracker has a range of approximately 45 degrees.
  At approximately 30 degrees, the thumb is roughly parallel with the index finger. 
  Beyond 30 degrees, it actually crosses over to the palm. So, we approximate this motion
  by adding the angle to the upper thumb servo (right pitch servo) up to 30 degrees and then
  subtract it off quickly beyond that to allow the thumb to extend back out past the fingers.

  This is not really a "mathematically correct" solution per se, but gives the intended output
  without very difficult math, and it approximates the fidelity of the data that MediaPipe Hand
  Tracker returns at an appropriate level on the hardware for now.
*/

#define YAW_THRESHOLD 30
void Thumb::updatePitchServos() {

  // If thumb is in yaw range before crossing over the palm, perform regular calculation and apply to right servo
  if (mYawTarget < YAW_THRESHOLD) {
    int32_t clamped = CLAMP(mYawTarget, mYawRange[0], YAW_THRESHOLD);
    int32_t rightPitch = mapInteger(clamped, mYawRange[0], YAW_THRESHOLD,
      mRightPitchServo.getMinPosition(), mRightPitchServo.getMaxPosition());

    mRightPitchServo.setServoPosition(static_cast<uint8_t>(rightPitch));
  }
  else {
    // Thumb is crossing over face of palm - subtract off 2X the overage amount so that
    // we mix the right pitch servo angle out while increasing the left servo.
    int32_t yawOver = mYawTarget-YAW_THRESHOLD;

    // Subtract overage from right servo
    int32_t clamped = CLAMP(YAW_THRESHOLD-2*yawOver, mYawRange[0], YAW_THRESHOLD);
    int32_t rightPitch = mapInteger(clamped, mYawRange[0], YAW_THRESHOLD,
      mRightPitchServo.getMinPosition(), mRightPitchServo.getMaxPosition());

    mRightPitchServo.setServoPosition(static_cast<uint8_t>(rightPitch));

    #ifdef DEBUG
    Serial.print("Yaw over:");
    Serial.print(yawOver);
    Serial.print(" Adj rt pitch:");
    Serial.print(clamped);
    #endif
  }

  // Apply pitch to left servo
  int32_t leftPitch = mapInteger(mPitchTarget, mPitchRange[0], mPitchRange[1],
      mLeftPitchServo.getMinPosition(), mLeftPitchServo.getMaxPosition());
  
  mLeftPitchServo.setServoPosition(static_cast<uint8_t>(leftPitch));
  

}
