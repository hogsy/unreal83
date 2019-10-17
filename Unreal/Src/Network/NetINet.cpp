/*=============================================================================
	NetINet.cpp: Unreal Windows/WinSock Internet networking

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include <windows.h>
#include <windowsx.h>
#include "UnBuild.h"
#include "UnPlatfm.h"
#include "Net.h"
#include "NetPrv.h"
#include "NetINet.h"
#include "Resource.h"

//
// Host entry information used by async WSAAsyncGetHostByName call:
//
union
	{
	hostent HostEnt;
	BYTE Buffer[MAXGETHOSTSTRUCT];
	} GHostEnt;
	

/*------------------------------------------------------------------------------
	Errors
------------------------------------------------------------------------------*/

//
// Windows sockets errors
//
char *wsaError(void)
	{
	switch (WSAGetLastError())
		{
		case WSAEINTR:				return "WSAEINTR";
		case WSAEBADF:				return "WSAEBADF";
		case WSAEACCES:				return "WSAEACCES";
		case WSAEFAULT:				return "WSAEFAULT";
		case WSAEINVAL:				return "WSAEINVAL";
		case WSAEMFILE:				return "WSAEMFILE";
		case WSAEWOULDBLOCK:		return "WSAEWOULDBLOCK";
		case WSAEINPROGRESS:		return "WSAEINPROGRESS";
		case WSAEALREADY:			return "WSAEALREADY";
		case WSAENOTSOCK:			return "WSAENOTSOCK";
		case WSAEDESTADDRREQ:		return "WSAEDESTADDRREQ";
		case WSAEMSGSIZE:			return "WSAEMSGSIZE";
		case WSAEPROTOTYPE:			return "WSAEPROTOTYPE";
		case WSAENOPROTOOPT:		return "WSAENOPROTOOPT";
		case WSAEPROTONOSUPPORT:	return "WSAEPROTONOSUPPORT";
		case WSAESOCKTNOSUPPORT:	return "WSAESOCKTNOSUPPORT";
		case WSAEOPNOTSUPP:			return "WSAEOPNOTSUPP";
		case WSAEPFNOSUPPORT:		return "WSAEPFNOSUPPORT";
		case WSAEAFNOSUPPORT:		return "WSAEAFNOSUPPORT";
		case WSAEADDRINUSE:			return "WSAEADDRINUSE";
		case WSAEADDRNOTAVAIL:		return "WSAEADDRNOTAVAIL";
		case WSAENETDOWN:			return "WSAENETDOWN";
		case WSAENETUNREACH:		return "WSAENETUNREACH";
		case WSAENETRESET:			return "WSAENETRESET";
		case WSAECONNABORTED:		return "WSAECONNABORTED";
		case WSAECONNRESET:			return "WSAECONNRESET";
		case WSAENOBUFS:			return "WSAENOBUFS";
		case WSAEISCONN:			return "WSAEISCONN";
		case WSAENOTCONN:			return "WSAENOTCONN";
		case WSAESHUTDOWN:			return "WSAESHUTDOWN";
		case WSAETOOMANYREFS:		return "WSAETOOMANYREFS";
		case WSAETIMEDOUT:			return "WSAETIMEDOUT";
		case WSAECONNREFUSED:		return "WSAECONNREFUSED";
		case WSAELOOP:				return "WSAELOOP";
		case WSAENAMETOOLONG:		return "WSAENAMETOOLONG";
		case WSAEHOSTDOWN:			return "WSAEHOSTDOWN";
		case WSAEHOSTUNREACH:		return "WSAEHOSTUNREACH";
		case WSAENOTEMPTY:			return "WSAENOTEMPTY";
		case WSAEPROCLIM:			return "WSAEPROCLIM";
		case WSAEUSERS:				return "WSAEUSERS";
		case WSAEDQUOT:				return "WSAEDQUOT";
		case WSAESTALE:				return "WSAESTALE";
		case WSAEREMOTE:			return "WSAEREMOTE";
		case WSAEDISCON:			return "WSAEDISCON";
		case WSASYSNOTREADY:		return "WSASYSNOTREADY";
		case WSAVERNOTSUPPORTED:	return "WSAVERNOTSUPPORTED";
		case WSANOTINITIALISED:		return "WSANOTINITIALISED";
		case WSAHOST_NOT_FOUND:		return "WSAHOST_NOT_FOUND";
		case WSATRY_AGAIN:			return "WSATRY_AGAIN";
		case WSANO_RECOVERY:		return "WSANO_RECOVERY";
		case WSANO_DATA:			return "WSANO_DATA";
		case 0:						return "WSANO_NO_ERROR";
		default:					return "WSA_Unknown";
		};
	};

