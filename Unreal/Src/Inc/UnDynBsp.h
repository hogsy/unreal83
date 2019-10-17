/*=============================================================================
	UnDynBsp.h: Unreal dynamic Bsp object support

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNDYNBSP
#define _INC_UNDYNBSP

/*---------------------------------------------------------------------------------------
	Temporary globals
---------------------------------------------------------------------------------------*/

//
// Will be replaced with FMovingBrushTracker after initial development
//
UNREAL_API void sporeInit(ULevel *Level);
UNREAL_API void sporeExit(void);
UNREAL_API void sporeLock(ILevel *Level);
UNREAL_API void sporeUnlock(void);
UNREAL_API void sporeUpdate(INDEX iActor);
UNREAL_API void sporeFlush(INDEX iActor);
UNREAL_API int  sporeSurfIsDynamic(INDEX iSurf);

/*---------------------------------------------------------------------------------------
	The End
---------------------------------------------------------------------------------------*/
#endif // _INC_UNDYNBSP

