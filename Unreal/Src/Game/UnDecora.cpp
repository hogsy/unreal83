/*
==============================================================================
UnDecora.cpp: Unreal decoration actors

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0, Calling method=__fastcall

Revision history:
    * 08/12/96: Created by Mark
==============================================================================
*/

#include "UnGame.h"
#include "UnFActor.h"
#include "UnInput.h"

//----------------------------------------------------------------------------
//                    Decoration processing
//----------------------------------------------------------------------------
int ADecorations::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    FActor & Actor = FActor::Actor(*this);
//tbd:    switch( Message.Index )
//tbd:    {
//tbd:        case ACTOR_Spawn:
//tbd:        case ACTOR_BeginPlay:
//tbd:        {
//tbd:            AActorAI & AI = Actor.AI();
//tbd:            AI.Initialize();
//tbd:            Actor.Animate( 1, 0 );
//tbd:            break;
//tbd:        }
//tbd:    }
    return ProcessParent; 
    UNGUARD( "ADecoration::Process" );
}

//----------------------------------------------------------------------------
//                    Vase processing
//----------------------------------------------------------------------------
int AVase::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent; 
    UNGUARD( "AVase::Process" );
}

//----------------------------------------------------------------------------
//                    Chandelier processing
//----------------------------------------------------------------------------
int AChandelier::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent; 
    UNGUARD( "AChandelier::Process" );
}

//----------------------------------------------------------------------------
//                    Hammok processing
//----------------------------------------------------------------------------
int AHammok::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent; 
    UNGUARD( "AHammok::Process" );
}

//----------------------------------------------------------------------------
//                    Flame processing
//----------------------------------------------------------------------------
int AFlame::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
//tbd:    AnimLast = 11;
    switch( Message.Index )
    {
        case ACTOR_Tick:
        {
            static int dYaw = 180;
            #if 0 //tbd:
            if( (GServer.Ticks & 0x0) == 0 )
            {
                if( GInput.Switches[FInput::S_F6].IsOn )
                {
                    dYaw += 10;
                    GInput.Switches[FInput::S_F6].IsOn = FALSE;
                    debugf( LOG_Info, "dYaw: %i", dYaw );
                }
                if( GInput.Switches[FInput::S_F7].IsOn )
                {
                    dYaw -= 10;
                    GInput.Switches[FInput::S_F7].IsOn = FALSE;
                    debugf( LOG_Info, "dYaw: %i", dYaw );
                }
            }
            #endif
            DrawRot.Yaw += dYaw;
            break;
        }
    }   
    return ProcessParent; 
    UNGUARD( "AFlame::Process" );
}
