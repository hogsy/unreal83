#ifndef _INC_AIMOTION
#define _INC_AIMOTION
/*
==============================================================================
AImotion.h: AI (Artificial Intelligence) Movement Definition
Used by: Actor AI routines

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Here we have declarations for the expression of AI motion.
    The idea is to simplify and encapsulate the management of motion-oriented 
    AI goals. Here are some examples of such goals:
      - move to a particular spot or towards a particular actor
      - turn to face a particular spot or particular actor
      - move in a particular pattern
      
Revision history:
    * 04/25/96, Created by Mark
==============================================================================
*/

#include "UnReal.h"
#include "UnMath.h"

struct AIMotion 
{
    typedef enum
    {
        None            = 0 // (Always 0) No location goal.
    ,   ToLocation          // Move/turn towards a 3-D location.
    ,   ToActor             // Move/turn towards an actor.
    ,   AsActor             // Mimic the movements of an actor.
    ,   InPattern           // A pattern (such as marching back and forth).
    ,   AlongVector         // Move/turn in a specific direction.
    ,   UniformChange       // Move with constant velocity, turn with constant changes.
    ,   RandomizedChange    // Move with randomized velocity, turn with randomized changes.
    }
    TKind;

    struct TPattern
    {
        //tba: ?
    };

    struct TGoal
    {
        TKind       Kind    ; // Kind of motion goal.
        union                 // Details which depend on Kind.
        {                     
            FVector     Location        ; // [ToLocation]
            INDEX       iActor          ; // [ToActor,AsActor]
            TPattern    Pattern         ; // [InPattern]
            FVector     Direction       ; // [AlongVector,UniformChange]
            FVector     dLocation       ; // [UniformChange(location), RandomizedChange(location)]
            FRotation   dRotation       ; // [UniformChange(rotation), RandomizedChange(rotation)]
            // Notes:
            //   1. Each above member is meaningful only then Kind is
            //      one of the values shown in brackets [] beside the member.
        };
        void Empty() { Kind = None; }
        BOOL IsEmpty() const { return Kind==0; }
        void ToLocation( const FVector & Location )
        {
            this->Kind      = AIMotion::ToLocation  ;
            this->Location  = Location              ;
        }
        void ToActor( INDEX Actor )
        {
            this->Kind      = AIMotion::ToActor     ;
            this->iActor    = Actor                 ;
        }
        void AsActor( INDEX Actor )
        {
            this->Kind      = AIMotion::AsActor     ;
            this->iActor    = Actor                 ;
        }
        void InPattern() //tba: define the kind of pattern
        {
            this->Kind      = AIMotion::InPattern   ;
            //tba:this->PatternState = ???;
        }
        void AlongVector( const FVector & Direction )
        {
            this->Kind      = AIMotion::AlongVector ;
            this->Direction = Direction             ;
        }
    };

    TGoal   Location    ; // Where we want to be.
    TGoal   Rotation    ; // Where we want to turn.

    void Empty()
    {
        Location.Empty();
        Rotation.Empty();
    }

    BOOL HasLocation() const { return !Location.IsEmpty(); }
    BOOL HasRotation() const { return !Rotation.IsEmpty(); }
    BOOL IsEmpty() const { return Location.IsEmpty() && Rotation.IsEmpty(); }

    // Convenience routines:
    void GoTo(INDEX Actor) // Go to an actor (move towards him, face him).
    {
        Location.ToActor(Actor);
        Rotation.ToActor(Actor);
    }
    void GoTo(const FVector & Location) // Go to a location (move towards it, face it).
    {
        this->Location.ToLocation(Location);
        this->Rotation.ToLocation(Location);
    }
    void Face(INDEX Actor) // Turn towards an actor.
    {
        Location.Empty();
        Rotation.ToActor(Actor);
    }
    void GoAlongVector(const FVector & Direction)
    {
        Rotation.AlongVector(Direction);
        Location.AlongVector(Direction);
    }
    void Spin( int dPitch, int dYaw, int dRoll ) //tbi: WORD
    {
        Rotation.Kind               = UniformChange ;
        Rotation.dRotation.Pitch    = dPitch        ;
        Rotation.dRotation.Yaw      = dYaw          ;
        Rotation.dRotation.Roll     = dRoll         ;
    }
};

#endif
