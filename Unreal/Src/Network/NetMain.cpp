/*=============================================================================
	NetMain.cpp: Main networking code

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include <windows.h>
#include "DPlay.h"
#include "Resource.h"

#include "Net.h"
#include "NetPrv.h"

#include "NetINet.h"
#include "NetDPlay.h"

/*-----------------------------------------------------------------------------
	Globals
-----------------------------------------------------------------------------*/

UNNETWORK_API NManager NetManager; // The one global network manager object

BOOL CALLBACK MainWizardDialogProc			(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK NetWizDialogProc				(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK JoinTypeDialogProc			(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK HostTypeDialogProc			(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK DirectPlayProviderDialogProc	(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK JoinInternetDialogProc		(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

/*-----------------------------------------------------------------------------
	NPacket implementation
-----------------------------------------------------------------------------*/

//
// Initialize a packet.
//
void NPacket::Init(NSocket *SocketToSendThrough)
	{
	GUARD;
	//
	Size	= 0;
	Crc		= 0;
	MaxSize	= SocketToSendThrough->MaxPacketSize();
	DestSocketForSanityCheck = SocketToSendThrough;
	//
	UNGUARD("NPacket::Init");
	};

/*-----------------------------------------------------------------------------
	NSocket default implementation
-----------------------------------------------------------------------------*/

//
// Standard socket init code, called by all derived socket
// class Init() routines before they perform their own specialized
// initialization.  Initializes all standard socket information to defaults.
//
void NSocket::Init(NDriver *CreatingDriver)
	{
	GUARD;
	//
	// Set socket to "negotiating" to start with:
	//
	Action = NS_INITIALIZING;
	Driver = CreatingDriver;
	//
	// Init all file transfer and statistics info:
	//
	/*...*/
	//
	// Set action to negotiating:
	//
	SetAction(NS_NEGOTIATING);
	//
	UNGUARD("NSocket::Init");
	};

//
// Standard socket exit code, called by all derived socket class Delete() 
// routines after they have performed their own specialized exit code.  
// Frees any generic socket-related stuff that has been allocated.
//
void NSocket::Delete(void)
	{
	GUARD;
	//
	// Set socket to "disconnected":
	//
	SetAction(NS_DISCONNECTED);
	//
	// Free all file transfer info and other info:
	//
	Driver->RemoveSocketFromList(this);
	delete this;
	//
	UNGUARD("NSocket::Exit");
	};

//
// Standard socket tick code, called by all derived socked class Tick()
// routines after they have performed their own custom tick code, which
// may affect the socket's Action.  This routine exists in order to update
// the socket statistics, perform any background file transfer, or other
// actions that must be done.
//
void NSocket::Tick(void)
	{
	GUARD;
	//
	//
	UNGUARD("NSocket::Tick");
	};

//
// Return the action of the specified socket.
// Performs a validity check.
//
ESocketAction NSocket::GetAction(void)
	{
	GUARD;
	switch (Action)
		{
		case NS_DISCONNECTED:	return Action;
		case NS_NEGOTIATING:	return Action;
		case NS_PLAY:			return Action;
		case NS_FILE_SEND:		return Action;
		case NS_FILE_RECEIVE:	return Action;
		};
	appError ("Invalid current socket action"); return Action;
	UNGUARD("NSocket::GetAction");
	};

//
// Set the action of a socket.
// Performs a validity check and causes a critical error if invalid.
//
void NSocket::SetAction(ESocketAction NewAction)
	{
	GUARD;
	int Ok;
	switch (Action)
		{
		case NS_DISCONNECTED:
			Ok= (NewAction==NS_DISCONNECTED);
			break;
		case NS_NEGOTIATING:
			Ok=	(NewAction==NS_DISCONNECTED) ||
				(NewAction==NS_NEGOTIATING ) ||
				(NewAction==NS_PLAY        ) ||
				(NewAction==NS_FILE_SEND   ) ||
				(NewAction==NS_FILE_RECEIVE);
			break;
		case NS_PLAY:
			Ok=	(NewAction==NS_DISCONNECTED) ||
				(NewAction==NS_NEGOTIATING );
			break;
		case NS_FILE_SEND:
			Ok=	(NewAction==NS_DISCONNECTED) ||
				(NewAction==NS_NEGOTIATING );
			break;
		case NS_FILE_RECEIVE:
			Ok=	(NewAction==NS_DISCONNECTED) ||
				(NewAction==NS_NEGOTIATING );
			break;
		case NS_INITIALIZING:
			Ok= (NewAction==NS_DISCONNECTED) ||
				(NewAction==NS_NEGOTIATING);
		default:
			Ok=0;
		};
	if (!Ok) appErrorf("Invalid action transition from %i to %i",Action,NewAction);
	Action = NewAction;
	UNGUARD("NSocket::SetAction");
	};

//
// Make sure that the specified packet is OK prior to sending
// it out on this socket.
//
void NSocket::AssertPacketValidBeforeSend(NPacket *Packet)
	{
	GUARD;
	//
	if (Packet->Size > Packet->MaxSize) appErrorf("Packet size %i overflowed %i maximum",Packet->Size,Packet->MaxSize);
	if (Packet->DestSocketForSanityCheck != this) appError("Packet init and send sockets differ");
	//
	UNGUARD("NSocket::AssertPacketValidBeforeSend");
	};

//
// Assure that this socket is in a valid state.
//
void NSocket::AssertValid(void)
	{
	GUARD;
	GetAction(); // Validates the current action
	UNGUARD("NSocket::AssertValid");
	};

/*-----------------------------------------------------------------------------
	NServerAd implementation
-----------------------------------------------------------------------------*/

//
// Assign a level to an assumed-empty advertisement:
//
void NServerAd::GenericAssign(char *LevelName)
	{
	GUARD;
	//
	if (Name[0]) appError("Server ad is already in use");
	if (strlen(LevelName) > NAME_SIZE) appErrorf("Level name %s too large",LevelName);
	strcpy(Name,LevelName);
	//
	UNGUARD("NServerAd::Empty");
	};

/*-----------------------------------------------------------------------------
	NManager public implementation
-----------------------------------------------------------------------------*/

//
// Start advertising a level, and return a unique ID associated with that
// level.  The caller uses this ID to uniquely identify itself.
// Can not fail.
//
int NManager::BeginAdvertisingLevel(char *LevelName)
	{
	GUARD;
	AssertInitialized();
	//
	int	ServerID = FindAvailableAd(LevelName);
	Ads[ServerID] = new NServerAd;
	//
	for (int i=0; i<MAX_DRIVERS; i++)
		{
		if (Drivers[i]) Drivers[i]->BeginAdvertising(Ads[ServerID]);
		};
	return ServerID;
	//
	UNGUARD("NManager::BeginAdvertisingLevel");
	};

//
// Stop advertising a level.
//
void NManager::EndAdvertisingLevel(int ServerID)
	{
	GUARD;
	AssertInitialized();
	//
	for (int i=0; i<MAX_DRIVERS; i++)
		{
		if (Drivers[i]) Drivers[i]->EndAdvertising(Ads[ServerID]);
		};
	DeleteAd(ServerID);
	//
	UNGUARD("NManager::EndAdvertisingLevel");
	};

//
// See if any connections are waiting for a particular level.  If so,
// creates a new socket in the state NS_NEGOTIATING and returns it.
// If no connections are waiting, returns NULL.
//
NSocket *NManager::ServerAcceptConnection(int ServerID)
	{
	GUARD;
	AssertInitialized();
	//
	for (int i=0; i<MAX_DRIVERS; i++)
		{
		if (Drivers[i])
			{
			NSocket *Result = Drivers[i]->ServerAcceptConnection(ServerID);
			if (Result) return Result;
			};
		};
	return NULL;
	//
	UNGUARD("NManager::ServerAcceptConnection");
	};

//
// Try to open a client connection to a specified server.  If successful,
// returns a new socket connecting the client to the server.
// If fails, returns NULL.  If the specified server isn't local,
// connecting may take time, so a dialog box must appear with a "Cancel"
// option.
//
NSocket *NManager::ClientOpenServer(char *ServerURL, char *ErrorMessage)
	{
	GUARD;
	AssertInitialized();
	//
	for (int i=0; i<MAX_DRIVERS; i++)
		{
		if (Drivers[i])
			{
			NSocket *Result = Drivers[i]->ClientOpenServer(ServerURL,ErrorMessage);
			if (Result) return Result;
			};
		};
	return NULL;
	//
	UNGUARD("NManager::ClientOpenServer");
	};

//
// Try to open a client connection to a server, which the user may
// enter via the user interface.  If successful, returns a client socket
// and sets ResultURL to the URL which the user navigated to, or a blank
// string if the connection doesn't correspond to a representable URL.
// If fails, returns NULL.  If the specified server isn't local,
// connecting may take time, so a dialog box must appear with a "Cancel"
// option.
//
// Assumes: NetManager is not initialized.
//
ENetworkPlayMode NManager::BeginGameByUI(char *ResultURL,NSocket **ClientSocket)
	{
	GUARD;
	//
	debugf(LOG_Net,"BeginGameByUI begin");
	//
	if (DialogBox
		(
		(HINSTANCE)hInstance,
		MAKEINTRESOURCE(IDD_FAKEWIZARD),
		NULL,
		(DLGPROC)MainWizardDialogProc
		))
		{
		*ClientSocket = ResultClientSocket;
		debugf(LOG_Net,"BeginGameByUI not implemented");
		if (ResultPlayMode==PM_NONE) appError("Inconsistency");
		return ResultPlayMode;
		}
	else
		{
		debugf(LOG_Net,"BeginGameByUI canceled");
		return PM_NONE;
		};
	UNGUARD("NManager::BeginGameByUI");
	};

//
// Register a new network driver with the network manager.
// Initializes the driver.  If successful, returns 1.  If fails,
// returns 0 and sets ErrorMessage to a description of the problem.
//
// Assumes that the driver has been initialized and is ready
// to begin functioning.
//
int NManager::RegisterDriver(NDriver *Driver)
	{
	GUARD;
	AssertInitialized();
	//
	for (int i=0; i<MAX_DRIVERS; i++)
		{
		if (!Drivers[i])
			{
			// Successfully initialized driver:
			Driver->AssertInitialized();
			Drivers[i] = Driver;
			return 1;
			};
		};
	appError ("Too many network drivers registered");
	return 0;
	UNGUARD("NManager::RegisterDriver");
	};

//
// Unregister a network driver:
//
void NManager::UnregisterDriver(NDriver *Driver)
	{
	for (int i=0; i<MAX_DRIVERS; i++)
		{
		if (Drivers[i]==Driver) Drivers[i]=NULL;
		};
	};

/*-----------------------------------------------------------------------------
	NManager private implementation
-----------------------------------------------------------------------------*/

//
// Initialize the generic internals of the network manager:
//
void NManager::Init(void)
	{
	GUARD;
	//
	// Init net manager:
	//
	int i;
	for (    i=0; i<MAX_ACTIVE_LEVELS; i++) Ads    [i]=NULL;
	for (    i=0; i<MAX_DRIVERS;       i++) Drivers[i]=NULL;
	//
	Initialized=1;
	Ticks=0;
	//
	GTaskManager->AddTask(this,NULL,GApp,PRIORITY_Realtime,TASK_NoUserKill);
	//
	// Init drivers:
	//
	char Error[256];
	//
	InternetDriver = new(NInternetDriver);
	if (!InternetDriver->Init("",Error)) InternetDriver = NULL;
	//
	DirectPlayDriver = new(NDirectPlayDriver);
	if (!DirectPlayDriver->Init("",Error)) DirectPlayDriver = NULL;
	//
	// Success:
	//
	debugf(LOG_Init,"Networking initialized");
	//
	UNGUARD("NManager::Init");
	};

void NManager::Exit(void)
	{
	GUARD;
	AssertInitialized();
	//
	GTaskManager->KillTask(this);
	//
	// Shut down all drivers:
	//
	for (int i=0; i<MAX_DRIVERS; i++)
		{
		if (Drivers[i])
			{
			//
			// Retract all ads from the driver:
			//
			for (int j=0; j<MAX_ACTIVE_LEVELS; j++) Drivers[i]->EndAdvertising(Ads[i]);
			//
			// Shut down the driver:
			//
			Drivers[i]->Exit();
			Drivers[i]=NULL;
			};
		};
	Initialized=0;
	//
	debugf(LOG_Exit,"Networking shut down");
	//
	UNGUARD("NManager::Exit");
	};

//
// Create an advertisement for a specified level. Returns the
// index, or 0 if none are available.
// Performs strict checking.
//
int NManager::FindAvailableAd(char *LevelName)
	{
	GUARD;
	AssertInitialized();
	//
	AssertInitialized();
	if ((!LevelName) || (!*LevelName)) appError ("Null level name");
	//
	int Available=0;
	int i;
	for (i=1; i<MAX_ACTIVE_LEVELS; i++)
		{
		if (!Ads[i])
			{
			Available = i;
			}
		else
			{
			if (stricmp(Ads[i]->Name,LevelName)) appErrorf("Duplicate level advertisement for %s",LevelName);
			};
		};
	if (!Available) appError("Level advertisement limit exceeded");
	//
	return i;
	//
	UNGUARD("NManager::GenericFindAvailableAd");
	};

//
// Delete an advertisement for a specified level.
// Performs strict checking.
//
void NManager::DeleteAd(int ServerID)
	{
	GUARD;
	AssertInitialized();
	//
	if ((ServerID<=0) || (ServerID>=MAX_ACTIVE_LEVELS)) appErrorf("Invalid ServerID %i",ServerID);
	//
	NServerAd *Ad = Ads[ServerID];
	//
	if (!Ad) appErrorf("ServerID %i is already empty",ServerID);
	//
	UNGUARD("NManager::GenericDeleteAd");
	};

//
// Make sure that the manager and everything it references are valid.
//
void NManager::AssertValid(void)
	{
	GUARD;
	AssertInitialized();
	//
	for (int i=0; i<MAX_DRIVERS; i++)
		{
		if (Drivers[i]) Drivers[i]->AssertValid();
		};
	UNGUARD("NManager::AssertValid");
	};

//
// Network manager command line interface
//
int NManager::Exec(const char *Cmd,FOutputDevice *Out)
	{
	GUARD;
	const char *Str = Cmd;
	//
	if (NetGetCMD(&Str,"STATUS"))
		{
		if (NetGetCMD(&Str,"NET") || !Str[0])
			{
			int i;
			int n=0; for (    i=0; i<MAX_DRIVERS;       i++) if (Drivers[i]) n++;
			int m=0; for (    i=0; i<MAX_ACTIVE_LEVELS; i++) if (Ads    [i]) m++;
			//
			Out->Logf("   NET - Ok: %i drivers, %i ads, %i ticks",n,m,Ticks);
			//
			if (Str[0]==0) goto PassToDrivers;
			return 1;
			}
		else return 0;
		}
	else
		{
		PassToDrivers:
		for (int i=0; i<MAX_DRIVERS; i++)
			{
			if (Drivers[i] && Drivers[i]->Exec(Cmd,Out)) return 1;
			};
		return 0; // Not executed
		};
	UNGUARD("NManager::Exec");
	};

/*-----------------------------------------------------------------------------
	NManager task interface
-----------------------------------------------------------------------------*/

//
// Shut down the generic internals of the network manager.
// Retracts all still-active server ads from all drivers.
// Shuts down all registered drivers.
//
void NManager::TaskExit(void)
	{
	GUARD;
	UNGUARD("NManager::TaskExit");
	};

//
// Return the status of the network manager task
//
char *NManager::TaskStatus(char *Name,char *Desc)
	{
	GUARD;
	//
	sprintf(Name,"NetManager");
	sprintf(Desc,"");
	return Name;
	//
	UNGUARD("NManager::TaskStatus");
	};

//
// Network manager tick function.
//
void NManager::TaskTick(void)
	{
	GUARD;
	//
	// Nothing to do..
	//
	UNGUARD("NManager::TaskTick");
	};

/*-----------------------------------------------------------------------------
	NDriver implementation
-----------------------------------------------------------------------------*/

//
// Make a specified ad available on a driver.
// The default implementation does nothing.
// Particular drivers may want to broadcast the ad on the network.
//
void NDriver::BeginAdvertising(NServerAd *Ad)
	{
	GUARD;
	AssertInitialized();
	//
	// Default implementation does nothing
	//
	UNGUARD("NDriver::BeginAdvertising");
	};

//
// Retract a specified ad from a driver.
// The default implementation does nothing.
// Particular drivers may want to broadcast the ad retraction on the network.
//
void NDriver::EndAdvertising(NServerAd *Ad)
	{
	GUARD;
	AssertInitialized();
	//
	// Default implementation does nothing
	//
	UNGUARD("NDriver::EndAdvertising");
	};

//
// Remove a socket from the driver's socket list
//
void NDriver::RemoveSocketFromList(NSocket *Socket)
	{
	GUARD;
	AssertInitialized();
	//
	for (int i=0; i<MAX_DRIVER_SOCKETS; i++)
		{
		if (Sockets[i]==Socket)
			{
			Sockets[i]=NULL;
			return;
			};
		};
	appError ("Socket not found");
	//
	UNGUARD("NDriver::RemoveSocketFromList");
	};

//
// Find an available index position to add a
// socket at.  Returns index, or 0 if failure.
//
int NDriver::FindAvailableSocketIndex(void)
	{
	GUARD;
	AssertInitialized();
	//
	for (int i=1; i<MAX_DRIVER_SOCKETS; i++)
		{
		if (!Sockets[i]) return i;
		};
	return 0; // No sockets are available
	//
	UNGUARD("NDriver::RemoveSocketFromList");
	};

//
// Initialize the driver's defaults:
//
int NDriver::Init(const char *ParamBuffer,char *ErrorMessage)
	{
	GUARD;
	//
	DriverDescription = "Uninitialized";
	//
	for (int i=0; i<MAX_DRIVER_SOCKETS; i++)
		{
		Sockets[i]=NULL;
		};
	Initialized=1;
	return 1;
	//
	UNGUARD("NDriver::Init");
	};

//
// Shut down the driver.
// Close all sockets.
// Note as deinitialized.
//
void NDriver::Exit(void)
	{
	GUARD;
	AssertInitialized();
	//
	for (int i=0; i<MAX_DRIVER_SOCKETS; i++)
		{
		if (Sockets[i]) Sockets[i]->Delete();
		if (Sockets[i]) appError ("Socket failed to delete itself");
		};
	UNGUARD("NDriver::Exit");
	};

//
// Make sure this driver is in a valid state:
// All sockets are in an acceptable state and are properly linked to the driver.
//
void NDriver::AssertValid(void)
	{
	GUARD;
	//
	for (int i=0; i<MAX_DRIVER_SOCKETS; i++)
		{
		if (Sockets[i])
			{
			Sockets[i]->AssertValid();
			if (Sockets[i]->Driver != this) appErrorf("Driver %s improperly linked to socket",DriverDescription);
			};
		};
	UNGUARD("NDriver::AssertValid");
	};

//
// Network driver command line interface.
//
int NDriver::Exec(const char *Cmd,FOutputDevice *Out)
	{
	GUARD;
	//
	return 0;
	//
	UNGUARD("NDriver::Exec");
	};

/*------------------------------------------------------------------------------
	URL routines
------------------------------------------------------------------------------*/

//
// Create a new, full URL in *Result, based on the current URL (which may be NULL
// or empty) and a new, possibly-relative URL name.
//
void MakeFullURL(const char *CurrentURL, const char *PossiblyRelativeURL, char *Result)
	{
	GUARD;
	#if 0
	//
	char SourceURL[256];
	//
	// Remove padding from PossiblyRelativeURL:
	//
	while (*PossiblyRelativeURL==' ') PossiblyRelativeURL++;
	strcpy(SourceURL,PossiblyRelativeURL);
	while ((strlen(SourceURL)>0) && (SourceURL[strlen(SourceURL)-1]==' ')
		{
		SourceURL[strlen(SourceURL)-1] = 0;
		};
	//
	// See if PossiblyRelativeURL is a full URL or a partial URL:
	//
	if (strnicmp(SourceURL,"file:",5))
		{
		}
	else if (strnicmp(SourceURL,"
	if ((!CurrentUrl) || (!*CurrentURL))
		{
		};
	#endif
	UNGUARD("MakeFullURL");
	};

void ParseFullURL(char *SourceURL,EURLType *Type, char **URLName)
	{
	*Type    = URL_INVALID;
	*URLName = SourceURL;
	};

/*-----------------------------------------------------------------------------
	Misc helper functions
-----------------------------------------------------------------------------*/

//
// Get a command from the input stream.
//
int UNNETWORK_API NetGetCMD (const char **Stream, const char *Match)
	{
	GUARD;
	while ((**Stream==' ')||(**Stream==9)) (*Stream)++;
	if (strnicmp(*Stream,Match,strlen(Match))==0)
		{
		*Stream += strlen(Match);
		if (!isalnum(**Stream))
			{
			while ((**Stream==' ')||(**Stream==9)) (*Stream)++;
			return 1; // Success
			}
		else
			{
			*Stream -= strlen(Match);
			return 0; // Only found partial match
			};
		}
	else return 0; // No match
	UNGUARD("NetGetCMD");
	};

//
// Grab the next string from the input stream.
//
int UNNETWORK_API NetGrabSTRING(const char *&Str,char *Result, int MaxLen)
	{
	int Len=0;
	while ((*Str==' ')||(*Str==9)) Str++;
	while ((*Str) && (*Str!=' ') && (*Str!=9) && ((Len+1)<MaxLen))
		{
		Result[Len++] = *Str++;
		};
	Result[Len]=0;
	return Len!=0;
	};

/*------------------------------------------------------------------------------
	Fake wizard background dialog box
------------------------------------------------------------------------------*/

BOOL CALLBACK MainWizardDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	static HWND hWndChild;
	GUARD;
	if (uMsg==WM_COMMAND) switch(LOWORD(wParam))
		{
		case IDCANCEL:
			GApp->Enable();
			DestroyWindow(hWndChild);
			EndDialog(hDlg,0);
			return TRUE;
		case ID_SUCCESS:
			GApp->Enable();
			DestroyWindow(hWndChild);
			EndDialog(hDlg,1);
			return TRUE;
		default:
			return FALSE;
		}
	else switch(uMsg)
		{
		case WM_INITDIALOG:
			{
			GApp->Disable();
			NetManager.hWndFakeWizard=(DWORD)hDlg;
			CreateDialogParam
				(
				(HINSTANCE)NetManager.hInstance,
				MAKEINTRESOURCE(IDD_NETWIZ),
				(HWND)NetManager.hWndFakeWizard,
				(DLGPROC)NetWizDialogProc,
				(LPARAM)hDlg
				);
			return TRUE;
			};
		default:
			return FALSE;
		};
	UNGUARD("MainWizardDialogProc");
	};

/*------------------------------------------------------------------------------
	Network play wizard dialog box
------------------------------------------------------------------------------*/

BOOL CALLBACK NetWizDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	static HWND hWndReturn;
	GUARD;
	if (uMsg==WM_COMMAND) switch(LOWORD(wParam))
		{
		case IDCANCEL:
		case ID_SUCCESS:
			SendMessage(hWndReturn,WM_COMMAND,wParam,0);
			DestroyWindow(hDlg);
			return TRUE;
		case ID_NEXT:
			{
			int JoinGame = IsDlgButtonChecked(hDlg,IDC_JOIN);
			GApp->PutProfileInteger("NetGame","JoinGame",JoinGame);
			//
			if (JoinGame)
				{
				CreateDialogParam // Go to join-a-game dialog:
					(
					(HINSTANCE)NetManager.hInstance,
					MAKEINTRESOURCE(IDD_JOINTYPE),
					(HWND)NetManager.hWndFakeWizard,
					(DLGPROC)JoinTypeDialogProc,
					(LPARAM)hDlg
					);
				}
			else
				{
				CreateDialogParam // Go to host-a-game dialog:
					(
					(HINSTANCE)NetManager.hInstance,
					MAKEINTRESOURCE(IDD_HOSTTYPE),
					(HWND)NetManager.hWndFakeWizard,
					(DLGPROC)HostTypeDialogProc,
					(LPARAM)hDlg
					);
				};
			};
			return TRUE;
		default:
			return FALSE;
		}
	else switch(uMsg)
		{
		case WM_INITDIALOG:
			{
			hWndReturn = (HWND)lParam;
			//
			// Check either IDC_JOIN or IDC_HOST:
			//
			int JoinGame=GApp->GetProfileInteger("NetGame","JoinGame",1);
			CheckDlgButton(hDlg,JoinGame ? IDC_JOIN : IDC_HOST,1);
			//
			// Display this dialog:
			//
			SetParent(hDlg,(HWND)NetManager.hWndFakeWizard);
			SendMessage(hDlg,WM_USER_REFRESH,0,0);
			return FALSE;
			};
		case WM_USER_REFRESH:
			SetWindowText((HWND)NetManager.hWndFakeWizard,"Network Game Wizard");
			ShowWindow(hDlg,SW_SHOW);
			SetFocus(GetDlgItem(hDlg,ID_NEXT));
			return TRUE;
		default:
			return FALSE;
		};
	UNGUARD("NetWizDialogProc");
	};

/*-----------------------------------------------------------------------------
	Join-a-game type dialog box
-----------------------------------------------------------------------------*/

BOOL CALLBACK JoinTypeDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	static HWND hWndReturn;
	GUARD;
	if (uMsg==WM_COMMAND) switch(LOWORD(wParam))
		{
		case IDCANCEL:
		case ID_SUCCESS:
			SendMessage(hWndReturn,WM_COMMAND,wParam,0);
			DestroyWindow(hDlg);
			return TRUE;
		case ID_NEXT:
			{
			int InternetGame = IsDlgButtonChecked(hDlg,IDC_INET);
			GApp->PutProfileInteger("NetGame","InternetGame",InternetGame);
			//
			if (InternetGame)
				{
				if (!NetManager.InternetDriver) appError("Internet inconsistency");
				CreateDialogParam // Go to join-Internet dialog:
					(
					(HINSTANCE)NetManager.hInstance,
					MAKEINTRESOURCE(IDD_INETPROVIDER),
					(HWND)NetManager.hWndFakeWizard,
					(DLGPROC)JoinInternetDialogProc,
					(LPARAM)hDlg
					);
				}
			else
				{
				if (!NetManager.DirectPlayDriver) appError("DirectPlay inconsistency");
				CreateDialogParam // Go to join-DirectPlay dialog:
					(
					(HINSTANCE)NetManager.hInstance,
					MAKEINTRESOURCE(IDD_DPLAYPROVIDER),
					(HWND)NetManager.hWndFakeWizard,
					(DLGPROC)DirectPlayProviderDialogProc,
					(LPARAM)hDlg
					);
				};
			};
			return TRUE;
		case ID_BACK:
			SendMessage(hWndReturn,WM_USER_REFRESH,0,0);
			DestroyWindow(hDlg);
			return TRUE;
		default:
			return FALSE;
		}
	else switch(uMsg)
		{
		case WM_INITDIALOG:
			{
			hWndReturn = (HWND)lParam;
			// Set up IDC_DPLAY or IDC_INET buttons:
			EnableWindow(GetDlgItem(hDlg,IDC_DPLAY),!!NetManager.DirectPlayDriver);
			EnableWindow(GetDlgItem(hDlg,IDC_INET ),!!NetManager.InternetDriver);
			EnableWindow(GetDlgItem(hDlg,ID_NEXT ),NetManager.InternetDriver || NetManager.DirectPlayDriver);
			// Look up default option from profile:
			int InternetGame = GApp->GetProfileInteger("NetGame","InternetGame",!!NetManager.InternetDriver);
			if (!NetManager.DirectPlayDriver) InternetGame = 1;
			if (!NetManager.InternetDriver)   InternetGame = 0;
			CheckDlgButton(hDlg,InternetGame ? IDC_INET : IDC_DPLAY,1);
			// Display this dialog:
			SetParent(hDlg,(HWND)NetManager.hWndFakeWizard);
			ShowWindow(hWndReturn,SW_HIDE);
			SendMessage(hDlg,WM_USER_REFRESH,0,0);
			return FALSE;
			};
		case WM_USER_REFRESH:
			SetWindowText((HWND)NetManager.hWndFakeWizard,"Join An Existing Network Game");
			ShowWindow(hDlg,SW_SHOW);
			SetFocus(GetDlgItem(hDlg,ID_NEXT));
			return TRUE;
		default:
			return FALSE;
		};
	UNGUARD("JoinTypeDialogProc");
	};

/*-----------------------------------------------------------------------------
	Host-a-game type dialog box
-----------------------------------------------------------------------------*/

BOOL CALLBACK HostTypeDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	static HWND hWndReturn;
	GUARD;
	if (uMsg==WM_COMMAND) switch(LOWORD(wParam))
		{
		case IDCANCEL:
		case ID_SUCCESS:
			SendMessage(hWndReturn,WM_COMMAND,wParam,0);
			DestroyWindow(hDlg);
			return TRUE;
		case ID_NEXT:
			{
			int InternetGame = IsDlgButtonChecked(hDlg,IDC_INET);
			GApp->PutProfileInteger("NetGame","InternetGame",InternetGame);
			//
			if (IsDlgButtonChecked(hDlg,IDC_DPLAY) && NetManager.DirectPlayDriver)
				{
				}
			else if (IsDlgButtonChecked(hDlg,IDC_INET) && NetManager.InternetDriver)
				{
				};
			return TRUE;
			};
		case ID_BACK:
			SendMessage(hWndReturn,WM_USER_REFRESH,0,0);
			DestroyWindow(hDlg);
			return TRUE;
		default:
			return FALSE;
		}
	else switch(uMsg)
		{
		case WM_INITDIALOG:
			{
			hWndReturn = (HWND)lParam;
			// Set up IDC_DPLAY or IDC_INET buttons:
			EnableWindow(GetDlgItem(hDlg,IDC_DPLAY),!!NetManager.DirectPlayDriver);
			EnableWindow(GetDlgItem(hDlg,IDC_INET ),!!NetManager.InternetDriver);
			EnableWindow(GetDlgItem(hDlg,ID_NEXT  ),NetManager.InternetDriver || NetManager.DirectPlayDriver);
			// Look up game type from profile:
			int InternetGame = GApp->GetProfileInteger("NetGame","InternetGame",!!NetManager.InternetDriver);
			if (!NetManager.DirectPlayDriver) InternetGame = 1;
			if (!NetManager.InternetDriver)   InternetGame = 0;
			CheckDlgButton(hDlg,InternetGame ? IDC_INET : IDC_DPLAY,1);
			// Display this dialog:
			SetParent(hDlg,(HWND)NetManager.hWndFakeWizard);
			ShowWindow(hWndReturn,SW_HIDE);
			SendMessage(hDlg,WM_USER_REFRESH,0,0);
			return FALSE;
			};
		case WM_USER_REFRESH:
			SetWindowText((HWND)NetManager.hWndFakeWizard,"Host A New Network Game");
			ShowWindow(hDlg,SW_SHOW);
			SetFocus(GetDlgItem(hDlg,ID_NEXT));
			return TRUE;
		default:
			return FALSE;
		};
	UNGUARD("HostTypeDialogProc");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
