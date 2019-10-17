/*=============================================================================
	UnWn.cpp: Unreal startup
	Used by: Log window

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "StdAfx.h"
#include "UnWn.h"
#include "Unreal.h"
#include "Net.h"
#include "UnWnCam.h"
#include "UnConfig.h"
#include "UnWnProp.h" // Properties dialog.

/////////////////////////////////////////////////////////////////////////////
// Globals
/////////////////////////////////////////////////////////////////////////////

CUnrealWnApp			App;
FWindowsCameraManager	CameraManager;
extern FTaskManager		*TaskManager;

extern UNRENDER_API  FGlobalRender  *GRendPtr;
extern UNGAME_API    FVirtualGame   *GGamePtr;
extern UNNETWORK_API NDriver		*GDirectPlayDriver;
extern UNEDITOR_API  FEditor		GUnrealEditor;

FRenderDevice *FindRenderDevice(void);

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnApp
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CUnrealWnApp, CWinApp)
	//{{AFX_MSG_MAP(CUnrealWnApp)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_WINDOW_NEWCAMERA, OnWindowNewCamera)
	ON_COMMAND(ID_HELP_ABOUTUNREAL, OnHelpAboutUnreal)
	ON_COMMAND(ID_HELP_EPICSWEBSITE, OnHelpEpicsWebSite)
	ON_COMMAND(ID_HELP_HELPTOPICS, OnHelpHelpTopics)
	ON_COMMAND(ID_HELP_ORDERINGUNREAL, OnHelpOrderingUnreal)
	ON_COMMAND(ID_HELP_ORDERNOW, OnHelpOrderNow)
	ON_COMMAND(ID_WINDOW_ALWAYSONTOP, OnWindowAlwaysOnTop)
	ON_COMMAND(ID_LOG_CLOSELOG, OnLogCloseLog)
	ON_COMMAND(ID_LOG_OPENUNREALLOG, OnLogOpenUnrealLog)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_FILE_BEGINGAME, OnFileBeginGame)
	ON_COMMAND(ID_FILE_ENDGAME, OnFileEndGame)
	ON_COMMAND(ID_FILE_LOADGAME, OnFileLoadGame)
	ON_COMMAND(ID_FILE_SAVEGAME, OnFileSaveGame)
	ON_COMMAND(ID_NETGAME, OnNetGame)
	ON_COMMAND(ID_PROPERTIES_PROPERTIES, OnPropertiesProperties)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnApp construction
/////////////////////////////////////////////////////////////////////////////

//
// Constructor.  Initializes critical variables.
// Not guarded.
//
CUnrealWnApp::CUnrealWnApp()
	{
	//***********************************************************
	// DANGER: Be wary of placing code in this constructor which
	// may rely on uninitialized data.  You must not call any
	// FGlobalPlatform functions, including the Log functions.
	//***********************************************************
	AlwaysOnTop					= 0;
	InOle						= 0;
	OleCrashed					= 0;
	PlatformCrashed				= 0;
	hWndMain					= NULL;
	hWndEdCallback				= NULL;
	UsageCount					= 0;
	Dialog						= NULL;
	//
	strcpy (Error,"General Protection Fault in Unreal\r\n\r\n"); // Default (undetectable) error
	};

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnApp initialization
/////////////////////////////////////////////////////////////////////////////

//
// Do all significant initialization, bring up the Unreal log (which may
// masquerade as a splash screen), launch the Unreal engine, let gameplay
// continue while the model CUnrealWnDlg dialog is active, then shut down
// and exit the application.
//
// This is like a main() function; all program execution occurs within the
// execution scope of this function.  Execution never reaches the standard
// MFC message pump.
//
// Not guarded.
//
BOOL CUnrealWnApp::InitInstance()
	{
	int Slave = RunEmbedded() || RunAutomated();
	char *Ptr;
	//
	// Figure out what directory we're in and switch into 
	// the Unreal\System directory:
	//
	strcpy(BaseDir,m_pszHelpFilePath);
	strupr(BaseDir);
	//
	Ptr=(char *)mystrstr(BaseDir,MFC_HELP_PARTIAL);
	if (Ptr==NULL)
		{
		AfxMessageBox("Unreal is not installed properly.  Please reinstall.");
		return FALSE;
		};
	*Ptr=0; // Now BaseDir is set.
	SetCurrentDirectory(BaseDir);
	//
	// Parse the command line to see if launched as OLE server:
	//
	strcpy(CmdLine,m_lpCmdLine);
	if (Slave)
		{
		strcat(CmdLine," -EDITOR AUDIO=OFF");
		#ifdef _DEBUG
		strcat(CmdLine," -DEBUG");
		#endif
		};
	//
	// Initialize the global platform information.  Prior to this point of
	// Unreal's execution, do not call any FGlobalPlatform functions, including
	// the log functions.
	//
	Platform.Init(CmdLine,BaseDir); // Set global platform information
	//
	// Set globals:
	//
	GApp					= &Platform;
	Platform.CameraManager  = &CameraManager;
	Platform.RenDev			= FindRenderDevice();
	//
	// Password dialog:
	//
	#if 0
		{
		Platform.GetProfile("Login","Name",Platform.LockName,"",sizeof(Platform.LockName));
		Platform.GetProfile("Login","Password",Platform.LockPassword,"",sizeof(Platform.LockPassword));
		//
		if (!Platform.PasswordDialog
			(
			"Confidential Unreal version","Password check",
			Platform.LockName,Platform.LockPassword)
			)
			return FALSE;
		Platform.SetProfile("Login","Name",    Platform.LockName);
		Platform.SetProfile("Login","Password",Platform.LockPassword);
		};
	#endif
	//
	// Initialize OLE libraries
	//
	if (!AfxOleInit())
		{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
		};
	//
	// Standard initialization:
	//
	Enable3dControls();
	//
	// Handle OLE or regular startup:
	//
	if (Slave)
		{
		//
		// Register all OLE server (factories) as running.  This enables the
		// OLE libraries to create objects from other applications.
		//
		COleTemplateServer::RegisterAll();
		}
	else
		{
		//
		// Register all OLE classes:
		//
		COleObjectFactory::UpdateRegistryAll();
		RegisterFileTypes(BaseDir);
		//
		// Register our type library:
		//
		CString		cTlbFname = (CString)BaseDir + TYPELIB_PARTIAL;
		CString		cHlpFname = (CString)BaseDir + HELP_RELATIVE_FNAME;
		BSTR		bTlbFname = cTlbFname.AllocSysString();
		BSTR		bHlpFname = cHlpFname.AllocSysString();
		ITypeLib	*tlUnreal;
		//
		if (LoadTypeLib(bTlbFname,&tlUnreal) ||
			RegisterTypeLib (tlUnreal,bTlbFname,bHlpFname))
			{
			char Msg[256];
			sprintf
				(
				Msg,
				"Type library %s couldn't be registered.  Please reinstall Unreal.",
				cTlbFname
				);
			AfxMessageBox(Msg);
			return FALSE;
			};
		};
	if (mystrstr(CmdLine,"/REGSERVER")||mystrstr(CmdLine,"-REGSERVER")) return FALSE; // Only register the server
	if (!(mystrstr(CmdLine,"-LOG")||mystrstr(CmdLine,"-EDITOR"))) Platform.LaunchWithoutLog=1;
	//
	// Start up:
	//
	AfxOleLockApp();
	Dialog = new CUnrealWnDlg;
	//
	Timer.Init(40);
	int ExceptionExit=0;
	if (Platform.Debugging)
		{
		AfxMessageBox("Running in debug mode");
		m_pMainWnd		= Dialog;
		//
		Dialog->Create(IDD_UNWN_DIALOG);
		MessagePump();
		//
		Timer.Exit();
		GUnreal.Exit();
		}
	else
		{
		try
			{
			GUARD;
			//
			m_pMainWnd	= Dialog;
			//
			Dialog->Create(IDD_UNWN_DIALOG);
			MessagePump();
			//
			Platform.ServerAlive = 0;
			if (OleCrashed || Platform.InAppError) goto Crashed;
			//
			Timer.Exit();
			GUnreal.Exit();
			//
			UNGUARD("CUnrealWnApp::InitInstance");
			}
		catch(...) // Exception due to appError call or GPF
			{
			Crashed:
			ExceptionExit			= 1;
			PlatformCrashed			= 1;
			Timer.Exit();
			//
			if (OleCrashed || InOle)
				{
				// Ole has already cleaned up after the error
				}
			else
				{
				Platform.Logf("Shutting down Unreal engine");
				Platform.ShutdownAfterError();
				//
				FinalizeErrorText();
				Platform.Logf("Exiting due to error");
				Platform.CloseLog();
				//
				Dialog->SetFocus();
				Dialog->MessageBox(Platform.ErrorHist,"Unrecoverable Unreal Error",MB_OK|MB_ICONEXCLAMATION|MB_APPLMODAL);
				Dialog->ShowWindow(SW_HIDE);
				};
			delete Dialog;
			ExitProcess(1);
			goto Out;
			};
		};
	Platform.CheckAllocations();
	Platform.CloseLog();
	Platform.Exit();
	delete Dialog;
	//
	// Since the dialog has been closed, return FALSE so that we exit the
	// application, rather than start the MFC message pump.
	//
	Out:
	while (!AfxOleCanExitApp()) AfxOleUnlockApp();
	GdiFlush();
	//
	return FALSE;
	};

/////////////////////////////////////////////////////////////////////////////
// Unreal initialization
/////////////////////////////////////////////////////////////////////////////

//
// Launch the Unreal engine.
// Not guarded.
//
void CUnrealWnApp::InitializeUnreal(void)
	{
	GUARD;
	//
	Platform.Startup();
	int IsEditor = mystrstr(CmdLine,"-EDITOR") ? 1 : 0;
	int IsNet    = mystrstr(CmdLine,"-NET")    ? 1 : 0;
	//
	GUnreal.Init
		(
		&Platform,
		TaskManager,
		Platform.CameraManager,
		GRendPtr,
		GGamePtr,
		IsNet    ? &NetManager    : NULL,
		IsEditor ? &GUnrealEditor : NULL
		);
	UNGUARD("CUnrealWnApp::InitializeUnreal");
	};

/////////////////////////////////////////////////////////////////////////////
// Global Ole application locking/unlocking
/////////////////////////////////////////////////////////////////////////////

//
// Lock the app.
// This replaces AfxOleLockApp.
//
void CUnrealWnApp::UnrealLockApp(void)
	{
	GUARD;
	//
	UsageCount++;
	debugf(LOG_Win,"UnrealLockApp %i",UsageCount);
	//
	UNGUARD("CUnrealWnApp::UnrealLockApp");
	};

//
// Unlock the app and release it if the lock count is zero.
// This replaces AfxOleUnlockApp.
//
void CUnrealWnApp::UnrealUnlockApp(void)
	{
	GUARD;
	//
	debugf(LOG_Win,"UnrealUnlockApp %i",UsageCount);
	if (--UsageCount <= 0) App.Dialog->Exit();
	//
	UNGUARD("CUnrealWnApp::UnrealUnlockApp");
	};

/////////////////////////////////////////////////////////////////////////////
// Error
/////////////////////////////////////////////////////////////////////////////

//
// Add the final bit of text to an error message.
// Not guarded.
//
void CUnrealWnApp::FinalizeErrorText(void)
	{
	strcat(App.Platform.ErrorHist, ".  " GAME_NAME " " GAME_VERSION
		" was compiled on " __DATE__ ".");
	strcat(App.Platform.ErrorHist,"\r\n\r\nTo report this error, send the "
		" " LOG_PARTIAL " file to Epic along with a description of "
		" what you were doing.");
	};

/////////////////////////////////////////////////////////////////////////////
// Registering file types
/////////////////////////////////////////////////////////////////////////////

// Not guarded.
void RegDoSet(HKEY Key,CString Value)
	{
	RegSetValue(Key,"",REG_SZ,LPCSTR(Value),Value.GetLength());
	};

// Not guarded.
void RegSetEx(HKEY Key,CString SubKey,CString Value)
	{
	RegSetValueEx(Key,SubKey,0,REG_SZ,(BYTE *)LPCSTR(Value),Value.GetLength());
	};

// Not guarded.
void CUnrealWnApp::RegisterFileTypes(char *BaseDir)
	{
	CString Base = BaseDir;
	HKEY Key1,Key2,Key3,Key4;
	FILE *Temp;
	//
	// Register map file type:
	//
	RegCreateKey				(HKEY_CLASSES_ROOT,MAP_EXTENSION,&Key1);
		RegDoSet				(Key1,OLE_MAP_TYPE);
	RegCloseKey					(Key1);
	//
	RegCreateKey				(HKEY_CLASSES_ROOT,OLE_APP,&Key1);
		RegDoSet				(Key1,OLE_APP_DESCRIPTION);
	RegCloseKey					(Key1);
	//
	// Register application/unreal MIME type
	//
	/*
	RegCreateKey				(HKEY_CLASSES_ROOT,"MIME",&Key1);
		RegCreateKey			(Key1,"Database",&Key2);
			RegCreateKey		(Key2,"Content Type",&Key3);
				RegCreateKey	(Key3,MIME_MAP_TYPE,&Key4);
					RegSetEx	(Key4,"Extension",MAP_EXTENSION);
				RegCloseKey		(Key4);
			RegCloseKey			(Key3);
		RegCloseKey				(Key2);
	RegCloseKey					(Key1);
	*/
	//
	// Register UnServer.exe (and optionally UnrealEd.exe) shell launch info:
	//
	RegCreateKey				(HKEY_CLASSES_ROOT,OLE_MAP_TYPE,&Key1);
		RegDoSet				(Key1,OLE_MAP_DESCRIPTION);
		RegCreateKey			(Key1,"DefaultIcon",&Key2);
			RegDoSet			(Key2,Base + ENGINE_PARTIAL + ",0");
		RegCloseKey				(Key2);
		/*
		RegCreateKey			(Key1,"ContentType",&Key2);
			RegDoSet			(Key2,MIME_MAP_TYPE);
		RegCloseKey				(Key2);
		*/
		RegCreateKey			(Key1,"shell",&Key2);
			RegDoSet			(Key2,"open");
			RegCreateKey		(Key2,"open",&Key3);
				RegDoSet		(Key3,PLAY_COMMAND);
				RegCreateKey	(Key3,"command",&Key4);
					RegDoSet	(Key4,Base + ENGINE_PARTIAL + " FILE=\"%1\"");
				RegCloseKey		(Key4);
			RegCloseKey			(Key3);
			//
			Temp=fopen(EDITOR_FNAME,"rb");
			if (Temp)
				{
				fclose(Temp);
				RegCreateKey	(Key2,"edit",&Key3);
					RegDoSet	(Key3,EDIT_COMMAND);
					RegCreateKey(Key3,"command",&Key4);
						RegDoSet(Key4,Base + EDITOR_PARTIAL + " FILE=\"%1\"");
					RegCloseKey	(Key4);
				RegCloseKey		(Key3);
				};
		RegCloseKey				(Key2);
	RegCloseKey					(Key1);
	//
	// Register with Internet Explorer:
	//
	RegCreateKey				(HKEY_CLASSES_ROOT,"Unreal",&Key1);
		RegDoSet				(Key1,"URL:Unreal Protocol");
		RegCreateKey			(Key1,"DefaultIcon",&Key2);
			RegDoSet			(Key2,Base + ENGINE_PARTIAL);
		RegCloseKey				(Key2);
		RegSetEx				(Key1,"URL Protocol","");
		RegCreateKey			(Key1,"shell",&Key2);
			RegDoSet			(Key2,"open");
			RegCreateKey		(Key2,"open",&Key3);
				RegDoSet		(Key3,PLAY_COMMAND);
				RegCreateKey	(Key3,"command",&Key4);
					RegDoSet	(Key4,Base + ENGINE_PARTIAL + " URL=\"%1\"");
				RegCloseKey		(Key4);
			RegCloseKey			(Key3);
		RegCloseKey				(Key2);
	RegCloseKey					(Key1);
	//
	// Register with Netscape:
	//
	/*
	RegCreateKey				(HKEY_CURRENT_USER,"Software",&Key1);
		RegCreateKey			(Key1,"Netscape",&Key2);
			RegCreateKey		(Key2,"Netscape Navigator",&Key3);
				RegCreateKey	(Key3,"Suffixes",&Key4);
					RegSetEx	(Key4,"application/x-unreal","unr,unreal");
				RegCloseKey		(Key4);
				RegCreateKey	(Key3,"Viewers",&Key4);
					RegSetEx	(Key4,"application/x-unreal",Base + ENGINE_PARTIAL);
					RegSetEx	(Key4,"application/unreal",  Base + ENGINE_PARTIAL);
					RegSetEx	(Key4,"TYPE0",  "application/x-unreal");
				RegCloseKey		(Key4);
			RegCloseKey			(Key3);
		RegCloseKey				(Key2);
	RegCloseKey					(Key1);
	*/
	};

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnApp 'File' menu command handlers
/////////////////////////////////////////////////////////////////////////////

