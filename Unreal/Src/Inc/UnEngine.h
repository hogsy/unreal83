/*=============================================================================
	UnEngine.h: Unreal engine definition

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNENGINE
#define _INC_UNENGINE

/*-----------------------------------------------------------------------------
	FUnrealEngine: Unreal engine
-----------------------------------------------------------------------------*/

class UNREAL_API FUnrealEngine
	{
	public:
	//
	int Init
		(
		class FGlobalPlatform	*Platform, 
		class FTaskManager		*TaskManager,
		class FCameraManager	*CameraManager, 
		class FGlobalRender		*Rend, 
		class FVirtualGame		*Game,
		class NManager			*NetManager,
		class FEditor			*Editor
		);
	void Exit(void);
	void ErrorExit(void);
	//
	UCamera *OpenCamera(void);
	void Draw (UCamera *Camera, int Scan);
	//
	void EnterWorld(const char *WorldURL);
	void GetWorldInfo(char *WorldURL, char *WorldTitle);
	//
	void InitGame(void);
	void ExitGame(void);
	//
	int Exec(const char *Cmd,FOutputDevice *Out=GApp);
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
#endif // _INC_UNENGINE

