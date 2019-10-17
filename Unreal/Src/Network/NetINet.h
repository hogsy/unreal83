/*=============================================================================
	NetINet.h: Unreal Windows/WinSock Internet networking

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_NETINET
#define _INC_NETINET

/*------------------------------------------------------------------------------
	Internet-specific classes
------------------------------------------------------------------------------*/

//
// An Internet address
//
typedef union
	{
	DWORD D;
	struct
		{
		BYTE B1;
		BYTE B2;
		BYTE B3;
		BYTE B4;
		};
	} FInetAddr;

//
// Internet driver:
//
class NInternetDriver : public NDriver
	{
	public:
	//
	// NDriver interface:
	//
	int Init(char *ParamBuffer,char *ErrorMessage);
	void Exit(void);
	int Exec(const char *Cmd,FOutputDevice *Out);
	int CanHandleURL(char *ServerURL);
	NSocket *ServerAcceptConnection(int ServerID);
	NSocket *ClientOpenServer(char *ServerURL,char *ErrorMessage);
	void BeginAdvertising(NServerAd *Ad);
	void EndAdvertising(NServerAd *Ad);
	//
	// FTask implementation:
	//
	void TaskTick(void);
	virtual void TaskExit(void);
	virtual char *TaskStatus(char *Name,char *Desc);
	//
	// Custom variables:
	//
	enum {WM_WSA_GetHostByName=WM_USER+0x200};
	HANDLE hGetHostByName;
	WNDCLASS wcAsyncResults;
	ATOM aAsyncResults;
	HWND hWndAsyncResults;
	char HostName[256];
	WSADATA WSAData;
	SOCKET TelnetListener;
	FInetAddr HostAddr;
	//
	// Custom functions:
	//
	NInternetDriver() {Initialized=0;};
	void AssertInitialized(void) {if (!Initialized) appError("Internet not initialized");};
	//
	// Dialog functions:
	//
	friend BOOL CALLBACK JoinInternetDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	};

//
// An Internet play socket, based on the unreliable UDP protocol of TCP/IP.
//
class NInternetSocket : public NSocket
	{
	public:
	};

//
// A telnet task; enables a remote person to connect to
// an UnrealServer via telnet and access the command
// line.
//
class FTaskTelnet : public FTask, public FOutputDevice
	{
	public:
	//
	// FTask interface:
	//
	void TaskTick(void);
	void TaskExit(void);
	char *TaskStatus(char *Name,char *Desc);
	//
	// FOutputDevice interface:
	//
	void Log(ELogType MsgType, const char *Text);
	//
	// Custom interface:
	//
	int Guest;
	char UserName[NAME_SIZE];
	SOCKET Socket;
	SOCKADDR_IN SockAddr;
	enum {MAX_INPUT_LEN=255};
	char Input[MAX_INPUT_LEN+1];
	//
	int Exec(const char *Cmd,FOutputDevice *Out=GApp);
	void Send(const char *Str);
	void Init(SOCKET NewSocket,SOCKADDR_IN SockAddr);
	};

//
// Return a string describing the most recent Windows Sockets error.
//
char *wsaError(void);

/*------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------*/
#endif // _INC_NETINET
