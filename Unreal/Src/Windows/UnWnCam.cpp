/*=============================================================================
	UnWnCam.cpp: Unreal Windows-platform specific camera manager implementation

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	The FWindowsCameraManager tracks all windows resources associated
	with an Unreal camera: Windows, DirectDraw info, mouse movement, menus, 
	states, etc.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "StdAfx.h"		/* Precompiled Windows headers */
#include "UnWn.h"		/* UnWn header */

enum EWinCameraStatus
	{
	WIN_CameraOpening	= 0, // Camera is opening and hWndCamera is still unknown
	WIN_CameraNormal	= 1, // Camera is operating normally, hWndCamera is known
	WIN_CameraClosing	= 2, // Camera is closing and CloseCamera has been called
	};

//
// Platform-specific camera info
//
class FWinCamera
	{
	public:
	//
	// This is typecasted to the BYTE Platform[64] field of the
	// CAMERA_STRUCT structure to hold platform-specific camera info.
	//
	EWinCameraStatus Status;	// See below
	HWND        hWndCamera;		// hWnd of camera window
	BITMAPINFO	*BitmapInfo;    // Handle to DIB header (and screen memory in 16-bit)
	HBITMAP     hBitmap;		// Handle to bitmap (32-bit only)
	HANDLE		hFile;			// Handle to file mapping for CreateDIBSection
	HMENU		hMenu;			// Menu handle. Always present, never changes.
	FLOAT		Aspect;			// Aspect ratio in last controlled mode change
	//
	// If NeedResize=1, then the server should resize the virtual screen
	// buffer as soon as it's unlocked with a call to pBlitAndUnlockCamera.
	//
	int         NeedResize;		// 0=okay, 1=need to resize buffer as soon as we can!
	int         ResizeSXR;		// Destination view X size
	int         ResizeSYR;		// Destination view Y size
	int			ResizeColorBytes;// Color bytes requested upon resize
	//
	// Saved cursor position on button up/down events:
	//
	POINT		SaveCursor;		// 0,0 = Not saved
	//
	// Saved window information from going in/out of fullscreen:
	//
	RECT		SavedWindowRect;
	int			SavedColorBytes;
	int			SavedSXR,SavedSYR;
	int			SavedCaps;
	};

//
// The following define causes Unreal.h to place the FWinCamera
// class into UCamera's platform-specific union.  This enables
// us to access the windows-specific information in a camera
// via Camera->Win.
//
#define PLATFORM_CAMERA_CLASS FWinCamera
#include "InitGuid.h"	/* Initialize GUID's */

#include "Unreal.h"
#include "UnWnCam.h"
#include "UnRenDev.h"
#include "DMouse.h"
#include "UnInput.h"
#include "UnConfig.h"
#include "UnAction.h"

FVector FMeshViewStartLocation = {100.0,100.0,+60.0};

static inline BOOL PlayingGame(DWORD ShowFlags) { return GEditor == 0 || (ShowFlags&SHOW_PlayerCtrl)!=0; }

//----------------------------------------------------------------------------
//                Mouse management help
//----------------------------------------------------------------------------
// We save the previous mouse position so we can interpret mouse
// movements (changes). If PreviousMousePoint.x < 0, then the
// previous position is not available yet.
static POINT PreviousMousePoint; // Position in client area.

static void ResetMousePosition(HWND Window)
{
    GetCursorPos( &PreviousMousePoint ); // Gets position in screen coordinates
    ScreenToClient( Window, &PreviousMousePoint ); // Convert to client coordinates
}

static HWND StartedClippingCursorInWindow; //tbi: Pretty gross (find out where the window handle is kept!)

//todo: Move this into FWindowsCameraManager, like StopClippingCursor
static void StartClippingCursor(FWindowsCameraManager & Manager, UCamera * Camera, HWND Window )
{
    StartedClippingCursorInWindow = Window;
    POINT Point;
    GetCursorPos(&Point);
    Camera->Win.SaveCursor = Point;
    RECT Region;
    GetClientRect(Window,&Region);
    MapWindowPoints(Window,NULL,(POINT*)&Region, 2); // Get screen coords of this window
    SetCursorPos((Region.left+Region.right)/2,(Region.top+Region.bottom)/2);
    {
        ResetMousePosition(Window);
    }
    ClipCursor(&Region); // Confine cursor to window
    ShowCursor(FALSE);
    if( !Manager.FullscreenCamera ) 
    {
        SetCapture(Window);
    }
    if(Manager.UseDirectMouse) 
    {
        Manager.dmStart();
    }
}

/*-----------------------------------------------------------------------------
	FWindowsCameraManager constructor
-----------------------------------------------------------------------------*/

//
// Constructor.  Initializes all vital info that doesn't have
// to be validated.  Not guarded.
//
FWindowsCameraManager::FWindowsCameraManager()
	{
	Initialized		= 0;
	UseDirectMouse	= 0;
	UseDirectDraw	= 0;
	InMenuLoop		= 0;
	//
	dd				= NULL;
	ddFrontBuffer	= NULL;
	ddBackBuffer	= NULL;
	ddPalette		= NULL;
	ddNumModes		= 0;
	//
	hMemScreenDC	= NULL;
	DMouseHandle	= NULL;
	};

/*--------------------------------------------------------------------------------
	FWindowsCameraManager general utility functions
--------------------------------------------------------------------------------*/

//
// Return the current camera.  Returns NULL if no camera has focus.
//
UCamera *FWindowsCameraManager::CurrentCamera(void)
	{
	GUARD;
	//
	UCamera *TestCamera;
	if (!CameraArray->Num) return NULL; // No cameras exist
	//
	int i;
	for (i=0; i<CameraArray->Num; i++)
	   	{
		TestCamera = CameraArray->Element(i);
     	if (TestCamera->Current) break;
		};
	if (i>=CameraArray->Num) return NULL;
	//
	HWND hWndActive = GetActiveWindow();
	if	(
		(hWndActive==FullscreenhWndDD) ||
		(hWndActive==TestCamera->Win.hWndCamera)
		)
		{
		return TestCamera;
		}
	else return NULL;
	//
	UNGUARD("FWindowsCameraManager::CurrentCamera");
	};

//
// Make this camera the current.
//
void FWindowsCameraManager::MakeCurrent(UCamera *Camera)
	{
	GUARD;
	//
	Camera->Current = 1;
	//
	for (int i=0; i<CameraArray->Num; i++)
		{
		UCamera *OldCamera = CameraArray->Element(i);
		if (OldCamera->Current && (OldCamera!=Camera))
			{
			OldCamera->Current = 0;
			OldCamera->UpdateWindow();
			};
		};
	Camera->UpdateWindow();
	UNGUARD("FCameraManager::MakeCurrent");
	};

//
// Try to make this camera fullscreen, matching the fullscreen
// mode of the nearest x-size to the current window. If already in
// fullscreen, returns to non-fullscreen.
//
void FWindowsCameraManager::MakeFullscreen(UCamera *Camera)
	{
	GUARD;
	//
	if (FullscreenCamera) // Go back to running in a window
		{
		EndFullscreen();
		}
	else // Go into fullscreen, matching closest DirectDraw mode to current window size
		{
		int BestMode=-1;
		int BestDelta=MAXINT;
		for (int i=0; i<ddNumModes; i++)
			{
			int Delta = OurAbs(ddModeWidth[i]-Camera->SXR);
			if (Delta < BestDelta)
				{
				BestMode  = i;
				BestDelta = Delta;
				};
			};
		if (BestMode>=0)
			{
			ddSetCamera
				(
				Camera,
				ddModeWidth [BestMode],
				ddModeHeight[BestMode],
				IsHardware3D(Camera) ? 2 : Camera->ColorBytes,
				IsHardware3D(Camera) ? CC_Hardware3D : 0
				);
			};
		};
	UNGUARD("FCameraManager::MakeFullscreen");
	};

int	FWindowsCameraManager::IsHardware3D(UCamera *Camera)
	{
	//return GApp->RenDev && GetMenuState(GetMenu(Camera->Win.hWndCamera),ID_HARDWARE_3D,MF_BYCOMMAND)&MF_CHECKED;
	return 0;
	};

//
// Return 1 if the cursor is captured, 0 if not.  If the cursor is captured,
// no Windows UI interaction is taking place.  Otherwise, windows UI interaction
// may be taking place.
//
int FWindowsCameraManager::CursorIsCaptured()
	{
	GUARD;
	return (FullscreenhWndDD || GetCapture());
	UNGUARD("FWindowsCameraManager::CursorIsCaptured");
	};

//
// Save the camera's current window placement.
//
void FWindowsCameraManager::SaveFullscreenWindowRect(UCamera *Camera)
	{
	GUARD;
	//
	GetWindowRect (Camera->Win.hWndCamera,&Camera->Win.SavedWindowRect);
	//
	Camera->Win.SavedColorBytes = Camera->ColorBytes;
	Camera->Win.SavedCaps		= Camera->Caps;
	Camera->Win.SavedSXR		= Camera->SXR;
	Camera->Win.SavedSYR		= Camera->SYR;
	//
	UNGUARD("SaveFullscreenWindowRect");
	};

//
// Toggle a menu item and return 0 if it's now off, 1 if it's now on.
//
int FWindowsCameraManager::Toggle (HMENU hMenu,int Item)
	{
	GUARD;
	//
	if (GetMenuState(hMenu,Item,MF_BYCOMMAND)&MF_CHECKED)
		{
		CheckMenuItem(hMenu,Item,MF_UNCHECKED);
		return 0; // Now unchecked
		}
	else
		{
		CheckMenuItem(hMenu,Item,MF_CHECKED);
		return 1; // Now checked
		};
	UNGUARD("Toggle");
	};

//
// Reset all stored/cached information in a camera.
//
void FWindowsCameraManager::ResetModes(void)
	{
	GUARD;
	//
	StoredMove.X		= 0.0;
	StoredMove.Y		= 0.0;
	StoredMove.Z		= 0.0;
	//
	StoredRot.Pitch		= 0.0;
	StoredRot.Yaw		= 0.0;
	StoredRot.Roll		= 0.0;
	//
	UNGUARD("FWindowsCameraManager::ResetModes");
	};

//
// Init all fullscreen globals to defaults.
//
void FWindowsCameraManager::InitFullscreen(void)
	{
	GUARD;
	FullscreenCamera 		= NULL;
	FullscreenhWndDD		= NULL;
	UNGUARD("FWindowsCameraManager::InitFullscreen");
	};

//
// Store some mouse movement for later retrieval by the game.
//
void FWindowsCameraManager::StoreMove(FLOAT MouseX,FLOAT MouseY,int LButton,int MButton,int RButton)
	{
	if (FullscreenhWndDD && !(LButton||RButton||MButton))
		{
		LButton=1;
		};
	if (LButton && RButton)
		{
		// Both buttons: Vertical
		StoredMove.Z  		-= MouseY * 0.30;
		StoredMove.Y  		+= MouseX * 0.25;
		}
	else if (RButton)
		{
		// Right button: Look around
		StoredRot.Pitch    -= MouseY * 1.0;
		StoredRot.Yaw      += MouseX * 0.7;
		}
	else if (LButton)
		{
		// Left button: move ahead and yaw (normal movement)
		StoredMove.X  	   -= MouseY * 0.30;
		StoredRot.Yaw 	   += MouseX * 0.7;
		};
	};

/*-----------------------------------------------------------------------------
	FGlobalPlatform Command line
-----------------------------------------------------------------------------*/

int FWindowsCameraManager::Exec(const char *Cmd,FOutputDevice *Out)
	{
	GUARD;
	const char *Str = Cmd;
	//
	if (GetCMD(&Str,"STATUS"))
		{
		if (GetCMD(&Str,"CAMERAMAN") || !Str[0])
			{
			Out->Logf("   CAMERAMAN - %i camera(s)",CameraArray->Num);
			return Str[0]!=0;
			}
		else return 0;
		}
	else if (GetCMD(&Str,"CAMERA"))
		{
		if (GetCMD(&Str,"LIST"))
			{
			Out->Log("Cameras:");
			for (int i=0; i<CameraArray->Num; i++)
				{
				UCamera *Camera = CameraArray->Element(i);
				Out->Logf("   %s (%ix%i)",Camera->Name,Camera->SXR,Camera->SYR);
				};
			return 1;
			}
		else if (GetCMD(&Str,"OPEN")) // CAMERA OPEN
			{
			ULevel *Level = GServer.Levels->Num ? GServer.Levels->Element(0) : NULL;
			if (Level)
				{
				UCamera *Camera;
				int Temp=0;
				char TempStr[NAME_SIZE];
				if (GetSTRING(Str,"NAME=",TempStr,NAME_SIZE))
					{
					Camera = new(TempStr,FIND_Optional)UCamera;
					if (!Camera) Camera = new(TempStr,CREATE_Unique)UCamera(Level);
					else Temp=1;
					}
				else Camera = new(NULL,CREATE_Unique)UCamera(Level);
				//
				AActor *CameraActor  = &Camera->GetActor();
				//
				DWORD hWndParent=0; GetDWORD(Str,"HWND=",&hWndParent);
				//
				GetINT (Str,"X=", &Camera->OpenX);
				GetINT (Str,"Y=", &Camera->OpenY);
				GetINT (Str,"XR=",&Camera->SXR);
				GetINT (Str,"YR=",&Camera->SYR);
				GetFLOAT(Str,"FOV=",&CameraActor->CameraStatus.FOVAngle);
				//
				if (Camera->SXR<0) Camera->SXR=0;
				if (Camera->SYR<0) Camera->SYR=0;
				//
				CameraActor->CameraStatus.Misc1=0;
				CameraActor->CameraStatus.Misc2=0;
				GetWORD(Str,"FLAGS=",&CameraActor->CameraStatus.ShowFlags);
				GetWORD(Str,"REN=",  &CameraActor->CameraStatus.RendMap);
				GetWORD(Str,"MISC1=",&CameraActor->CameraStatus.Misc1);
				GetWORD(Str,"MISC2=",&CameraActor->CameraStatus.Misc2);
				//
				if ((CameraActor->CameraStatus.RendMap==REN_MeshView)&&!Temp)
					{
					CameraActor->Location = FMeshViewStartLocation;
					CameraActor->ViewRot.Yaw=0x6000;
					};
				switch (CameraActor->CameraStatus.RendMap)
					{
					case REN_TexView:
						GetUTexture(Str,"TEXTURE=",(UTexture **)&Camera->MiscRes); 
						Camera->ColorBytes = 1;
						break;
					case REN_MeshView:
						GetUMeshMap(Str,"MESH=",(UMeshMap **)&Camera->MiscRes); 
						Camera->ColorBytes = 1;
						break;
					case REN_TexBrowser:
						GetNAME(Str,"FAMILY=",&Camera->MiscName); 
						Camera->ColorBytes = 1;
						break;
					case REN_MeshBrowser:
						GetNAME(Str,"FAMILY=",	&Camera->MiscName); 
						Camera->ColorBytes = 1;
						break;
					default:
						if(Camera->IsOrtho()) Camera->ColorBytes = 1;
						break;
					};
				Camera->OpenWindow(hWndParent,0);
				}
			else Out->Log("Can't find level");
			return 1;
			}
		else if (GetCMD(&Str,"HIDESTANDARD")) // CAMERA HIDESTANDARD
			{
			GCameraManager->ShowCameraWindows(SHOW_StandardView,0);
			return 1;
			}
		else if (GetCMD(&Str,"CLOSE")) // CAMERA CLOSE
			{
			UCamera *Camera;
			if (GetCMD(&Str,"ALL")) // CAMERA CLOSE ALL
				{
				GCameraManager->CloseWindowChildren(0);
				}
			else if (GetCMD(&Str,"FREE")) // CAMERA CLOSE FREE
				{
				GCameraManager->CloseWindowChildren(MAXDWORD);
				}
			else if (GetUCamera(Str,"NAME=",&Camera))
				{
				Camera->Kill();
				}
			else Out->Log("Missing name");
			return 1;
			}
		else return 0;
		}
	else return 0; // Not executed
	//
	UNGUARD("FWindowsCameraManager::Exec");
	};

/*-----------------------------------------------------------------------------
	DirectDraw support
-----------------------------------------------------------------------------*/

