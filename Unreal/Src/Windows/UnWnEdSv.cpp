/*=============================================================================
	UnWnEdSv.cpp: CUnrealEdServer's OLE interface
	Used by: Unreal/Windows interface

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "StdAfx.h"
#include "UnWn.h"
#include "Unreal.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnrealEdServer
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CUnrealEdServer, CWnd)

CUnrealEdServer::CUnrealEdServer()
	{
	EnableAutomation();
	App.UnrealLockApp();
	App.Platform.Log(LOG_Win,"Created CUnrealEdServer");
	};

CUnrealEdServer::~CUnrealEdServer()
	{
	};

void CUnrealEdServer::OnFinalRelease()
	{
	App.Platform.Log(LOG_Win,"OnFinalRelease CUnrealEdServer");
	App.UnrealUnlockApp();
	CWnd::OnFinalRelease();
	};

BEGIN_MESSAGE_MAP(CUnrealEdServer, CWnd)
	//{{AFX_MSG_MAP(CUnrealEdServer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BEGIN_DISPATCH_MAP(CUnrealEdServer, CWnd)
	//{{AFX_DISPATCH_MAP(CUnrealEdServer)
	DISP_FUNCTION(CUnrealEdServer, "Exec", Exec, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CUnrealEdServer, "SlowExec", SlowExec, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CUnrealEdServer, "GetProp", GetProp, VT_BSTR, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CUnrealEdServer, "SetProp", SetProp, VT_EMPTY, VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CUnrealEdServer, "Enable", Enable, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CUnrealEdServer, "Disable", Disable, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CUnrealEdServer, "Init", Init, VT_EMPTY, VTS_I4 VTS_I4)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

//
// Note: we add support for IID_IUnrealEdServer to support typesafe binding
// from VBA.  This IID must match the GUID that is attached to the 
// dispinterface in the .ODL file.
//

// {D0EB88E6-2016-11CF-98C0-0000C06958A7}
static const IID IID_IUnrealEdServer =
{ 0xd0eb88e6, 0x2016, 0x11cf, { 0x98, 0xc0, 0x0, 0x0, 0xc0, 0x69, 0x58, 0xa7 } };

BEGIN_INTERFACE_MAP(CUnrealEdServer, CWnd)
	INTERFACE_PART(CUnrealEdServer, IID_IUnrealEdServer, Dispatch)
END_INTERFACE_MAP()

// {F936C3A7-1FF8-11CF-98C0-0000C06958A7}
IMPLEMENT_OLECREATE(CUnrealEdServer, "Unreal.UnrealEdServer", 0xf936c3a7, 0x1ff8, 0x11cf, 0x98, 0xc0, 0x0, 0x0, 0xc0, 0x69, 0x58, 0xa7)

/////////////////////////////////////////////////////////////////////////////
// CUnrealEdServer OLE handlers
/////////////////////////////////////////////////////////////////////////////

void HandleOleError(char *Module)
	{
	App.InOle=0;
	App.OleCrashed=1;
	//
	App.Platform.ShutdownAfterError();
	//
	strcat(App.Platform.ErrorHist," <- Ole call to ");
	strcat(App.Platform.ErrorHist,Module);
	App.FinalizeErrorText();
	//
	App.Platform.Logf("Exiting due to Ole error");
	App.Platform.CloseLog();
	//
	if (App.Platform.Debugging) DebugBreak();
	else AfxThrowOleDispatchException(1,App.Platform.ErrorHist);
	};

int inline CheckUnrealState(char *Descr)
	{
	if (App.PlatformCrashed)
		{
		char Error[16384];
		strcpy(Error,"Unrecoverable: ");
		strcat(Error,App.Platform.ErrorHist);
		//
		if (App.Platform.Debugging) DebugBreak();
		else AfxThrowOleDispatchException(1,Error);
		return 0;
		}
	else if (!App.Platform.ServerAlive)
		{
		App.Platform.Logf(LOG_Critical,"Ole '%s' after server shutdown",Descr);
		return 0;
		}
	else return 1;
	};

void CUnrealEdServer::Exec(LPCTSTR Cmd) 
	{
	static void *MemTop=GMem.Get(0);
	//
	if (!CheckUnrealState("Exec")) return;
	try	{
		App.InOle=1;
		//
		if (MemTop!=GMem.Get(0)) appError("Memory leak");
		//
		App.Platform.Log(LOG_Cmd,Cmd);
		//
		GUnreal.Exec(Cmd);
		//
		App.InOle=0;
		}
	catch(...)
		{
		char C[256]="Exec (";
		char *S=&C[strlen(C)];
		strncpy(S,Cmd,80);
		S[80]=0;
		strcat(S,")");
		HandleOleError(C);
		};
	};

void CUnrealEdServer::SlowExec(LPCTSTR Cmd) 
	{
	if (!CheckUnrealState("SlowExec")) return;
	try	{
		App.InOle=1;
		HCURSOR SavedCursor = SetCursor(LoadCursor(NULL,IDC_WAIT));
		App.Platform.Log(LOG_Cmd,Cmd);
		//
		GUnreal.Exec(Cmd);
		//
		SetCursor(SavedCursor);
		App.InOle=0;
		}
	catch(...) {HandleOleError("SlowExec");};
	};

BSTR CUnrealEdServer::GetProp(LPCTSTR Topic, LPCTSTR Item) 
	{
	if (!CheckUnrealState("GetProp")) return NULL;
	try	{
		CString	strResult;
		char	szResult[16384];
		App.InOle=1;
		GTopics.Get(NULL,Topic,Item,szResult);
		strResult = szResult;
		App.InOle=0;
		return strResult.AllocSysString();
		}
	catch(...) {HandleOleError("GetProp");};
	return NULL;
	};

void CUnrealEdServer::SetProp(LPCTSTR Topic, LPCTSTR Item, LPCTSTR NewValue)
	{
	if (!CheckUnrealState("SetProp")) return;
	try	{
		App.InOle=1;
		GTopics.Set(NULL,Topic,Item,const_cast<char *>(NewValue));
		App.InOle=0;
		}
	catch(...) {HandleOleError("SetProp");};
	};

void CUnrealEdServer::Init(long hWndMain, long hWndEdCallback)
	{
	if (!CheckUnrealState("Init")) return;
	try	{
		App.InOle=1;
		App.hWndMain       = (HWND)hWndMain;
		App.hWndEdCallback = (HWND)hWndEdCallback;
		App.InOle=0;
		}
	catch(...) {HandleOleError("Init");};
	};

void CUnrealEdServer::Enable() 
	{
	if (!CheckUnrealState("Enable")) return;
	App.Platform.Enable();
	};

void CUnrealEdServer::Disable() 
	{
	if (!CheckUnrealState("Disable")) return;
	App.Platform.Disable();
	};