/*------------------------------------------------------------------------------
	Windows Sockets async results processing window
------------------------------------------------------------------------------*/

LRESULT CALLBACK AsyncResultsWndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
	static NInternetDriver *This=NULL;
	//
	switch(uMsg)
		{
		case WM_CREATE:
			This = (NInternetDriver *)(((CREATESTRUCT *)lParam)->lpCreateParams);
			return 0;
		case NInternetDriver::WM_WSA_GetHostByName:
			{
			if (This && This->Initialized)
				{
				int Error  = WSAGETASYNCERROR(lParam);
				int BufLen = WSAGETASYNCBUFLEN(lParam);
				//
				if (Error)
					{
					debugf(LOG_Net,"WinSock: WSAAsyncGetHostByName failed");
					}
				else if (GHostEnt.HostEnt.h_addrtype!=PF_INET)
					{
					debugf(LOG_Net,"WinSock: WSAAsyncGetHostByName returned non-Internet address");
					}
				else
					{
					This->HostAddr = *(FInetAddr *)(*GHostEnt.HostEnt.h_addr_list);
					debugf(LOG_Net,"WinSock: I am %s (%i.%i.%i.%i)", This->HostName,
						This->HostAddr.B1, This->HostAddr.B2, This->HostAddr.B3, This->HostAddr.B4);
					};
				This->hGetHostByName=NULL;
				};
			return 0;
			};
		};
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
	};

/*------------------------------------------------------------------------------
	NInternetDriver implementation
------------------------------------------------------------------------------*/

