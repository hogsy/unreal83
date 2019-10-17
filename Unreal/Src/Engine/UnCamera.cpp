/*=============================================================================
	UnCamerea.cpp: Generic Unreal camera code

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	UCamera resource implementation
-----------------------------------------------------------------------------*/

//
// Resource functions
//
void UCamera::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UCamera);
	Type->RecordSize = 0;
	Type->Version    = 1;
	Type->TypeFlags  = RTF_ScriptReferencable;
	strcpy (Type->Descr,"Camera");
	UNGUARD("UCamera::Register");
	};
void UCamera::InitHeader(void)
	{
	GUARD;
	//
	// Start camera looking ahead, facing along the X axis, looking
	// towards the origin:
	//
	Locked      = 0;
	OnHold		= 0;
	Texture     = NULL;
	Level       = GServer.Levels->Element(0); // Temp hack !!
	iActor      = MAXWORD;
	MiscRes		= NULL;
	MiscName	= NAME_NONE;
	DataSize    = 0;
	LastUpdateTime = 0;
	//
	// Default sizes (can be changed before call to pOpenCameraWindow):
	//
	SXR         = GDefaults.CameraSXR;
	SYR         = GDefaults.CameraSYR;
	Caps		= 0;
	//
	Current		= 0;
	ClickFlags  = 0;
	//
	OpenX		  = -1;				// No default opening position
	OpenY		  = 0;				// No default opening size
	ParentWindow  = 0;
	//
	GCameraManager->InitCameraWindow (this);
	UNGUARD("UCamera::InitHeader");
	};
void UCamera::FreeData(void)
	{
	GUARD;
	GCameraManager->CloseCameraWindow(this);
	UNGUARD("UCamera::FreeData");
	};
void UCamera::QueryHeaderReferences(FResourceCallback &Callback)
	{
	GUARD;
	Callback.Resource (this,(UResource **)&Texture,0);
	UNGUARD("UCamera::QueryHeaderReferences");
	};
AUTOREGISTER_RESOURCE(RES_Camera,UCamera,0xB2D90852,0xCCD211cf,0x91360000,0xC028B992);

/*-----------------------------------------------------------------------------
	Custom camera creation and destruction
-----------------------------------------------------------------------------*/

//
// Create a new camera and return an ID to it or NULL.  Does not open a window on screen.
// Call pOpenCameraWindow for that.  Set Level to the level, or NULL=first level on file.
//
// Cameras can't be tracked transactionally because of the special textures that must go
// along with cameras.
//
UCamera::UCamera(ULevel *CLevel)
	{
	GUARD;
	ILevel			LevelInfo;
	AActor			*Actor;
	//
	if (CLevel==NULL)
		{
		if (GServer.Levels->Num==0)	appError ("Can't create camera in empty world");
		else						Level = GServer.Levels->Element(0);
		}
	else Level = CLevel;
	//
	Level->Lock(&LevelInfo,LOCK_NoTrans);
	//
	GCameraManager->CameraArray->Add(this);
	//
	// Find an available camera actor or spawn a new one.  Sends ACTOR_SPAWN message followed
	// by ACTOR_POSSESS message.
	//
	if (LevelInfo.State==LEVEL_UpEdit)
		{
		iActor = LevelInfo.SpawnCameraActor (this,NAME_NONE);
		if (iActor==INDEX_NONE) appError ("SpawnCameraActor failed");
		}
	else
		{
		iActor = LevelInfo.SpawnPlayActor (this);
		if (iActor==INDEX_NONE) appError
			(
			"Can't play this level: No 'PlayerStart' actor was found to "
			"specify the player's starting position."
			);
		};
	Actor							= &GetActor();
	Actor->Camera					= this;
	Actor->CameraStatus.ShowFlags	= GGfx.DefaultCameraFlags;
	Actor->CameraStatus.RendMap		= GGfx.DefaultRendMap;
	Actor->CameraStatus.OrthoZoom	= 40000.0;
	Actor->CameraStatus.FOVAngle	= GDefaults.FOV;
	Actor->CameraStatus.Misc1		= 0;
	Actor->CameraStatus.Misc2		= 0;
	//
	// Allocate texture for the camera:
	//
	Texture = new(Name,CREATE_Unique)UTexture;
	//
	// Init player console:
	//
	Console = GVirtualGame->CreateCameraConsole(this);
	Console->Init(this);
	//
	Level->Unlock(&LevelInfo);
	UNGUARD("UCamera::UCamera");
	};

