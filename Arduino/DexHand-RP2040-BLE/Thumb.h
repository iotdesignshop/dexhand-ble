#ifndef THUMB_H
#define THUMB_H

/*
Thumb Definition

Thumb is a class that represents the thumb on the DexHand. It is
responsible for managing the servos that control the thumb joints.

Each thumb consists of 4 servos. There are two differential servos
that control the knuckle pitch and yaw. The third servo controls the
thumb's flexion which is the tendon running through the finger
to the tip. And the fourth controls the roll of the thumb.

The thumb class mixes together the signals for the two differential
servos to create the poses specified by the pitch and yaw angles from
the controller.

Roll is also automatically computed based on the pitch and yaw angles.

*/
#include <Arduino.h>

class ManagedServo;

class Thumb {
    public:
        Thumb(ManagedServo& leftPitchServo, ManagedServo& rightPitchServo, ManagedServo& flexionServo, ManagedServo& rollServo);
        virtual ~Thumb();

        // Loop
        void update();          // Called from the main loop to update the thumb servos

        // Positioning
        inline void setPosition(uint8_t pitch, uint8_t yaw, uint8_t flexion) { setPitch(pitch); setYaw(yaw); setFlexion(flexion);}
        void setPitch(uint8_t pitch);
        void setYaw(int8_t yaw);
        void setFlexion(uint8_t flexion);
        void setRoll(uint8_t roll);
        
        inline uint8_t getPitch() const { return mPitchTarget;}
        inline int8_t getYaw() const { return mYawTarget;}
        inline uint8_t getFlexion() const { return mFlexionTarget;}

        // Ranges
        inline void setPitchRange(uint8_t min, uint8_t max) { mPitchRange[0] = min; mPitchRange[1] = max; }
        inline void setYawRange(int8_t min, int8_t max) { mYawRange[0] = min; mYawRange[1] = max; }
        inline void setYawBias(uint8_t bias) { mYawBias = bias; }
        inline void setFlexionRange(uint8_t min, uint8_t max) { mFlexionRange[0] = min; mFlexionRange[1] = max; }
        inline void setRollRange(uint8_t min, uint8_t max) { mRollRange[0] = min; mRollRange[1] = max; }    

        inline uint8_t getPitchMin() const { return mPitchRange[0]; }
        inline uint8_t getPitchMax() const { return mPitchRange[1]; }
        
        inline int8_t getYawMin() const { return mYawRange[0]; }
        inline int8_t getYawMax() const { return mYawRange[1]; }
        inline uint8_t getYawBias() const { return mYawBias; }
        
        inline uint8_t getFlexionMin() const { return mFlexionRange[0]; }
        inline uint8_t getFlexionMax() const { return mFlexionRange[1]; }

        inline uint8_t getRollMin() const { return mRollRange[0]; }
        inline uint8_t getRollMax() const { return mRollRange[1]; }




    private:
        ManagedServo& mLeftPitchServo;
        ManagedServo& mRightPitchServo;
        ManagedServo& mFlexionServo;
        ManagedServo& mRollServo;

        uint8_t mPitchTarget;
        int8_t mYawTarget;
        uint8_t mFlexionTarget;
        uint8_t mRollTarget;

        uint8_t mPitchRange[2];
        int8_t mYawRange[2];
        uint8_t mFlexionRange[2];
        uint8_t mRollRange[2];
        uint8_t mYawBias;

        void updatePitchServos();
};


#endif