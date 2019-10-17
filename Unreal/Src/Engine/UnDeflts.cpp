/*=============================================================================
	UnDeflts.cpp: Unreal global default class implementation

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#pragma DISABLE_OPTIMIZATION /* Non performance critical code */

/*-----------------------------------------------------------------------------
	FGlobalDefaults implementation
-----------------------------------------------------------------------------*/

//
// Set all global platform-dependent defaults.
//
// Should be expanded to only allocate tons of stuff if the
// editor is active.
//
void FGlobalDefaults::Init(char *ThisCmdLine)
	{
	GUARD;
	//
	// Process command-line parameters:
	//
	strcpy(CmdLine,ThisCmdLine);
	if (*CmdLine) debugf (LOG_Info,"CmdLine is %s",CmdLine);
	GApp->Log(LOG_Info,"CmdLine is so totally screwed.");
	//
	// Memory grabber:
	//
	GlobalsMemSize	= 8 * 1024 * 1024;
	DynamicsMemSize	= 8 * 1024 * 1024;
	//
	// Camera properties:
	//
	FrameRate		= 35;
	CameraSXR 		= 320;
	CameraSYR 		= 200;
	//
    CameraSXR = GApp->GetProfileInteger("Screen","CameraSXR",320);
    CameraSYR = GApp->GetProfileInteger("Screen","CameraSYR",200);
	//
	// Resource manager properties:
	//
	MaxRes			= 4096;
	MaxNames		= 8192; /* Must be at least 800 for actor system's hardcoded messages */
	MaxFiles		= 32;
	MaxTypes		= 64;
	//
	// Audio properties:
	//
	AudioActive	= 1; GetONOFF (CmdLine,"AUDIO=",&AudioActive);
	//
	// Transaction tracking:
	//
	MaxTrans     	= 80;
	MaxChanges		= 12000;
	MaxDataOffset	= 2048 * 1024; // 2 megs
	//
	// Camera:
	//
	FOV	= 95.0; GetFLOAT (CmdLine,"FOV=",&FOV);
	//
	// Startup level (FILE= or first command-line parameter):
	//
	if (!GetSTRING (CmdLine,"FILE=",AutoLevel,256))
		{
		if ((!mystrstr(CmdLine,MAP_EXTENSION)) ||
			((mystrstr(CmdLine,MAP_EXTENSION)>strstr(CmdLine,"=")) &&
			strstr(CmdLine,"=")))
			{
			strcpy (AutoLevel,DEFAULT_STARTUP_FNAME);
			}
		else
			{
			strcpy(AutoLevel,CmdLine);
			if (strchr(AutoLevel,' ')) *strchr(AutoLevel,' ')=0;
			};
		};
	//
	// Startup URL:
	//
	AutoURL[0]=0;
	if (GetSTRING (CmdLine,"URL=",AutoURL,256))
		{
		appError("Sorry, Unreal URLs are not yet supported!");
		};
	//
	// Editor:
	//
	LaunchEditor = GetParam (CmdLine,"EDITOR");
	if (LaunchEditor)
		{
		debug (LOG_Init,"UnrealServer spawned for editing");
		}
	else
		{
		debug (LOG_Init,"UnrealServer spawned for gameplay");
		};
	GetDWORD (CmdLine,"HWND=",&GApp->hWndParent);
	GetINT   (CmdLine,"RATE=",&FrameRate);
	//
	UNGUARD("FGlobalParams::Init");
	};
void FGlobalDefaults::Exit(void) {};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