//
// Close a camera.
// WARNING: Lots of interrelated stuff is happening here.
//
void UCamera::PreKill()
	{
	GUARD;
	ICamera CameraInfo;
	//
	// Unlink then delete actor unless camera is already locked (which happens
	// when camera is closed upon destroying a camera actor):
	//
	if (Lock(&CameraInfo))
		{
		CameraInfo.Actor->Camera = NULL;
		CameraInfo.Level.DestroyActor(CameraInfo.iActor);
		Unlock(&CameraInfo,0);
		Console->Exit();
		};
	GVirtualGame->DestroyCameraConsole(Console);
	//
	// Close the camera window:
	//
	GCameraManager->CloseCameraWindow(this);
	//
	// Shut down Windows resources.
	//
	GCameraManager->CameraArray->Delete(this);
	Texture->Kill();
	//
	// Remove resource reference from global camera list:
	//
	UNGUARD("UCamera::PreKill");
	};

/*-----------------------------------------------------------------------------
	Camera action handlers
-----------------------------------------------------------------------------*/

//
// Handle all camera movement.  Returns 1 if the camera movement was processed
// by UnrealEd, or 0 if it should be queued up for later gameplay use.
//
int UCamera::Move(BYTE Buttons, SWORD MouseX, SWORD MouseY, int Shift, int Ctrl)
	{
	GUARD;
	//
	if (GEditor)
		{
		GEditor->edcamMove (this,Buttons,MouseX,MouseY,Shift,Ctrl);
		return 1;
		}
	else return 0;
	//
	UNGUARD("UCamera::Move");
	};

//
// Handle general keypresses.  Routes to a mode-specific handler.
//
int UCamera::Key(int Key)
	{
	GUARD;
	//
	if (Console->Key(Key)) return 1; // Player console keypress handler
	//
	if (GEditor) return GEditor->edcamKey (this,Key);
	else return 0;
	//
	UNGUARD("UCamera::Key");
	};

//
// General purpose mouse click handling. Returns 1 if processed, 0 if not.
//
int UCamera::Click (BYTE Buttons, SWORD MouseX, SWORD MouseY,int Shift, int Ctrl)
	{
	GUARD;
	//
	ELevelState State = Level->GetState();
	if (State==LEVEL_UpEdit)
		{
		if (GEditor) GEditor->edcamClick (this,Buttons,MouseX,MouseY,Shift,Ctrl);
		return 1;
		}
	else return 0;
	//
	UNGUARD("UCamera::Click");
	};

void UCamera::Draw(int Scan)
	{
	if ((SXR>0)&&(SYR>0))
		{
		if (GEditor)	GEditor->edcamDraw(this,Scan);
		else			GUnreal.Draw(this,Scan);
		};
	};

/*---------------------------------------------------------------------------------------
	ICamera implementation
---------------------------------------------------------------------------------------*/

//
// Build camera's coordinate system:
//
void ICamera::BuildCoords(void)
	{
	GUARD;
	FCalcViewCoords(Actor->Location,Actor->ViewRot,Coords,Uncoords);
	UNGUARD("ICamera::BuildCoords");
	};

//
// If camera is in an orthogonal mode, return its viewing plane normal.
// Otherwise, return NULL.
//
FVector *ICamera::GetOrthoNormal(void)
	{
	switch(RendMap)
		{
		case REN_OrthXY:	return &GMath.ZAxisVector;
		case REN_OrthXZ:	return &GMath.YAxisVector;
		case REN_OrthYZ:	return &GMath.XAxisVector;
		default:			return NULL;
		};
	};

