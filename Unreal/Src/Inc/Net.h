/*=============================================================================
	Net.h: Unreal networking, public (engine-accessible) header file

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNNETPUB
#define _INC_UNNETPUB

#ifndef _INC_UNPLATFM
#include "UnPlatfm.h"
#endif

#ifndef _INC_UNPLATFM
#include "UnPlatfm.h"
#endif

/*------------------------------------------------------------------------------
	Globals
------------------------------------------------------------------------------*/

enum {MAX_PACKET_SIZE = 512};

//
// The action a particular socket is performing.
//
enum ESocketAction
	{
	/////////////////////////////////////////////////////////////////////////////////////////
	// Action tag        //   Value   // Meaning      // Possible next actions             //
	/////////////////////////////////////////////////////////////////////////////////////////
	NS_INVALID		= 0, // Socket is erroneous       // N/A                               //
	NS_DISCONNECTED	= 1, // Socket is disconnected    // Can't transition elsewhere        //
	NS_NEGOTIATING	= 2, // Negotiating action change // Any other state                   //
	NS_PLAY			= 3, // Gameplay in progress      // NS_NEGOTIATING, NS_DISCONNECTED   //
	NS_FILE_SEND	= 4, // Sending a file            // NS_NEGOTIATING, NS_DISCONNECTED   //
	NS_FILE_RECEIVE	= 5, // Receiving a file		  // NS_NEGOTIATING, NS_DISCONNECTED   //
	NS_INITIALIZING	= 6, // Used only in Init()		  // NS_NEGOTIATING, NS_DISCONNECTED   //
	/////////////////////////////////////////////////////////////////////////////////////////
	};

//
// What the local computer is doing related to network play
//
enum ENetworkPlayMode
	{
	PM_NONE				= 0,	// This machine is idle (not playing)
	PM_LOCAL			= 1,	// This is a local-only game
	PM_CLIENT			= 2,	// This machine is a client to a remote server
	PM_CLIENT_SERVER	= 3,	// This machine is a client and a server
	PM_DEDICATED_SERVER	= 4,	// This machine is a dedicated server
	};

//
// Forward declarations:
//
class NSocket;
class NPacket;
class NDriver;
class NManager;

//
// The global network manager object:
//
UNNETWORK_API extern NManager NetManager;

/*------------------------------------------------------------------------------
	Global URL routines
------------------------------------------------------------------------------*/

//
// URL types:
//
enum EURLType
	{
	URL_UNREAL_FILE		= 0,	// A local Unreal file
	URL_UNREAL_SERVER	= 1,	// An Unreal server on the Internet
	URL_EXTERNAL		= 2,	// Some external URL like a Web page
	URL_INVALID			= 3,	// An invalid/unrecognizable URL
	};

void MakeFullURL (const char *CurrentURL, const char *PossiblyRelativeURL, char *Result);
void ParseFullURL(char *SourceURL,EURLType *Type, char **URLName);

/*-----------------------------------------------------------------------------
	FSocket
-----------------------------------------------------------------------------*/

//
// Temporary, bogus
//
class FSocket
	{
	public:
	int Unused;
	};

/*------------------------------------------------------------------------------
	NPacket
------------------------------------------------------------------------------*/

//
// A generic packet containing data:
//
class NPacket
	{
	public:
	//
	// Variables:
	//
	int		Size,MaxSize;
	int		Crc;
	BYTE	Data[MAX_PACKET_SIZE];
	NSocket *DestSocketForSanityCheck;
	//
	// Functions:
	//
	virtual void Init(NSocket *SocketToSendThrough);
	inline void Finalize(void)
		{
		GUARD;
		if (Size>MaxSize) appError ("Packet size exceeds maximum");
		UNGUARD("NPacket::Finalize");
		};
	inline int AppendData(void *Src,int SrcSize)
		{
		GUARD;
		if ((Size+SrcSize) <= MaxSize)
			{
			memcpy(&Data[Size],Src,SrcSize);
			Size += SrcSize;
			return 1;
			}
		else return 0;
		UNGUARD("NPacket::AppendData");
		};
	inline int Remaining(void)
		{
		return MaxSize-Size;
		};
	};

/*------------------------------------------------------------------------------
	NSocket
------------------------------------------------------------------------------*/