//
// Return a DirectDraw error message.
// Error messages commented out are DirectDraw II error messages.
//
const char *FWindowsCameraManager::ddError(HRESULT Result)
	{
	GUARD;
	switch (Result)
		{
		case DD_OK:									return "DD_OK";
		case DDERR_ALREADYINITIALIZED:				return "DDERR_ALREADYINITIALIZED";
		case DDERR_BLTFASTCANTCLIP:					return "DDERR_BLTFASTCANTCLIP";
		case DDERR_CANNOTATTACHSURFACE:				return "DDERR_CANNOTATTACHSURFACE";
		case DDERR_CANNOTDETACHSURFACE:				return "DDERR_CANNOTDETACHSURFACE";
		case DDERR_CANTCREATEDC:					return "DDERR_CANTCREATEDC";
		case DDERR_CANTDUPLICATE:					return "DDERR_CANTDUPLICATE";
		case DDERR_CLIPPERISUSINGHWND:				return "DDERR_CLIPPERISUSINGHWND";
		case DDERR_COLORKEYNOTSET:					return "DDERR_COLORKEYNOTSET";
		case DDERR_CURRENTLYNOTAVAIL:				return "DDERR_CURRENTLYNOTAVAIL";
		case DDERR_DIRECTDRAWALREADYCREATED:		return "DDERR_DIRECTDRAWALREADYCREATED";
		case DDERR_EXCEPTION:						return "DDERR_EXCEPTION";
		case DDERR_EXCLUSIVEMODEALREADYSET:			return "DDERR_EXCLUSIVEMODEALREADYSET";
		case DDERR_GENERIC:							return "DDERR_GENERIC";
		case DDERR_HEIGHTALIGN:						return "DDERR_HEIGHTALIGN";
		case DDERR_HWNDALREADYSET:					return "DDERR_HWNDALREADYSET";
		case DDERR_HWNDSUBCLASSED:					return "DDERR_HWNDSUBCLASSED";
		case DDERR_IMPLICITLYCREATED:				return "DDERR_IMPLICITLYCREATED";
		case DDERR_INCOMPATIBLEPRIMARY:				return "DDERR_INCOMPATIBLEPRIMARY";
		case DDERR_INVALIDCAPS:						return "DDERR_INVALIDCAPS";
		case DDERR_INVALIDCLIPLIST:					return "DDERR_INVALIDCLIPLIST";
		case DDERR_INVALIDDIRECTDRAWGUID:			return "DDERR_INVALIDDIRECTDRAWGUID";
		case DDERR_INVALIDMODE:						return "DDERR_INVALIDMODE";
		case DDERR_INVALIDOBJECT:					return "DDERR_INVALIDOBJECT";
		case DDERR_INVALIDPARAMS:					return "DDERR_INVALIDPARAMS";
		case DDERR_INVALIDPIXELFORMAT:				return "DDERR_INVALIDPIXELFORMAT";
		case DDERR_INVALIDPOSITION:					return "DDERR_INVALIDPOSITION";
		case DDERR_INVALIDRECT:						return "DDERR_INVALIDRECT";
		case DDERR_LOCKEDSURFACES:					return "DDERR_LOCKEDSURFACES";
		case DDERR_NO3D:							return "DDERR_NO3D";
		case DDERR_NOALPHAHW:						return "DDERR_NOALPHAHW";
		case DDERR_NOBLTHW:							return "DDERR_NOBLTHW";
		case DDERR_NOCLIPLIST:						return "DDERR_NOCLIPLIST";
		case DDERR_NOCLIPPERATTACHED:				return "DDERR_NOCLIPPERATTACHED";
		case DDERR_NOCOLORCONVHW:					return "DDERR_NOCOLORCONVHW";
		case DDERR_NOCOLORKEY:						return "DDERR_NOCOLORKEY";
		case DDERR_NOCOLORKEYHW:					return "DDERR_NOCOLORKEYHW";
		case DDERR_NOCOOPERATIVELEVELSET:			return "DDERR_NOCOOPERATIVELEVELSET";
		case DDERR_NODC:							return "DDERR_NODC";
		case DDERR_NODDROPSHW:						return "DDERR_NODDROPSHW";
		case DDERR_NODIRECTDRAWHW:					return "DDERR_NODIRECTDRAWHW";
		case DDERR_NOEMULATION:						return "DDERR_NOEMULATION";
		case DDERR_NOEXCLUSIVEMODE:					return "DDERR_NOEXCLUSIVEMODE";
		case DDERR_NOFLIPHW:						return "DDERR_NOFLIPHW";
		case DDERR_NOGDI:							return "DDERR_NOGDI";
		case DDERR_NOHWND:							return "DDERR_NOHWND";
		case DDERR_NOMIRRORHW:						return "DDERR_NOMIRRORHW";
		case DDERR_NOOVERLAYDEST:					return "DDERR_NOOVERLAYDEST";
		case DDERR_NOOVERLAYHW:						return "DDERR_NOOVERLAYHW";
		case DDERR_NOPALETTEATTACHED:				return "DDERR_NOPALETTEATTACHED";
		case DDERR_NOPALETTEHW:						return "DDERR_NOPALETTEHW";
		case DDERR_NORASTEROPHW:					return "DDERR_NORASTEROPHW";
		case DDERR_NOROTATIONHW:					return "DDERR_NOROTATIONHW";
		case DDERR_NOSTRETCHHW:						return "DDERR_NOSTRETCHHW";
		case DDERR_NOT4BITCOLOR:					return "DDERR_NOT4BITCOLOR";
		case DDERR_NOT4BITCOLORINDEX:				return "DDERR_NOT4BITCOLORINDEX";
		case DDERR_NOT8BITCOLOR:					return "DDERR_NOT8BITCOLOR";
		case DDERR_NOTAOVERLAYSURFACE:				return "DDERR_NOTAOVERLAYSURFACE";
		case DDERR_NOTEXTUREHW:						return "DDERR_NOTEXTUREHW";
		case DDERR_NOTFLIPPABLE:					return "DDERR_NOTFLIPPABLE";
		case DDERR_NOTFOUND:						return "DDERR_NOTFOUND";
		case DDERR_NOTLOCKED:						return "DDERR_NOTLOCKED";
		case DDERR_NOTPALETTIZED:					return "DDERR_NOTPALETTIZED";
		case DDERR_NOVSYNCHW:						return "DDERR_NOVSYNCHW";
		case DDERR_NOZBUFFERHW:						return "DDERR_NOZBUFFERHW";
		case DDERR_NOZOVERLAYHW:					return "DDERR_NOZOVERLAYHW";
		case DDERR_OUTOFCAPS:						return "DDERR_OUTOFCAPS";
		case DDERR_OUTOFMEMORY:						return "DDERR_OUTOFMEMORY";
		case DDERR_OUTOFVIDEOMEMORY:				return "DDERR_OUTOFVIDEOMEMORY";
		case DDERR_OVERLAYCANTCLIP:					return "DDERR_OVERLAYCANTCLIP";
		case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:	return "DDERR_OVERLAYCOLORKEYONLYONEACTIVE";
		case DDERR_OVERLAYNOTVISIBLE:				return "DDERR_OVERLAYNOTVISIBLE";
		case DDERR_PALETTEBUSY:						return "DDERR_PALETTEBUSY";
		case DDERR_PRIMARYSURFACEALREADYEXISTS:		return "DDERR_PRIMARYSURFACEALREADYEXISTS";
		case DDERR_REGIONTOOSMALL:					return "DDERR_REGIONTOOSMALL";
		case DDERR_SURFACEALREADYATTACHED:			return "DDERR_SURFACEALREADYATTACHED";
		case DDERR_SURFACEALREADYDEPENDENT:			return "DDERR_SURFACEALREADYDEPENDENT";
		case DDERR_SURFACEBUSY:						return "DDERR_SURFACEBUSY";
		case DDERR_SURFACEISOBSCURED:				return "DDERR_SURFACEISOBSCURED";
		case DDERR_SURFACELOST:						return "DDERR_SURFACELOST";
		case DDERR_SURFACENOTATTACHED:				return "DDERR_SURFACENOTATTACHED";
		case DDERR_TOOBIGHEIGHT:					return "DDERR_TOOBIGHEIGHT";
		case DDERR_TOOBIGSIZE:						return "DDERR_TOOBIGSIZE";
		case DDERR_TOOBIGWIDTH:						return "DDERR_TOOBIGWIDTH";
		case DDERR_UNSUPPORTED:						return "DDERR_UNSUPPORTED";
		case DDERR_UNSUPPORTEDFORMAT:				return "DDERR_UNSUPPORTEDFORMAT";
		case DDERR_UNSUPPORTEDMASK:					return "DDERR_UNSUPPORTEDMASK";
		case DDERR_UNSUPPORTEDMODE:					return "DDERR_UNSUPPORTEDMODE";
		case DDERR_VERTICALBLANKINPROGRESS:			return "DDERR_VERTICALBLANKINPROGRESS";
		case DDERR_WASSTILLDRAWING:					return "DDERR_WASSTILLDRAWING";
		case DDERR_WRONGMODE:						return "DDERR_WRONGMODE";
		case DDERR_XALIGN:							return "DDERR_XALIGN";
#ifdef DDRAW_2
		case DDERR_CANTPAGELOCK:					return "DDERR_CANTPAGELOCK";
		case DDERR_CANTPAGEUNLOCK:					return "DDERR_CANTPAGEUNLOCK";
		case DDERR_DCALREADYCREATED:				return "DDERR_DCALREADYCREATED";
		case DDERR_INVALIDSURFACETYPE:				return "DDERR_INVALIDSURFACETYPE";
		case DDERR_NOMIPMAPHW:						return "DDERR_NOMIPMAPHW";
		case DDERR_NOTPAGELOCKED:					return "DDERR_NOTPAGELOCKED";
		case DDERR_CANTLOCKSURFACE:					return "DDERR_CANTLOCKSURFACE";
#endif
		default:									return "Unknown error";
		};
	UNGUARD("FWindowsCameraManager::ddError");
	};

//
// DirectDraw mode enumeration callback.
//
HRESULT WINAPI ddEnumModesCallback(DDSURFACEDESC *SurfaceDesc,void *Context)
	{
	GUARD;
	//
	FWindowsCameraManager *CameraManager = (FWindowsCameraManager *)Context;
	if (CameraManager->ddNumModes < CameraManager->DD_MAX_MODES)
		{
		if (SurfaceDesc->dwWidth>800) goto SkipThisMode; // Unreasonably high-res
		for (int i=0; i<CameraManager->ddNumModes; i++)
			{
			if (((DWORD)CameraManager->ddModeWidth [i]==SurfaceDesc->dwWidth) &&
				((DWORD)CameraManager->ddModeHeight[i]==SurfaceDesc->dwHeight))
				goto SkipThisMode; // Duplicate
			};
		CameraManager->ddModeWidth [CameraManager->ddNumModes] = SurfaceDesc->dwWidth;
		CameraManager->ddModeHeight[CameraManager->ddNumModes] = SurfaceDesc->dwHeight;
		CameraManager->ddNumModes++;
		};
	SkipThisMode:
	return DDENUMRET_OK;
	//
	UNGUARD("ddEnumModesCallback");
	};

//
// Find all available DirectDraw modes for a certain number of color bytes.
//
void FWindowsCameraManager::FindAvailableModes(UCamera *Camera)
	{
	GUARD;
	//
	ddNumModes=0;
	if (dd)
		{
		HWND hWndFocus = GetFocus(); // Prevent SetCooperativeLevel from changing focus
		//
		HRESULT Result;
		Result = dd->SetCooperativeLevel(App.Dialog->m_hWnd,DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX | DDSCL_ALLOWREBOOT | DDSCL_NOWINDOWCHANGES);
		if (Result!=DD_OK) appErrorf("SetCooperativeLevel failed 1: %s",ddError(Result));
		//
		DDSURFACEDESC SurfaceDesc; 
		memset(&SurfaceDesc,0,sizeof(DDSURFACEDESC));
		//
		SurfaceDesc.dwSize		= sizeof(DDSURFACEDESC);
		SurfaceDesc.dwFlags		= DDSD_PIXELFORMAT;
		//
		SurfaceDesc.ddpfPixelFormat.dwSize			= sizeof(DDPIXELFORMAT);
		SurfaceDesc.ddpfPixelFormat.dwFlags			= (Camera->ColorBytes==1) ? DDPF_PALETTEINDEXED8 : DDPF_RGB;
		SurfaceDesc.ddpfPixelFormat.dwRGBBitCount   = Camera->ColorBytes*8;
		//
		dd->EnumDisplayModes(0,&SurfaceDesc,this,ddEnumModesCallback);
		//
		dd->SetCooperativeLevel(App.Dialog->m_hWnd,DDSCL_NORMAL);
		if (Result!=DD_OK) appErrorf("SetCooperativeLevel failed 2: %s",ddError(Result));
		//
		SetFocus(hWndFocus);
		};
	if (Camera->Win.hWndCamera)
		{
		HMENU hMenu  = GetMenu(Camera->Win.hWndCamera);
		if (!hMenu) appErrorf("GetMenu failed %i",GetLastError());
		//
		int nMenu;
		if (GDefaults.LaunchEditor)	nMenu = 3; // Is there a better way to do this?
		else if (GNetManager)		nMenu = 3;
		else						nMenu = 2;
		//
		HMENU hSizes = GetSubMenu(hMenu,nMenu);
		if (!hSizes) goto Done;
		//
		// Completely rebuild the "Size" submenu based on what modes are available:
		//
		int n=GetMenuItemCount(hSizes);
		for (int i=0; i<n; i++) if (!DeleteMenu(hSizes,0,MF_BYPOSITION)) appErrorf("DeleteMenu failed %i",GetLastError());
		//
		AppendMenu(hSizes,MF_STRING,ID_COLOR_8BIT,"&8-bit color");
		AppendMenu(hSizes,MF_STRING,ID_COLOR_16BIT,"&16-bit color");
		AppendMenu(hSizes,MF_STRING,ID_COLOR_32BIT,"&32-bit color");
		//
		if (!(Camera->GetActor().CameraStatus.ShowFlags & SHOW_ChildWindow))
			{
			AppendMenu(hSizes,MF_SEPARATOR,0,NULL);
			//
			AppendMenu(hSizes,MF_STRING,ID_WIN_320,"320x200");
			AppendMenu(hSizes,MF_STRING,ID_WIN_400,"400x300");
			AppendMenu(hSizes,MF_STRING,ID_WIN_512,"512x384");
			AppendMenu(hSizes,MF_STRING,ID_WIN_640,"640x400");
			//
			OSVERSIONINFO Version; Version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			GetVersionEx(&Version);
			if (ddNumModes>0) AppendMenu(hSizes,MF_SEPARATOR,0,NULL);
			for (i=0; i<ddNumModes; i++)
				{
				char Text[256];
				sprintf (Text,"DirectDraw %ix%i",ddModeWidth[i],ddModeHeight[i]);
				//
				if (!AppendMenu(hSizes,MF_STRING,ID_DDMODE_0+i,Text)) appErrorf("AppendMenu failed %i",GetLastError());
				};
			DrawMenuBar(Camera->Win.hWndCamera);
			};
		};
	Done:;
	UNGUARD("FWindowsCameraManager::FindAvailableModes");
	};

//
// DirectDraw driver enumeration callback.
//
BOOL WINAPI ddEnumDriversCallback(GUID *GUID, char *DriverDescription, char *DriverName,
    void *Context)
	{
	GUARD;
	//
	FWindowsCameraManager *CameraManager = (FWindowsCameraManager *)Context;
	debugf(LOG_Win,"   %s (%s)",DriverName,DriverDescription);
	return DDENUMRET_OK;
	//
	UNGUARD("ddEnumDriversCallback");
	};
//
// Init all DirectDraw stuff.
//
int FWindowsCameraManager::ddInit(void)
	{
	GUARD;
	//
	HINSTANCE		Instance;
	HRESULT 		Result;
	//
	// Load DirectDraw DLL
	//
	Instance = LoadLibrary("ddraw.dll");
	if (Instance==NULL)
		{
		debug(LOG_Init,"DirectDraw not installed");
		return 0;
		};
	ddCreateFunc = (DD_CREATE_FUNC)GetProcAddress(Instance,"DirectDrawCreate");
	ddEnumFunc   = (DD_ENUM_FUNC  )GetProcAddress(Instance,"DirectDrawEnumerateA"); // Non-unicode
	if (!(ddCreateFunc && ddEnumFunc))
		{
		debug(LOG_Init,"DirectDraw GetProcAddress failed");
		return 0;
		};
	//
	// Init direct draw and see if it's available
	//
	IDirectDraw *dd1;
	Result = (*ddCreateFunc)(NULL, &dd1, NULL);
	if (Result != DD_OK)
		{
		debugf(LOG_Init,"DirectDraw created failed: %s",ddError(Result));
   		return 0;
		};
	#ifdef DDRAW_1
		dd = dd1;
	#else
		Result = dd1->QueryInterface(IID_IDirectDraw2,(void**)&dd);
		if (Result != DD_OK)
			{
			dd1->Release();
			debugf(LOG_Init,"DirectDraw2 interface not available");
   			return 0;
			};
	#endif
	debug(LOG_Init,"DirectDraw initialized successfully");
	//
	// Find out DirectDraw capabilities:
	//
	DDCAPS D; ZeroMemory(&D,sizeof(D)); D.dwSize=sizeof(D);
	DDCAPS E; ZeroMemory(&E,sizeof(E)); E.dwSize=sizeof(E);
	//
	Result = dd->GetCaps(&D,&E); 
	if (Result!=DD_OK) appErrorf("DirectDraw GetCaps failed: %s",ddError(Result));
	//
	char Caps[256]="DirectDraw caps:"; 
	if (D.dwCaps  & DDCAPS_NOHARDWARE)		strcat(Caps," NOHARD");
	if (D.dwCaps  & DDCAPS_BANKSWITCHED)	strcat(Caps," BANKED");
	if (D.dwCaps  & DDCAPS_3D)				strcat(Caps," 3D");
	if (D.dwCaps  & DDCAPS_BLTFOURCC)		strcat(Caps," FOURCC");
	if (D.dwCaps  & DDCAPS_GDI)				strcat(Caps," GDI");
	if (D.dwCaps  & DDCAPS_PALETTEVSYNC)	strcat(Caps," PALVSYNC");
	if (D.dwCaps  & DDCAPS_VBI)				strcat(Caps," VBI");
	if (D.dwCaps2 & DDCAPS2_CERTIFIED)		strcat(Caps," CERTIFIED");
	sprintf(Caps+strlen(Caps)," MEM=%i",D.dwVidMemTotal);
	debug(LOG_Init,Caps);
	//
	// Show available DirectDraw drivers. This is useful in analyzing the
	// log for the cause of errors.
	//
	debug(LOG_Win,"DirectDraw drivers:");
	ddEnumFunc(ddEnumDriversCallback,this);
	//
	return 1;
	//
	UNGUARD("FWindowsCameraManager::ddInit");
	};