//
// Precompute CameraInfo rendering parameters.
//
void ICamera::PrecomputeRenderInfo(int CamSXR, int CamSYR)
	{
	//
	// Stride:
	//
	Camera->Texture->USize = SXStride;
	//
	// Sizing:
	//
	SXR 		= CamSXR;
	SYR 		= CamSYR;
	//
	// Integer precomputes:
	//
	SXR2 		= SXR/2;
	SYR2 		= SYR/2;
	FixSXR 		= FIX(SXR);
	//
	// Float precomputes:
	//
	FSXR 		= (FLOAT)SXR;
	FSYR 		= (FLOAT)SYR;
	FSXR1 		= 65536.0 * (FSXR + 1);
	FSYR1 		= FSYR + 1;
	FSXR2		= FSXR * 0.5;
	FSYR2		= FSYR * 0.5;	
	FSXR15		= (FSXR+1.0001) * 0.5;
	FSYR15		= (FSYR+1.0001) * 0.5;	
	//
	ProjZ		= FCalcFOV (FSXR, FOVAngle);
	Zoom 		= OrthoZoom / (SXR * 15.0);
	RZoom       = 1.0/Zoom;
	RProjZ		= 1.0/ProjZ;
	//
	// The following have a fudge factor to adjust for floating point imprecision in
	// the screenspace clipper.
	//
	ProjZRSX2	= 1.00005 * ProjZ / FSXR2;
	ProjZRSY2	= 1.00005 * ProjZ / FSYR2;
	//
	// Calculate camera view coordinates:
	//
	BuildCoords();
	//
	// Precomputed camera view frustrum edges (worldspace):
	//
	FLOAT TempSigns[2]={-1.0,+1.0};
	for (int i=0; i<2; i++) for (int j=0; j<2; j++)
		{
		ViewSides[i*2+j].X = TempSigns[i] * FSXR2;
		ViewSides[i*2+j].Y = TempSigns[j] * FSYR2;
		ViewSides[i*2+j].Z = ProjZ;
		ViewSides[i*2+j].Normalize();
		ViewSides[i*2+j].TransformVector(Uncoords);
		};
	};

/*---------------------------------------------------------------------------------------
	Camera information functions
---------------------------------------------------------------------------------------*/

int UCamera::IsOrtho(void)
	{
	GUARD;
	int RendMap = GetActor().CameraStatus.RendMap;
	return ((RendMap==REN_OrthXY)||(RendMap==REN_OrthXZ)||(RendMap==REN_OrthYZ));
	UNGUARD("UCamera::IsOrtho");
	};

int UCamera::IsRealWire(void)
	{
	GUARD;
	int RendMap = GetActor().CameraStatus.RendMap;
	return ((RendMap==REN_OrthXY)||(RendMap==REN_OrthXZ)||(RendMap==REN_OrthYZ)||(RendMap==REN_Wire));
	UNGUARD("UCamera::IsRealWire");
	};

int UCamera::IsRealtime(void)
	{
	GUARD;
	return (GetActor().CameraStatus.ShowFlags & (SHOW_RealTime | SHOW_PlayerCtrl))!=0;
	UNGUARD("UCamera::IsRealTime");
	};

int UCamera::IsInvalidBsp(void)
	{
	GUARD;
	if (Level->Model->ModelFlags & MF_InvalidBsp) return 1;
	else return 0;
	UNGUARD("UCamera::IsInvalidBsp");
	};

int UCamera::IsWire(void)
	{
	GUARD;
	int RendMap = GetActor().CameraStatus.RendMap;
	return ((GEditor&&GEditor->MapEdit) || (RendMap==REN_OrthXY)||(RendMap==REN_OrthXZ)||(RendMap==REN_OrthYZ)||(RendMap==REN_Wire));
	UNGUARD("UCamera::IsWire");
	};

