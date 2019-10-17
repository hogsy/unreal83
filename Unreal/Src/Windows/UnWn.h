/*=============================================================================
	UnWn.h: Main header for UnWn application
	Used by: Log window

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef __AFXWIN_H__
	#error "include 'StdAfx.h' before including this file for PCH"
#endif

#include "Resource.h"		// main symbols

#include "UnWnDlg.h"
#include "UnWnEdSv.h"

extern class CUnrealWnApp App;

/////////////////////////////////////////////////////////////////////////////
// CTimer
/////////////////////////////////////////////////////////////////////////////

class CUnrealTimer
	{
	public:
	//
	MMRESULT	TimerHandle;
	MMRESULT	TimerPeriod;
	DWORD		TimerRate;
	DWORD		TimerTicks;
	DWORD		TimerBug;
	DWORD		TimerAlive;
	//
	// Functions:
	//
	CTimer(void);
	void Enable(void);
	void Disable(void);
	void Init(DWORD TimerRate);
	void Exit(void);
	};

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnApp:
// See UnWn.cpp for the implementation of this class
/////////////////////////////////////////////////////////////////////////////

class CUnrealWnApp : public CWinApp
	{
	public:
	CUnrealWnApp();
	//
	// Custom:
	//
	DWORD AlwaysOnTop;
	CUnrealWnDlg *Dialog;
	HWND hWndMain;
	HWND hWndEdCallback;
	void UpdateUI(void);
	void RegisterFileTypes(char *BaseDir);
	char CmdLine[256];
	char Error[1024];
	char BaseDir[256];
	int InOle,OleCrashed,UsageCount,InError,PlatformCrashed;
	//
	FGlobalPlatform Platform;
	CUnrealTimer Timer;
	//
	void RouteMessage(void *Msg);
	void MessagePump(void);
	void InitializeUnreal(void);
	void UnrealLockApp(void);
	void UnrealUnlockApp(void);
	void FinalizeErrorText(void);
	//
	// Overrides
	//
	//{{AFX_VIRTUAL(CUnrealWnApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL
	//
	// Implementation
	//
	//{{AFX_MSG(CUnrealWnApp)
	afx_msg void OnFileExit();
	afx_msg void OnWindowNewCamera();
	afx_msg void OnHelpAboutUnreal();
	afx_msg void OnHelpEpicsWebSite();
	afx_msg void OnHelpHelpTopics();
	afx_msg void OnHelpOrderingUnreal();
	afx_msg void OnHelpOrderNow();
	afx_msg void OnWindowAlwaysOnTop();
	afx_msg void OnLogCloseLog();
	afx_msg void OnLogOpenUnrealLog();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnFileBeginGame();
	afx_msg void OnFileEndGame();
	afx_msg void OnFileLoadGame();
	afx_msg void OnFileSaveGame();
	afx_msg void OnNetGame();
	afx_msg void OnPropertiesProperties();
	//}}AFX_MSG
	//
	DECLARE_MESSAGE_MAP()
protected:
	};

/////////////////////////////////////////////////////////////////////////////
