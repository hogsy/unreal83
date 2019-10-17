/*
==============================================================================
UnExplos.cpp: Actor explosion handling

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0, Calling method=__fastcall

Description:
    Functions for handling actors of class Explosion.

Revision history:
    * 09/26/96: Created by Mark
==============================================================================
*/

#include "UnGame.h"
#include "UnFActor.h"

//---------------------------------------------------------------------------
//              AExplosion::Process
//---------------------------------------------------------------------------
int AExplosion::Process(ILevel *Level, FName Message, void *Params)
{
   GUARD;
   FActor & Actor = FActor::Actor(*this);
   switch (Message.Index)
    {
        //--------------------------------------------------------------------
        //                      Initialization
        //--------------------------------------------------------------------
        case ACTOR_Spawn: //tbi: Once all actors get BeginPlay, remove Spawn
        case ACTOR_BeginPlay:
        {
            //todo: Add
            break;
        }
        //--------------------------------------------------------------------
        //                      Time passes...
        //--------------------------------------------------------------------
        case ACTOR_Tick:
        {
            break;
        }
    }
    return ProcessParent;
    UNGUARD( "AExplosion::Process" );
}