//
// Initialize the driver, parsing any non-default parameters from ParamBuffer.
// If successful, returns 1.  If failure, returns 0 and sets Error to
// an appropriate description.
//
int NInternetDriver::Init(char *ParamBuffer,char *ErrorMessage)
	{
	GUARD;
	if (Initialized) appError("Internet already initialized");
	//
	// Create window class for async call results window
	//
	ZeroMemory(&wcAsyncResults,sizeof(wcAsyncResults));
    wcAsyncResults.lpfnWndProc		= AsyncResultsWndProc; 
    wcAsyncResults.hInstance		= (HINSTANCE)NetManager.hInstance; 
    wcAsyncResults.lpszClassName	= "AsyncResults";
	hGetHostByName=NULL;
	//
	aAsyncResults = RegisterClass(&wcAsyncResults);
	if (!aAsyncResults) appError("RegisterClass failed");
	//
	hWndAsyncResults=CreateWindow
		(
		"AsyncResults",
		"Windows Sockets Async Results",
		WS_OVERLAPPEDWINDOW,
		0,0,
		0,0,
		NULL,
		NULL,
		(HINSTANCE)NetManager.hInstance,
		(LPVOID)this
		);
	if (!hWndAsyncResults) appError("CreateWindow failed");
	//
	if (WSAStartup(0x0101,&WSAData))
		{
		debugf(LOG_Init,"WinSock: WSAStartup failed (%s)",wsaError()); 
		return 0;
		};
	Initialized=1;
	TelnetListener=NULL;
	strcpy(HostName,"");
	//
	debugf
		(
		LOG_Init,"WinSock: version %i.%i (%i.%i), MaxSocks=%i, MaxUdp=%i",
		WSAData.wVersion>>8,WSAData.wVersion&255,
		WSAData.wHighVersion>>8,WSAData.wHighVersion&255,
		WSAData.iMaxSockets,WSAData.iMaxUdpDg
		);
	debugf(LOG_Init,"WinSock: %s",WSAData.szDescription);
	//
	if (gethostname(HostName,256))
		{
		debugf(LOG_Init,"WinSock: gethostname failed (%s)",wsaError());
		Exit(); 
		return 0;
		};
	HostAddr.D=0;
	hGetHostByName = WSAAsyncGetHostByName(hWndAsyncResults,WM_WSA_GetHostByName,HostName,(char *)&GHostEnt,sizeof(GHostEnt));
	if (!hGetHostByName)
		{
		debugf(LOG_Init,"WinSock: WSAAsyncGetHostByName failed (%s)",wsaError());
		Exit(); 
		return 0;
		};
	//
	// Listen for telnet connections:
	//
	TelnetListener = socket(PF_INET,SOCK_STREAM,0);
	if (TelnetListener==INVALID_SOCKET)
		{
		debugf(LOG_Init,"WinSock: Telnet listener socket failed (%s)",wsaError());
		TelnetListener=NULL;
		};
	if (TelnetListener!=NULL)
		{
		SOCKADDR_IN SockAddr;
		SockAddr.sin_family			= AF_INET;
		SockAddr.sin_port			= htons(IPPORT_TELNET);
		SockAddr.sin_addr.s_addr	= 0;
		if (bind(TelnetListener,(SOCKADDR *)&SockAddr,sizeof(SOCKADDR_IN))!=0)
			{
			debugf(LOG_Init,"WinSock: Telnet listener bind failed (%s)",wsaError());
			closesocket(TelnetListener);
			TelnetListener=NULL;
			};
		};
	if (TelnetListener!=NULL)
		{
		u_long NoBlocking=1;
		if (ioctlsocket(TelnetListener,FIONBIO,&NoBlocking))
			{
			debugf(LOG_Init,"WinSock: Telnet listener ioctlsocket failed (%s)",wsaError());
			closesocket(TelnetListener);
			TelnetListener=NULL;
			};
		};
	if (TelnetListener!=NULL)
		{
   		if (listen(TelnetListener,4)!=0)
			{
			debugf(LOG_Init,"WinSock: Telnet listener listen failed (%s)",wsaError());
			closesocket(TelnetListener);
			TelnetListener=NULL;
			};
		};
	if (TelnetListener!=NULL) debugf(LOG_Init,"WinSock: Listening for telnet on port %i",IPPORT_TELNET);
	//
	// Start task:
	//
	GTaskManager->AddTask(this,NULL,GApp,PRIORITY_Realtime,TASK_NoUserKill);
	NetManager.RegisterDriver(this);
	//
	// Success:
	//
	return 1;
	//
	UNGUARD("NInternetDriver::Init");
	};

void NInternetDriver::Exit(void)
	{
	GUARD;
	AssertInitialized();
	//
	GTaskManager->KillTask(this);
	if (hGetHostByName)
		{
		if (WSACancelAsyncRequest(hGetHostByName)) debugf(LOG_Exit,"WSACancelAsyncRequest failed (%s)",wsaError());
		};
	if (TelnetListener && closesocket(TelnetListener))
		{
		debugf(LOG_Exit,"WinSock closesocket error(%s)",wsaError());
		};
	if (WSACleanup())
		{
		debugf(LOG_Exit,"WinSock WSACleanup error (%s)",wsaError());
		};
	debugf(LOG_Exit,"WinSock shut down");
	//
	if (!DestroyWindow(hWndAsyncResults)) appError("DestroyWindow failed");
	//
	UNGUARD("NInternetDriver::Exit");
	};

void NInternetDriver::TaskExit(void)
	{
	GUARD;
	//
	UNGUARD("NInternetDriver::TaskExit");
	};

char *NInternetDriver::TaskStatus(char *Name, char *Desc)
	{
	GUARD;
	//
	sprintf(Name,"Internet");
	sprintf(Desc,"");
	return Name;
	//
	UNGUARD("NInternetDriver::TaskStatus");
	};

void NInternetDriver::TaskTick(void)
	{
	GUARD;
	AssertInitialized();
	//
	SOCKADDR_IN SockAddr;
	int Size = sizeof(SOCKADDR_IN);
	//
	SOCKET Result = accept(TelnetListener,(SOCKADDR *)&SockAddr,&Size);
	if (Result!=INVALID_SOCKET)
		{
		FTaskTelnet *Telnet = new FTaskTelnet;
		Telnet->Init(Result,SockAddr);
		};
	UNGUARD("NInternetDriver::Tick");
	};