int UCamera::WireMode(void) // Returns wireframe mode for camera
	{
	GUARD;
	int RendMap = GetActor().CameraStatus.RendMap;
	if ((RendMap==REN_OrthXY)||(RendMap==REN_OrthXZ)||(RendMap==REN_OrthYZ)||(RendMap==REN_Wire))
		{
		return RendMap;
		}
	else return REN_Wire;
	//
	UNGUARD("UCamera::WireMode");
	};

int UCamera::IsBrowser(void)
	{
	GUARD;
	//
	int RendMap = GetActor().CameraStatus.RendMap;
	switch (RendMap)
		{
		case REN_TexView:		return 1;
		case REN_TexBrowser:	return 1;
		case REN_MeshBrowser:	return 1;
		default:				return 0;
		};
	UNGUARD("UCamera::IsBrowser");
	};

int UCamera::IsGame(void)
	{
	GUARD;
	return Level->GetState()==LEVEL_UpPlay;
	UNGUARD("UCamera::IsGame");
	};

int UCamera::IsEditor (void)
	{
	GUARD;
	return Level->GetState()==LEVEL_UpEdit;
	UNGUARD("UCamera::IsGame");
	};

AActor &UCamera::GetActor(void)
	{
	GUARD;
	if (iActor==INDEX_NONE) appError ("INDEX_NONE");
	return Level->ActorList->Element(iActor);
	UNGUARD("UCamera::GetActor");
	};

/*-----------------------------------------------------------------------------
	Camera locking & unlocking
-----------------------------------------------------------------------------*/

//
// Locks the camera structure.  When locked, a Camera's virtual buffer will
// not be resized by other processes.  When a camera is unlocked, you can't
// count on the size remaining constant (the user may resize it or close it).
//
// Returns 0 if ok, nonzero if the user has closed the camera window.  Any code that
// locks the camera must check for a nonzero result, which can happen in the normal
// course of operation.
//
int UCamera::Lock (ICamera *CameraInfo)
	{
	GUARD;
	if (Locked) appError("Camera is already locked");
	if (OnHold || (SXR==0) || (SYR==0)) return 0;
	//
	// Copy all information:
	//
	CameraInfo->Camera 		= this;
	CameraInfo->iActor 		= iActor;
	CameraInfo->SXR			= SXR;
	CameraInfo->SYR			= SYR;
	CameraInfo->SXStride	= SXR;
	CameraInfo->Caps		= Caps;
	CameraInfo->ColorBytes	= ColorBytes;
	CameraInfo->Texture		= Texture;
	//
	// Call the platform-specific camera manager to lock the camera's window
	// and frame buffer.  It sets CameraInfo->SXStride and CameraInfo->RealScreen.
	//
	if (!GCameraManager->LockCameraWindow(this,CameraInfo)) return 0; // Sets SXStride, RealScreen
	CameraInfo->Screen = CameraInfo->RealScreen;
	//
	// Lock the level and snag the camera actor:
	//
	Level->Lock(&CameraInfo->Level,LOCK_Trans);
	CameraInfo->Actor		= &GetActor();
	//
	CameraInfo->RendMap 	= CameraInfo->Actor->CameraStatus.RendMap;
	CameraInfo->ShowFlags 	= CameraInfo->Actor->CameraStatus.ShowFlags;
	CameraInfo->FOVAngle 	= CameraInfo->Actor->CameraStatus.FOVAngle;
	CameraInfo->OrthoZoom 	= CameraInfo->Actor->CameraStatus.OrthoZoom;
	//
	CameraInfo->ExtraPolyFlags = 0;
	if ((CameraInfo->RendMap==REN_PolyCuts) || (CameraInfo->RendMap==REN_Zones))
		{
		CameraInfo->ExtraPolyFlags = PF_NoMerge;
		};
	//
	// Cached & Precomputed info:
	//
	CameraInfo->PrecomputeRenderInfo(SXR,SYR);
	//
	// Successfully locked it and set Screen pointer.
	//
	Locked = 1;
	return 1;
	UNGUARD("UCamera::Lock");
	};

