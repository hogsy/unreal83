/*=============================================================================
	UnOutDev.cpp: Unreal FOutputDevice implementation

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

/*-----------------------------------------------------------------------------
	FOutputDevice implementation
-----------------------------------------------------------------------------*/

//
// Print a message on the output device using LOG_Info type.
//
void FOutputDevice::Log(const char *Text)
	{
	GUARD;
	Log(LOG_Info,Text);
	UNGUARD("FOutputDevice::Log");
	};

//
// Print a message on the output device, variable parameters.
//
void VARARGS FOutputDevice::Logf(ELogType Event,const char *Fmt,...)
	{
	char TempStr[4096];
	va_list  ArgPtr;
	//
	va_start (ArgPtr,Fmt);
	vsprintf (TempStr,Fmt,ArgPtr);
	va_end   (ArgPtr);
	//
	GUARD;
	Log(Event,TempStr);
	UNGUARD("FOutputDevice::Logf");
	};

//
// Print a message on the output device, variable parameters.
//
void VARARGS FOutputDevice::Logf(const char *Fmt,...)
	{
	char TempStr[4096];
	va_list  ArgPtr;
	//
	va_start (ArgPtr,Fmt);
	vsprintf (TempStr,Fmt,ArgPtr);
	va_end   (ArgPtr);
	//
	GUARD;
	Log(LOG_Info,TempStr);
	UNGUARD("FOutputDevice::Logf");
	};

//
// Output device startup message.
//
void FOutputDevice::SpawnConsoleMessage(void)
	{
	GUARD;
	//
	Logf(CONSOLE_SPAWN_1);
	Logf(CONSOLE_SPAWN_2);
	Logf("");
	//
	UNGUARD("FOutputDevice::SpawnConsoleMessage");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