//
// Set DirectDraw to a particular mode, with full error checking
// Returns 1 if success, 0 if failure.
//
int FWindowsCameraManager::ddSetMode(HWND hWndParent,int Width, int Height,int ColorBytes,int &Caps)
	{
	GUARD;
	//
	HRESULT 		Result;
	DDSCAPS 		caps;
	HDC 			hDC;
	char			*Descr;
	//
	if (!dd)
		{
		debug (LOG_Win,"DirectDraw: Not initialized");
		return 0; // DirectDraw not available or not installed
		};
	//
	// Grab exclusive access to DirectDraw:
	//
	GAudio.Pause();
	Result = dd->SetCooperativeLevel(hWndParent,DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX | DDSCL_ALLOWREBOOT);
	if (Result != DD_OK)
		{
		debugf(LOG_Win,"DirectDraw SetCooperativeLevel: %s",ddError(Result));
		GAudio.UnPause();
   		return 0;
		};
	debugf(LOG_Info,"Setting %ix%i %i",Width,Height,ColorBytes*8);
	#ifdef DDRAW_1
		Result = dd->SetDisplayMode (Width,Height,ColorBytes*8);
	#else
		Result = dd->SetDisplayMode (Width,Height,ColorBytes*8,0,0);
	#endif
	if (Result != DD_OK)
		{
		debugf(LOG_Win,"DirectDraw Failed %ix%ix%i: %s",Width,Height,ColorBytes,ddError(Result));
		ddEndMode();
		GAudio.UnPause();
   		return 0;
		};
	//
	// Create surfaces
	//
	DDSURFACEDESC SurfaceDesc;
	memset(&SurfaceDesc,0,sizeof(DDSURFACEDESC));
	//
	SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
	SurfaceDesc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	SurfaceDesc.dwBackBufferCount = 2;
	SurfaceDesc.ddsCaps.dwCaps    =
		DDSCAPS_PRIMARYSURFACE |
		DDSCAPS_FLIP           |
		DDSCAPS_COMPLEX        |
		DDSCAPS_VIDEOMEMORY;
	if (Width==320)
		{
		SurfaceDesc.ddsCaps.dwCaps |= DDSCAPS_MODEX;
		}
	else
		{
		Result = dd->CreateSurface(&SurfaceDesc, &ddFrontBuffer, NULL); // Try triple-buffered video memory surface
		Descr  = "Triple buffer";
		};
	if ((Width==320) || (Result != DD_OK))
   		{
		// Try to get a double buffered video memory surface
		SurfaceDesc.dwBackBufferCount = 1; 
		Result = dd->CreateSurface(&SurfaceDesc, &ddFrontBuffer, NULL);
		Descr  = "Double buffer";
    	};
	if (Result != DD_OK)
	   	{
		// Settle for a main memory surface
		SurfaceDesc.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
		Result = dd->CreateSurface(&SurfaceDesc, &ddFrontBuffer, NULL);
		Descr  = "System memory";
    	};
	if (Result != DD_OK)
		{
		debugf(LOG_Win,"DirectDraw, no available modes %s",ddError(Result));
		ddEndMode();
		GAudio.UnPause();
	   	return 0;
		};
	debugf (LOG_Win,"DirectDraw: %s, %ix%i, Stride=%i",Descr,Width,Height,SurfaceDesc.lPitch);
	//
	#ifdef DDRAW_2
	debugf (LOG_Win,"DirectDraw: Rate=%i",SurfaceDesc.dwRefreshRate);
	#endif
	//
	// Get a pointer to the back buffer
	//
	caps.dwCaps = DDSCAPS_BACKBUFFER;
	if (ddFrontBuffer->GetAttachedSurface(&caps, &ddBackBuffer) != DD_OK)
		{
		debugf(LOG_Win,"DirectDraw GetAttachedSurface failed %s",ddError(Result));
		ddEndMode();
		GAudio.UnPause();
		};
	//
	// Get pixel format:
	//
	DDPIXELFORMAT PixelFormat;
	PixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	Result = ddFrontBuffer->GetPixelFormat(&PixelFormat);
	if (Result != DD_OK)
		{
		ddEndMode();
		appErrorf("DirectDraw GetPixelFormat failed: %s",ddError(Result));
		};
	if ((ColorBytes==2) && (PixelFormat.dwRBitMask==0xf800)) Caps |= CC_RGB565;
	else Caps &= ~CC_RGB565;
	//
	// Create a palette if we are in a paletized display mode.
	//
	hDC = GetDC(NULL);
	if (GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE)
   		{
		PALETTEENTRY Temp[256];
		Result = dd->CreatePalette(DDPCAPS_8BIT, Temp, &ddPalette, NULL);
		if (Result != DD_OK)
			{
			ddEndMode();
			appErrorf("DirectDraw CreatePalette failed: %s",ddError(Result));
			};
		Result = ddFrontBuffer->SetPalette(ddPalette);
		if (Result != DD_OK)
			{
			ddEndMode();
			appErrorf("DirectDraw SetPalette failed: %s",ddError(Result));
			};
		if (!(SurfaceDesc.dwFlags & DDPF_PALETTEINDEXED8))
			{
			ddEndMode();
			appError("Palette not expected");
			};
    	}
	else if (ColorBytes==1)
		{
		ddEndMode();
		appError("Palette expected");
		};
	ReleaseDC(NULL, hDC);
	//
	// Init hardware if we're 3D accelerated:
	//
	/**/
	//
	GAudio.UnPause();
	return 1;
	//
	UNGUARD("FWindowsCameraManager::ddSetMode");
	};

//
// End the current DirectDraw mode.
//
void FWindowsCameraManager::ddEndMode(void)
	{
	GUARD;
	//
	HRESULT Result;
	debugf(LOG_Win,"DirectDraw End Mode");
	if (dd)
		{
		//
		// Release all buffers
		//
		if (ddBackBuffer)  {ddBackBuffer->Release();     ddBackBuffer = NULL;};
		if (ddFrontBuffer) {ddFrontBuffer->Release();    ddFrontBuffer = NULL;};
		if (ddPalette)     {ddPalette->Release();        ddPalette = NULL;};
		//
		Result = dd->SetCooperativeLevel (App.Dialog->m_hWnd,DDSCL_NORMAL);
		if (Result!=DD_OK) debugf(LOG_Win,"DirectDraw SetCooperativeLevel: %s",ddError(Result));
		//
		Result = dd->RestoreDisplayMode();
		if (Result!=DD_OK) debugf(LOG_Win,"DirectDraw RestoreDisplayMode: %s",ddError(Result));
		//
		Result = dd->FlipToGDISurface(); // Ignore error (this is ok)
		//
		SetCapture (NULL);
		if (ShowCursor(TRUE)>0) ShowCursor(FALSE);
		};
	UNGUARD("FWindowsCameraManager::ddEndMode");
	};

//
// Shut DirectDraw down.
//
void FWindowsCameraManager::ddExit()
	{
	GUARD;
	HRESULT Result;
	if (dd)
		{
		ddEndMode();
		Result = dd->Release();
		if (Result != DD_OK) debugf(LOG_Exit,"DirectDraw Release failed: %s",ddError(Result));
		else debug(LOG_Exit,"DirectDraw released");
		dd = NULL;
		};
	UNGUARD("ddExit");
	};

/*-----------------------------------------------------------------------------
	DirectMouse support
-----------------------------------------------------------------------------*/

//
// Start DirectMouse capture if possible.
//
void FWindowsCameraManager::dmStart(void)
	{
	GUARD;
	if (UseDirectMouse)
		{
		if (DMouseHandle==NULL) DMouseHandle = DMouseOpen();
		ResetModes();
		StoredMouseTime = 0;
		};
	UNGUARD("FWindowsCameraManager::dmStart");
	};

//
// End DirectMouse capture if it's active.
//
void FWindowsCameraManager::dmEnd(void)
	{
	GUARD;
	if (DMouseHandle)
		{
		DMouseClose(DMouseHandle);
		DMouseHandle=NULL;
		};
	UNGUARD("FWindowsCameraManager::dmEnd");
	};

//
// Sample the mouse movement and logaritmically adjust it.
//
void FWindowsCameraManager::dmTick(void)
	{
	GUARD;
	if (UseDirectMouse && DMouseHandle)
		{
		DMOUSE_STATE DMouseState;
		QWORD        Time;
		//
		DMouseGetState (DMouseHandle,&DMouseState);
		DWORD State = DMouseState.mouse_state;
		//
		// Find time since last move:
		//
		Time = GApp->TimeUSec();
		FLOAT TimeDelta	= ((FLOAT)(SQWORD)(Time - StoredMouseTime) + 1.0)/1000000.0;  // Seconds
		if (StoredMouseTime==0) // Skip first movement
			{
			StoredMouseTime=Time;
			return;
			}
		else StoredMouseTime = Time;
		FLOAT MouseUnitSpeed = 1000.0; // Pixels per second = 1.0 before gamma adjustment
		FLOAT MouseGamma	 = 1.75;   // Power to adjust mouse movement speed by
		FLOAT Cutoff		 = 8.0;
		//
		// Find X and Y velocities, where 1.0 = MouseUnitSpeed per second.
		//
		FLOAT XV			= ((FLOAT)(INT)DMouseState.mouse_delta_x) / (MouseUnitSpeed * TimeDelta);
		FLOAT YV			= ((FLOAT)(INT)DMouseState.mouse_delta_y) / (MouseUnitSpeed * TimeDelta);
		//
		// Adjust velocities by some power:
		//
		FLOAT AdjustedXV	= OurSgn(XV) * exp(MouseGamma * log(0.001+OurMin(Cutoff,OurAbs(XV))));
		FLOAT AdjustedYV	= OurSgn(YV) * exp(MouseGamma * log(0.001+OurMin(Cutoff,OurAbs(YV))));
		//
		// Resulting movement = time * adjusted velocity
		//
		FLOAT MouseX		= AdjustedXV * (TimeDelta * MouseUnitSpeed);
		FLOAT MouseY		= AdjustedYV * (TimeDelta * MouseUnitSpeed);
		//
		StoreMove
			(
			MouseX,
			MouseY,
			State & MOUSE_BUTTON1_ASYNC, // Left button
			State & MOUSE_BUTTON3_ASYNC, // Middle button
			State & MOUSE_BUTTON2_ASYNC  // Right button
			);
        if( GInput.CapturingMouse() )
            {
            const int dX = MouseX; /* DMouseState.mouse_delta_x;*/
            const int dY = MouseY; /* DMouseState.mouse_delta_y;*/
            GInput.NoteMovement
            ( 
                FPlatformInput::M_MouseR
            ,   FPlatformInput::M_MouseL
            ,   GInput.NewMouseX() + dX
            );
            GInput.NoteMovement
            ( 
                FPlatformInput::M_MouseB
            ,   FPlatformInput::M_MouseF
            ,   GInput.NewMouseY() + dY
            );
            }
		};
	UNGUARD("FWindowsCameraManager::dmTick");
	};

//
// Return the stored mouse movement, and reset it.
//
void FWindowsCameraManager::GetStoredMove(UCamera *Camera,FVector *Move,FFloatRotation *Rot)
	{
	GUARD;
	//
	if (Camera==CurrentCamera())
		{
		if (DMouseHandle) dmTick();
		//
		*Move = StoredMove;
		*Rot  = StoredRot;
		//
		ResetModes();
		}
	else Move->X = Move->Y = Move->Z = Rot->Pitch = Rot->Yaw = Rot->Roll = 0.0;
	//
	UNGUARD("FWindowsCameraManager::GetStoredMove");
	};

/*--------------------------------------------------------------------------------
	DirectDraw
--------------------------------------------------------------------------------*/

//
// Try to set DirectDraw mode with a particular camera.
// Returns 1 if success, 0 if failure.
//
int FWindowsCameraManager::ddSetCamera (UCamera *Camera, int Width, int Height,
	int ColorBytes, int RequestedCaps)
	{
	GUARD;
	//
	if (FullscreenCamera) EndFullscreen();
	SaveFullscreenWindowRect(Camera);
	//
	Camera->Hold();
	FullscreenCamera = Camera;
	//
	if (!ddSetMode(Camera->Win.hWndCamera,Width,Height,ColorBytes,RequestedCaps))
		{
		RECT *Rect = &Camera->Win.SavedWindowRect;
		//
		Camera->Unhold	();
		InitFullscreen	();
		MoveWindow		(Camera->Win.hWndCamera,Rect->left,Rect->top,Rect->right-Rect->left,Rect->bottom-Rect->top,1);
		SetOnTop		(Camera->Win.hWndCamera);
		GApp->MessageBox("DirectDraw was unable to set the requested video mode.","Can't use DirectDraw support",0);
		return 0;
		}
	else Camera->Unhold();
	//
	FullscreenhWndDD = Camera->Win.hWndCamera;
	//
	// Resize frame buffer without redrawing:
	//
	ResizeCameraFrameBuffer
		(
		Camera,
		Width,
		Height,
		ColorBytes,
		BLIT_DIRECTDRAW,
		0
		);
	SetCapture (Camera->Win.hWndCamera);
	ShowCursor (FALSE);
	//
	GGfx.SetPalette();
	//
	if (UseDirectMouse) dmStart();
	//
	Camera->Caps = RequestedCaps;
	if (Camera->Caps & CC_Hardware3D)
		{
		GApp->RenDev->Init3D();
		};
	return 1;
	//
	UNGUARD("FWindowsCameraManager::ddSetCamera");
	};

/*-----------------------------------------------------------------------------
	Platform-specific palette functions
-----------------------------------------------------------------------------*/

