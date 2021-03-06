/*=============================================================================
	stdafx.h: Include file for all commonly used but infrequently
	modified headers.

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#define _WIN32_WINNT     0x05010000

/////////////////////////////////////////////////////////////////////////////
// ANSI C components
/////////////////////////////////////////////////////////////////////////////
#include <math.h>

/////////////////////////////////////////////////////////////////////////////
// MFC components
/////////////////////////////////////////////////////////////////////////////
#include <afxwin.h>		// MFC core and standard components
#include <afxext.h>		// MFC extensions
#include <afxdisp.h>	// MFC OLE automation classes
#include <afxtempl.h>	// MFC template classes
#include <afxcmn.h>		// MFC support for Windows 95 Common Controls

/////////////////////////////////////////////////////////////////////////////
// Windows API components
/////////////////////////////////////////////////////////////////////////////
#include <mmsystem.h>

/////////////////////////////////////////////////////////////////////////////
// Unreal components
/////////////////////////////////////////////////////////////////////////////
#include "UnPort.h"			// Unreal porting defines
#include "UnPlatfm.h"		// Unreal platform-specific exchange stuff

/////////////////////////////////////////////////////////////////////////////
// Custom
/////////////////////////////////////////////////////////////////////////////

//
// Standard Unreal windows messages
//
enum ECustomUnrealWindowsMessages
	{
	WM_UNREALTIMER   = WM_USER+0x201,
	WM_RAWSYSCOMMAND = WM_USER+0x202,
	};

/////////////////////////////////////////////////////////////////////////////
// The End
/////////////////////////////////////////////////////////////////////////////