int NInternetDriver::Exec(const char *Cmd,FOutputDevice *Out)
	{
	GUARD;
	const char *Str = Cmd;
	//
	if (NetGetCMD(&Str,"STATUS"))
		{
		if (NetGetCMD(&Str,"INTERNET") || !Str[0])
			{
			Out->Logf("   INTERNET - Ok");
			//
			return !Str[0];
			}
		else return 0;
		}
	else if (NetGetCMD(&Str,"PING"))
		{
		Out->Log("Ping not yet implemented");
		return 1;
		}
	else if (NetGetCMD(&Str,"WHOAMI"))
		{
		Out->Logf("I am %s (%i.%i.%i.%i)",HostName,HostAddr.B1,HostAddr.B2,HostAddr.B3,HostAddr.B4);
		return 1;
		}
	else return 0;
	//
	UNGUARD("NInternetDriver::Exec");
	};

int NInternetDriver::CanHandleURL(char *ServerURL)
	{
	GUARD;
	//
	return 0;
	//
	UNGUARD("NInternetDriver::CanHandleURL");
	};

NSocket *NInternetDriver::ServerAcceptConnection(int ServerID)
	{
	GUARD;
	//
	return NULL;
	//
	UNGUARD("NInternetDriver::ServerAcceptConnection");
	};

NSocket *NInternetDriver::ClientOpenServer(char *ServerURL,char *ErrorMessage)
	{
	GUARD;
	//
	return NULL;
	//
	UNGUARD("NInternetDriver::ClientOpenServer");
	};

void NInternetDriver::BeginAdvertising(NServerAd *Ad)
	{
	GUARD;
	//
	UNGUARD("NInternetDriver::BeginAdvertising");
	};

void NInternetDriver::EndAdvertising(NServerAd *Ad)
	{
	GUARD;
	//
	UNGUARD("NInternetDriver::EndAdvertising");
	};

/*------------------------------------------------------------------------------
	NInternetSocket implementation
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Join-Internet-game dialog box
-----------------------------------------------------------------------------*/

BOOL CALLBACK JoinInternetDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	GUARD;
	//
	HWND hWndHost = GetDlgItem(hDlg,IDC_HOST);
	static HWND hWndReturn;
	//
	if (uMsg==WM_COMMAND) switch(LOWORD(wParam))
		{
		case IDCANCEL:
		case ID_SUCCESS:
			SendMessage(hWndReturn,WM_COMMAND,wParam,0);
			DestroyWindow(hDlg);
			return TRUE;
		case ID_NEXT:
			{
			char Location[256];
			Edit_GetText(hWndHost,Location,256);
			//
			GApp->PutProfileValue("NetGame","Location",Location);
			//
			// Go to Internet 'launch' screen!
			//
			return TRUE;
			};
		case ID_BACK:
			SendMessage(hWndReturn,WM_USER_REFRESH,0,0);
			DestroyWindow(hDlg);
			return TRUE;
		case IDC_EPICSERVER:
			Edit_SetText(hWndHost,URL_GAME);
			return TRUE;
		case IDC_DIRECTORY:
			GApp->LaunchURL(WEB_LINK_FNAME,"");
			return TRUE;
		default:
			return FALSE;
		}
	else switch(uMsg)
		{
		case WM_INITDIALOG:
			{
			//
			// Look up default starting URL:
			//
			char Location[256];
			GApp->GetProfileValue("NetGame","Location",Location,URL_GAME,256);
			//
			hWndReturn = (HWND)lParam;
			// Set up IDC_HOST edit box:
			Edit_SetText(hWndHost,Location);
			// Display this dialog:
			SetParent(hDlg,(HWND)NetManager.hWndFakeWizard);
			ShowWindow(hWndReturn,SW_HIDE);
			SendMessage(hDlg,WM_USER_REFRESH,0,0);
			return FALSE;
			};
		case WM_USER_REFRESH:
			SetWindowText((HWND)NetManager.hWndFakeWizard,"Join An Internet Game");
			ShowWindow(hDlg,SW_SHOW);
			SetFocus(GetDlgItem(hDlg,ID_NEXT));
			return TRUE;
		default:
			return FALSE;
		};
	UNGUARD("JoinInternetDialogProc");
	};

/*------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------*/