//
// Set window position according to menu's on-top setting:
//
void FWindowsCameraManager::SetOnTop(HWND hWnd)
	{
	GUARD;
	if (GetMenuState(GetMenu(hWnd),ID_WIN_TOP,MF_BYCOMMAND)&MF_CHECKED)
		{
		SetWindowPos(hWnd,(HWND)-1,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
		}
	else
		{
		SetWindowPos(hWnd,(HWND)1,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
		SetWindowPos(hWnd,(HWND)0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
		};
	UNGUARD("FWindowsCameraManager::SetOnTop");
	};

/*-----------------------------------------------------------------------------
	General gfx-related & window-related functions
-----------------------------------------------------------------------------*/

//
// Change a camera window's client size without moving the upper left corner.
//
void FWindowsCameraManager::SetSize (HWND hWnd, int NewWidth, int NewHeight, int HasMenu)
	{
	GUARD;
	//
	RECT rWindow,rClient;
	GetWindowRect(hWnd,&rWindow);
	rClient.top		= 0;
	rClient.left	= 0;
	rClient.bottom	= NewHeight;
	rClient.right	= NewWidth;
	AdjustWindowRect(&rClient,GetWindowLong(hWnd,GWL_STYLE),HasMenu);
	//
	MoveWindow
		(
   		hWnd,rWindow.left,rWindow.top,
		rClient.right-rClient.left,
		rClient.bottom-rClient.top,
		TRUE // TRUE=Repaint window
		); 
	UNGUARD("FWindowsCameraManager::SetSize");
	};

//
// Set the client size (camera view size) of a camera.
//
void FWindowsCameraManager::SetCameraClientSize (UCamera *Camera, int NewWidth, int NewHeight, int UpdateProfile)
	{
	GUARD;
	//
	Camera->Win.Aspect = NewHeight ? ((FLOAT)NewWidth / (FLOAT)NewHeight) : 1.0;
	//
	AActor *Actor  = &Camera->GetActor();
	StopClippingCursor	(Camera,0);
	SetSize				(Camera->Win.hWndCamera,NewWidth,NewHeight,(Actor->CameraStatus.ShowFlags & SHOW_Menu)?TRUE:FALSE);
	//
	if (UpdateProfile)
		{
		GApp->PutProfileInteger("Screen","CameraSXR",NewWidth);
		GApp->PutProfileInteger("Screen","CameraSYR",NewHeight);
		};
	UNGUARD("FWindowsCameraManager::SetCameraClientSize");
	};

//
// Set the camera's frame buffer size.  If the camera is locked or on hold, waits
// until the camera is available.
//
void FWindowsCameraManager::SetCameraBufferSize (UCamera *Camera, int NewWidth, int NewHeight, 
	int NewColorBytes)
	{
	GUARD;
	if (Camera->Locked || Camera->OnHold)
		{
		//
		// Virtual buffer is being written to by rendering routines.
		// Set NeedResize to cause virtual screen buffer to be
		// resized on next call to cameraUnlock.
		//
		Camera->Win.NeedResize			= 1;
		Camera->Win.ResizeSXR			= NewWidth;
		Camera->Win.ResizeSYR			= NewHeight;
		Camera->Win.ResizeColorBytes	= NewColorBytes;
		}
	else ResizeCameraFrameBuffer (Camera,NewWidth,NewHeight,NewColorBytes,BLIT_DEFAULT,1);
	//
	UNGUARD("FWindowsCameraManager::SetCameraBufferSize");
	};

void FWindowsCameraManager::SetColorDepth(UCamera *Camera, int NewColorBytes)
	{
	for (int i=0; i<CameraArray->Num; i++)
		{
		UCamera *TempCamera = CameraArray->Element(i);
		if ((Camera==TempCamera)||(Camera==NULL))
			{
			if (TempCamera->Win.hWndCamera==FullscreenhWndDD)
				{
				ddSetCamera
					(
					TempCamera,
					TempCamera->SXR,
					TempCamera->SYR,
					(TempCamera->Caps & CC_Hardware3D) ? 2 : NewColorBytes,
					(TempCamera->Caps & CC_Hardware3D) ? 1 : 0
					);
				GGfx.SetPalette();
				}
			else
				{
				SetCameraBufferSize(TempCamera,TempCamera->SXR,TempCamera->SYR,NewColorBytes);
				FindAvailableModes(TempCamera);
				};
			};
		};
	};

/*--------------------------------------------------------------------------------
	DIB Allocation
--------------------------------------------------------------------------------*/

//
// Free a DIB.
//
void FWindowsCameraManager::FreeCameraStuff (UCamera *Camera)
	{
	GUARD;
	if (Camera->Win.BitmapInfo	!= NULL) appFree		(Camera->Win.BitmapInfo);
	if (Camera->Win.hBitmap		!= NULL) DeleteObject	(Camera->Win.hBitmap);
	if (Camera->Win.hFile		!= NULL) CloseHandle	(Camera->Win.hFile);
	UNGUARD("FWindowsCameraManager::FreeCameraStuff");
	};

//
// Allocate a DIB for a camera.  Camera must not be locked.
// Can't fail.
//
void FWindowsCameraManager::AllocateCameraDIB(UCamera *Camera, int BlitType)
	{
	GUARD;
	//
	FreeCameraStuff (Camera); // Free existing DIBSection stuff if it has been allocated
	//
	UTexture *Texture = Camera->Texture;
	Texture->USize		= 0;
	Texture->VSize		= 0;
	Texture->ColorBytes	= 0;
	//
	BYTE *TextureData = NULL;
	// Allocate DIB for the camera
	//
	//
	DWORD DataSize = sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD);
	if ((BlitType!=BLIT_DIBSECTION)&&(BlitType!=BLIT_DIRECTDRAW))
		{
		DataSize += Align4(Camera->SXR) * Camera->SYR * Camera->ColorBytes;
		};
	BITMAPINFO *BitmapInfo		= (BITMAPINFO *)appMalloc(DataSize,"BitmapInfo");
	BITMAPINFOHEADER *Header	= &BitmapInfo->bmiHeader;
	Header->biSize				= sizeof(BITMAPINFOHEADER);
	Header->biWidth				= (int)Camera->SXR;
	Header->biHeight			= -(int)Camera->SYR; // Direction = 1 bottom-up, -1 top-down
	Header->biPlanes			= 1;
	Header->biBitCount			= Camera->ColorBytes * 8;
	Header->biSizeImage			= Camera->SXR * Camera->SYR * Camera->ColorBytes;
	Header->biXPelsPerMeter		= 0;
	Header->biYPelsPerMeter		= 0;
	Header->biClrUsed			= 0;
	Header->biClrImportant		= 0;
	//
	if (Camera->ColorBytes==1) // 256-color
		{
		Camera->Caps &= ~(CC_RGB565);
		Header->biCompression = BI_RGB;
		for (int i=0; i<256; i++)
			{
			BitmapInfo->bmiColors[i].rgbRed   = GGfx.DefaultColors[i].Red;
			BitmapInfo->bmiColors[i].rgbGreen = GGfx.DefaultColors[i].Green;
			BitmapInfo->bmiColors[i].rgbBlue  = GGfx.DefaultColors[i].Blue;
			}
		}
	else if (Camera->ColorBytes==2) // 16-bit color (565)
		{
		Camera->Caps |= CC_RGB565;
		Header->biCompression = BI_BITFIELDS;
		*(DWORD *)&BitmapInfo->bmiColors[0]=0xF800;
		*(DWORD *)&BitmapInfo->bmiColors[1]=0x07E0;
		*(DWORD *)&BitmapInfo->bmiColors[2]=0x001F;
		/* Use this to default to RGB-555)
		Camera->Caps &= ~(CC_RGB565);
		Header->biCompression = BI_RGB;
		*(DWORD *)&BitmapInfo->bmiColors[0]=0;
		*/
		}
	else if (Camera->ColorBytes==3) // 24-bit color
		{
		Camera->Caps &= ~(CC_RGB565);
		Header->biCompression = BI_RGB;
		*(DWORD *)&BitmapInfo->bmiColors[0]=0;
		}
	else if (Camera->ColorBytes==4) // 32-bit color
		{
		Camera->Caps &= ~(CC_RGB565);
		Header->biCompression = BI_RGB;
		*(DWORD *)&BitmapInfo->bmiColors[0]=0;
		}
	else appError("Invalid color bytes");
	Camera->Win.BitmapInfo = BitmapInfo;
	//
	if (BlitType == BLIT_DIBSECTION)
		{
		//
		// Create file mapping for DIB section:
		//
		char Name[256];
		// Create unique name for file mapping
		sprintf(Name,"CameraDIB%i_%i",(int)GApp->hWndLog,(int)Camera);
		Camera->Win.hFile = CreateFileMapping ((HANDLE)0xFFFFFFFF,NULL,PAGE_READWRITE,0,0x800000,Name);
		//
		if (Camera->Win.hFile==NULL) appError
			(
			"Unreal has run out of virtual memory. "
			"To prevent this condition, you must free up more space "
			"on your primary hard disk."
			);
		if (Camera->SXR && Camera->SYR)
			{
			HDC TempDC = GetDC(0);
			Camera->Win.hBitmap = CreateDIBSection(TempDC,Camera->Win.BitmapInfo,DIB_RGB_COLORS,(void**)&TextureData,Camera->Win.hFile,0);
			ReleaseDC (0,TempDC);
			if (Camera->Win.hBitmap==NULL)
				{
				//
				// CreateDibSection fails mainly when we run out of memory:
				//
				GApp->Logf("CreateDIBSection failed, Size=%ix%i",Camera->SXR,Camera->SYR);
				appError
					(
					"Unreal has run out of virtual memory. "
					"To prevent this condition, you must free up more space "
					"on your primary hard disk."
					);
				};
			}
		else Camera->Win.hBitmap=NULL;
		}
	else if (BlitType == BLIT_DIRECTDRAW)
		{
		TextureData = (BYTE *)Camera->Win.BitmapInfo + sizeof(BITMAPINFOHEADER);
		if (Camera->ColorBytes==1) TextureData += 256*sizeof(RGBQUAD);
		};
	//
	// Set texture resource data
	//
	Camera->Data        = &Camera->Win.BitmapInfo;
	Texture->Data		= TextureData;
	//
	Texture->USize		= Camera->SXR;
	Texture->VSize		= Camera->SYR;
	Texture->ColorBytes	= Camera->ColorBytes;
	Texture->Palette	= GGfx.GammaPalette;
	//
	// Flag texture and camera to force custom freeing:
	//
	Camera->SetFlags(RF_NoFree);
	Camera->Texture->SetFlags(RF_NoFree);
	//
	UNGUARD("FWindowsCameraManager::AllocateCameraDIB");
	};

/*-----------------------------------------------------------------------------
	Camera functions
-----------------------------------------------------------------------------*/

//
// Resize the camera's frame buffer. Unconditional.
//
void FWindowsCameraManager::ResizeCameraFrameBuffer (UCamera *Camera, 
	int NewSXR, int NewSYR, int NewColorBytes, int BlitType, int Redraw)
	{
	GUARD;
	//
	Camera->Win.NeedResize = 0;
	//
	// Ignore resize orders if no change in sizes:
	//
	Camera->SXR				= NewSXR;
	Camera->SYR				= NewSYR;
	Camera->ColorBytes		= NewColorBytes;
	//
	Camera->Console->NoteResize();
	//
	AllocateCameraDIB(Camera,BlitType);
	Camera->UpdateWindow(); // Redraw camera menu/title
	if (Redraw && Camera->SXR && Camera->SYR) Camera->Draw(0);
	//
	UNGUARD("FWindowsCameraManager::ResizeCameraFrameBuffer");
	};

/*-----------------------------------------------------------------------------
	Window/menu functions:
-----------------------------------------------------------------------------*/

//
// If the cursor is currently being captured, stop capturing, clipping, and 
// hiding it, and move its position back to where it was when it was initially
// captured.
//
void FWindowsCameraManager::StopClippingCursor (UCamera *Camera, int RestorePos)
	{
	GUARD;
	int DoShowCursor=0;
	//
	if (Camera->Win.SaveCursor.x>=0)
		{
		if (RestorePos) 
            {
            SetCursorPos(Camera->Win.SaveCursor.x,Camera->Win.SaveCursor.y);
            ResetMousePosition(StartedClippingCursorInWindow);
            StartedClippingCursorInWindow = 0;
            }
		Camera->Win.SaveCursor.x=-1;
		Camera->Move(BUT_LASTRELEASE,0,0,0,0);
		DoShowCursor=1;
		};
  	ClipCursor(NULL); // Unclip cursor
	//
	if (!FullscreenCamera)	SetCapture (NULL);
	if (DoShowCursor)
		{
		ShowCursor (TRUE);
		};
	if (DMouseHandle && !FullscreenCamera) dmEnd();
	//
	UNGUARD("FWindowsCameraManager::StopClippingCursor");
	};

/*-----------------------------------------------------------------------------
	Camera Window WndProc
-----------------------------------------------------------------------------*/

//
// Camera window WndProc.  This is just a stub that calls the global
// camera managers CameraWndProc class function.
//
LRESULT FAR PASCAL CameraWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
	GUARD;
	//
	if (GApp && GApp->ServerAlive && !GApp->InAppError)
		{
		return ((FWindowsCameraManager *)(GApp->CameraManager))->CameraWndProc
			(
			hWnd,iMessage,wParam,lParam
			);
		}
	else return DefWindowProc(hWnd,iMessage,wParam,lParam);
	//
	UNGUARD("CameraWndProc");
	};

//
// Main camera window function.
//
LRESULT FWindowsCameraManager::CameraWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
	GUARD;
	//
	static int		MovedSinceLeftClick		= 0;
	static int		MovedSinceRightClick 	= 0;
	static int		MovedSinceMiddleClick	= 0;
	static DWORD	StartTimeLeftClick   	= 0;
	static DWORD	StartTimeRightClick  	= 0;
	static DWORD	StartTimeMiddleClick	= 0;
	UCamera			*Camera;
	AActor			*Actor;
	DWORD			ShowFlags;
	//
	// Figure out which Camera structure we corrspond to.  If we don't
	// find a camera (Camera==NULL), that means something got hosed.
	//
	int Found=0;
	for (int i=0; i<CameraArray->Num; i++)
	   	{
		Camera = CameraArray->Element(i);
		if (Camera->Win.hWndCamera==hWnd)
			{
			Found=1;
			break; // Found existing camera!
			}
     	else if ((Camera->Win.hWndCamera==NULL) && (iMessage==WM_CREATE))
       		{
			Found=1;
       		break; // Found camera that is being created
			};
		};
	if (!Found) // The desired camera wasn't found
		{
		//
		// Camera was not found!  Process messages that can be handled without
		// a camera assocation:
		//
		switch (iMessage)
			{
			case WM_CREATE:
				debug (LOG_Win,"Couldn't find camera on create");
				break;
			case WM_DESTROY:
				debug (LOG_Win,"Couldn't find camera on destroy");
				break;
			case WM_PAINT:
				debug (LOG_Win,"Couldn't find camera on paint");
				break;
			};
		return DefWindowProc (hWnd,iMessage,wParam,lParam);
		};
	if (Camera->OnHold) return 0;
	//
	Actor		= &Camera->GetActor();
	ShowFlags	= Actor->CameraStatus.ShowFlags;
	//
	// Message handler:
	//
	switch (iMessage)
		{
		case WM_CREATE:
			GUARD;
			if (Camera->Win.hWndCamera != NULL)
				{
				appError("Window already exists");
				return -1;
				}
			else
				{
         		//
         		// Set hWndCamera:
         		//
         		Camera->Win.hWndCamera = hWnd;
				Camera->Win.Status     = WIN_CameraNormal; 
				//
         		// Set up the the palette for this window:
         		//
         		HDC hDC = GetDC(hWnd);
         		SelectPalette  (hDC,hLogicalPalette,PalFlags()==0);
         		RealizePalette (hDC);
         		ReleaseDC      (hWnd,hDC);
         		//
         		// Paint DIB (which was allocated before window was created):
         		//
				if (Camera->SXR && Camera->SYR)
					{
					ICamera CameraInfo;
         			if (Camera->Lock(&CameraInfo)) Camera->Unlock (&CameraInfo,1);
					};
				//
				// Make this camera current and update its title bar:
				//
				MakeCurrent(Camera);
				return 0;
				};
			UNGUARD("WM_CREATE");
			break;
		case WM_DESTROY:
			GUARD;
			//
			// If there's an existing Camera structure corresponding to
			// this window, deactivate it:
			//
			if (FullscreenCamera) EndFullscreen();
			//
			FreeCameraStuff   (Camera); // Free DIB section stuff (if any)
			StopClippingCursor(Camera,0); // Stop clipping but don't restore cursor position
			//
			if (Camera->Win.Status==WIN_CameraNormal)
				{
				//
				// Closed by user clicking on window's close button.
				// Must call general-purpose camera closing routine.
				//
				Camera->Win.Status = WIN_CameraClosing; // Prevent recursion
				Camera->Kill();
				};
			debug(LOG_Win,"Closed camera");
			return 0;
			UNGUARD("WM_DESTROY");
		case WM_PAINT:
			GUARD;
			//
			if (IsWindowVisible(Camera->Win.hWndCamera) && Camera->SXR && 
				(!FullscreenCamera) && Camera->SYR && !Camera->OnHold)
				{
				PAINTSTRUCT ps;
				ICamera CameraInfo;
				//
				BeginPaint(hWnd,&ps);
         		if (Camera->Lock(&CameraInfo)) Camera->Unlock (&CameraInfo,1);
				EndPaint(hWnd,&ps);
				return 0;
				}
			else return -1;
			UNGUARD("WM_PAINT");
			break;
		case WM_PALETTECHANGED:
			GUARD;
			if (((HWND)wParam != hWnd) && (Camera->ColorBytes==1) && (!FullscreenCamera))
				{				 
         		HDC hDC = GetDC(hWnd);
				SelectPalette  (hDC,hLogicalPalette,PalFlags()==0);
				if (RealizePalette(hDC)) InvalidateRect(hWnd, NULL, TRUE);
         		ReleaseDC      (hWnd,hDC);
				};
			return 0;
			UNGUARD("WM_PALETTECHANGED");
			break;
		case WM_QUERYNEWPALETTE:
			GUARD;
			if ((Camera->ColorBytes==1) && (!FullscreenCamera))
				{
				HDC hDC = GetDC(hWnd);
				SelectPalette  (hDC,hLogicalPalette,PalFlags()==0);
				RealizePalette (hDC);
        		ReleaseDC      (hWnd,hDC);
				return TRUE;
				}
			else return FALSE;
			UNGUARD("WM_QUERYNEWPALETTE");
		case WM_COMMAND:
			GUARD;
      		switch (wParam)
				{
				case ID_MAP_DYNLIGHT:	Actor->CameraStatus.RendMap=REN_DynLight; break;
				case ID_MAP_PLAINTEX:	Actor->CameraStatus.RendMap=REN_PlainTex; break;
				case ID_MAP_WIRE:		Actor->CameraStatus.RendMap=REN_Wire; break;
				case ID_MAP_OVERHEAD:	Actor->CameraStatus.RendMap=REN_OrthXY; break;
				case ID_MAP_XZ:  		Actor->CameraStatus.RendMap=REN_OrthXZ; break;
				case ID_MAP_YZ:  		Actor->CameraStatus.RendMap=REN_OrthYZ; break;
				case ID_MAP_POLYS:		Actor->CameraStatus.RendMap=REN_Polys; break;
				case ID_MAP_POLYCUTS:	Actor->CameraStatus.RendMap=REN_PolyCuts; break;
				case ID_MAP_ZONES:		Actor->CameraStatus.RendMap=REN_Zones; break;
				case ID_WIN_320:		SetCameraClientSize(Camera,320,200,1); break;
				case ID_WIN_400:		SetCameraClientSize(Camera,400,300,1); break;
				case ID_WIN_512:		SetCameraClientSize(Camera,512,384,1); break;
				case ID_WIN_640:		SetCameraClientSize(Camera,640,400,1); break;
				//
				case ID_COLOR_8BIT:		SetColorDepth(Camera,1); break;
				case ID_COLOR_16BIT:	SetColorDepth(Camera,2); break;
				case ID_COLOR_32BIT:	SetColorDepth(Camera,4); break;
				//
				case ID_SHOW_BACKDROP:	Actor->CameraStatus.ShowFlags ^= SHOW_Backdrop; break;
				//
				case ID_ACTORS_SHOW:
					Actor->CameraStatus.ShowFlags &= ~(SHOW_Actors | SHOW_ActorIcons | SHOW_ActorRadii); 
					Actor->CameraStatus.ShowFlags |= SHOW_Actors; 
					break;
				case ID_ACTORS_ICONS:
					Actor->CameraStatus.ShowFlags &= ~(SHOW_Actors | SHOW_ActorIcons | SHOW_ActorRadii); 
					Actor->CameraStatus.ShowFlags |= SHOW_Actors | SHOW_ActorIcons;
					break;
				case ID_ACTORS_RADII:
					Actor->CameraStatus.ShowFlags &= ~(SHOW_Actors | SHOW_ActorIcons | SHOW_ActorRadii); 
					Actor->CameraStatus.ShowFlags |= SHOW_Actors | SHOW_ActorIcons | SHOW_ActorRadii;
					break;
				case ID_ACTORS_HIDE:
					Actor->CameraStatus.ShowFlags &= ~(SHOW_Actors | SHOW_ActorIcons | SHOW_ActorRadii); 
					break;
				case ID_SHOW_COORDS:	Actor->CameraStatus.ShowFlags ^= SHOW_Coords; break;
				case ID_SHOW_BRUSH:		Actor->CameraStatus.ShowFlags ^= SHOW_Brush; break;
				case ID_SHOW_MOVINGBRUSHES: Actor->CameraStatus.ShowFlags ^= SHOW_MovingBrushes; break;
				//
				case ID_HELP_ABOUT:		PostMessage(App.Dialog->m_hWnd,WM_COMMAND,IDC_HELP_ABOUT,0); break;
				case ID_HELP_TOPICS:	PostMessage(App.Dialog->m_hWnd,WM_COMMAND,IDC_HELP_TOPICS,0); break;
				case ID_HELP_ORDER:		PostMessage(App.Dialog->m_hWnd,WM_COMMAND,IDC_HELP_ORDER,0); break;
				case ID_HELP_ORDERNOW:	PostMessage(App.Dialog->m_hWnd,WM_COMMAND,IDC_HELP_ORDERNOW,0); break;
				case ID_HELP_EPICSWEBSITE: PostMessage(App.Dialog->m_hWnd,WM_COMMAND,IDC_HELP_WEB,0); break;
				//
				case ID_SHOWLOG: GApp->Show(); break;
				//
				case ID_PROPERTIES_PROPERTIES: PostMessage(App.Dialog->m_hWnd,WM_COMMAND,ID_PROPERTIES_PROPERTIES,0); break;
				case ID_PROPERTIES_PROPERTIES2: PostMessage(App.Dialog->m_hWnd,WM_COMMAND,ID_PROPERTIES_PROPERTIES,0); break;
				case ID_FILE_ENDGAME:	PostMessage(App.Dialog->m_hWnd,WM_COMMAND,ID_FILE_ENDGAME,0); break;
				case ID_FILE_BEGINGAME:	PostMessage(App.Dialog->m_hWnd,WM_COMMAND,ID_FILE_BEGINGAME,0); break;
				case ID_FILE_SAVEGAME:	PostMessage(App.Dialog->m_hWnd,WM_COMMAND,ID_FILE_SAVEGAME,0); break;
				case ID_FILE_LOADGAME:	PostMessage(App.Dialog->m_hWnd,WM_COMMAND,ID_FILE_LOADGAME,0); break;
				case ID_NETGAME:		PostMessage(App.Dialog->m_hWnd,WM_COMMAND,ID_NETGAME,0); break;
				//
				case ID_TRUE_COLOR: // Toggle ColorBytes between 1 and 4
					SetCameraBufferSize(Camera,Camera->SXR,Camera->SYR,5-Camera->ColorBytes);
					FindAvailableModes(Camera);
					break;
				case ID_FILE_EXIT:
					DestroyWindow(hWnd);
					return (LRESULT)0;
				case ID_WIN_TOP:
					Toggle(GetMenu(hWnd),ID_WIN_TOP);
					SetOnTop(hWnd);
					break;
				default:
				if ((wParam>=ID_DDMODE_0) && (wParam<=ID_DDMODE_9))
					{
					ddSetCamera
						(
						Camera,
						ddModeWidth[wParam-ID_DDMODE_0],
						ddModeHeight[wParam-ID_DDMODE_0],
						IsHardware3D(Camera) ? 2 : Camera->ColorBytes,
						IsHardware3D(Camera) ? CC_Hardware3D : 0
						);
					};
				};
			Camera->Draw(0);
			Camera->UpdateWindow();
			return 0;
			UNGUARD("WM_COMMAND");
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			{
			GUARD;
			int Key = wParam;
			GApp->KeyDown [(BYTE)Key] = 1;
			if (!GApp->KeyDown[(BYTE)Key]) GApp->KeyPressed[(BYTE)Key] = 1;
			//
			if (((Key==VK_UP) || (Key==VK_DOWN) || (Key==VK_LEFT) || (Key==VK_RIGHT) ||
				(Key==VK_PRIOR) || (Key==VK_NEXT)) &&
				Camera->Key(Key+256)) // Game absorbed the keystroke
				{
				if (!Camera->IsRealtime()) Camera->Draw(0);
				}
			else if	(Key==K_F1)		PostMessage ((HWND)Camera->ParentWindow,iMessage,K_F2,lParam);
			else if (Key==K_DELETE)	Camera->Key(Key);
			else if (Key==K_ENTER)	Camera->Key(Key);
			else if (Key==VK_F11)	Camera->Key(Key+256);
			else if (Key!=VK_TAB)	PostMessage ((HWND)Camera->ParentWindow,iMessage,wParam,lParam);
            if( PlayingGame(ShowFlags) && Key < 256 && GInput.CapturingKeyboard() ) // < 256 is a safety check
			{
                GInput.Press( GInput.WindowsKeySwitches[Key] );
            }
			//
			SetModeCursor(Camera);
			//
			return 0;
			UNGUARD("WM_KEYDOWN");
			};
		case WM_KEYUP:
		case WM_SYSKEYUP:
			{
			GUARD;
			int Key = wParam;
			GApp->KeyDown[(BYTE)Key] = 0;
			//
			if (Camera->ParentWindow) // Pass keystroke on to UnrealEd
				{
				if		(Key==K_F1)		PostMessage((HWND)Camera->ParentWindow,iMessage,K_F2,lParam);
				else if (Key!=VK_TAB)	PostMessage((HWND)Camera->ParentWindow,iMessage,wParam,lParam);
				};
			SetModeCursor(Camera);
            if( PlayingGame(ShowFlags) && Key < 256 && GInput.CapturingKeyboard() ) // <256 is a safety check
    		{
                GInput.Release( GInput.WindowsKeySwitches[Key] );
            }
			return 0;
			UNGUARD("WM_KEYUP");
			};
		case WM_SYSCHAR:
		case WM_CHAR:
			{
			GUARD;
			int Key = wParam;
			if ((Key!=K_ENTER) && Camera->Key(Key))
				{
				if (!Camera->IsRealtime()) Camera->Draw(0);
				SetModeCursor(Camera);
				}
			else if (iMessage==WM_SYSCHAR) return DefWindowProc (hWnd,iMessage,wParam,lParam);
			return 0;
			UNGUARD("WM_CHAR");
			};
		case WM_KILLFOCUS:
			GUARD;
			//
			StopClippingCursor (Camera,0);
			GApp->ResetKeyboard();
            if( PlayingGame(ShowFlags) )
                { 
                    //todo: [Mark] We'll have to add this eventually to stop
                    // the capture of joystick movements when the game is no longer
                    // the active window. But this also requires recapturing the
                    // devices when a full-screen window is started, and I don't
                    // know exactly where to do this.
                    //Add here: GInput.CaptureDevices(FALSE); 
                    //Add when fullscreen started: GInput.CaptureDevices(TRUE); 
                }
			return 0;
			UNGUARD("WM_KILLFOCUS");
			break;
		case WM_SETFOCUS:
			GUARD;
			//
			// This window has just received the input focus.  Update the "*"'s in the
			// camera titles to show which one is current.
			//
			GApp->ResetKeyboard();
			if (Camera) MakeCurrent(Camera);
            if( PlayingGame(ShowFlags) )
                { 
                GInput.CaptureDevices(TRUE); 
                ResetMousePosition(hWnd);
                GInput.StartNextInputCycle();
                }
			return 0;
			UNGUARD("WM_SETFOCUS");
			break;
		case WM_ERASEBKGND:
			// Prevents Windows from repainting client background in white.
			return 0;
			break;
		case WM_SETCURSOR:
			GUARD;
			if ((LOWORD(lParam)==1) || GApp->InSlowTask) // In client area or processing slow task
				{
				SetModeCursor(Camera);
				return 0;
				}
			else return DefWindowProc (hWnd,iMessage,wParam,lParam); // Out of client area
			UNGUARD("WM_SETCURSOR");
			break;
		case WM_MBUTTONDBLCLK: 
        {
            //todo: Investigate if this is necessary (since GInput times clicks to determine doubling)
            GUARD;
            if( PlayingGame(ShowFlags) && GInput.CapturingMouse() )
                {
                GInput.DoublePress( FInput::S_MiddleMouse );
                }
			return 0;
			UNGUARD("WM_MBUTTONDBLCLK");
			break;
        }
		case WM_RBUTTONDBLCLK: 
        {
            GUARD;
            //todo: Investigate if this is necessary (since GInput times clicks to determine doubling)
            if( PlayingGame(ShowFlags) && GInput.CapturingMouse() )
                {
                GInput.DoublePress( FInput::S_RightMouse );
                }
			return 0;
			UNGUARD("WM_RBUTTONDBLCLK");
			break;
        }
		case WM_LBUTTONDBLCLK:
			GUARD;
            //todo: Investigate if this is necessary (since GInput times clicks to determine doubling)
            if( PlayingGame(ShowFlags) && GInput.CapturingMouse() )
                {
                GInput.DoublePress( FInput::S_LeftMouse );
                }
			if (ShowFlags & SHOW_NoCapture)
				{
				Camera->Click(BUT_LEFTDOUBLE,LOWORD(lParam),HIWORD(lParam),0,0);
				if (!Camera->IsRealtime()) Camera->Draw (0);
				};
			return 0;
			UNGUARD("WM_LBUTTONDBLCLK");
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			GUARD;
			if (InMenuLoop) return DefWindowProc (hWnd,iMessage,wParam,lParam);
			if (iMessage == WM_LBUTTONDOWN)
				{
                if( PlayingGame(ShowFlags) && GInput.CapturingMouse() )
                    {
                    GInput.Press( FInput::S_LeftMouse );
                    }
				GApp->KnownButtons |= MK_LBUTTON;
				MovedSinceLeftClick = 0;
				StartTimeLeftClick = GetMessageTime();
				GApp->KeyDown [K_LBUTTON] = 1;
				GApp->KeyPressed [K_LBUTTON] = 1;
				}
			else if (iMessage == WM_RBUTTONDOWN)
				{
                if( PlayingGame(ShowFlags) && GInput.CapturingMouse() )
                    {
                    GInput.Press( FInput::S_RightMouse );
                    }
				GApp->KnownButtons |= MK_RBUTTON;
				MovedSinceRightClick = 0;
				StartTimeRightClick = GetMessageTime();
				GApp->KeyDown [K_RBUTTON] = 1;
				GApp->KeyPressed [K_RBUTTON] = 1;
				}
			else if (iMessage == WM_MBUTTONDOWN)
				{
                if( PlayingGame(ShowFlags) && GInput.CapturingMouse() )
                    {
                    GInput.Press( FInput::S_MiddleMouse );
                    }
				GApp->KnownButtons |= MK_MBUTTON;
				MovedSinceMiddleClick = 0;
				StartTimeMiddleClick = GetMessageTime();
				GApp->KeyDown [K_MBUTTON] = 1;
				GApp->KeyPressed [K_MBUTTON] = 1;
				};
			if (ShowFlags & SHOW_NoCapture)
				{
				if (iMessage==WM_LBUTTONDOWN) Camera->Click(BUT_LEFT,LOWORD(lParam),HIWORD(lParam),0,0);
				if (iMessage==WM_RBUTTONDOWN) Camera->Click(BUT_RIGHT,LOWORD(lParam),HIWORD(lParam),0,0);
				if (!Camera->IsRealtime()) Camera->Draw(0);
				}
			else
				{
				if (Camera->Win.SaveCursor.x==-1)
					{
                    StartClippingCursor(*this, Camera, hWnd);
                    //todo: (Mark)
                    // I use the code in StartClippingCursor, but I'm a little afraid to 
                    // delete this until I'm sure things work:
					//tbd:RECT TempRect;
					//tbd:GetCursorPos    (&(Camera->Win.SaveCursor));
					//tbd:GetClientRect   (hWnd,&TempRect);
					//tbd:MapWindowPoints (hWnd,NULL,(POINT *)&TempRect, 2); // Get screen coords of this window
					//tbd:SetCursorPos    ((TempRect.left+TempRect.right)/2,(TempRect.top+TempRect.bottom)/2);
					//tbd:ClipCursor      (&TempRect); // Confine cursor to window
					//tbd:ShowCursor      (FALSE);
					//tbd:if (!FullscreenCamera) SetCapture (hWnd);
					//tbd:if (UseDirectMouse()) dmStart();
					//
					// Tell camera that an initial mouse button was just hit:
					//
					Camera->Move(BUT_FIRSTHIT,0,0,0,0);
					};
				};
			return 0;
			UNGUARD("WM_BUTTONDOWN");
			break;
		case WM_MOUSEACTIVATE:
			return MA_ACTIVATE; // Activate this window and send the mouse-down message
		case WM_ACTIVATE:
			GUARD;
			if (wParam==0) // Window becoming inactive, make sure we fix up mouse cursor
				{
				StopClippingCursor (Camera,0); // Stop clipping but don't restore cursor position
				};
			return 0;
			UNGUARD("WM_ACTIVATE");
			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			{
			GUARD;
			//
			// Exit if cursor not captured or window is unidentified
			//
			if (InMenuLoop) return DefWindowProc (hWnd,iMessage,wParam,lParam);
			//
            if( PlayingGame(ShowFlags) && GInput.CapturingMouse() )
            {
                GInput.Release
                ( 
                    iMessage == WM_LBUTTONUP    ? FInput::S_LeftMouse 
                :   iMessage == WM_RBUTTONUP    ? FInput::S_RightMouse
                :                                 FInput::S_MiddleMouse
                );
            }
			if (Camera->Win.SaveCursor.x==-1) break;
			//
			// Remember mouse cursor position of original click:
			//
			POINT TempPoint;
			TempPoint.x = 0;
			TempPoint.y = 0;
			ClientToScreen(hWnd,&TempPoint);
			//
			int X = Camera->Win.SaveCursor.x - TempPoint.x;
			int Y = Camera->Win.SaveCursor.y - TempPoint.y;
			int DeltaTime;
			//
			// Stop clipping mouse to current window, and restore original position:
			//
			if ((!(wParam&GApp->KnownButtons)&(MK_LBUTTON|MK_MBUTTON|MK_RBUTTON))) 
                {
                    {
                    StopClippingCursor (Camera,1);
                    }
                }
			//
			// Get time interval to determine if a click occured:
			//
			if 		(iMessage == WM_LBUTTONUP) DeltaTime = GetMessageTime() - StartTimeLeftClick;
			else if (iMessage == WM_MBUTTONUP) DeltaTime = GetMessageTime() - StartTimeMiddleClick;
			else if (iMessage == WM_RBUTTONUP) DeltaTime = GetMessageTime() - StartTimeRightClick;
			//
			int Click = (DeltaTime>20) && (DeltaTime<600); // Prevent bounce
			//
			// Handle mouse clicks in the editor:
			//
			int j = (wParam & MK_SHIFT)   != 0;
			int k = (wParam & MK_CONTROL) != 0;
			//
			if (iMessage==WM_LBUTTONUP)
				{
				GApp->KnownButtons &= ~MK_LBUTTON;
				GApp->KeyDown [K_LBUTTON] = 0;
				if (Click && !(MovedSinceLeftClick||MovedSinceRightClick||MovedSinceMiddleClick))
					{
					if (Camera && (!FullscreenCamera) &&
						(!Camera->OnHold) && Camera->SXR && Camera->SYR)
						{
						Camera->Click(BUT_LEFT,X,Y,j,k);
						if (!Camera->IsRealtime()) Camera->Draw(0);
						};
					};
				MovedSinceLeftClick   = 0;
				}
			else if (iMessage==WM_RBUTTONUP)
				{
				GApp->KnownButtons &= ~MK_RBUTTON;
				GApp->KeyDown [K_RBUTTON] = 0;
				if (Click && !(MovedSinceLeftClick||MovedSinceRightClick||MovedSinceMiddleClick))
					{
					if (Camera && (!FullscreenCamera) &&
						(!Camera->OnHold) && Camera->SXR && Camera->SYR)
						{
						Camera->Click(BUT_RIGHT,X,Y,j,k);
						if (!Camera->IsRealtime()) Camera->Draw(0);
						};
					};
				MovedSinceRightClick   = 0;
				}
			else if (iMessage==WM_MBUTTONUP)
				{
				GApp->KnownButtons &= ~MK_MBUTTON;
				GApp->KeyDown [K_MBUTTON] = 0;
				if (Click && !(MovedSinceLeftClick||MovedSinceRightClick||MovedSinceMiddleClick))
					{
					if (Camera && (!FullscreenCamera) &&
						(!Camera->OnHold) && Camera->SXR && Camera->SYR)
						{
						Camera->Click(BUT_MIDDLE,X,Y,j,k);
						if (!Camera->IsRealtime()) Camera->Draw(0);
						};
					};
				MovedSinceMiddleClick = 0;
				};
			return 0;
			UNGUARD("WM_MBUTTONUP");
			};
		case WM_ENTERMENULOOP:
			GUARD;
			//
			InMenuLoop = 1;
			StopClippingCursor (Camera,0);
			Camera->UpdateWindow(); // Update checkmarks and such
			//
			return 0;
			UNGUARD("WM_ENTERMENULOOP");
		case WM_EXITMENULOOP:
			GUARD;
			//
			InMenuLoop = 0;
			//
			return 0;
			UNGUARD("WM_EXITMENULOOP");
			break;
		case WM_CANCELMODE:
			GUARD;
			//
			StopClippingCursor(Camera,0);
			return 0;
			//
			UNGUARD("WM_CANCELMODE");
			break;
		case WM_MOUSEMOVE:
			{
			GUARD;
			//
			// If in a window, see if cursor has been captured; if not, ignore mouse movement:
			//
			if (InMenuLoop) break;
			//todo:[delete this: (moved to later)] if ((!FullscreenCamera) && (Camera->Win.SaveCursor.x==-1)) break;
			//todo:[delete this: (moved to later)] if (UseDirectMouse() && Camera->IsRealtime() && (ShowFlags & SHOW_PlayerCtrl)) break;
			//
			int Updated  = 0;
			RECT TempRect;
			GetClientRect(hWnd,&TempRect);
			//
			POINT TempPoint;
			TempPoint.x=(TempRect.left+TempRect.right)/2;
			TempPoint.y=(TempRect.top+TempRect.bottom)/2;

            {
                const WORD Buttons = wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON);
                if( PlayingGame(ShowFlags) && Buttons!=0 && Camera->Win.SaveCursor.x==-1 )
                {
                    // We are not saving the cursor, but we should be...
                    StartClippingCursor(*this,Camera,hWnd);
                }
                if( PlayingGame(ShowFlags) && GInput.CapturingMouse() )
                {
                    if( Buttons==0 )
                    {
                        // Sanity check to make sure no mouse release events were missed:
                        GInput.Release( FInput::S_LeftMouse   ); 
                        GInput.Release( FInput::S_RightMouse  );
                        GInput.Release( FInput::S_MiddleMouse );
                    }
                    GAction.UsePlainMouseMovements( FullscreenCamera != 0 );
                    //todo: [Mark] We don't have to set the movement range every time the mouse moves!
                    GInput.SetMovementRange
                    (
                        FPlatformInput::M_MouseR
                    ,   FPlatformInput::M_MouseL
                    ,   FLOAT(TempRect.left)
                    ,   FLOAT(TempRect.right)
                    );
                    GInput.SetMovementRange
                    (
                        FPlatformInput::M_MouseB
                    ,   FPlatformInput::M_MouseF
                    ,   FLOAT(TempRect.top)
                    ,   FLOAT(TempRect.bottom)
                    );
                }
            }

            if ( !FullscreenCamera && Camera->Win.SaveCursor.x==-1 ) 
            {
                const int AbsoluteX = (int)LOWORD(lParam);
                const int AbsoluteY = (int)HIWORD(lParam);
                int dX = AbsoluteX - PreviousMousePoint.x;
                int dY = AbsoluteY - PreviousMousePoint.y;
                PreviousMousePoint.x = AbsoluteX;
                PreviousMousePoint.y = AbsoluteY;
                if( PlayingGame(ShowFlags) && GInput.CapturingMouse() )
                {
                    GInput.NoteMovement
                    ( 
                        FPlatformInput::M_MouseR
                    ,   FPlatformInput::M_MouseL
                    ,   GInput.NewMouseX() + dX
                    );
                    GInput.NoteMovement
                    ( 
                        FPlatformInput::M_MouseB
                    ,   FPlatformInput::M_MouseF
                    ,   GInput.NewMouseY() + dY
                    );
                }
                break; // <================ Unstructured exit!
            }
            //todo:[delete-redundant due to previous if...] if ((!FullscreenCamera) && (Camera->Win.SaveCursor.x==-1)) break;
			if (UseDirectMouse && Camera->IsRealtime() && (ShowFlags & SHOW_PlayerCtrl)) break;

            int dX = 0;
            int dY = 0;
			//
			// Make relative to window center
			//
			Loop:
			WORD Buttons = wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON);
			int X        = LOWORD(lParam) - TempPoint.x;
			int Y        = HIWORD(lParam) - TempPoint.y;
			int j        = (wParam & MK_SHIFT)   != 0;
			int k        = (wParam & MK_CONTROL) != 0;

            if( PlayingGame(ShowFlags) )
                {
                POINTS Points = MAKEPOINTS(lParam);
                const int AbsoluteX = Points.x;
                const int AbsoluteY = Points.y;
                dX += AbsoluteX - PreviousMousePoint.x;
                dY += AbsoluteY - PreviousMousePoint.y;
                PreviousMousePoint.x = AbsoluteX;
                PreviousMousePoint.y = AbsoluteY;
                }
			//
			if ((OurAbs(X)>2)||(OurAbs(Y)>2))
				{
				if (wParam & MK_LBUTTON) MovedSinceLeftClick   = 1;
				if (wParam & MK_RBUTTON) MovedSinceRightClick  = 1;
				if (wParam & MK_MBUTTON) MovedSinceMiddleClick = 1;
				};
			if ( Buttons || FullscreenCamera)
				{
				int CameraButtonFlags = 0;
				if (Buttons & MK_LBUTTON) CameraButtonFlags |= BUT_LEFT;
				if (Buttons & MK_RBUTTON) CameraButtonFlags |= BUT_RIGHT;
				if (Buttons & MK_MBUTTON) CameraButtonFlags |= BUT_MIDDLE;
				//
				if (FullscreenCamera && (CameraButtonFlags==0)) CameraButtonFlags = BUT_LEFT;
				//
				// Move camera with buttons:
				//
				if (!Camera->Move(CameraButtonFlags,X,Y,j,k))
					{
					StoreMove(X,Y,Buttons & MK_LBUTTON,Buttons & MK_MBUTTON,Buttons & MK_RBUTTON);			
					};
				Updated=1;
				//
				MSG Msg;
				if (PeekMessage(&Msg,Camera->Win.hWndCamera,WM_MOUSEMOVE,WM_MOUSEMOVE,PM_REMOVE))
					{
					TempPoint.x = LOWORD(lParam);
					TempPoint.y = HIWORD(lParam);
					lParam      = Msg.lParam;
					wParam      = Msg.wParam;
					goto Loop;
					};
				};
            if( PlayingGame(ShowFlags) && GInput.CapturingMouse() )
                {   
                GInput.NoteMovement
                ( 
                    FPlatformInput::M_MouseR
                ,   FPlatformInput::M_MouseL
                ,   GInput.NewMouseX() + dX
                );
                GInput.NoteMovement
                ( 
                    FPlatformInput::M_MouseB
                ,   FPlatformInput::M_MouseF
                ,   GInput.NewMouseY() + dY
                );
                }
			if ((Camera->Win.SaveCursor.x>=0 && Buttons) || (FullscreenCamera && !InMenuLoop) )
				{
				RECT TempRect;
				GetClientRect  (hWnd,&TempRect);
				//
				POINT TempPoint;
				TempPoint.x =  (TempRect.left+TempRect.right)/2;
				TempPoint.y =  (TempRect.top+TempRect.bottom)/2;
				ClientToScreen (hWnd,&TempPoint);
				SetCursorPos   (TempPoint.x,TempPoint.y);
                //todo:
                // The above call SetCursorPos() seems to add an extraneous WM_MOUSEMOVE event
                // into the message queue. We don't want this, so see if such a message
                // exists. It is possible that a "real" WM_MOUSEMOVE is also queued, 
                // before or after the extraneous one.
                // Find a cleaner way to handle this, perhaps forcing the remembered mouse position
                // into the new position so that the extraneous message has no effect (is not 
                // perceived as a change in mouse position).
                // **OR**
                // It seems this is no longer necessary - I'm not sure why. In fact, the fix
                // below created a bug once it was integrated with 0.76 (jerky mouse movements
                // when turning left or right). Sigh. So many hours spent fiddling with WM_MOUSEMOVE's.
                if( FALSE )
                    {
                    MSG Message;
                    if( PeekMessage(&Message,Camera->Win.hWndCamera,WM_MOUSEMOVE,WM_MOUSEMOVE,PM_REMOVE) )
                        {
                        const int X = int(LOWORD(Message.lParam));
                        const int Y = int(HIWORD(Message.lParam));
                        if( X == TempPoint.x && Y == TempPoint.y )
                            {
                            // The WM_MOUSEMOVE message matches the position we just set with SetCursorPos().
                            // Ignore it (leave it off the message queue).
                            }
                        else
                            {
                            // Requeue the message.
                            PostMessage(0,Message.message, Message.wParam, Message.lParam );
                            }
                        }
                    }
                ResetMousePosition(hWnd);
				};
			if (Updated && !Camera->IsRealtime())
				{
				//
				// Camera isn't realtime, so we must update the frame here and now.
				//
				if (!GApp->KeyDown[K_TAB]) Camera->Draw(0);
				else for (int i=0; i<CameraArray->Num; i++) CameraArray->Element(i)->Draw(0);
				};
			return 0;
			UNGUARD("WM_MOUSEMOVE");
			};
		case WM_SIZE:
			GUARD;
			if (!FullscreenCamera)
				{
				int X = LOWORD(lParam);	// New width of client area
				int Y = HIWORD(lParam); // New height of client area
				SetCameraBufferSize(Camera,Align4(X),Y,Camera->ColorBytes);
				};
      		return 0;
			UNGUARD("WM_SIZE");
		case WM_SIZING:
			GUARD;
			if (!(Actor->CameraStatus.ShowFlags & SHOW_ChildWindow))
				{
				RECT *Rect = (RECT *)lParam,rClient;
				rClient.top		= 0;
				rClient.left	= 0;
				rClient.bottom	= 0;
				rClient.right	= 0;
				AdjustWindowRect(&rClient,GetWindowLong(hWnd,GWL_STYLE),(Actor->CameraStatus.ShowFlags & SHOW_Menu)?TRUE:FALSE);
				int ExtraX = rClient.right  - rClient.left;
				int ExtraY = rClient.bottom - rClient.top;
				int  X     = Rect->right    - Rect->left - ExtraX;
				int  Y     = Rect->bottom   - Rect->top  - ExtraY;
				if (X && Y) // Force aspect ratio to be reasonable
					{
					//
					if ((wParam==WMSZ_LEFT) || (wParam==WMSZ_RIGHT))
						{
						Rect->bottom = Rect->top + ExtraY + X / Camera->Win.Aspect + 0.5;
						}
					else if ((wParam==WMSZ_TOP) || (wParam==WMSZ_BOTTOM))
						{
						Rect->right = Rect->left + ExtraX + Y * Camera->Win.Aspect + 0.5;
						}
					else
						{
						FLOAT Aspect	= (FLOAT)X/(FLOAT)Y;
						FLOAT NewAspect = OurClamp(Aspect,(FLOAT)1.25,(FLOAT)1.6);
						//
						if (Aspect>NewAspect)
							{
							Camera->Win.Aspect = NewAspect;
							Rect->bottom = Rect->top  + ExtraY + X / NewAspect;
							}
						else if (Aspect<NewAspect)
							{
							Camera->Win.Aspect = NewAspect;
							Rect->right  = Rect->left + ExtraX + Y * NewAspect;
							};
						};
					};
				};
			return TRUE;
			UNGUARD("WM_SIZING");
		case WM_SYSCOMMAND:
			{
			GUARD;
			int nID = wParam & 0xFFF0;
			if ((nID==SC_SCREENSAVE) || (nID==SC_MONITORPOWER))
				{
				if (nID==SC_SCREENSAVE) debugf(LOG_Win,"Received SC_SCREENSAVE");
				else debugf(LOG_Win,"Received SC_MONITORPOWER");
				//
				if( GInput.CapturingJoystick() )
					{
					return 1; // Inhibit screen saver
					}
				else return 0; // Allow screen saved to take over
				}
			else return DefWindowProc(hWnd,iMessage,wParam,lParam);
			UNGUARD("WM_SYSCOMMAND");
			};
		case WM_POWER:
			GUARD;
			if (wParam)
				{
				if (wParam==PWR_SUSPENDREQUEST)
					{
					debugf(LOG_Win,"Received WM_POWER suspend");
					if (GInput.CapturingJoystick()) return PWR_FAIL;
					else return PWR_OK;
					}
				else
					{
					debugf(LOG_Win,"Received WM_POWER");
					return DefWindowProc(hWnd,iMessage,wParam,lParam);
					};
				};
			return 0;
			UNGUARD("WM_POWER");
		case WM_DISPLAYCHANGE:
			GUARD;
			debugf(LOG_Win,"Camera %s: WM_DisplayChange",Camera->Name);
			UNGUARD("WM_DISPLAYCHANGE");
			return 0;
		case WM_WININICHANGE:
			GUARD;
			if (!DeleteDC(hMemScreenDC)) appErrorf("DeleteDC failed %i",GetLastError());
			hMemScreenDC = CreateCompatibleDC (NULL);
			return 0;
			UNGUARD("WM_WININICHANGE");
		default:
			GUARD;
			return DefWindowProc (hWnd,iMessage,wParam,lParam);
			UNGUARD("DefWindowProc");
		};
	UNGUARD("FWindowsCameraManager::CameraWndProc");
	};

