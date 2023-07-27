#ifndef FINGER_H
#define FINGER_H

/*
Finger Definition

Finger is a class that represents a single finger on the DexHand. It is
responsible for managing the servos that control the finger's joints.

Each finger consists of 3 servos. There are two differential servos
that control the knuckle pitch and yaw. The third servo controls the
finger's flexion which is the tendon running through the finger
to the tip.

The finger class mixes together the signals for the two differential
servos to create the poses specified by the pitch and yaw angles from
the controller.

Finger yaw is also modulated by the flexion angle. As the finger flexes,
the yaw angle is reduced to keep the finger from bending sideways as the
fingers align into more of a fist.
*/
#include <Arduino.h>

class ManagedServo;

class Finger {
    public:
        Finger(ManagedServo& leftPitchServo, ManagedServo& rightPitchServo, ManagedServo& flexionServo);
        virtual ~Finger();

        // Loop
        void update();          // Called from the main loop to update the finger's servos

        // Positioning
        inline void setPosition(int16_t pitch, int16_t yaw, int16_t flexion) { setPitch(pitch); setYaw(yaw); setFlexion(flexion);}
        void setPitch(int16_t pitch);
        void setYaw(int16_t yaw);
        void setFlexion(int16_t flexion);
        
        inline int16_t getPitch() const { return mPitchTarget;}
        inline int16_t getYaw() const { return mYawTarget;}
        inline int16_t getFlexion() const { return mFlexionTarget;}

        // Ranges
        inline void setPitchRange(int16_t min, int16_t max) { mPitchRange[0] = min; mPitchRange[1] = max; }
        inline void setYawRange(int16_t min, int16_t max) { mYawRange[0] = min; mYawRange[1] = max; }
        inline void setYawBias(int16_t bias) { mYawBias = bias; }
        inline void setFlexionRange(int16_t min, int16_t max) { mFlexionRange[0] = min; mFlexionRange[1] = max; }
        
        inline int16_t getPitchMin() const { return mPitchRange[0]; }
        inline int16_t getPitchMax() const { return mPitchRange[1]; }
        
        inline int16_t getYawMin() const { return mYawRange[0]; }
        inline int16_t getYawMax() const { return mYawRange[1]; }
        inline int16_t getYawBias() const { return mYawBias; }
        
        inline int16_t getFlexionMin() const { return mFlexionRange[0]; }
        inline int16_t getFlexionMax() const { return mFlexionRange[1]; }



    private:
        ManagedServo& mLeftPitchServo;
        ManagedServo& mRightPitchServo;
        ManagedServo& mFlexionServo;

        int16_t mPitchTarget;
        int16_t mYawTarget;
        int16_t mFlexionTarget;

        int16_t mPitchRange[2];
        int16_t mYawRange[2];
        int16_t mFlexionRange[2];
        int16_t mYawBias;

        void updatePitchServos();

 


        

};


#endif