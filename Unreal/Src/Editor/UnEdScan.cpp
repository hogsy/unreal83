/*=============================================================================
	UnEdScan.cpp: Unreal editor click scanner

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

/*-----------------------------------------------------------------------------
	FScan implementation
-----------------------------------------------------------------------------*/

//
// Set up scanner to look at current pixels.  Called immediately before drawing each
// and every element of the screen (Bsp node, lines, etc).
//
void FScan::PreScan (void)
	{
	int i,j,Ofs;
	int X1   = OurMax (0,X - EDSCAN_RADIUS);
	int Y1   = OurMax (0,Y - EDSCAN_RADIUS);
	int XL   = OurMin (X + EDSCAN_RADIUS, Camera->SXR -1) - X1;
	int YL   = OurMin (Y + EDSCAN_RADIUS, Camera->SYR -1) - Y1;
	//
	if (Camera->ColorBytes==1)
		{
		for (i=0; i <= XL; i++) for (j=0; j <= YL; j++)
			{
			Ofs = i + X1 + (j + Y1) * Camera->SXStride;
			Pixels[i][j] = Camera->Screen [Ofs];
			Camera->Screen [Ofs] = EDSCAN_IGNORE;
			};
		}
	else if (Camera->ColorBytes==2)
		{
		for (i=0; i <= XL; i++) for (j=0; j <= YL; j++)
			{
			Ofs = (i + X1 + (j + Y1) * Camera->SXStride) << 1;
			Pixels[i][j] = *(WORD *)&Camera->Screen [Ofs];
			*(WORD *)&Camera->Screen [Ofs] = (WORD)EDSCAN_IGNORE;
			};
		}
	else
		{
		for (i=0; i <= XL; i++) for (j=0; j <= YL; j++)
			{
			Ofs = (i + X1 + (j + Y1) * Camera->SXStride) << 2;
			Pixels[i][j] = *(DWORD *)&Camera->Screen [Ofs];
			*(DWORD *)&Camera->Screen [Ofs] = (DWORD)EDSCAN_IGNORE;
			};
		};
	};

//
// Called immediately after drawing each element of the screen.  Sees if pixels
// in the scan region have changed and, if so, makes note of them.
//
void FScan::PostScan (EEdScan ScanType,
	int NewIndex, int NewA, int NewB, FVector *NewV)
	{
	int i,j,Ofs;
	int X1   = OurMax (0,X - EDSCAN_RADIUS);
	int Y1   = OurMax (0,Y - EDSCAN_RADIUS);
	int XL   = OurMin (X + EDSCAN_RADIUS, Camera->SXR -1) - X1;
	int YL   = OurMin (Y + EDSCAN_RADIUS, Camera->SYR -1) - Y1;
	//
	if (Camera->ColorBytes==1)
		{
		for (i=0; i <= XL; i++) for (j=0; j <= YL; j++)
			{
			Ofs = i + X1 + (j + Y1) * Camera->SXStride;
			if (Camera->Screen [Ofs] != EDSCAN_IGNORE)
				{
				Type   		= ScanType;
				Index  		= NewIndex;
				A      		= NewA;
				B      		= NewB;
				if (NewV) V = *NewV;
				};
			Camera->Screen[Ofs] = Pixels[i][j];
			};
		}
	else if (Camera->ColorBytes==2)
		{
		for (i=0; i <= XL; i++) for (j=0; j <= YL; j++)
			{
			Ofs = (i + X1 + (j + Y1) * Camera->SXStride) << 1;
			if (*(WORD *)&Camera->Screen [Ofs] != (DWORD)EDSCAN_IGNORE)
				{
				Type   		= ScanType;
				Index  		= NewIndex;
				A      		= NewA;
				B      		= NewB;
				if (NewV) V = *NewV;
				};
			*(WORD *)&Camera->Screen[Ofs] = Pixels[i][j];
			};
		}
	else
		{
		for (i=0; i <= XL; i++) for (j=0; j <= YL; j++)
			{
			Ofs = (i + X1 + (j + Y1) * Camera->SXStride) << 2;
			if (*(DWORD *)&Camera->Screen [Ofs] != (DWORD)EDSCAN_IGNORE)
				{
				Type   		= ScanType;
				Index  		= NewIndex;
				A      		= NewA;
				B      		= NewB;
				if (NewV) V = *NewV;
				};
			*(DWORD *)&Camera->Screen[Ofs] = Pixels[i][j];
			};
		};
	};

void FScan::Init(ICamera *NewCamera)
	{
	// Caller must set X, Y
	Camera = NewCamera;
	Active = 1;
	Type   = EDSCAN_None; // Nothing found yet
	Index  = 0;
	A      = 0;
	B      = 0;
	};

void FScan::Exit(void)
	{
	Active = 0;
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