/*-----------------------------------------------------------------------------
	FWindowsCameraManager Init & Exit
-----------------------------------------------------------------------------*/

//
// Initialize the platform-specific camera manager subsystem.
// Must be called after the Unreal resource manager has been initialized.
// Must be called before any cameras are created.
//
void FWindowsCameraManager::Init(void)
	{
	GUARD;
	//
	// Define camera window class.  There can be zero or more camera windows.
	// They are usually visible, and they're not treated as children of the
	// server window (since the server is usually hidden/minimized).
	//
	CameraWndClass.style          = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | CS_OWNDC | CS_DBLCLKS;
	CameraWndClass.lpfnWndProc    = ::CameraWndProc;
	CameraWndClass.cbClsExtra     = 0;
	CameraWndClass.cbWndExtra     = 0;
	CameraWndClass.hInstance      = AfxGetInstanceHandle();
	CameraWndClass.hIcon          = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	CameraWndClass.hCursor        = NULL;
	CameraWndClass.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
	CameraWndClass.lpszMenuName   = "";
	CameraWndClass.lpszClassName  = CAMERA_NAME;
	if (!RegisterClass(&CameraWndClass)) GApp->Error ("RegisterClass failed");
	//
	// Create a working DC compatible with the screen, for CreateDibSection:
	//
	hMemScreenDC = CreateCompatibleDC (NULL);
	if (!hMemScreenDC) appErrorf("CreateCompatibleDC failed %i",GetLastError());
	//
	// Create a Windows logical palette:
	//
	LogicalPalette                = (LOGPALETTE *)appMalloc(sizeof(LOGPALETTE)+256*sizeof(PALETTEENTRY),"LogicalPalette");
	LogicalPalette->palVersion    = 0x300;
	LogicalPalette->palNumEntries = 256;
	//
	HDC Screen                    = GetDC(0);
	GetSystemPaletteEntries(Screen,0,256,LogicalPalette->palPalEntry);
	ReleaseDC(0,Screen);
	//
	for (int i=0; i<256; i++) LogicalPalette->palPalEntry[i].peFlags=PalFlags();
	//
	hLogicalPalette = CreatePalette(LogicalPalette);
	if (!hLogicalPalette) appErrorf("CreatePalette failed %i",GetLastError());
	//
	// DirectMouse:
	//
	OSVERSIONINFO Version; Version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&Version);
	if (Version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
		UseDirectMouse=1;
		GetONOFF(GApp->CmdLine,"DMOUSE=",&UseDirectMouse);
		}
	else UseDirectMouse=0;
	//
	// Init fullscreen information:
	//
	InitFullscreen();
	//
	// Add camera array to root of the resource tree:
	//
	CameraArray = new("Cameras", CREATE_Unique)TArray<UCamera>(MAX_CAMERAS);
	GRes.Root->Add(CameraArray);
	//
	// Initialize DirectDraw. This must happen after the camera array is allocated.
	//
	UseDirectDraw=1;
	GetONOFF(GApp->CmdLine,"DDRAW=",&UseDirectDraw);
	if (UseDirectDraw) ddInit();
	//
	debug(LOG_Init,"Camera manager initialized");
	//
	// Add this to the task list:
	//
	GTaskManager->AddTask(this,NULL,GApp,PRIORITY_Camera,TASK_NoUserKill);
	//
	UNGUARD("FWindowsCameraManager::Init");
	};

