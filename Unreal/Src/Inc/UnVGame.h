/*=============================================================================
	UnVGame.h: Definition of FVirtualGame, base class of game specific stuff

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Contains routines for: Messages, menus, status bar
=============================================================================*/

#ifndef _INC_UNVGAME
#define _INC_UNVGAME

/*------------------------------------------------------------------------------
	FGame
------------------------------------------------------------------------------*/

//
// Base class of all game-specific stuff.  This enables all
// game-specific routines to be moved to a DLL which simply
// exports its global FGame-derived class.  This is for
// convenience; it's not intended to be an OOP++ overkill.
//
class FVirtualGame
	{
	public:
	//
	// Init and exit the game-specific code.  Called only once per
	// session.
	//
	virtual void Init(void)=0;
	virtual void Exit(void)=0;
	virtual void CheckState(void)=0;
	//
	// Creating and destroying game-specific camera consoles:
	//
	virtual class FVirtualCameraConsole *CreateCameraConsole(UCamera *Camera)=0;
	virtual void DestroyCameraConsole(class FVirtualCameraConsole *Console)=0;
	//
	// Other:
	//
	virtual int Exec(const char *Cmd,FOutputDevice *Out=GApp)=0;
	};

/*------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------*/
#endif // _INC_UNVGAME
