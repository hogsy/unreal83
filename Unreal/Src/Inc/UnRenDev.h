/*=============================================================================
	UnRenDev.h: 3D rendering device class

	Copyright 1995 Epic MegaGames, Inc.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNRENDEV // Prevent header from being included multiple times
#define _INC_UNRENDEV

/*------------------------------------------------------------------------------------
	FRenderDevice
------------------------------------------------------------------------------------*/

//
// Class representing a low-level 3D rendering device, such as a 3D hardware
// accelerator.
//
class FRenderDevice
	{
	public:
	//
	// Init, Exit & Flush:
	//
	int Active,Locked;
	FRenderDevice();
	~FRenderDevice();
	virtual void Init3D(void)=0;
	virtual void Exit3D(void)=0;
	virtual void Flush3D(void)=0;
	//
	// Lock & Unlock:
	//
	virtual void Lock(ICamera *Camera)=0;
	virtual void Unlock(ICamera *Camera)=0;
	//
	// Draw a polygon using texture vectors:
	//
	virtual void DrawPolyV(ICamera *Camera,UTexture *Texture,class FTransform *Pts,int NumPts,
		FVector &Base,FVector &Normal,FVector &U,FVector &V)=0;
	//
	// Draw a polygon using texture coordinates:
	//
	virtual void DrawPolyC(ICamera *Camera,UTexture *Texture,class FTransTex *Pts,int NumPts)=0;
	//
	// Draw a polygon flat-shaded:
	//
	virtual void DrawPolyF(ICamera *Camera,class FTransform *Pts,int NumPts,FColor Color)=0;
	};

/*------------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------------*/
#endif // _INC_UNRENDEV

