/*=============================================================================
	UnRenDev.cpp: Unreal generic 3D rendering device support code

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "StdAfx.h"

#include "Unreal.h"
#include "UnRender.h"
#include "UnRaster.h"
#include "UnRenDev.h"

/*-----------------------------------------------------------------------------
	FRenderDevice generic implementation
-----------------------------------------------------------------------------*/

//
// Constructor.
//
FRenderDevice::FRenderDevice()
	{
	Active = 0;
	Locked = 0;
	};

//
// Destructor.
//
FRenderDevice::~FRenderDevice()
	{
	Active = 0;
	Locked = 0;
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
