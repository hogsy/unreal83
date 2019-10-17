/*=============================================================================
	UnWnTime.cpp: Timer-related functions
	Used by: Unreal/Windows interface

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "StdAfx.h"
#include "UnWn.h"
#include "Unreal.h"

extern CUnrealWnApp theApp;

#ifdef _DEBUG
	#define new DEBUG_NEW
#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#define ENABLE_TIMER /* Undefine this to disable timer support */

void CALLBACK TimerCallback (UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

/////////////////////////////////////////////////////////////////////////////
// Timer Callback: A non-class function
/////////////////////////////////////////////////////////////////////////////

void CALLBACK TimerCallback (UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
	{
	CUnrealTimer *ThisTimer = (CUnrealTimer *)dwUser;
	//
	if (!App.Platform.ServerAlive) return;
	if (uID != ThisTimer->TimerHandle) ThisTimer->TimerBug=1;
	//
	ThisTimer->TimerTicks++;
	//
	PostMessage(App.Dialog->m_hWnd,WM_UNREALTIMER,0,(LPARAM)App.Platform.TimeUSec());
	};

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////

CUnrealTimer::CUnrealTimer(void)
	{
	TimerHandle	= 0;
	TimerPeriod	= 0;
	TimerRate	= 1000;
	TimerTicks	= 0;
	TimerBug	= 0;
	TimerAlive	= 0;
	};

/////////////////////////////////////////////////////////////////////////////
// Enable and disable
/////////////////////////////////////////////////////////////////////////////

void CUnrealTimer::Enable (void)
	{
	if (!TimerAlive)
		{
		return;
		}
	else if (TimerHandle!=0)
		{
		App.Platform.Log (LOG_Win,"CUnrealTimer::Enable: Already enabled");
		}
	else
		{
		#ifdef ENABLE_TIMER
			TimerHandle = timeSetEvent(1000/TimerRate,250/TimerRate, TimerCallback,(DWORD)this,TIME_PERIODIC);
			if (TimerHandle==0) App.Platform.Error ("CUnrealTimer::Enable: Timer failed");
		#endif
		};
	};

void CUnrealTimer::Disable (void)
	{
	if (!TimerAlive)
		{
		return;
		}
	else if (TimerHandle==0)
		{
		App.Platform.Log(LOG_Win,"CUnrealTimer::Disable: Timer already disabled");
		}
	else
		{
		#ifdef ENABLE_TIMER
			timeKillEvent (TimerHandle);
		#endif
		TimerHandle=0;
		};
	};

/////////////////////////////////////////////////////////////////////////////
// Init and exit
/////////////////////////////////////////////////////////////////////////////

void CUnrealTimer::Init (DWORD NewTimerRate)
	{
	TIMECAPS Caps;
	//
	TimerTicks  = 0;
	TimerRate   = NewTimerRate;
	//
	if (timeGetDevCaps(&Caps,sizeof(Caps))!=TIMERR_NOERROR) App.Platform.Error("CUnrealTimer::Enable: Timer is unavailable");
	//
	App.Platform.Logf (LOG_Init,"Timer initialized, %i fps",TimerRate);
	//
	TimerPeriod = timeBeginPeriod(250/TimerRate);
	if (TimerPeriod!=TIMERR_NOERROR)App.Platform.Error ("CUnrealTimer::Enable: Timer is not working");
	//
	TimerAlive = 1;
	Enable();
	};

void CUnrealTimer::Exit (void)
	{
	if (TimerAlive)
		{
		Disable();
		if (timeEndPeriod(250/TimerRate)!=TIMERR_NOERROR)
			{
			App.Platform.Log(LOG_Critical,"Error in timeEndPeriod");
			};
		App.Platform.Log(LOG_Exit,"Timer shut down");
		};
	TimerAlive = 0;
	};

/////////////////////////////////////////////////////////////////////////////
// The End
/////////////////////////////////////////////////////////////////////////////