void CUnrealWnApp::OnFileExit() 
	{
	GUARD;
	Dialog->Exit();
	UNGUARD("CUnrealWnApp::OnFileExit");
	};

void CUnrealWnApp::OnFileBeginGame() 
	{
	GUARD;
	AfxMessageBox("Begin game: Unimplemented");
	UNGUARD("CUnrealWnApp::OnFileBeginGame");
	};

void CUnrealWnApp::OnFileEndGame() 
	{
	GUARD;
	AfxMessageBox("End game: Unimplemented");
	UNGUARD("CUnrealWnApp::OnFileEndGame");
	};

void CUnrealWnApp::OnFileLoadGame() 
	{
	GUARD;
	//
	SetCurrentDirectory((CString)BaseDir + MAP_RELATIVE_PATH);
	CFileDialog LoadDialog
		(
		TRUE,MAP_EXTENSION,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LOAD_MAP_MASK,
		NULL
		);
	if (LoadDialog.DoModal()==IDOK)
		{
		// Load it
		};
	SetCurrentDirectory(BaseDir);
	//
	UNGUARD("CUnrealWnApp::OnFileLoadGame");
	};

void CUnrealWnApp::OnFileSaveGame() 
	{
	GUARD;
	//
	SetCurrentDirectory((CString)BaseDir + MAP_RELATIVE_PATH);
	CFileDialog SaveDialog
		(
		FALSE,MAP_EXTENSION,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		SAVE_MAP_MASK,
		NULL
		);
	if (SaveDialog.DoModal()==IDOK)
		{
		// Save it
		};
	SetCurrentDirectory(BaseDir);
	//
	UNGUARD("CUnrealWnApp::OnFileSaveGame");
	};

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnApp 'NetGame' menu command handlers
/////////////////////////////////////////////////////////////////////////////

