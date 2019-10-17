/*=============================================================================
	NetDPlay.h: Unreal Windows DirectPlay networking

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_NETDPLAY
#define _INC_NETDPLAY

/*------------------------------------------------------------------------------
	DirectPlay-specific classes
------------------------------------------------------------------------------*/

//
// DirectPlay driver:
//
class NDirectPlayDriver : public NDriver
	{
	public:
	//
	// NDriver interface:
	//
	int Init(const char *ParamBuffer,char *ErrorMessage);
	void Exit(void);
	int Exec(const char *Cmd,FOutputDevice *Out);
	int CanHandleURL(char *ServerURL);
	NSocket *ServerAcceptConnection(int ServerID);
	NSocket *ClientOpenServer(char *ServerURL,char *ErrorMessage);
	void BeginAdvertising(NServerAd *Ad);
	void EndAdvertising(NServerAd *Ad);
	//
	// FTask interface:
	//
	void TaskTick(void);
	void TaskExit(void);
	char *TaskStatus(char *Name,char *Desc);
	//
	// Custom variables:
	//
	typedef HRESULT (WINAPI *pDirectPlayEnumerate)(LPDPENUMDPCALLBACK,LPVOID);
	typedef HRESULT (WINAPI *pDirectPlayCreate)(LPGUID lpGUID, LPDIRECTPLAY FAR * lplpDP, IUnknown FAR * pUnkOuter);
	//
	pDirectPlayEnumerate DirectPlayEnumerate;
	pDirectPlayCreate	 DirectPlayCreate;
	IDirectPlay			 *DirectPlay;
	//
	// Custom dialogs:
	//
	friend BOOL CALLBACK DirectPlayProviderDialogProc(HWND,UINT,WPARAM,LPARAM);
	friend BOOL CALLBACK DirectPlaySessionDialogProc (HWND,UINT,WPARAM,LPARAM);
	};

//
// A direct play socket, encapsulating the functionality of Microsoft DirectPlay
// drivers.
//
class NDirectPlaySocket : public NSocket
	{
	public:
	//
	// Variables:
	//
	/**/
	//
	// Standard NSocket functions:
	//
	int GetPacket (NPacket *PacketToGet);
	void SendPacket (NPacket *PacketToSend);
	int MaxPacketSize(void);
	void Tick(void);
	void Delete(void);
	//
	private:
	void Init(NDriver *CreatingDriver);
	};

/*------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------*/
#endif // _INC_NETDPLAY
