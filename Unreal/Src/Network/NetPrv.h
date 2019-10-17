/*=============================================================================
	NetPrv.h: Unreal networking, private (non engine-accessible) header

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_NETPRV
#define _INC_NETPRV

/*------------------------------------------------------------------------------
	Net-only globals
------------------------------------------------------------------------------*/

int UNNETWORK_API NetGetCMD (const char **Stream, const char *Match);
int UNNETWORK_API NetGrabSTRING(const char *&Str,char *Result, int MaxLen);

/*------------------------------------------------------------------------------
	NServerAd
------------------------------------------------------------------------------*/

//
// Information about a server that is making itself available to clients
// over the network.
//
class NServerAd
	{
	public:
	//
	// Variables:
	//
	char Name[NAME_SIZE];
	//
	// Functions:
	//
	void GenericAssign(char *LevelName);
	};

/*------------------------------------------------------------------------------
	NDriver
------------------------------------------------------------------------------*/

//
// A generic Unreal network driver.  Tracks all sockets and manages
// startup, shutdown, and moderation of driver-specific items.  This
// is a virtual base class from which specific drivers are derived.
//
class NDriver : public FTask
	{
	public:
	//
	// Variables:
	//
	int Initialized;
	NDriver() {Initialized=0;};
	//
	enum {MAX_DRIVER_SOCKETS=256};
	NSocket *Sockets[MAX_DRIVER_SOCKETS];
	//
	char *DriverDescription;
	//
	// Functions which must be overridden:
	//
	virtual int Init(char *ParamBuffer,char *ErrorMessage);
	virtual void Exit(void);
	//
	virtual int Exec(const char *Cmd,FOutputDevice *Out=GApp)=0;
	//
	virtual int CanHandleURL(char *ServerURL)=0;
	//
	virtual NSocket *ServerAcceptConnection(int ServerID)=0;
	virtual NSocket *ClientOpenServer(char *ServerURL,char *ErrorMessage)=0;
	//
	// Functions which may optionally be overridden:
	//
	virtual void BeginAdvertising(NServerAd *Ad);
	virtual void EndAdvertising(NServerAd *Ad);
	//
	// Standard, non-overridden functions:
	//
	void AssertInitialized(void) {if (!Initialized) appError ("Network driver not initalized");};
	void AssertValid(void);
	void RemoveSocketFromList(NSocket *Socket);
	int FindAvailableSocketIndex(void);
	//
	};

/*------------------------------------------------------------------------------
	Private globals
------------------------------------------------------------------------------*/

#ifdef _WINDOWS_
#define WM_USER_REFRESH (WM_USER+0x123)
#endif

/*------------------------------------------------------------------------------
	The end
------------------------------------------------------------------------------*/
#endif // _INC_NETPRV
