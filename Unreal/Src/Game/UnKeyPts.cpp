/*=============================================================================
	UnKeyPts.cpp: Keypoint actor code

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "UnGame.h"
#include "UnFActor.h"

/*-----------------------------------------------------------------------------
	Keypoint base class
-----------------------------------------------------------------------------*/

int AKeypoint::Process(ILevel *Level, FName Message, void *Params)
	{
	GUARD;
	switch (Message.Index)
		{
		case ACTOR_Tick:
			return 0;
		};
	return 0;
	UNGUARD("AKeypoint::Process");
	};

/*-----------------------------------------------------------------------------
	Teleporter
-----------------------------------------------------------------------------*/

int ATeleporter::Process(ILevel *Level, FName Message, void *Params)
	{
	GUARD;
	switch (Message.Index)
		{
        //--------------------------------------------------------------------
        //            Something touched the teleporter...
        //--------------------------------------------------------------------
		case ACTOR_Touch:
            {
            const PTouch & Info = PTouch::Convert(Params);
            const INDEX iToucher = Info.iActor;
            const INDEX iTeleporter = iMe;
            FActor & Toucher = FActor::Actor(iToucher);
            if( Toucher.Send_PreTeleport(iTeleporter) == 1 )
                {
                // Toucher agreed to teleport.
                // We don't do anything now - we count on the actor doing its own teleportation later.
                }
			return ProcessDone;
            }
		};
	return 0;
	UNGUARD("ATeleporter::Process");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
