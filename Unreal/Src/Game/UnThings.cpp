/*=============================================================================
	UnThings.cpp: Various actor routines

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "UnGame.h"

/*-----------------------------------------------------------------------------
	Light base class
-----------------------------------------------------------------------------*/

int ALight::Process(ILevel *Level, FName Message, void *Params)
	{
	GUARD;
	switch (Message.Index)
		{
		case ACTOR_Tick:
			return 0;
		case ACTOR_Trigger: // Temporary hack for testing triggers. You can remove this
			LightType = LT_Steady;
			return 1;
		case ACTOR_UnTrigger: // Temporary hack for testing triggers. You can remove this
			LightType = LT_None;
			return 1;
		};
	return 0;
	UNGUARD("ALight::Process");
	};

/*-----------------------------------------------------------------------------
	Camera base class
-----------------------------------------------------------------------------*/

int ACamera::Process(ILevel *Level, FName Message, void *Params)
	{
	GUARD;
	switch (Message.Index)
		{
		case ACTOR_PlayerTick:
			return GGame.PlayerTick(iMe,Params);
		case ACTOR_PlayerCalcView:
			break;
			//return aPawn (Level,iActor,Msg,Params);
		};
	return 0;
	UNGUARD("ACamera::Process");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
