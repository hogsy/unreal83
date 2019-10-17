/*=============================================================================
	UnLaunch.cpp: Unreal splash screen & launcher

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include <direct.h>
#include "Windows.h"
#include "UnBuild.h"
#include "Resource.h"

/*-----------------------------------------------------------------------------
	Launcher WinMain
-----------------------------------------------------------------------------*/

int WINAPI WinMain(HINSTANCE  hInstance, HINSTANCE  hPrevInstance,
	LPSTR lpCmdLine,int nShowCmd)
	{
	char Path[256],*c=Path;
	strcpy(Path,__argv[0]);
	if (strchr(Path,' ')) *strchr(Path,' ')=0;
	while(1)
		{
		if		(strchr(c,'/' )) c = strchr(c,'/' )+1;
		else if	(strchr(c,'\\')) c = strchr(c,'\\')+1;
		else break;
		};
	if (_strnicmp(c,LAUNCH_PARTIAL,strlen(c)))
		{
		MessageBox
			(
			NULL,
			GAME_NAME " has been renamed.  "
			"Please check your configuration, consider reinstalling, and try again.",
			"Failed to launch " GAME_NAME,
			MB_ICONEXCLAMATION | MB_OK
			);
		return 0;
		};
	*c = 0;
	strcat(Path,SYSTEM_PARTIAL);
	//
	if (_chdir(Path))
		{
		MessageBox
			(
			NULL,
			GAME_NAME " can't find " SYSTEM_PARTIAL " directory.  "
			"Please check your configuration, consider reinstalling, and try again.",
			"Could not find " SYSTEM_PARTIAL " directory.",
			MB_ICONEXCLAMATION | MB_OK
			);
		return 0;
		};
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;
	//
	GetStartupInfo(&StartupInfo);
	char lpNewCmdLine[1024];
	strcpy (lpNewCmdLine,"-LAUNCHED ");
	strcat (lpNewCmdLine,lpCmdLine);
	//
	if (!CreateProcess
		(
		ENGINE_PARTIAL, lpNewCmdLine, NULL, NULL,
		FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL,
		&StartupInfo,
		&ProcessInfo
		))
		{
		MessageBox
			(
			NULL,
			GAME_NAME " is not installed properly.  "
			"Please check your configuration, consider reinstalling, and try again.",
			"Failed to launch " GAME_NAME,
			MB_ICONEXCLAMATION | MB_OK
			);
		return 0;
		};
	return 0;
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