//
// Shut down the platform-specific camera manager subsystem.
//
void FWindowsCameraManager::Exit(void)
	{
	GUARD;
	//
	GRes.Root->Delete(CameraArray);
	//
	// Shut down DirectDraw:
	//
	if (UseDirectDraw) ddExit();
	//
	// Clean up Windows resources:
	//
	if (!DeleteObject(hLogicalPalette))	debugf(LOG_Exit,"DeleteObject failed %i",GetLastError());
	if (!DeleteDC	 (hMemScreenDC))	debugf(LOG_Exit,"DeleteDC failed %i",GetLastError());
	//
	debug(LOG_Exit,"Camera manager shut down");
	//
	UNGUARD("FWindowsCameraManager::Exit");
	};

//
// Failsafe routine to shut down camera manager subsystem
// after an error has occured. Not guarded.
//
void FWindowsCameraManager::ShutdownAfterError(void)
	{
	try
		{
		EndFullscreen();
		//
		SetCapture	(NULL);
  		ShowCursor  (TRUE);
		ClipCursor  (NULL);
		ddExit		();
		//
		for (int i=CameraArray->Num-1; i>=0; i--)
   			{
			UCamera *Camera = CameraArray->Element(i);
			DestroyWindow(Camera->Win.hWndCamera);
			};
		}
	catch(...)
		{
		debugf(LOG_Win,"Double fault in FWindowsCameraManager::ShutdownAfterError");
		};
	};

