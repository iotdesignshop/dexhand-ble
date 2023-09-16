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
        inline void setPosition(int16_t pitch, int16_t yaw, int16_t flexion) { setPitch(pitch); setYaw(yaw); setFlexion(flexion);}
        void setPitch(int16_t pitch);
        void setYaw(int16_t yaw);
        void setFlexion(int16_t flexion);
        void setRoll(int16_t roll);
        void setMaxPosition();
        void setMinPosition();
        void setExtension(int16_t percent);     // Sets overall thumb extension from 0 (closed) to 100 (open)
        
        
        inline int16_t getPitch() const { return mPitchTarget;}
        inline int16_t getYaw() const { return mYawTarget;}
        inline int16_t getFlexion() const { return mFlexionTarget;}

        // Ranges
        inline void setPitchRange(int16_t min, int16_t max) { mPitchRange[0] = min; mPitchRange[1] = max; }
        inline void setYawRange(int16_t min, int16_t max) { mYawRange[0] = min; mYawRange[1] = max; }
        inline void setFlexionRange(int16_t min, int16_t max) { mFlexionRange[0] = min; mFlexionRange[1] = max; }
        inline void setRollRange(int16_t min, int16_t max) { mRollRange[0] = min; mRollRange[1] = max; }    

        inline int16_t getPitchMin() const { return mPitchRange[0]; }
        inline int16_t getPitchMax() const { return mPitchRange[1]; }
        
        inline int16_t getYawMin() const { return mYawRange[0]; }
        inline int16_t getYawMax() const { return mYawRange[1]; }
        
        inline int16_t getFlexionMin() const { return mFlexionRange[0]; }
        inline int16_t getFlexionMax() const { return mFlexionRange[1]; }

        inline int16_t getRollMin() const { return mRollRange[0]; }
        inline int16_t getRollMax() const { return mRollRange[1]; }




    private:
        ManagedServo& mLeftPitchServo;
        ManagedServo& mRightPitchServo;
        ManagedServo& mFlexionServo;
        ManagedServo& mRollServo;

        int16_t mPitchTarget;
        int16_t mYawTarget;
        int16_t mFlexionTarget;
        int16_t mRollTarget;

        int16_t mPitchRange[2];
        int16_t mYawRange[2];
        int16_t mFlexionRange[2];
        int16_t mRollRange[2];
        
        void updatePitchServos();
};


#endif