void CUnrealWnApp::OnNetGame() 
	{
	GUARD;
	//
	char URL[256];
	//
	ENetworkPlayMode ResultMode;
	NSocket          *ClientSocket;
	//
	ResultMode = GNetManager->BeginGameByUI(URL,&ClientSocket);
	switch (ResultMode)
		{
		case PM_NONE:
			break;
		case PM_LOCAL:
			break;
		case PM_CLIENT:
			break;
		case PM_CLIENT_SERVER:
			break;
		case PM_DEDICATED_SERVER:
			break;
		default:
			GApp->Error("Illegal network play mode");
			break;
		};
	UNGUARD("CUnrealWnApp::OnNetGame");
	};

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnApp 'Window' menu command handlers
/////////////////////////////////////////////////////////////////////////////

void CUnrealWnApp::OnWindowNewCamera() 
	{
	GUARD;
	GUnreal.OpenCamera();
	UNGUARD("CUnrealWnApp::OnWindowNewCamera");
	};

void CUnrealWnApp::OnWindowAlwaysOnTop() 
	{
	GUARD;
	//
	AlwaysOnTop = !AlwaysOnTop;
	if (AlwaysOnTop)	Dialog->SetWindowPos(&Dialog->wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	else				Dialog->SetWindowPos(&Dialog->wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	UpdateUI();
	//
	UNGUARD("CUnrealWnApp::OnWindowAlwaysOnTop");
	};

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnApp 'Edit' menu command handlers
/////////////////////////////////////////////////////////////////////////////

void CUnrealWnApp::OnEditCopy() 
	{
	GUARD;
	if (Dialog->FocusEditControl) Dialog->FocusEditControl->Copy();
	UNGUARD("CUnrealWnApp::OnEditCopy");
	};

void CUnrealWnApp::OnEditPaste()
	{
	GUARD;
	if (Dialog->FocusEditControl) Dialog->FocusEditControl->Paste();
	UNGUARD("CUnrealWnApp::OnEditPaste");
	};

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnApp 'Help' menu command handlers
/////////////////////////////////////////////////////////////////////////////

void CUnrealWnApp::OnHelpAboutUnreal() 
	{
	GUARD;
	::WinHelp (Dialog->m_hWnd,HELP_LINK_FNAME,1,200);;
	UNGUARD("CUnrealWnApp::OnHelpAboutUnreal");
	};

void CUnrealWnApp::OnHelpEpicsWebSite() 
	{
	GUARD;
	GApp->LaunchURL(URL_WEB,"");
	UNGUARD("CUnrealWnApp::OnHelpEpicsWebSite");
	};

void CUnrealWnApp::OnHelpHelpTopics() 
	{
	GUARD;
	::WinHelp (Dialog->m_hWnd,HELP_LINK_FNAME,11,0);
	UNGUARD("CUnrealWnApp::OnHelpHelpTopics");
	};

void CUnrealWnApp::OnHelpOrderingUnreal()
	{
	GUARD;
	::WinHelp (Dialog->m_hWnd,HELP_LINK_FNAME,1,202);	
	UNGUARD("CUnrealWnApp::OnHelpOrderingUnreal");
	};

void CUnrealWnApp::OnHelpOrderNow()
	{
	GUARD;
	// TODO: Launch order program
	UNGUARD("CUnrealWnApp::OnHelpOrderNow");
	};

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnApp 'Log' menu command handlers
/////////////////////////////////////////////////////////////////////////////

void CUnrealWnApp::OnLogCloseLog()
	{
	GUARD;
	//
	Platform.CloseLog();
	UpdateUI();
	//
	UNGUARD("CUnrealWnApp::OnLogCloseLog");
	};

void CUnrealWnApp::OnLogOpenUnrealLog()
	{
	GUARD;
	//
	Platform.OpenLog(LOG_PARTIAL);
	UpdateUI();
	//
	UNGUARD("CUnrealWnApp::OnLogOpenUnrealLog");
	};

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnApp user interface updater
/////////////////////////////////////////////////////////////////////////////

void CUnrealWnApp::UpdateUI(void)
	{
	GUARD;
	//
	// Log:
	//
	Dialog->GetMenu()->EnableMenuItem(ID_LOG_OPENUNREALLOG,MF_BYCOMMAND | ((Platform.LogFile==NULL)?MF_ENABLED:MF_GRAYED));
	Dialog->GetMenu()->EnableMenuItem(ID_LOG_CLOSELOG,     MF_BYCOMMAND | ((Platform.LogFile!=NULL)?MF_ENABLED:MF_GRAYED));
	//
	// Always on top:
	//
	Dialog->GetMenu()->CheckMenuItem(ID_WINDOW_ALWAYSONTOP,MF_BYCOMMAND | (AlwaysOnTop?MF_CHECKED:MF_UNCHECKED));
	//
	UNGUARD("CUnrealWnApp::UpdateUI");
	};

/////////////////////////////////////////////////////////////////////////////
// Message routing
/////////////////////////////////////////////////////////////////////////////

//
// Route a message through Windows.  Necessary in order for
// MFC accelerators to work.
//
void CUnrealWnApp::RouteMessage(void *Msg)
	{
	GUARD;
	if (!App.PreTranslateMessage((MSG *)Msg))
		{
		TranslateMessage((MSG *)Msg);
		DispatchMessage ((MSG *)Msg);
		};
	UNGUARD("CUnrealWnApp::RouteMessage");
	};

/////////////////////////////////////////////////////////////////////////////
// String translation
/////////////////////////////////////////////////////////////////////////////

//
// Lookup a string from the string table.  Expected to be called through
// the US(id) macro.  Strings must be 255 characters or less.  The 
// ACTIVE_STRINGS most recently returned string pointers are valid.
//
#if 0
#define ACTIVE_STRINGS 16
#define US(tag) (GTranslate(#tag,ghModule))
char *Translate(DWORD ID,DWORD hModule)
	{
	GUARD;
	//
	static char Buffer[16][256];
	static int	n					= 0;
	static		HINSTANCE hModule	= AfxGetInstanceHandle();
	//
	if (!Tag)					appError("Null string");
	if (++n >= ACTIVE_STRINGS)	n = 0;
	//
	char *Result	= Buffer[n];
	HRSRC hR		= FindResourceEx(hModule,RT_STRING,Tag,MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
	//EnumResourceLanguages
	//
	if		(!hR) sprintf(Result, "<%s NOT FOUND>",Tag);
	else if (!LoadString(hModule,hR,Result,255)) sprintf(Result,"<%s LOAD FAILED>",Tag);
	//
	return Result;
	//
	UNGUARD("Translate");
	};
#endif

/////////////////////////////////////////////////////////////////////////////
//           Menu "Properties" handlers...
/////////////////////////////////////////////////////////////////////////////

void CUnrealWnApp::OnPropertiesProperties() 
{
    CDialog * ExistingDialog = CDialogProperties::LatestInstance;
    if( ExistingDialog != 0 )
    {
        // The properties dialog already exists, just show it.
        ExistingDialog->SetActiveWindow();
    }
    else
    {
        // Create a new properties dialog and display it.
        CDialog * Dialog = new CDialogProperties();
        Dialog->Create(CDialogProperties::IDD);
        Dialog->ShowWindow(SW_SHOW);
    }
}


/////////////////////////////////////////////////////////////////////////////
// The End
/////////////////////////////////////////////////////////////////////////////
