#ifndef _INC_AIANIMAT
#define _INC_AIANIMAT

/*
==============================================================================
AIanimat.h: AI animations
Used by: Actor AI code

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Here are some simple classes that help in the management
    of animations for AI.

Revision history:
    * 04/23/96, Created by Mark
==============================================================================
*/

#include "UnGame.h"
#include "UnChecks.h"

//----------------------------------------------------------------------------
//                           A single animation sequence.
//----------------------------------------------------------------------------
class AIAnimation 
{
public:
    typedef BYTE TSequence  ; // Identifies an animation sequence.
        // TSequence:
        //     1. The value 0 means no sequence.
        //     2. For mesh animations, this is the mesh animation sequence number (1..)
    TSequence       Sequence    ;
    BYTE            First       ; // The first frame number. 
    BYTE            Last        ; // The last frame number. 
    BYTE            Count       ; // How many times to do animation. 0 means forever.
    FLOAT           Rate        ; // The rate of animation. Added to Frame each tick.
    EActorMessage   Message :16 ; // The parameterless message to send at the end of the animation.

    void CopyTo(ARoot & Actor) const; // Copy the animation information to an actor. This starts Actor using this animation.
    void CopyFrom(const ARoot & Actor); // Copy the current animation of Actor into *this.
    void Make(TSequence Sequence, BYTE First, BYTE Last) // Make a standard, repeating, normal speed animation.
    {
        this->Sequence  = Sequence  ;
        this->First     = First     ;
        this->Last      = Last      ;
        this->Rate      = 1.0       ;
        this->Count     = 0         ;
        this->Message   = ACTOR_Null;
    }
};

//----------------------------------------------------------------------------
//                   A short list of animation sequences.
//----------------------------------------------------------------------------
class AIAnimations 
{
public:
    enum { MaxCount = 8 }; // Maximum number of animations.
    void MoveNextTo(ARoot & Actor); // Make Actor start using the next animation and remove it from the list.
    void GetFirstFrom(ARoot & Actor); // Copy the animation in Actor to the front of the list.
    int Count() const { return State.Count; }
    AIAnimation & First() // The first animation.
    {
        checkInput(Count()>0);
        return State.List[State.First];
    }
    const AIAnimation & First() const // The first animation.
    {
        checkInput(Count()>0);
        return State.List[State.First];
    }
    const AIAnimation & operator[](int Which) const // The N'th animation, N = 1,2, ..., Count()
    { 
        checkInput(1<=Which && Which<=Count()); 
        return State.List[ (State.First+Which-1) % MaxCount ]; 
    }
    void Add( const AIAnimation & Animation ); // Add a new animation to the end of the list.
    void Add( AIAnimation::TSequence Sequence, BYTE Count = 0, FLOAT Rate = 1.0 ); // Add a new animation to the end of the list.
    void Empty() { State.Count = 0; State.First = 0; }
    void FinishSoon(); // Set all animations to finish as soon as possible.
    BOOL IsEmpty() const { return State.Count==0; }
    void RemoveFirst(); // Remove the first animation, if there is one.
private:
    struct 
    {
        int Count; //tbm:private. Number of animations in List[].
        AIAnimation List[MaxCount];
        int First; // Index into List[] of first (current) animation.
        // Notes:
        //   1. The animations are in List[First],List[(First+1)%MaxCount],...
    }
    State;
};


#endif
