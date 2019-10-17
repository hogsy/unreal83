/*
==============================================================================
AIanimat.cpp: AI animations

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Refer to the associated header file.

Revision history:
    * 04/23/96: Created by Mark
==============================================================================
*/

#include "AIAnimat.h"
#include "Unreal.h"

#if 0 //tbd: Debugging
static void Dump(const AIAnimation & Animation)
{
    debugf
    (
        LOG_INFO
    ,   " [Seq=%i Count=%i Rate=%3.2f First=%i Last=%i]"
    ,   int(Animation.Sequence)
    ,   int(Animation.Count)
    ,   float(Animation.Rate)
    ,   int(Animation.First)
    ,   int(Animation.Last)
    );
}

static void Dump(const AIAnimations & Animations,int First, int Count)
{
    debugf( LOG_INFO, "Animations:[First=%i Count=%i]", First, Count );
    for( int Which = 1; Which <= Animations.Count(); ++Which )
    {
        Dump( Animations[Which] );
    }
}
#endif

//----------------------------------------------------------------------------
//               Copy an animation to an Actor
//----------------------------------------------------------------------------
// This makes the Actor start doing the animation.
void AIAnimation::CopyTo(ARoot & Actor) const
{
    Actor.AnimSeq       = Sequence  ;
    Actor.AnimRate      = Rate      ;
    Actor.AnimBase      = First     ;
    Actor.AnimCount     = Count     ;
    Actor.AnimFirst     = First     ;
    Actor.AnimLast      = Last      ;
    Actor.AnimMessage   = Message   ;
    Actor.bAnimate      = TRUE      ;
}

//----------------------------------------------------------------------------
//               Copy an Actor's animation 
//----------------------------------------------------------------------------
// This preserves the animation, although the current frame is not saved.
void AIAnimation::CopyFrom(const ARoot & Actor)
{
    Sequence    = Actor.AnimSeq     ;
    Rate        = Actor.AnimRate    ;
    Count       = Actor.AnimCount   ;
    First       = Actor.AnimFirst   ;
    Last        = Actor.AnimLast    ;
    Message     = EActorMessage(Actor.AnimMessage);
}

//----------------------------------------------------------------------------
//            Remove the next animation to Actor.
//----------------------------------------------------------------------------
void AIAnimations::MoveNextTo(ARoot & Actor)
{
    if( Count() > 0 )
    {
        const AIAnimation & Animation = First();
        Animation.CopyTo(Actor);
        RemoveFirst();
    }
}

//----------------------------------------------------------------------------
//      Move the animation in Actor to the front of the list.
//----------------------------------------------------------------------------
void AIAnimations::GetFirstFrom(ARoot & Actor)
{
    //tba (if we need this) or //tbd (if we don't need this)
    checkVital( FALSE, "GetFirstFrom not working" );
}

//----------------------------------------------------------------------------
//         Add a new animation to the end of the list.
//----------------------------------------------------------------------------
void AIAnimations::Add( const AIAnimation & Animation )
{
    if( State.Count==MaxCount )
    {
        // There is no more room for animations. Let's make a debug entry
        // the first few times this happens:
        static int Limiter = 0;
        if( Limiter <= 10 )
        {
            debug( LOG_Info, "Too many animations*************" );
            Limiter++;
        }
        // In order to make room for the new animation, let's drop the oldest one.
        RemoveFirst();
    }
    //tbi: efficiency of %?
    AIAnimation & NextAnimation = State.List[ (State.First+State.Count) % MaxCount ];
    State.Count++;
    NextAnimation = Animation;
}

//----------------------------------------------------------------------------
//         Add a new animation to the end of the list.
//----------------------------------------------------------------------------
void AIAnimations::Add( AIAnimation::TSequence Sequence, BYTE Count, FLOAT Rate )
{
    AIAnimation Animation;
    Animation.Make( Sequence, 0, 0 );
    Animation.Count = Count ;
    Animation.Rate  = Rate  ;
    Add( Animation );
}

//----------------------------------------------------------------------------
//      Set all animations to finish as soon as possible.
//----------------------------------------------------------------------------
void AIAnimations::FinishSoon()
{
    const int Last = State.Count-1;
    for( int Which = 0; Which <= Last; ++Which )
    {
        AIAnimation & Animation = State.List[ (State.First+Which) % MaxCount ];
        Animation.Count = 1;
    }
}

//----------------------------------------------------------------------------
//          Remove the first animation, if there is one.
//----------------------------------------------------------------------------
void AIAnimations::RemoveFirst()
{
    if( State.Count > 0 )
    {
        State.First = (State.First+1)%MaxCount;
        State.Count--;
    }
}
