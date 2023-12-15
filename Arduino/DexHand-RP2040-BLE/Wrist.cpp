#include <assert.h>

#include "ManagedServo.h"
#include "MathUtils.h"
#include "Wrist.h"




Wrist::Wrist(ManagedServo& leftPitchServo, ManagedServo& rightPitchServo) 
: mLeftPitchServo(leftPitchServo), mRightPitchServo(rightPitchServo) {
    // Default ranges to something sane, but they can be overriden by a tuning
    // routine or by the user if desired.
    mPitchRange[0] = -40;
    mPitchRange[1] = 40;
    mYawRange[0] = -40;
    mYawRange[1] = 40;
    
    // Targets to nominal
    mPitchTarget = 0;
    mYawTarget = 0;
    
}

Wrist::~Wrist() {
}

/*  The wrist angles are created by two differential servos. Combinations of moving these
    servos with, and against each other create the range of motion on the wrist. 
    
    To help visualize this, you can use this table to understand how the range of motion is 
    attained.

    Left Pitch    Right Pitch     Result

    MIN           MIN             Hand centered (pitch) and to the left (yaw)
    MAX           MAX             Hand centered (pitch) and to the right (yaw)

    MIN           MAX             Hand to the back (pitch) and centered(yaw)           
    MAX           MIN             Hand to the front (pitch) and centered (yaw)

    With the orientation of servos in the hand and windings of the cables, the equations
    below are used to calculate the angles of the servos to achieve the desired wrist
    position.

    Left Pitch Servo = pitch + yaw
    Right Pitch Servo = yaw - pitch

*/
void Wrist::update() {

  int32_t leftCumulative = mPitchTarget + mYawTarget;
  int32_t rightCumulative = mYawTarget - mPitchTarget;

  int32_t leftPos = mapInteger(leftCumulative, mPitchRange[0], mPitchRange[1], 
      mLeftPitchServo.getMinPosition(), mLeftPitchServo.getMaxPosition());
  int32_t rightPos = mapInteger(rightCumulative, mPitchRange[0], mPitchRange[1], 
      mRightPitchServo.getMinPosition(), mRightPitchServo.getMaxPosition());

  mLeftPitchServo.setServoPosition(static_cast<uint8_t>(leftPos));
  mRightPitchServo.setServoPosition(static_cast<uint8_t>(rightPos));
}



void Wrist::setPitch(int16_t pitch) {
    mPitchTarget = CLAMP(pitch, getPitchMin(), getPitchMax());


}

void Wrist::setYaw(int16_t yaw) {
    mYawTarget = CLAMP(yaw, getYawMin(), getYawMax());
}



