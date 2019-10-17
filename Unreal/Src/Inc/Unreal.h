/*=============================================================================
	Unreal.h: Main header for the Unreal engine

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNREAL
#define _INC_UNREAL

/*----------------------------------------------------------------------------
	Low level includes
----------------------------------------------------------------------------*/


//
// Version info and compile options specific to this build of the Unreal engine
//
#ifndef  _INC_UNBUILD
#include "UnBuild.h"
#endif

//
// Unreal Platform-specific defines for portability
//
#ifndef  _INC_UNPORT
#include "UnPort.h" 
#endif

//
// All platform-independent hooks visible to Unreal which
// call platform-specific routines elsewhere
//
#ifndef __PLATFORM__
#include "UnPlatfm.h"
#endif

/*-----------------------------------------------------------------------------
	Global variables
-----------------------------------------------------------------------------*/

//
// Every major, Unreal subsystem has a global class associated with it,
// and a global variable named GSomthing.
//
UNREAL_API extern class FUnrealEngine			GUnreal;
UNREAL_API extern class FGlobalResourceManager	GRes;
UNREAL_API extern class FUnrealServer			GServer;
UNREAL_API extern class FGlobalDefaults			GDefaults;
UNREAL_API extern class FGlobalPlatform			*GApp;
UNREAL_API extern class FGlobalTopicTable		GTopics;
UNREAL_API extern class FGlobalClasses			GClasses;
UNREAL_API extern class FMemoryCache			GCache;
UNREAL_API extern class FMemPool				GMem,GDynMem;
UNREAL_API extern class FGlobalGfx				GGfx;
UNREAL_API extern class FGlobalMath				GMath;
UNREAL_API extern class FGlobalAudio			GAudio;
UNREAL_API extern class FGlobalRender			*GRend;
UNREAL_API extern class FCameraManager			*GCameraManager;
UNREAL_API extern class UTransBuffer			*GTrans;
UNREAL_API extern class FEditor					*GEditor;
UNREAL_API extern class FVirtualGame			*GVirtualGame;
UNREAL_API extern class NManager				*GNetManager;

/*-----------------------------------------------------------------------------
	High-level includes
-----------------------------------------------------------------------------*/

#ifndef _INC_UNFILE		// Low-level, platform-independent file functions
#include "UnFile.h"
#endif

#ifndef _INC_UNMATH		// Vector math functions
#include "UnMath.h"
#endif

#ifndef _INC_UNPARAMS	// Parameter parsing routines
#include "UnParams.h"
#endif

#ifndef _INC_UNCACHE	// In-memory object caching
#include "UnCache.h"
#endif

#ifndef _INC_UNMEM		// Fast memory pool allocation
#include "UnMem.h"
#endif

#ifndef _INC_UNRESTYP	// Resource type definitions
#include "UnResTyp.h"
#endif

#ifndef _INC_UNRES		// Resource manager
#include "UnRes.h"
#endif

#ifndef _INC_UNTOPICS	// Topic handlers for editor/server communication
#include "UnTopics.h"
#endif

#ifndef _INC_UNACTOR	// Actor subsystem
#include "UnActor.h"
#endif

#ifndef _INC_UNLEVEL	// Level resource
#include "UnLevel.h"
#endif

#ifndef _INC_UNCAMERA	// Camera subsystem
#include "UnCamera.h"
#endif

#ifndef _INC_UNFGAUD	// Audio subsystem FGlobalAudio
#include "UnFGAud.h"
#endif

#ifndef _INC_UNSOUND	// Audio subsystem main
#include "UnSound.h"
#endif

#ifndef _INC_UNGFX		// Graphics subsystem
#include "UnGfx.h"
#endif

#ifndef _INC_UNSERVER	// Unreal server
#include "UnServer.h"
#endif

#ifndef _INC_UNDEFLTS	// Unreal defaults
#include "UnDeflts.h"
#endif

#ifndef _INC_UNENGINE	// Unreal engine
#include "UnEngine.h"
#endif

#ifndef _INC_UNVGAME	// Virtual game class
#include "UnVGame.h"
#endif

//
// Files required by the Unreal editor
//
#ifdef EDITOR

	#ifndef _INC_UNEDITOR	// Unreal editor
	#include "UnEditor.h"
	#endif

	#ifndef _INC_UNEDTRAN	// Transaction tracking system
	#include "UnEdTran.h"
	#endif

#endif
/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
#endif // _INC_UNREAL
