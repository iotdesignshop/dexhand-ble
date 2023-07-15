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
        Finger(ManagedServo& leftPitchServo, ManagedServo& rightPitchServo, ManagedServo& yawServo, ManagedServo& flexionServo);
        virtual ~Finger();

        // Loop
        void update();          // Called from the main loop to update the finger's servos

        // Positioning
        inline void setPosition(uint8_t pitch, uint8_t yaw, uint8_t flexion) { setPitch(pitch); setYaw(yaw); setFlexion(flexion);}
        void setPitch(uint8_t pitch);
        void setYaw(uint8_t yaw);
        void setFlexion(uint8_t flexion);
        
        inline uint8_t getPitch() const { return mPitchTarget;}
        inline int8_t getYaw() const { return mYawTarget;}
        uint8_t getFlexion() const;

        // Ranges
        inline void setPitchRange(uint8_t min, uint8_t max) { mPitchRange[0] = min; mPitchRange[1] = max; }
        inline void setYawRange(int8_t min, int8_t max) { mYawRange[0] = min; mYawRange[1] = max; }
        inline void setYawBias(uint8_t bias) { mYawBias = bias; }
        inline void setFlexionRange(uint8_t min, uint8_t max) { mFlexionRange[0] = min; mFlexionRange[1] = max; }
        
        inline uint8_t getPitchMin() const { return mPitchRange[0]; }
        inline uint8_t getPitchMax() const { return mPitchRange[1]; }
        
        inline int8_t getYawMin() const { return mYawRange[0]; }
        inline int8_t getYawMax() const { return mYawRange[1]; }
        inline uint8_t getYawBias() const { return mYawBias; }
        
        inline uint8_t getFlexionMin() const { return mFlexionRange[0]; }
        inline uint8_t getFlexionMax() const { return mFlexionRange[1]; }



    private:
        ManagedServo& mLeftPitchServo;
        ManagedServo& mRightPitchServo;
        ManagedServo& mFlexionServo;

        uint8_t mPitchTarget;
        int8_t mYawTarget;

        uint8_t mPitchRange[2];
        int8_t mYawRange[2];
        uint8_t mFlexionRange[2];
        uint8_t mYawBias;

        void updatePitchServos();

        float normalizedValue(int32_t value, int32_t min, int32_t max) const;
        int32_t mapInteger(int32_t value, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax) const;



        

};


#endif