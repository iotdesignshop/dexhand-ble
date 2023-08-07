#ifndef WRIST_H
#define WRIST_H

/*
Wrist Definition

Wrist is a class that represents the wrist on the DexHand. It is
responsible for managing the servos that control the wrist joints.

Each wrust consists of 2 differential servos that control the 
wrist pitch and yaw. 

The wrist class mixes together the signals for the two differential
servos to create the poses specified by the pitch and yaw angles from
the controller.
*/

#include <Arduino.h>

class ManagedServo;

class Wrist {
    public:
        Wrist(ManagedServo& leftPitchServo, ManagedServo& rightPitchServo);
        virtual ~Wrist();

        // Loop
        void update();          // Called from the main loop to update the wrist servos

        // Positioning
        inline void setPosition(int16_t pitch, int16_t yaw) { setPitch(pitch); setYaw(yaw); }
        void setPitch(int16_t pitch);
        void setYaw(int16_t yaw);
        inline void setDefaultPosition() { setPitch(0); setYaw(0); }
        
        inline int16_t getPitch() const { return mPitchTarget;}
        inline int16_t getYaw() const { return mYawTarget;}
        
        // Ranges
        inline void setPitchRange(int16_t min, int16_t max) { mPitchRange[0] = min; mPitchRange[1] = max; }
        inline void setYawRange(int16_t min, int16_t max) { mYawRange[0] = min; mYawRange[1] = max; }
        
        inline int16_t getPitchMin() const { return mPitchRange[0]; }
        inline int16_t getPitchMax() const { return mPitchRange[1]; }
        
        inline int16_t getYawMin() const { return mYawRange[0]; }
        inline int16_t getYawMax() const { return mYawRange[1]; }
        
        

    private:
        ManagedServo& mLeftPitchServo;
        ManagedServo& mRightPitchServo;
        
        int16_t mPitchTarget;
        int16_t mYawTarget;
        
        int16_t mPitchRange[2];
        int16_t mYawRange[2];

};


#endif