/*-----------------------------------------------------------------------------
	FWindowsCameraManager Camera Open, Close, Init

Order of calls when opening a camera:

	UCamera *MyCamera = new("MyCamera",CREATE_Unique)UCamera;
		// This allocates the camera as an Unreal resource.
		// UCamera::UCamera adds the camrea to CameraArray.
		// UCamera::UCamera assigns an actor to the camera.
		// UCamera::UCamera allocates a texture for the camera.
		// UCamera::InitHeader calls FCameraManager::InitCameraWindow().
		// On return, the camera resource is valid but its window is not, and it
		//     is still not visible.

	MyCamera->SXR=320; MyCamera->SYR=200; MyCamera->ColorBytes=1;
		// Here you are initializing the camera's vital properties that
		// must be set before opening the camera window.

	MyCamera->OpenCamera();
		// Here you are telling the UCamera resource to open up a window.
		// UCamera::OpenCamera calls FCameraManager::OpenCameraWindow().
		// GlobalCameraManager::OpenCameraWindow does whatever platform-specific
		//     stuff that is needed to open the camera window.
		// GlobalCameraManager::OpenCameraWindow sets the camera texture's size and data.
		// On return: If Temporary=1, the camera is visible. Otherwise it's invisible.
		// Now the camera is ready for use.

Order of calls when closing a camera via resource functions:

	MyCamera->Kill();
		// Here you are destroying the camera resource.
		// UCamera::PreKill destroys the camera's actor.
		// UCamera::PreKill calls FCameraManager::CloseCameraWindow.
		// UCamera::PreKill removes the camera from CameraArray.
		// UCamera::PreKill destroys the camera's texture.
		// The Unreal resource manager destroys the camera resource.

Order of calls when closing a camera by clicking on its close button:

-----------------------------------------------------------------------------*/

//
// Initialize the platform-specific information stored within the camera.
//
void FWindowsCameraManager::InitCameraWindow(UCamera *Camera)
	{
	GUARD;

	// Set color bytes based on screen resolution
	HWND hwndDesktop = GetDesktopWindow();
	HDC  hdcDesktop  = GetDC(hwndDesktop);
	switch(GetDeviceCaps(hdcDesktop,BITSPIXEL))
	{
		case 8:  Camera->ColorBytes = 1;  break;
		case 16: Camera->ColorBytes = 2;  break;
		case 24: Camera->ColorBytes = 2;  break;
		case 32: Camera->ColorBytes = 4;  break;
		default: Camera->ColorBytes = 16; break;
	}

	// Init other stuff
	ReleaseDC(hwndDesktop,hdcDesktop);
	Camera->Win.Status		= WIN_CameraOpening;
	Camera->Win.hWndCamera	= NULL;
	Camera->Win.BitmapInfo	= NULL;
	Camera->Win.hBitmap		= NULL;
	Camera->Win.hFile		= NULL;
	Camera->Win.NeedResize  = 0;
	Camera->Win.SaveCursor.x= -1;
	Camera->Win.Aspect		= Camera->SYR ? ((FLOAT)Camera->SXR/(FLOAT)Camera->SYR) : 1.0;
	UNGUARD("FWindowsCameraManager::InitCameraWindow");
	};

//
// Open a camera window.  Assumes that the camera has been initialized by
// InitCameraWindow().
//
// Before calling you should set the following UCamera members:
//		SXR,SYR		Screen X&Y resolutions, default is 320x200
//		ColorBytes	Color bytes (1,2,4), default is 1
//
void FWindowsCameraManager::OpenCameraWindow(UCamera *Camera,DWORD ParentWindow,int Temporary)
	{
	GUARD;
	UTexture		*Texture	= Camera->Texture;
	AActor			*Actor		= &Camera->GetActor();
	RECT       		rTemp;
	HWND       		hWnd;
	DWORD			Style;
	int				OpenX,OpenY,OpenXL,OpenYL,SetActive,IsNew;
	//
	// Align on 4-byte boundary for speed:
	//
	Camera->SXR			= Align4(Camera->SXR);
	Camera->SYR			= Camera->SYR;
	//
	// Create a DIB for the camera:
	//
	if (Temporary)
	   	{
		Camera->ColorBytes	= 1;
		//
		Texture->USize		= Camera->SXR;
		Texture->VSize		= Camera->SYR;
		Texture->DataSize	= Camera->SXR * Camera->SYR;
		//
		if (Camera->ColorBytes!=1) GApp->Error ("Can only open 8-bit temporary cameras");
		Camera->Texture->AllocData(0);
		debug (LOG_Info,"Opened temporary camera");
		//
		Camera->Win.hWndCamera = (HWND)NULL;
   		}
	else
	   	{
   		AllocateCameraDIB(Camera,BLIT_DEFAULT);
		//
		// Figure out size we must specify to get appropriate client area.
		//
		rTemp.left   = 100;
		rTemp.top    = 100;
		rTemp.right  = Camera->SXR+100;
		rTemp.bottom = Camera->SYR+100;
		//
		if (ParentWindow && (Actor->CameraStatus.ShowFlags & SHOW_ChildWindow))
			{
			Style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS;
   			AdjustWindowRect(&rTemp,Style,0);
			}
		else
			{
			Style = WS_OVERLAPPEDWINDOW;
   			AdjustWindowRect(&rTemp,Style,(Actor->CameraStatus.ShowFlags & SHOW_Menu)?TRUE:FALSE);
			};
		if (Camera->OpenX==-1)	OpenX = CW_USEDEFAULT; // Use default size
		else					OpenX = Camera->OpenX;
		//
		if (Camera->OpenY==-1)	OpenY = CW_USEDEFAULT; // Use default size
		else					OpenY = Camera->OpenY;
		//
		OpenXL = rTemp.right  - rTemp.left; 
		OpenYL = rTemp.bottom - rTemp.top;
		//
		// Create window:
		//
		if (Camera->Win.hWndCamera) // Resizing existing camera
			{
			if (Actor->CameraStatus.ShowFlags & SHOW_Menu) SetMenu(Camera->Win.hWndCamera,Camera->Win.hMenu);
			else SetMenu(Camera->Win.hWndCamera,NULL);
			//
			SetWindowPos(Camera->Win.hWndCamera,HWND_TOP,OpenX,OpenY,OpenXL,OpenYL,SWP_NOACTIVATE);
			SetActive = 0;
			IsNew = 0;
			}
		else // Creating new camera
			{
			Camera->ParentWindow	= ParentWindow;
			Camera->Win.hWndCamera 	= NULL; // So camera's message processor recognizes it.
			Camera->Win.Status		= WIN_CameraOpening; // Note that hWndCamera is unknown
			//
			Camera->Win.hMenu=LoadMenu
				(
				AfxGetInstanceHandle(),
				MAKEINTRESOURCE(GDefaults.LaunchEditor?IDR_EDITORCAM:IDR_PLAYERCAM)
				);
			if (ParentWindow && (Actor->CameraStatus.ShowFlags & SHOW_ChildWindow))
				{
				DeleteMenu(Camera->Win.hMenu,ID_WIN_TOP,MF_BYCOMMAND);
				};
			if ((!GDefaults.LaunchEditor) && (!GNetManager))
				{
				DeleteMenu(Camera->Win.hMenu,1,MF_BYPOSITION);
				DeleteMenu(Camera->Win.hMenu,ID_NETGAME,MF_BYCOMMAND);
				};
			if (!GApp->MMX) EnableMenuItem(Camera->Win.hMenu,ID_TRUE_COLOR,MF_BYCOMMAND|MF_GRAYED);
			//
			hWnd=CreateWindowEx(
				0,
				CAMERA_NAME,			// Class name
				CAMERA_NAME,			// Window name
				Style,					// Window style
				OpenX,          		// Window X, or CW_USEDEFAULT
				OpenY,          		// Window Y, or CW_USEDEFAULT
				OpenXL,					// Window Width, or CW_USEDEFAULT
				OpenYL,					// Window Height, or CW_USEDEFAULT
				(HWND)ParentWindow,		// Parent window handle or NULL
				Camera->Win.hMenu,		// Menu handle
				AfxGetInstanceHandle(), // Instance handle
				NULL);					// lpstr (NULL=unused)
			if (hWnd==NULL) GApp->Error("CreateWindow failed");
			if (!Camera->Win.hWndCamera) appErrorf("Camera window didn't recognize itself (%i)",CameraArray->Num);
			debug (LOG_Info,"Opened camera");
			SetActive = 1;
			IsNew = 1;
			//
			FindAvailableModes(Camera);
			};
		if (ParentWindow && (Actor->CameraStatus.ShowFlags & SHOW_ChildWindow)) // Force this to be a child
			{
			SetWindowLong(hWnd,GWL_STYLE,WS_VISIBLE|WS_POPUP);
			if (Actor->CameraStatus.ShowFlags & SHOW_Menu) SetMenu(hWnd,Camera->Win.hMenu);
			};
		if (Camera->SXR && Camera->SYR)
			{
			ShowWindow(Camera->Win.hWndCamera,SW_SHOWNORMAL);
			if (SetActive) SetActiveWindow(Camera->Win.hWndCamera);
			};
		Camera->Win.Aspect = Camera->SYR ? ((FLOAT)Camera->SXR/(FLOAT)Camera->SYR) : 1.0;
		if (!IsNew) Camera->Draw(0);
		};
	UNGUARD("FWindowsCameraManager::OpenCameraWindow");
	};

//
// Close a camera window.  Assumes that the camera has been openened with
// OpenCameraWindow.  Does not affect the camera's resource, only the
// platform-specific information associated with it.
//
void FWindowsCameraManager::CloseCameraWindow(UCamera *Camera)
	{
	GUARD;
	if ((Camera->Win.hWndCamera) && (Camera->Win.Status == WIN_CameraNormal))
		{
		Camera->Win.Status = WIN_CameraClosing; // So WM_DESTROY knows not to recurse
		//
		// WM_DETROY frees the camera's bitmaps and sets status to
		// WIN_CameraClosing.
		//
		DestroyWindow(Camera->Win.hWndCamera);
		};
	UNGUARD("FWindowsCameraManager::CloseCameraWindow");
	};

/*-----------------------------------------------------------------------------
	FWindowsCameraManager Camera Lock & Unlock
-----------------------------------------------------------------------------*/

//
// Lock the camera window and set the approprite Screen and RealScreen fields
// if CameraInfo.  Returns 1 if locked successfully, 0 if failed.  Note that a
// lock failing is not a critical error; it's a sign that a DirectDraw mode
// has ended or the user has closed a camera window.
//
int FWindowsCameraManager::LockCameraWindow(UCamera *Camera,ICamera *CameraInfo)
	{
	GUARD;
	//
	if (Camera->Win.hWndCamera)
		{
		if (!IsWindow(Camera->Win.hWndCamera))
	      	{
      		debugf (LOG_Win,"Lock: Window %i closed",(int)Camera->Win.hWndCamera);
      		return 0; // Window closed!
      		}
		CameraInfo->Data = (BYTE *)Camera->GetData();
		};
	//
	// Obtain pointer to screen
	//
	if (FullscreenCamera && FullscreenhWndDD)
		{
		HRESULT Result;
  		if (ddFrontBuffer->IsLost() == DDERR_SURFACELOST)
			{
			Result = ddFrontBuffer->Restore();
   			if (Result != DD_OK) debugf(LOG_Win,"DirectDraw Lock Restore failed %s",ddError(Result));
			EndFullscreen();
			return 0;
			};
		ZeroMemory(&ddSurfaceDesc,sizeof(ddSurfaceDesc));
  		ddSurfaceDesc.dwSize = sizeof(ddSurfaceDesc);
		//
		Result = ddBackBuffer->Lock(NULL,&ddSurfaceDesc,DDLOCK_WAIT,NULL);
  		if (Result != DD_OK)
			{
			debugf(LOG_Win,"DirectDraw Lock failed: %s",ddError(Result));
  			return 0;
			};
		CameraInfo->RealScreen  = (BYTE *)ddSurfaceDesc.lpSurface;
		//
		if (ddSurfaceDesc.lPitch)	CameraInfo->SXStride = ddSurfaceDesc.lPitch/CameraInfo->ColorBytes;
		else						CameraInfo->SXStride = CameraInfo->SXR;
		//
		Camera->Texture->Data		= CameraInfo->RealScreen;
		if (CameraInfo->RealScreen==NULL)
			{
			debug(LOG_Win,"Lock: DirectDraw Lock Refused");
			EndFullscreen();
			return 0;
			};
		}
	else CameraInfo->RealScreen = Camera->Texture->GetData();
	if (CameraInfo->RealScreen==NULL) return 0;
	//
	Camera->Texture->CameraCaps = Camera->Caps;
	return 1;
	//
	UNGUARD("FWindowsCameraManager::LockCameraWindow");
	};

//
// Unlock the camera window.  If Blit=1, blits the camera's frame buffer.
//
void FWindowsCameraManager::UnlockCameraWindow(UCamera *Camera,ICamera *CameraInfo,int Blit)
	{
	GUARD;
	//
	DrawTime=0;
	ALWAYS_BEGINTIME(DrawTime);
	//
	// Unlock DirectDraw:
	//
	if (FullscreenCamera && FullscreenhWndDD)
		{
		HRESULT Result;
		Result = ddBackBuffer->Unlock(ddSurfaceDesc.lpSurface);
		if (Result!=DD_OK) debugf(LOG_Win,"DirectDraw Unlock: %s",ddError(Result));
		};
	//
	// Blit, if desired:
	//
	if (Blit && Camera->Win.hWndCamera && IsWindow(Camera->Win.hWndCamera) && !Camera->OnHold)
		{
		if ((FullscreenCamera == Camera) && FullscreenhWndDD)
			{
			// Blitting with DirectDraw
			HRESULT Result = ddFrontBuffer->Flip(NULL,DDFLIP_WAIT);
			if (Result != DD_OK)
				{
				debugf(LOG_Win,"DirectDraw Flip failed: %s",ddError(Result));
				EndFullscreen();
				};
			}
		else
			{
			// Blitting with CreateDIBSection
			if (CameraInfo->Screen && Camera->Win.hBitmap)
				{
				HDC hDC=GetDC	(Camera->Win.hWndCamera);
				if (hDC==NULL) GApp->Error ("GetDC failed");
				SelectObject	(hMemScreenDC,Camera->Win.hBitmap);
				BitBlt			(hDC,0,0,Camera->SXR,Camera->SYR,hMemScreenDC,0,0,SRCCOPY);
				ReleaseDC		(Camera->Win.hWndCamera,hDC);
				};
			};
		};
	ALWAYS_ENDTIME(DrawTime);
	//
	UNGUARD("FWindowsCameraManager::UnlockCameraWindow");
	};

/*-----------------------------------------------------------------------------
	FWindowsCameraManager Cursor & Update Functions
-----------------------------------------------------------------------------*/