// 
// Unlock a camera.  When unlocked, you can't access the *Screen pointer because
// the corresponding window may have been resized or deleted.
//
void UCamera::Unlock (ICamera *CameraInfo,int Blit)
	{
	GUARD;
	//
	if (!Locked) appError ("Camera isn't locked");
	GCameraManager->UnlockCameraWindow(this,CameraInfo,Blit);
	//
	// Update all properties:
	//
	AActor *Actor					= &GetActor();
	Actor->CameraStatus.RendMap		= CameraInfo->RendMap;
	Actor->CameraStatus.ShowFlags 	= CameraInfo->ShowFlags;
	Actor->CameraStatus.FOVAngle 	= CameraInfo->FOVAngle;
	Actor->CameraStatus.OrthoZoom	= CameraInfo->OrthoZoom;
	//
	// Unlock the level:
	//
	Level->Unlock (&CameraInfo->Level);
	//
	// Done unlocking:
	//
	Locked = 0;
	//
	UNGUARD("UCamera::Unlock");
	};

/*-----------------------------------------------------------------------------
	Window-related
-----------------------------------------------------------------------------*/

void UCamera::OpenWindow(DWORD ParentWindow,int Temporary)
	{
	GCameraManager->OpenCameraWindow(this,ParentWindow,Temporary);
	};

void UCamera::UpdateWindow(void)
	{
	GCameraManager->UpdateCameraWindow(this);
	};

/*-----------------------------------------------------------------------------
	Holding and unholding
-----------------------------------------------------------------------------*/

void UCamera::Hold(void)
	{
	GUARD;
	OnHold=1;
	UNGUARD("UCamera::Hold");
	};

void UCamera::Unhold(void)
	{
	GUARD;
	if (OnHold)
		{
		OnHold = 0;
		UpdateWindow();
		};
	UNGUARD("UCamera::Unhold");
	};

/*-----------------------------------------------------------------------------
	Command line
-----------------------------------------------------------------------------*/

int UCamera::Exec(const char *Cmd,FOutputDevice *Out)
	{
	GUARD;
	const char *Str = Cmd;
	//
	if (GetCMD(&Str,"STATUS"))
		{
		if (GetCMD(&Str,"CAMERA") || !Str[0])
			{
			Out->Logf("   CAMERA - Ok in level %s",Level->Name);
			return Str[0]!=0;
			}
		else return 0;
		}
	if (GetCMD(&Str,"NOACTORS"))
		{
		Level->ActorList->Element(iActor).CameraStatus.ShowFlags ^= SHOW_Actors;
		return 1;
		}
	if (GetCMD(&Str,"BRUSHWIRES"))
		{
		GetActor().CameraStatus.ShowFlags ^= SHOW_MovingBrushes;
		return 1;
		}
	else if (GetCMD(&Str,"RMODE"))
		{
		int Mode = atoi(Str);
		if ((Mode>REN_None) && (Mode<REN_MAX))
			{
			GetActor().CameraStatus.RendMap = Mode;
			Out->Logf("Rendering mode set to %i",Mode);
			}
		else Out->Logf(LOG_ExecError,"Invalid mode");
		return 1;
		}
	else return 0; // Not executed
	//
	UNGUARD("UCamera::Exec");
	}

/*-----------------------------------------------------------------------------
	Task functions
-----------------------------------------------------------------------------*/

void UCamera::TaskTick(void)
	{
	GUARD;
	UNGUARD("UCamera::TaskTick");
	};

void UCamera::TaskExit(void)
	{
	GUARD;
	UNGUARD("UCamera::TaskExit");
	};

char *UCamera::TaskStatus(char *Name, char *Desc)
	{
	GUARD;
	//
	sprintf(Name,"Camera");
	sprintf(Desc,"");
	return Name;
	//
	UNGUARD("UCamera::TaskStatus");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