//
// A two-way, unreliable communications socket connecting its owner to
// some other entity, either local or remote.  This is a virtual base class from
// which media-specific socket types are derived.
//
// The socket's Action is set in ServerAcceptConnection or ClientOpenServerXYZ,
// can only change as follows: To the specified value in a call to GetAction (which
// is strictly checked according to the allowable transitions), and in calls
// to NSocket::Tick (which is always in accordance with the allowable transitions).
//
// Tick() performs general maintenance, such as timeout checking and connection
// status updates.  This should be called at the game's internal frequency, i.e. 35 times
// per second.
//
class NSocket
	{
	public:
	friend class NDriver;
	//
	// Unreal-callable functions:
	//
	virtual ESocketAction GetAction(void);
	virtual void SetAction(ESocketAction NewAction);
	//
	// Virtual functions that must be overridden:
	//
	virtual int GetPacket (NPacket *PacketToGet) = 0;
	virtual void SendPacket (NPacket *PacketToSend) = 0;
	virtual int MaxPacketSize(void) = 0;
	virtual void Tick(void);
	virtual void Delete(void);
	//
	// Inline:
	//
	inline void AssertInitialized(void) {if (!Initialized) appError("Socket not initialized");};
	inline int IsDead(void) {return GetAction()==NS_DISCONNECTED;};
	//
	// Variables:
	//
	int Initialized;
	//
	// Private implementation:
	//
	protected:
	#ifdef COMPILING_NETWORK
	NDriver *Driver;
	//
	NSocket() {Initialized=0;};
	ESocketAction Action;
	void AssertPacketValidBeforeSend(NPacket *Packet);
	void AssertValid(void);
	//
	virtual void Init(NDriver *CreatingDriver);
	#endif
	};

/*------------------------------------------------------------------------------
	NManager
------------------------------------------------------------------------------*/

//
// Global network manager.  There is always one, and only one, of these
// objects available globally.  It moderates all actions performed by
// the individual network drivers, and provides a transparent way of doing
// everything you'll need to do through the drivers.
//
class NManager : public FTask
	{
	//////////////////////
	// Public interface //
	//////////////////////
	public:
	//
	// General engine-callable functions:
	//
	virtual int BeginAdvertisingLevel(char *LevelName);
	virtual void EndAdvertisingLevel(int ServerID);
	//
	virtual NSocket *ServerAcceptConnection(int ServerID);
	virtual NSocket *ClientOpenServer(char *ServerURL,char *ErrorMessage);
	virtual ENetworkPlayMode BeginGameByUI(char *ResultURL,NSocket **ClientSocket);
	//
	virtual void Init(void);
	virtual void Exit(void);
	//
	virtual void TaskTick(void);
	virtual void TaskExit(void);
	virtual char *TaskStatus(char *Name,char *Desc);
	//
	virtual int Exec(const char *Cmd,FOutputDevice *Out=GApp);
	virtual int RegisterDriver(NDriver *Driver);
	virtual void UnregisterDriver(NDriver *Driver);
	virtual void AssertValid(void);
	//
	/////////////
	// Drivers //
	/////////////
	//
	class NInternetDriver	*InternetDriver;
	class NDirectPlayDriver *DirectPlayDriver;
	//
	///////////////////////
	// Private interface //
	///////////////////////
	//
	public:
	DWORD hInstance;
	int Initialized;
	int Ticks;
	DWORD hWndFakeWizard;
	NSocket *ResultClientSocket;
	ENetworkPlayMode ResultPlayMode;
	NDriver *LaunchDriver;
	char LaunchURL[256];
	//
	protected:
	inline void AssertInitialized(void){if (!Initialized)appError("Not initialized");};
	//
	int FindAvailableAd(char *LevelName);
	void DeleteAd(int ServerID);
	//
	enum {MAX_ACTIVE_LEVELS=64};
	class NServerAd *Ads[MAX_ACTIVE_LEVELS];
	//
	enum {MAX_DRIVERS=4};
	class NDriver *Drivers[MAX_DRIVERS];
	//
	};

/*------------------------------------------------------------------------------
	The end
------------------------------------------------------------------------------*/
#endif // _INC_UNNETPUB