//
// Set the mouse cursor according to Unreal or UnrealEd's mode, or to
// an hourglass if a slow task is active.
//
// TO DO: Improve so that the cursor is set properly according to whatever
// ctrl/alt/etc UnrealEd mode keys are held down; this currently only
// recognizes mode keys if the camera has focus and receives keyboard
// messages. Win32 makes this hard.
//
void FWindowsCameraManager::SetModeCursor(UCamera *Camera)
	{
	GUARD;
	//
	if (GApp->InSlowTask)
		{
		SetCursor (LoadCursor(NULL,IDC_WAIT));
		return;
		};
	int Mode = GEditor ? GEditor->edcamMode(Camera) : EM_None;
	HCURSOR hCursor;
	//
	switch (Mode)
		{
		case EM_None: 			hCursor = LoadCursor(NULL,IDC_CROSS); break;
		case EM_CameraMove: 	hCursor = LoadCursor(NULL,IDC_CROSS); break;
		case EM_CameraZoom:		hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_CAMERAZOOM)); break;
		case EM_BrushFree:		hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_BRUSHFREE)); break;
		case EM_BrushMove:		hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_BRUSHMOVE)); break;
		case EM_BrushRotate:	hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_BRUSHROT)); break;
		case EM_BrushSheer:		hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_BRUSHSHEER)); break;
		case EM_BrushScale:		hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_BRUSHSCALE)); break;
		case EM_BrushStretch:	hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_BRUSHSTRETCH)); break;
		case EM_BrushSnap:		hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_BRUSHSNAP)); break;
		case EM_BrushWarp:		hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_BRUSHWARP)); break;
		case EM_AddActor:		hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_ADDACTOR)); break;
		case EM_MoveActor:		hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_MOVEACTOR)); break;
		case EM_TexturePan:		hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_TEXPAN)); break;
		case EM_TextureSet:		hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_TEXSET)); break;
		case EM_TextureRotate:	hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_TEXROT)); break;
		case EM_TextureScale:	hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_TEXSCALE)); break;
		case EM_Terraform:		hCursor = LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_TERRAFORM)); break;
		case EM_TexView:		hCursor = LoadCursor(NULL,IDC_ARROW); break;
		case EM_TexBrowser:		hCursor = LoadCursor(NULL,IDC_ARROW); break;
		case EM_MeshView:		hCursor = LoadCursor(NULL,IDC_CROSS); break;
		case EM_MeshBrowser:	hCursor = LoadCursor(NULL,IDC_ARROW); break;
		default: 				hCursor = LoadCursor(NULL,IDC_ARROW); break;
		};
	if (!hCursor) GApp->Error ("Cursor not found");
	SetCursor (hCursor);
	//
	UNGUARD("FWindowsCameraManager::SetModeCursor");
	};

void FWindowsCameraManager::UpdateCameraWindow(UCamera *Camera)
	{
	GUARD;
	AActor	*Actor		= &Camera->GetActor();
	DWORD	RendMap		= Actor->CameraStatus.RendMap;
	DWORD	ShowFlags	= Actor->CameraStatus.ShowFlags;
	HMENU	hMenu		= Camera->Win.hMenu;
	ULevel	*Level;
	char 	WindowName [80];
	//
	if (Camera->Win.NeedResize)
		{
		ResizeCameraFrameBuffer (Camera,Camera->Win.ResizeSXR,Camera->Win.ResizeSYR,Camera->Win.ResizeColorBytes,BLIT_DEFAULT,1);
		};
	if ((Camera->Win.hWndCamera==NULL)||(Camera->OnHold)) return;
	//
	// Set camera window's name to show resolution
	//
	if ((Camera->Level->GetState()==LEVEL_UpPlay)||((Actor->CameraStatus.ShowFlags&SHOW_PlayerCtrl)))
		{
		Level = Camera->Level;
		sprintf(WindowName,"Unreal");
		}
	else
		{
		switch (Actor->CameraStatus.RendMap)
			{
			case REN_Wire:		strcpy(WindowName,"Persp map"); break;
			case REN_OrthXY:	strcpy(WindowName,"Overhead map"); break;
			case REN_OrthXZ:	strcpy(WindowName,"XZ map"); break;
			case REN_OrthYZ:	strcpy(WindowName,"YZ map"); break;
			default:			strcpy(WindowName,CAMERA_NAME); break;
			};
		};
	if (Camera->SXR && Camera->SYR)
		{
		sprintf(WindowName+strlen(WindowName)," (%i x %i)",Camera->SXR,Camera->SYR);
		if (Camera == CurrentCamera()) strcat (WindowName," *");
		};
	SetWindowText(Camera->Win.hWndCamera,WindowName);
	//
	if (Actor->CameraStatus.ShowFlags & SHOW_Menu) SetMenu(Camera->Win.hWndCamera,Camera->Win.hMenu);
	else SetMenu(Camera->Win.hWndCamera,NULL);
	//
	// Update menu, Map rendering:
	//
	CheckMenuItem(hMenu,ID_MAP_PLAINTEX, (RendMap==REN_PlainTex  ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_DYNLIGHT, (RendMap==REN_DynLight  ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_WIRE,     (RendMap==REN_Wire      ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_OVERHEAD, (RendMap==REN_OrthXY    ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_XZ, 		 (RendMap==REN_OrthXZ    ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_YZ, 		 (RendMap==REN_OrthYZ    ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_POLYS,    (RendMap==REN_Polys     ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_POLYCUTS, (RendMap==REN_PolyCuts  ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_ZONES,    (RendMap==REN_Zones     ? MF_CHECKED:MF_UNCHECKED));
	//
	// Show-attributes:
	//
	CheckMenuItem(hMenu,ID_SHOW_BRUSH,    ((ShowFlags&SHOW_Brush			)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_SHOW_BACKDROP, ((ShowFlags&SHOW_Backdrop  		)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_SHOW_COORDS,   ((ShowFlags&SHOW_Coords    		)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_SHOW_MOVINGBRUSHES,((ShowFlags&SHOW_MovingBrushes)?MF_CHECKED:MF_UNCHECKED));
	//
	// Actor showing:
	//
	DWORD ShowFilter = ShowFlags & (SHOW_Actors | SHOW_ActorIcons | SHOW_ActorRadii);
	CheckMenuItem(hMenu,ID_ACTORS_ICONS,MF_UNCHECKED);
	CheckMenuItem(hMenu,ID_ACTORS_RADII,MF_UNCHECKED);
	CheckMenuItem(hMenu,ID_ACTORS_SHOW,MF_UNCHECKED);
	CheckMenuItem(hMenu,ID_ACTORS_HIDE,MF_UNCHECKED);
	//
	if		(ShowFilter==(SHOW_Actors | SHOW_ActorIcons)) CheckMenuItem(hMenu,ID_ACTORS_ICONS,MF_CHECKED);
	else if (ShowFilter==(SHOW_Actors | SHOW_ActorRadii | SHOW_ActorIcons)) CheckMenuItem(hMenu,ID_ACTORS_RADII,MF_CHECKED);
	else if (ShowFilter==(SHOW_Actors)) CheckMenuItem(hMenu,ID_ACTORS_SHOW,MF_CHECKED);
	else CheckMenuItem(hMenu,ID_ACTORS_HIDE,MF_CHECKED);
	//
	// Color depth:
	//
	CheckMenuItem(hMenu,ID_COLOR_8BIT, ((Camera->ColorBytes==1)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_COLOR_16BIT,((Camera->ColorBytes==2)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_COLOR_32BIT,((Camera->ColorBytes==4)?MF_CHECKED:MF_UNCHECKED));
	//
	UNGUARD("FWindowsCameraManager::UpdateCameraWindow");
	};

//
// Enable or disable all camera windows that have ShowFlags set (or all if ShowFlags=0).
//
void FWindowsCameraManager::EnableCameraWindows(DWORD ShowFlags,int DoEnable)
	{
	GUARD;
  	for (int i=0; i<CameraArray->Num; i++)
	  	{
		UCamera *Camera   = CameraArray->Element(i);
		AActor  *Actor    = &Camera->GetActor();
		if ((Actor->CameraStatus.ShowFlags & ShowFlags)==ShowFlags)
			{
			EnableWindow(Camera->Win.hWndCamera,DoEnable);
			};
		};
	UNGUARD("FWindowsCameraManager::EnableCameraWindows");
	};

//
// Show or hide all camera windows that have ShowFlags set (or all if ShowFlags=0).
//
void FWindowsCameraManager::ShowCameraWindows(DWORD ShowFlags,int DoShow)
	{
	GUARD;
  	for (int i=0; i<CameraArray->Num; i++)
	  	{
		UCamera *Camera   = CameraArray->Element(i);
		AActor  *Actor    = &Camera->GetActor();
		if ((Actor->CameraStatus.ShowFlags & ShowFlags)==ShowFlags)
			{
			ShowWindow(Camera->Win.hWndCamera,DoShow?SW_SHOWNORMAL:SW_HIDE);
			};
		};
	UNGUARD("FWindowsCameraManager::ShowCameraWindows");
	};

/*-----------------------------------------------------------------------------
	FWindowsCameraManager Palette Functions
-----------------------------------------------------------------------------*/

//
// Set the palette in all active camera windows.  Call with both a regular
// palette and a gamma-corrected palette and the appropriate one will be.
// chosed.  Currently, the gamma-corrected palette is only used when in
// fullscreen mode because of 256-color Windows 95 GDI bugs which cause
// palettes to clash when changed dynamically; this bug results in colors going
// awry in UnrealEd if the palette is changed.
//
void FWindowsCameraManager::SetPalette(UPalette *Palette,UPalette *GammaPalette)
	{
	GUARD;
	PALETTEENTRY	PaletteEntry[256],GammaPaletteEntry[256];
	RGBQUAD			RGBColors[256],GammaRGBColors[256];
	//
	memcpy(PaletteEntry,     Palette->GetData(),     256*4); // Assumes same palette formats
	memcpy(GammaPaletteEntry,GammaPalette->GetData(),256*4);
	//
	for (int i=10; i<246; i++)
		{
		PaletteEntry 	[i].peFlags     = PalFlags();
		GammaPaletteEntry[i].peFlags    = 0;
		//
		RGBColors		[i].rgbRed      = PaletteEntry[i].peRed;
     	RGBColors    	[i].rgbGreen    = PaletteEntry[i].peGreen;
     	RGBColors    	[i].rgbBlue     = PaletteEntry[i].peBlue;
		RGBColors    	[i].rgbReserved = 0;
		//
		GammaRGBColors	[i].rgbRed      = GammaPaletteEntry[i].peRed;
     	GammaRGBColors	[i].rgbGreen    = GammaPaletteEntry[i].peGreen;
     	GammaRGBColors	[i].rgbBlue     = GammaPaletteEntry[i].peBlue;
		GammaRGBColors	[i].rgbReserved = 0;
		};
	if (!SetPaletteEntries(hLogicalPalette,10,236,&PaletteEntry[10])) appErrorf("SetPaletteEntries failed %i",GetLastError());
	//
	if (!FullscreenCamera) // Set palette in all open camera windows:
		{
		for (i=0; i<CameraArray->Num; i++)
	   		{
			UCamera *Camera = CameraArray->Element(i);
			//
			if ((Camera->ColorBytes==1) && Camera->Win.hBitmap)
				{
				if (!SelectObject(hMemScreenDC,Camera->Win.hBitmap))		appErrorf("SelectObject failed %i",GetLastError());
				if (!SetDIBColorTable(hMemScreenDC,10,236,&RGBColors[10]))	appErrorf("SetDIBColorTable failed %i",GetLastError());
				//
  				HDC hDC = GetDC (Camera->Win.hWndCamera); if (!hDC)			appErrorf("GetDC failed %i",GetLastError());
				if (!SelectPalette(hDC,hLogicalPalette,PalFlags()==0))		appErrorf("SelectPalette failed %i",GetLastError());
   				if (RealizePalette(hDC)==GDI_ERROR)							appErrorf("RealizePalette failed %i",GetLastError());
   				if (!ReleaseDC(Camera->Win.hWndCamera,hDC))					appErrorf("ReleaseDC failed %i",GetLastError());
				};
			};
		}
	else // We are in fullscreen DirectDraw
		{
		if (FullscreenCamera->ColorBytes==1)
			{
			if (FullscreenhWndDD) 
				{
				HRESULT Result = ddPalette->SetEntries(0,0,256,GammaPaletteEntry);
				if (Result!=DD_OK) appErrorf("SetEntries failed: %s",ddError(Result));
				}
			else appError("Fullscreen camera not found");
			};
		};
	UNGUARD("FWindowsCameraManager::SetPalette");
	};

/*-----------------------------------------------------------------------------
	FWindowsCameraManager Fullscreen Functions
-----------------------------------------------------------------------------*/

//
// If in fullscreen mode, end it and return to Windows.
//
void FWindowsCameraManager::EndFullscreen(void)
	{
	GUARD;
	UCamera	*Camera = FullscreenCamera;
	//
	if (Camera)
		{
		StopClippingCursor (Camera,0);
		//
		RECT *Rect		= &Camera->Win.SavedWindowRect;
		int  SXR		= Camera->Win.SavedSXR;
		int  SYR		= Camera->Win.SavedSYR;
		int  ColorBytes = Camera->Win.SavedColorBytes;
		int  Caps		= Camera->Win.SavedCaps;
		//
		Camera->Hold();
		if (FullscreenhWndDD)
			{
			if (Camera->Caps & CC_Hardware3D)
				{
				Camera->Caps &= ~CC_Hardware3D;
				GApp->RenDev->Exit3D();
				};
			debug(LOG_Win,"DirectDraw session ending");
			ddEndMode();
			}
		else GApp->Error ("Unknown fullscreen mode");
		//
		InitFullscreen	();
		MoveWindow		(Camera->Win.hWndCamera,Rect->left,Rect->top,Rect->right-Rect->left,Rect->bottom-Rect->top,1);
		//
		Camera->Unhold();
		Camera->ColorBytes = 0; // Force resize frame buffer
		//
		ResizeCameraFrameBuffer	(Camera,SXR,SYR,ColorBytes,BLIT_DIBSECTION,0);
		GGfx.SetPalette			();
		SetOnTop				(Camera->Win.hWndCamera); // Fix always-on-top status affected by DirectDraw
		};
	if (DMouseHandle) dmEnd();
	//
	UNGUARD("FWindowsCameraManager::EndFullscreen");
	};

/*-----------------------------------------------------------------------------
	FWindowsCameraManager Polling & Timing Functions
-----------------------------------------------------------------------------*/

//
// Perform background processing.  Should be called 100 times
// per second or more fore besr results.
//
void FWindowsCameraManager::Poll(void)
	{
	GUARD;
	//
	QWORD Time = GApp->TimeMSec();
	//
	// Tell DirectDraw to lock its locked surfaces.  When a DirectDraw surface
	// is locked, the Win16Mutex is held, preventing DirectSound mixing from
	// taking place.  If a DirectDraw surface is locked for more than
	// approximately 1/100th of a second, skipping can occur in the audio output.
	//
	//
	static QWORD LastTime = 0;
	if (dd && ddFrontBuffer && ((Time-LastTime) > DD_POLL_TIME))
		{
		HRESULT Result;
		Result   = ddBackBuffer->Unlock(ddSurfaceDesc.lpSurface);
		Result   = ddBackBuffer->Lock  (NULL,&ddSurfaceDesc,DDLOCK_WAIT,NULL);
		LastTime = Time;
		};
	UNGUARD("FWindowsCameraManager::Poll");
	};

/*-----------------------------------------------------------------------------
	Task functions
-----------------------------------------------------------------------------*/

//
// Perform timer-tick processing on all visible cameras.  This causes
// all realtime cameras, and all non-realtime cameras which have been
// updated, to be blitted.
//
void FWindowsCameraManager::TaskTick(void)
	{
	GUARD;
	//
	if (!GApp->ServerAlive) return;
	//
	// Exit if we're in game mode and all cameras are closed:
	//
	if ((CameraArray->Num==0) && (!GDefaults.LaunchEditor))
		{
		debug (LOG_Exit,"Tick: Requesting exit");
		GApp->RequestExit();
		return;
		};
	//
	// Blit any cameras that need blitting:
	//
	UCamera *BestCamera = NULL;
  	for (int i=0; i<CameraArray->Num; i++)
	   	{
		UCamera *Camera = CameraArray->Element(i);
		if (!IsWindow(Camera->Win.hWndCamera)) // Window was closed via close button
			{
			Camera->Kill();
			return;
			};
  		if (Camera->IsRealtime() && ((Camera==FullscreenCamera) || (FullscreenCamera==NULL)))
			{
			if (Camera->SXR && Camera->SYR && !Camera->OnHold)
				{
				if ((!BestCamera) || (Camera->LastUpdateTime < BestCamera->LastUpdateTime))
					{
					BestCamera = Camera;
					};
				};
			};
     	};
	if (BestCamera)
		{
		BestCamera->Draw(0);
		BestCamera->LastUpdateTime = GApp->TimeMSec();
		};
	UNGUARD("FWindowsCameraManager::TaskTick");
	};

void FWindowsCameraManager::TaskExit(void)
	{
	GUARD;
	//
	debugf(LOG_Exit,"Camera manager task exiting");
	GApp->RequestExit();
	//
	UNGUARD("FWindowsCameraManager::TaskExit");
	};

char *FWindowsCameraManager::TaskStatus(char *Name, char *Desc)
	{
	GUARD;
	//
	sprintf(Name,"CameraManager");
	sprintf(Desc,"%i Cameras",CameraArray->Num);
	return Name;
	//
	UNGUARD("FWindowsCameraManager::TaskStatus");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
