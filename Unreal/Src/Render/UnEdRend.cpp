/*=============================================================================
	UnEdRend.cpp: Unreal editor rendering functions.

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"
#include "UnRaster.h"

#define LINE_NEAR_CLIP_Z 1.0
#define ORTHO_LOW_DETAIL 40000.0

/*-----------------------------------------------------------------------------
	Line drawing routines
-----------------------------------------------------------------------------*/

//
// Convenience macros:
//
#define BLEND(Dest,Color)  *(Dest)=GGfx.Blender[(DWORD)*(Dest) + (((DWORD)Color) << 8)]
#define LOGADD(Dest,Color) *(Dest)=GGfx.LogAdd [(DWORD)*(Dest) + (((DWORD)Color) << 8)]
#define SHADE(Color,Shade) GGfx.Shader[Color + (Shade<<8)]

#define MAKELABEL(A,B,C,D) A##B##C##D

//
// Draw a regular line:
//
void FGlobalRender::DrawLine(ICamera *Camera, BYTE Color, int Dotted,
	FLOAT X1, FLOAT Y1, FLOAT X2, FLOAT Y2)
	{
	INT NewColor = Color-16;
	if (Camera->ColorBytes==1)
		{
		#define DRAWPIXEL(Dest)  *(Dest)=NewColor
		#define ASMPIXEL		 mov [edi],al
		#define SHIFT 0
		#define LABEL1(X) X##C1
		#include "UnLine1.cpp"
		#undef  LABEL1
		#undef  SHIFT
		#undef  DRAWPIXEL
		#undef  ASMPIXEL
		}
	else if (Camera->ColorBytes==2)
		{
		WORD HiColor;
		if (Camera->Caps & CC_RGB565)	HiColor = GGfx.DefaultColors[NewColor].HiColor565();
		else							HiColor = GGfx.DefaultColors[NewColor].HiColor555();
		//
		#define DRAWPIXEL(Dest)  *(WORD *)(Dest)=HiColor
		#define SHIFT 1
		#define LABEL1(X) X##C2
		#include "UnLine1.cpp"
		#undef  LABEL1
		#undef  SHIFT
		#undef  DRAWPIXEL
		}
	else
		{
		DWORD TrueColor = GGfx.TrueColors[NewColor].D;
		#define DRAWPIXEL(Dest)  *(DWORD *)(Dest)=TrueColor
		#define SHIFT 2
		#define LABEL1(X) X##C4
		#include "UnLine1.cpp"
		#undef  LABEL1
		#undef  SHIFT
		#undef  DRAWPIXEL
		};
	};

//
// Draw a depth-shaded line
//
void FGlobalRender::DrawDepthLine(ICamera *Camera, BYTE Color, int Dotted,
	FLOAT X1, FLOAT Y1, FLOAT RZ1, FLOAT X2, FLOAT Y2, FLOAT RZ2)
	{
	INT NewColor = Color-16;
	if (Camera->ColorBytes==1)
		{
		#define DRAWPIXEL(Dest)    *(Dest)=NewColor + (UNFIX(FixG1 += FixDG)<<3);
		#define ASMPIXEL \
			__asm{mov ebx,eax}\
			__asm{mov edx,[FixDG]}\
			__asm{shr ebx,13}\
			__asm{add eax,edx}\
			__asm{and ebx,0xf8}\
			__asm{mov edx,[NewColor]}\
			__asm{add ebx,edx}\
			__asm{mov [edi],bl}
		#define DEPTHSHADE
		#define SHIFT 0
		#define LABEL1(X) X##D1
		#include "UnLine1.cpp"
		#undef  LABEL1
		#undef  SHIFT
		#undef  DEPTHSHADE
		#undef  DRAWPIXEL
		#undef  ASMPIXEL
		}
	else if ((Camera->ColorBytes==2) && (Camera->Caps && CC_RGB565))
		{
		#define DRAWPIXEL(Dest)    *(WORD *)(Dest)=GGfx.DefaultColors[NewColor+(UNFIX(FixG1 += FixDG)<<3)].HiColor565();
		#define DEPTHSHADE
		#define SHIFT 1
		#define LABEL1(X) X##D2_565
		#include "UnLine1.cpp"
		#undef  LABEL1
		#undef  SHIFT
		#undef  DEPTHSHADE
		#undef  DRAWPIXEL
		}
	else if (Camera->ColorBytes==2)
		{
		#define DRAWPIXEL(Dest)    *(WORD *)(Dest)=GGfx.DefaultColors[NewColor+(UNFIX(FixG1 += FixDG)<<3)].HiColor555();
		#define DEPTHSHADE
		#define SHIFT 1
		#define LABEL1(X) X##D2_555
		#include "UnLine1.cpp"
		#undef  LABEL1
		#undef  SHIFT
		#undef  DEPTHSHADE
		#undef  DRAWPIXEL
		}
	else
		{
		#define DRAWPIXEL(Dest)    *(DWORD *)(Dest)=*(DWORD *)&GGfx.TrueColors[NewColor+(UNFIX(FixG1 += FixDG)<<3)];
		#define DEPTHSHADE
		#define SHIFT 2
		#define LABEL1(X) X##D4
		#include "UnLine1.cpp"
		#undef  SHIFT
		#undef  LABEL1
		#undef  DEPTHSHADE
		#undef  DRAWPIXEL
		};
	};

/*-----------------------------------------------------------------------------
	Low-level graphics drawing primitives
-----------------------------------------------------------------------------*/

//
// Draw a clipped rectangle, assumes X1<X2 and Y1<Y2:
//
void FGlobalRender::DrawRect(ICamera *Camera, BYTE Color, int X1, int Y1, int X2, int Y2)
	{
	GUARD;
	//
	if ((X2<0)||(Y2<0)) return;
	//
	const int SXR      = Camera->SXR;
	const int SYR      = Camera->SYR;
	const int SXStride = Camera->SXStride;
	//
	if ((X1>=SXR) || (Y1>=SYR)) return;
	//
	BYTE *Dest1,*Dest;
	int X,Y,XL,YL;
	//
	if (X1<0)				X1=0;
	if (Y1<0)				Y1=0;
	if (++X2>Camera->SXR)	X2=Camera->SXR; 
	if (++Y2>Camera->SYR)	Y2=Camera->SYR;
	//
	Color -= 16;
	//
	YL     = Y2-Y1;
	XL     = X2-X1;
	//
	if (Camera->ColorBytes==1)
		{
		Dest1 = &Camera->Screen[X1 + Y1*SXStride];
		//
		#ifdef ASM
		__asm
			{
			mov edi,[Dest1]		// Destination address
			mov ecx,[SXStride]	// Screen resolution
			mov edx,[YL]		// Number of lines to draw
			mov ebx,[XL]		// Loop counter
			mov al, [Color]		// Color
			sub ecx,ebx			// Stride skip
			;
			ALIGN 16
			Outer:				// Outer loop entry point
			Inner:				// Inner loop entry point
			mov [edi],al		// Store color onto screen
			inc edi				// Go to next screen pixel
			dec ebx				// Next pixel
			jg  Inner
			;
			add edi,ecx			// Skip SXR-XL pixels
			mov ebx,[XL]		// Get inner loop counter
			dec edx				// Next line
			jg  Outer
			};
		#else
		for (Y=0; Y<YL; Y++)
			{
			Dest = Dest1;
			for (X=0; X<XL; X++) *Dest++ = Color;
			Dest1 += SXStride;
			};
		#endif
		}
	else if (Camera->ColorBytes==2)
		{
		WORD HiColor;
		if (Camera->Caps & CC_RGB565)	HiColor = GGfx.DefaultColors[Color].HiColor565();
		else							HiColor = GGfx.DefaultColors[Color].HiColor555();
		Dest1 = &Camera->Screen[(X1 + Y1*SXStride)*2];
		for (Y=0; Y<YL; Y++)
			{
			Dest = Dest1;
			for (X=0; X<XL; X++)
				{
				*(WORD *)Dest  = HiColor;
				Dest          += 2;
				};
			Dest1 += SXStride << 1;
			};
		}
	else
		{
		DWORD TrueColor = GGfx.TrueColors [Color].D;
		Dest1 = &Camera->Screen[(X1 + Y1 * SXStride) << 2];
		for (Y=0; Y<YL; Y++)
			{
			Dest = Dest1;
			for (X=0; X<XL; X++)
				{
				*(DWORD *)Dest  = TrueColor;
				Dest           += 4;
				};
			Dest1 += SXStride << 2;
			};
		};
	UNGUARD("FGlobalRender::DrawRect");
	};

//
// Draw a circle.
//
void FGlobalRender::DrawCircle(ICamera *Camera, FVector &Location, int Radius, int Color, int Dotted)
	{
	FLOAT F = 0.0;
	FVector A,B,P1,P2;
	//
	if		(Camera->RendMap==REN_OrthXY)	{A=GMath.XAxisVector; B=GMath.YAxisVector;}
	else if (Camera->RendMap==REN_OrthXZ)	{A=GMath.XAxisVector; B=GMath.ZAxisVector;}
	else									{A=GMath.YAxisVector; B=GMath.ZAxisVector;};
	//
	P1 = Location + Radius * (A * cos(F) + B * sin(F));
	for (int i=0; i<8; i++)
		{
		F += 2.0*PI/8.0;
		P2 = Location + Radius * (A * cos(F) + B * sin(F));
		DrawOrthoLine(Camera,&P1,&P2,Color,Dotted);
		P1 = P2;
		};
	};

/*-----------------------------------------------------------------------------
	Misc
-----------------------------------------------------------------------------*/

//
// Clip a line in an orthogonal view and return 1 if the line is visible,
// 2 if it's visible as a point (parallel to line of sight), or 0 if it's obscured.
//
int FGlobalRender::OrthoClip(ICamera *Camera,const FVector *P1, const FVector *P2,
	FLOAT *ScreenX1, FLOAT *ScreenY1, FLOAT *ScreenX2, FLOAT *ScreenY2)
	{
	GUARD;
	const FVector *Origin = &Camera->Coords.Origin;
	FLOAT   X1,X2,Y1,Y2,Temp;
	int     Status=1;
	//
	// Get unscaled coordinates for whatever axes we're using:
	//
	switch (Camera->RendMap)
		{
		case REN_OrthXY:
			X1=P1->X - Origin->X; Y1=P1->Y - Origin->Y;
			X2=P2->X - Origin->X; Y2=P2->Y - Origin->Y;
			break;
		case REN_OrthXZ:
			X1=P1->X - Origin->X; Y1=Origin->Z - P1->Z;
			X2=P2->X - Origin->X; Y2=Origin->Z - P2->Z;
			break;
		case REN_OrthYZ:
			X1=P1->Y - Origin->Y; Y1=Origin->Z - P1->Z;
			X2=P2->Y - Origin->Y; Y2=Origin->Z - P2->Z;
			break;
		default:
			appError ("OrthoClip: Bad RendMap");
		};
	//
	// See if points for a line that's parallel to our line of sight (i.e. line appears
	// as a dot):
	//
	if ((OurAbs(X2-X1)+OurAbs(Y1-Y2))<0.5) Status=2; // Is point
	//
	// Zoom:
	//
	X1 = (X1 * Camera->RZoom) + Camera->FSXR2;
	X2 = (X2 * Camera->RZoom) + Camera->FSXR2;
	Y1 = (Y1 * Camera->RZoom) + Camera->FSYR2;
	Y2 = (Y2 * Camera->RZoom) + Camera->FSYR2;
	//
	// X-Clip:
	//
	if (X1 > X2) // Arrange so X1<X2.
		{
		Temp=X1; X1=X2; X2=Temp;
		Temp=Y1; Y1=Y2; Y2=Temp;
		};
	if (X2<0)            return 0;
	if (X1>Camera->SXR)  return 0;
	if (X1<0)
		{
		if (OurAbs(X2-X1)<0.001) return 0;
		Y1 += (0-X1)*(Y2-Y1)/(X2-X1);
		X1  = 0;
		};
	if (X2>=Camera->SXR)
		{
		if (OurAbs(X2-X1)<0.001) return 0;
		Y2 += ((Camera->FSXR-1.0)-X2)*(Y2-Y1)/(X2-X1);
		X2  = Camera->FSXR-1.0;
		};
	//
	// Y-Clip:
	//
	if (Y1 > Y2) // Arrange so Y1<Y2.
		{
		Temp=X1; X1=X2; X2=Temp;
		Temp=Y1; Y1=Y2; Y2=Temp;
		};
	if (Y2<0)            return 0;
	if (Y1>Camera->SYR)  return 0;
	if (Y1<0)
		{
		if (OurAbs(Y2-Y1)<0.001) return 0;
		X1 += (0-Y1)*(X2-X1)/(Y2-Y1);
		Y1  = 0;
		};
	if (Y2>=Camera->SYR)
		{
		if (OurAbs(Y2-Y1)<0.001) return 0;
		X2 += ((Camera->FSYR1-1.0)-Y2)*(X2-X1)/(Y2-Y1);
		Y2  = (Camera->FSYR1-1.0);
		};
	//
	// Return:
	//
	*ScreenX1=X1;
	*ScreenY1=Y1;
	*ScreenX2=X2;
	*ScreenY2=Y2;
	//
	return Status;
	UNGUARD("FGlobalRender::OrthoClip");
	};

//
// Figure out the unclipped screen location of a 3D point taking into account either
// a perspective or orthogonal projection.  Returns 1 if view is orthogonal or point 
// is visible in 3D view, 0 if invisible in 3D view (behind the viewer).
//
// Scale = scale of one world unit (at this point) relative to screen pixels,
// for example 0.5 means one world unit is 0.5 pixels.
//
int FGlobalRender::Project (ICamera *Camera, FVector *V, FLOAT *ScreenX, FLOAT *ScreenY, FLOAT *Scale)
	{
	GUARD;
	//
	const FVector *Origin = &Camera->Coords.Origin;
	FVector	Temp;
	FLOAT 	Z,RZ;
	//
	Temp.X = V->X - Origin->X;
	Temp.Y = V->Y - Origin->Y;
	Temp.Z = V->Z - Origin->Z;
	//
	if (Camera->RendMap==REN_OrthXY)
		{
		*ScreenX = +Temp.X * Camera->RZoom + Camera->FSXR2;
		*ScreenY = +Temp.Y * Camera->RZoom + Camera->FSYR2;
		if (Scale != NULL) *Scale = Camera->RZoom;
		return 1; // Ortho points are always visible
		}
	else if (Camera->RendMap==REN_OrthXZ)
		{
		*ScreenX = +Temp.X * Camera->RZoom + Camera->FSXR2;
		*ScreenY = -Temp.Z * Camera->RZoom + Camera->FSYR2;
		if (Scale != NULL) *Scale = Camera->RZoom;
		return 1;
		}
	else if (Camera->RendMap==REN_OrthYZ)
		{
		*ScreenX = +Temp.Y * Camera->RZoom + Camera->FSXR2;
		*ScreenY = -Temp.Z * Camera->RZoom + Camera->FSYR2;
		if (Scale != NULL) *Scale = Camera->RZoom;
		return 1;
		}
	else // Perspective view
		{
		Temp.TransformVector(Camera->Coords);
		//
		Z  = Temp.Z; if (OurAbs (Z)<0.01) Z+=0.02f;
		RZ	= Camera->ProjZ / Z;
		//
		*ScreenX = Temp.X * RZ + Camera->FSXR2;
		*ScreenY = Temp.Y * RZ + Camera->FSYR2;
		if (Scale != NULL) *Scale = RZ;
		return (Z>1.0);
		};
	UNGUARD("FGlobalRender::Project");
	};

/*-----------------------------------------------------------------------------
   Screen to world functions (inverse projection)
-----------------------------------------------------------------------------*/

//
// Convert a particular screen location to a world location.  In ortho views,
// sets non-visible component to zero.  In persp views, places at camera location
// unless UseEdScan=1 and the user just clicked on a wall (a Bsp polygon).
// Sets V to location and returns 1, or returns 0 if couldn't perform conversion.
//
int FGlobalRender::Deproject (ICamera *Camera,int ScreenX,int ScreenY,FVector *V,int UseEdScan,FLOAT Radius)
	{
	GUARD;
	//
	FVector	*Origin = &Camera->Coords.Origin;
	FLOAT	SX		= (FLOAT)ScreenX - Camera->FSXR2;
	FLOAT	SY		= (FLOAT)ScreenY - Camera->FSYR2;
	//
	switch (Camera->RendMap)
		{
		case REN_OrthXY:
			V->X = +SX / Camera->RZoom + Origin->X;
			V->Y = +SY / Camera->RZoom + Origin->Y;
			V->Z = 0;
			return 1;
		case REN_OrthXZ:
			V->X = +SX / Camera->RZoom + Origin->X;
			V->Y = 0.0;
			V->Z = -SY / Camera->RZoom + Origin->Z;
			return 1;
		case REN_OrthYZ:
			V->X = 0.0;
			V->Y = +SX / Camera->RZoom + Origin->Y;
			V->Z = -SY / Camera->RZoom + Origin->Z;
			return 1;
		default: // 3D view
			if (UseEdScan && GEditor)
				{
				IModel		*ModelInfo;
				FBspNode	*Node;
				FBspSurf	*Poly;
				FVector		*PlaneBase,*PlaneNormal;
				FVector		SightVector,SightX,SightY,SightDest;
				//
				if (GEditor->Scan.Type==EDSCAN_BspNodePoly)
					{
					ModelInfo   = &Camera->Level.ModelInfo;
					Node  		= &ModelInfo->BspNodes[GEditor->Scan.Index];
					Poly  		= &ModelInfo->BspSurfs[Node->iSurf];
					PlaneBase 	= &ModelInfo->FPoints [Poly->pBase];
					PlaneNormal = &ModelInfo->FVectors[Poly->vNormal];
					//
					// Find line direction vector of line-of-sight starting at camera
					// location:
					//
					SightVector = Camera->Coords.ZAxis;
					SightX      = Camera->Coords.XAxis * SX * Camera->RProjZ;
					SightY      = Camera->Coords.YAxis * SY * Camera->RProjZ;
					//
					SightVector += SightX;
					SightVector += SightY;
					SightDest    = *Origin + SightVector;
					//
					// Find intersection of line-of-sight and plane:
					//
					*V = FLinePlaneIntersection (*Origin,SightDest,*PlaneBase,*PlaneNormal);
					*V += *PlaneNormal * Radius; // Move destination point out of plane:
					//
					return 1;
					};
				}
			else
				{
				*V = *Origin;
				};
			return 0; // 3D not supported yet
		};
	UNGUARD("FGlobalRender::Deproject");
	};

/*-----------------------------------------------------------------------------
   High-level graphics primitives
-----------------------------------------------------------------------------*/

void FGlobalRender::DrawOrthoLine (ICamera *Camera, const FVector *P1, const FVector *P2,int Color,int Dotted)
	{
	GUARD;
	//
	FLOAT   X1,Y1,X2,Y2;
	int		Status;
	//
	Status=OrthoClip(Camera,P1,P2,&X1,&Y1,&X2,&Y2);
	//
	if (Status==1) // Line is visible as a line
		{
		DrawLine(Camera,Color,Dotted,X1,Y1,X2,Y2);
		}
	else if (Status==2) // Line is visible as a point
		{
		if (Camera->OrthoZoom < ORTHO_LOW_DETAIL) DrawRect(Camera,Color,X1-1,Y1-1,X1+1,Y1+1);
		};
	UNGUARD("FGlobalRender::DrawOrthoLine");
	};

void FGlobalRender::Draw3DLine (ICamera *Camera, const FVector *OrigP, const FVector *OrigQ, int MustTransform, 
	int Color,int DepthShade,int Dotted)
	{
	GUARD;
	//
	FLOAT   SX	= Camera->FSXR;
	FLOAT	SY  = Camera->FSYR;
	FLOAT   SX2 = Camera->FSXR2;
	FLOAT	SY2 = Camera->FSYR2;
	FLOAT   X1,Y1,RZ1,X2,Y2,RZ2;
	FLOAT   Dx,Dy,Dz;
	FVector P,Q;
	FLOAT	Temp,Alpha;
	//
	if (Camera->Camera->IsOrtho())
		{
		DrawOrthoLine (Camera,OrigP,OrigQ,Color,Dotted);
		return;
		};
	P=*OrigP; 
	Q=*OrigQ;
	if (MustTransform)
		{
   		P.TransformPoint (Camera->Coords); // Transform into screenspace
   		Q.TransformPoint (Camera->Coords);
		};
	//
	// Calculate delta, discard line if points are identical
	//
	Dx=(Q.X-P.X); Dy=(Q.Y-P.Y); Dz=(Q.Z-P.Z);
	//
	if (((Dx<0.01)&&(Dx>-0.01)) &&
		((Dy<0.01)&&(Dy>-0.01)) &&
		((Dz<0.01)&&(Dz>-0.01)))
		return; // Same point, divide would fail.
	//
	// Clip to near clipping plane:
	//
	if (P.Z<=LINE_NEAR_CLIP_Z) // Clip P to NCP
		{
		if (Q.Z<(LINE_NEAR_CLIP_Z-0.01)) return; // Prevent divide by zero when P-Q is tiny
		P.X +=  (LINE_NEAR_CLIP_Z-P.Z) * Dx/Dz; // Dz != 0
		P.Y +=  (LINE_NEAR_CLIP_Z-P.Z) * Dy/Dz;
		P.Z =   LINE_NEAR_CLIP_Z;
		}
	else if (Q.Z<(LINE_NEAR_CLIP_Z-0.01)) // Clip Q to NCP
		{
		Q.X += (LINE_NEAR_CLIP_Z-Q.Z) * Dx/Dz; // Dz != 0 from above
		Q.Y += (LINE_NEAR_CLIP_Z-Q.Z) * Dy/Dz;
		Q.Z =   LINE_NEAR_CLIP_Z;
		};
	//
	// Calculate perspective
	//
	RZ1 = 1.0/P.Z; X1=P.X * Camera->ProjZ * RZ1 + SX2; Y1=P.Y * Camera->ProjZ * RZ1 + SY2; 
	RZ2 = 1.0/Q.Z; X2=Q.X * Camera->ProjZ * RZ2 + SX2; Y2=Q.Y * Camera->ProjZ * RZ2 + SY2; 
	//
	// Arrange for X-clipping
	//
	if (X2<X1) // Flip so X2>X1
		{
		Temp=X1;  X1 =X2;  X2 =Temp;
		Temp=Y1;  Y1 =Y2;  Y2 =Temp;
		Temp=RZ1; RZ1=RZ2; RZ2=Temp;
		}
	else if ((X2-X1) < 0.01) // Special case vertical line
		{
		if ((X1<0)||(X1>=SX)) return; // X offscreen
		//
		if (Y1<0)
			{
			if (Y2<0) return;
			if (DepthShade) 
				{			
				Alpha = (0-Y1)/(Y2-Y1);
				RZ1   = RZ1 + Alpha * (RZ2-RZ1);
				};
			Y1=0;
			}
		else if (Y1>=SY)
			{
			if (Y2>=SY) return;
			if (DepthShade) 
				{			
				Alpha = (SY-1-Y1)/(Y2-Y1);
				RZ1   = RZ1 + Alpha * (RZ2-RZ1);
				};
			Y1=SY-1;
			};
		if (Y2<0)
			{
			if (DepthShade) 
				{			
				Alpha = (0-Y1)/(Y2-Y1);
				RZ2   = RZ1 + Alpha * (RZ2-RZ1);
				};
      		Y2=0;
      		}
		else if (Y2>=SY)
			{
			if (DepthShade) 
				{			
				Alpha = (SY-1-Y1)/(Y2-Y1);
				RZ2   = RZ1 + Alpha * (RZ2-RZ1);
				};
			Y2=SY-1;
			};
		goto Draw;
		};
	//
	// X-clip it.  X2>X1
	//
	if (X2<0.0) return; // Both points offscreen
	if (X1>=SX) return; // Both points offscreen
	//
	if (X1<0) // Bound X1 and calculate new Y1 for later Y-clipping
		{
		Alpha = (0-X1)/(X2-X1);
		Y1    = Y1 + Alpha * (Y2-Y1);
		//
		if (DepthShade) RZ1 = RZ1 + Alpha * (RZ2-RZ1);
		//
		X1=0;
		};
	if (X2>=SX) // Bound X2 and calculate new Y2 for later Y-clipping
		{
		Alpha = (SX-1-X1)/(X2-X1);
		Y2    = Y1 + Alpha * (Y2-Y1);
		//
		if (DepthShade) RZ2 = RZ1 + Alpha * (RZ2-RZ1);
		//
		X2 = SX-1;
		};
	//
	// Arrange for Y-clipping
	//
	if (Y2<Y1) // Flip so Y2>Y1
		{
		Temp=X1;  X1=X2;   X2=Temp;
		Temp=Y1;  Y1=Y2;   Y2=Temp;
		Temp=RZ1; RZ1=RZ2; RZ2=Temp;
		}
	else if ((Y2-Y1)<0.01) // -0.01 to 0.01 = horizontal line
		{
		//
		// Special case horizontal line (already x-clipped)
		//
		if ((Y1<0.0) || (Y2>=SY)) return; // Horizontal line is offscreen
		else goto Draw;
		};
	//
	// Y-clip it.  Y2>Y1
	//
	if (Y2<0)   return; // Both points offscreen
	if (Y1>=SY) return; // Both points offscreen
	//
	if (Y1<0) // Bound Y1 and calculate new X1, discard if out of range
		{
		Alpha = (0-Y1)/(Y2-Y1);
		X1    = X1 + Alpha * (X2-X1);
		//
		if 		((X1<0)||(X1>=SX)) 	return;
		else if (DepthShade) 		RZ1 = RZ1 + Alpha * (RZ2-RZ1);
		//
		Y1=0;
		};
	if (Y2>=SY) // Bound Y2 and calculate new X2, discard if out of range
		{
		Alpha 	= (SY-1-Y1)/(Y2-Y1);
		X2       = X1 + Alpha * (X2-X1);
		//
		if 		((X2<0)||(X2>=SX)) 	return;
		else if	(DepthShade) 		RZ2 = RZ1 + Alpha * (RZ2-RZ1);
		//
		Y2=SY-1;
		};
	Draw:
	if 		(DepthShade==0) DrawLine	  (Camera,Color,Dotted,X1,Y1,X2,Y2);
	else if (DepthShade==1)	DrawDepthLine (Camera,Color,Dotted,X1,Y1,RZ1,X2,Y2,RZ2);
	//
	UNGUARD("FGlobalRender::Draw3DLine");
	};

/*-----------------------------------------------------------------------------
	Edpoly drawers
-----------------------------------------------------------------------------*/

//
// Draw an editor polygon
//
void FGlobalRender::DrawFPoly(ICamera *Camera, FPoly *EdPoly, int WireColor, int FillColor,int Dotted)
	{
	GUARD;
	//
	const FVector	*Origin = &Camera->Coords.Origin;
	FVector			*Verts	= &EdPoly->Vertex[0];
	FVector			VPoly,*V1,*V2;
	INDEX			NumPts,i;
	//
	if (Camera->Camera->IsOrtho())
		{
		//
		// Orthogonal view
		//
		NumPts	= EdPoly->NumVertices;
		V1		= &EdPoly->Vertex[0];
		V2      = &EdPoly->Vertex[EdPoly->NumVertices-1];
		for (i=0; i<NumPts; i++)
			{
			if ((EdPoly->PolyFlags & PF_NotSolid) || (V1->X >= V2->X))
				{
        		DrawOrthoLine(Camera,V1,V2,WireColor,Dotted);
				};
			V2=V1++;
			};
		}
	else
		{
		//
		// Perspective view
		//
		if (FillColor && !(EdPoly->PolyFlags & PF_NotSolid))
			{
			//
			// Backface rejection
			//
			VPoly.X = Verts[0].X - Origin->X;
			VPoly.Y = Verts[0].Y - Origin->Y;
			VPoly.Z = Verts[0].Z - Origin->Z;
			//
			if ((
	            VPoly.X*EdPoly->Normal.X+
				VPoly.Y*EdPoly->Normal.Y+
				VPoly.Z*EdPoly->Normal.Z
				) >= 0.0)
				{
				return; // Backfaced (Normal and view vector are facing same way!)
				};
			};
		V1	= &EdPoly->Vertex[0];
		V2	= &EdPoly->Vertex[EdPoly->NumVertices-1];
		for (i=0; i<EdPoly->NumVertices; i++)
			{
			if ((EdPoly->PolyFlags & PF_NotSolid) || (V1->X >= V2->X))
				{
				Draw3DLine (Camera,V1,V2,1,WireColor,0,Dotted);
				};
			V2 = V1++;
			};
		};
	UNGUARD("FGlobalRender::DrawFPoly");
	};

//
// Draw a brush (with rotation/translation):
//
void FGlobalRender::DrawBrushPolys (ICamera *Camera, UModel *Brush, 
	int WireColor, int Dotted, FConstraints *Constraints, int DrawPivot,
	int DrawVertices, int DrawSelected, int DoScan)
	{
	GUARD;
	//
	// See if we can reject the brush:
	//
	if (Brush->Bound[1].Min.iTransform) // Transformed bound is valid
		{
		if (!BoundVisible (Camera,&Brush->Bound[1],NULL,NULL,NULL)) return;
		};
	AActor		*Actor = Camera->Actor;
	FCoords     Coords;		// Locally-rotated coordinates for brush
	FVector     Location;	// New (constrained) location
	FRotation	Rotation;	// New (constrained) rotation
	FVector		Vertex,*VertPtr,*V1,*V2,*OrthoNormal;
	FPoly       *TransformedEdPolys;
	FLOAT       X,Y;
	IModel 		BrushInfo;
	BYTE		DrawColor,VertexColor,PivColor;
	INDEX       i,j;
	int			Snapped;
	//
	OrthoNormal = Camera->GetOrthoNormal();
	//
	// Get model:
	//
	Brush->Lock(&BrushInfo,LOCK_Read);
	TransformedEdPolys = (FPoly *)GMem.Get(BrushInfo.NumFPolys * sizeof(FPoly));
	//
	// Make coordinate system from camera:
	//
	Coords        = GMath.UnitCoords;
	Coords.Origin = Camera->Coords.Origin;
	//
	// Figure out brush movement constraints
	//
	Location	= BrushInfo.Location;
	Rotation	= BrushInfo.Rotation;
	//
	if ((!Constraints)||(!GEditor)) Snapped = 0;
	else Snapped = GEditor->constraintApply (&Camera->Level.ModelInfo,&BrushInfo,&Location,&Rotation,Constraints);
	//
	if (BrushInfo.ModelFlags&MF_PostScale) Coords.TransformByScale (BrushInfo.PostScale);
	Coords.TransformByRotation	(Rotation);
	Coords.TransformByScale		(BrushInfo.Scale);
	//
	// Setup colors:
	//
	if (Snapped<0) DrawColor = InvalidColor;
	//
	if (DrawSelected) DrawColor = WireColor + BRIGHTNESS(4);
	else DrawColor = WireColor + BRIGHTNESS(10);
	//
	VertexColor = WireColor + BRIGHTNESS(2);
	PivColor    = WireColor + BRIGHTNESS(0);
	//
	// Transform and draw all FPolys:
	//
	int NumTransformed = 0;
	FPoly *EdPoly = &TransformedEdPolys[0];
	for (i=0; i<BrushInfo.NumFPolys; i++)
		{
		*EdPoly = BrushInfo.FPolys[i];
		EdPoly->Normal.TransformVector(Coords);
		//
		if ((!OrthoNormal) || (Camera->OrthoZoom < ORTHO_LOW_DETAIL) || 
			(EdPoly->PolyFlags & PF_NotSolid) || ((*OrthoNormal | EdPoly->Normal)!=0.0))
			{
			//
			// Transform it:
			//
			VertPtr = &EdPoly->Vertex[0];
			for (j=0; j<EdPoly->NumVertices; j++)
				{
				*VertPtr -= BrushInfo.PrePivot;
				VertPtr->TransformVector (Coords);
				*VertPtr += BrushInfo.PostPivot + Location;
				//
				*VertPtr++;
				};
			//
			// Draw this brush's EdPoly's:
			//
			if (DoScan && GEditor && GEditor->Scan.Active) GEditor->Scan.PreScan();
			DrawFPoly (Camera,EdPoly,DrawColor,0,Dotted);
			if (DoScan && GEditor && GEditor->Scan.Active) GEditor->Scan.PostScan (EDSCAN_BrushSide,(int)Brush,i,0,NULL);
			//
			NumTransformed++;
			EdPoly++;
			};
		};
	//
	// Draw all vertices:
	//
	if (DrawVertices && (BrushInfo.NumFPolys>0))
		{
		for (i=0; i<NumTransformed; i++)
			{
			EdPoly = &TransformedEdPolys[i];
			//
			V1 = &EdPoly->Vertex[0];
			V2 = &EdPoly->Vertex[EdPoly->NumVertices-1];
			for (j=0; j<EdPoly->NumVertices; j++)
				{
      			if (Project (Camera,V1,&X,&Y,NULL))
					{
					if (DoScan && GEditor && GEditor->Scan.Active) GEditor->Scan.PreScan();
         			DrawRect (Camera,VertexColor, X-1, Y-1, X+1, Y+1);
					if (DoScan && GEditor && GEditor->Scan.Active) GEditor->Scan.PostScan (EDSCAN_BrushVertex,(int)Brush,i,j,&EdPoly->Vertex[j]);
         			};
				V2 = V1++;
				};
			};
		//
		// Draw the origin:
		//
		Vertex = GMath.ZeroVector - BrushInfo.PrePivot;
		Vertex.TransformVector 	(Coords);
		Vertex += BrushInfo.PostPivot + Location;
		//
		if (Project (Camera,&Vertex,&X,&Y,NULL))
			{
			if (DoScan && GEditor && GEditor->Scan.Active) GEditor->Scan.PreScan();
			//
			DrawRect (Camera,VertexColor,X-1, Y-1, X+1, Y+1);
			if (memcmp(&BrushInfo.Scale,&GMath.UnitScale,sizeof(FScale)) || 
				memcmp(&BrushInfo.PostScale,&GMath.UnitScale,sizeof(FScale)))
				{
				DrawRect (Camera,BlackColor, X-3, Y-3, X-1, Y-1);
				DrawRect (Camera,BlackColor, X+1, Y-3, X+3, Y-1);
				DrawRect (Camera,BlackColor, X-3, Y+1, X-1, Y+3);
				DrawRect (Camera,BlackColor, X+1, Y+1, X+3, Y+3);
				};
			if (DoScan && GEditor && GEditor->Scan.Active) GEditor->Scan.PostScan(EDSCAN_BrushVertex,(int)Brush,-1,0,&Vertex); // -1 = origin
			};
		};
	//
	// Draw the current pivot:
	//
	if (DrawPivot && (BrushInfo.NumFPolys>0))
		{
		Vertex = BrushInfo.PostPivot + Location;
		if (Project (Camera,&Vertex,&X,&Y,NULL))
			{
			if (DoScan && GEditor && GEditor->Scan.Active) GEditor->Scan.PreScan();
			if (Snapped==0)
				{
         		DrawRect (Camera,PivColor, X-1, Y-1, X+1, Y+1);
        		DrawRect (Camera,PivColor, X,   Y-4, X,   Y+4);
         		DrawRect (Camera,PivColor, X-4, Y,   X+4, Y);
				}
			else
         		{
         		DrawRect (Camera,PivColor, X-1, Y-1, X+1, Y+1);
         		DrawRect (Camera,PivColor, X-4, Y-4, X+4, Y-4);
         		DrawRect (Camera,PivColor, X-4, Y+4, X+4, Y+4);
         		DrawRect (Camera,PivColor, X-4, Y-4, X-4, Y+4);
         		DrawRect (Camera,PivColor, X+4, Y-4, X+4, Y+4);
				};
			if (DoScan && GEditor && GEditor->Scan.Active) GEditor->Scan.PostScan(EDSCAN_BrushVertex,(int)Brush,-2,0,&Vertex); // -2 = pivot
			};
		};
	GMem.Release	(TransformedEdPolys);
	Brush->Unlock	(&BrushInfo);
	//
	UNGUARD("FGlobalRender::DrawBrushFPolys");
	};

void FGlobalRender::DrawLevelBrushes (ICamera *Camera)
	{
	GUARD;
	//
	ULevel			*Level		= Camera->Level.Level;
	UModel			*Brush;
	FConstraints	*Constraints;
	BYTE			WireColor;
	int				i,ShowVerts,Selected;
	//
	// Draw all regular brushes:
	//
	for (i=1; i<Level->BrushArray->Num; i++)
		{
		Brush = Level->BrushArray->Element(i);
		//
		if (Brush->ModelFlags & MF_Color)
			{
			WireColor = COLOR(Brush->Color,4);
			}
		else if (Brush->PolyFlags & PF_Portal)
			{
			WireColor = ScaleBoxHiColor;
			}
		else switch (Brush->CsgOper)
			{
			case CSG_Add:
				if		(Brush->PolyFlags & PF_Semisolid)	WireColor = SemiSolidWireColor;
				else if	(Brush->PolyFlags & PF_NotSolid)	WireColor = NonSolidWireColor;
				else										WireColor = AddWireColor;
				break;
			case CSG_Subtract:
	            WireColor = SubtractWireColor;
				break;
			default:
	            WireColor = GreyWireColor;
				break;
			};
		Selected  = GEditor && GEditor->MapEdit && (Brush->ModelFlags & MF_Selected);
		ShowVerts = Selected;
		//
		if (GEditor && GEditor->MapEdit && (Brush->ModelFlags & MF_ShouldSnap))
			{
			Constraints = &GEditor->Constraints;
			}
		else Constraints = NULL;
		//
		DrawBrushPolys(Camera,Brush,WireColor,0,Constraints,ShowVerts,ShowVerts,Selected,1);
		};
	UNGUARD("FGlobalRender::DrawLevelBrushes");
	};

void FGlobalRender::DrawActiveBrush (ICamera *Camera)
	{
	GUARD;
	//
	int	Selected = Camera->Level.Brush->ModelFlags & MF_Selected;
	DrawBrushPolys
		(
		Camera,Camera->Level.Brush,BrushWireColor,1,
		GEditor ? &GEditor->Constraints : NULL,1,GEditor ? GEditor->ShowVertices : 0,Selected, 1
		);
	UNGUARD("FGlobalRender::DrawActiveBrush");
	};

//
// Draw the brush's bounding box and pivots:
//
void FGlobalRender::DrawBoundingVolume(ICamera *Camera,FBoundingVolume *Bound)
	{
	GUARD;
	//
	FVector B[2],P,Q;
	FLOAT SX,SY;
	int i,j,k;
	//
	B[0]=Bound->Min;
	B[1]=Bound->Max;
	//
	for (i=0; i<2; i++) for (j=0; j<2; j++)
		{
		P.X=B[i].X; Q.X=B[i].X;
		P.Y=B[j].Y; Q.Y=B[j].Y;
		P.Z=B[0].Z; Q.Z=B[1].Z;
		Draw3DLine (Camera,&P,&Q,1,ScaleBoxColor,0,1);
		//
		P.Y=B[i].Y; Q.Y=B[i].Y;
		P.Z=B[j].Z; Q.Z=B[j].Z;
		P.X=B[0].X; Q.X=B[1].X;
		Draw3DLine (Camera,&P,&Q,1,ScaleBoxColor,0,1);
		//
		P.Z=B[i].Z; Q.Z=B[i].Z;
		P.X=B[j].X; Q.X=B[j].X;
		P.Y=B[0].Y; Q.Y=B[1].Y;
		Draw3DLine (Camera,&P,&Q,1,ScaleBoxColor,0,1);
		};
	for (i=0; i<2; i++) for (j=0; j<2; j++) for (k=0; k<2; k++)
		{
		P.X=B[i].X; P.Y=B[j].Y; P.Z=B[k].Z;
		if (Project (Camera,&P,&SX,&SY,NULL))
			{
			if (GEditor && GEditor->Scan.Active) GEditor->Scan.PreScan();
			DrawRect (Camera,ScaleBoxHiColor,SX-1,SY-1,SX+1,SY+1);
			if (GEditor && GEditor->Scan.Active) GEditor->Scan.PostScan(EDSCAN_BrushVertex,0,0,0,&P);
			};
		};
	UNGUARD("FGlobalRender::DrawBoundingVolume");
	};

/*-----------------------------------------------------------------------------
	Misc gfx
-----------------------------------------------------------------------------*/

//
// Draw a piece of an orthogonal grid (arbitrary axes):
//
void FGlobalRender::DrawGridSection (ICamera *Camera, int CameraLocX,
	int CameraSXR, int CameraGridY, FVector *A, FVector *B,
	FLOAT *AX, FLOAT *BX,int AlphaCase)
	{
	GUARD;
	//
	if (!CameraGridY) return;
	if (!Camera->Camera->IsOrtho()) appError("Ortho camera error");
	//
	FLOAT	Start = (int)((CameraLocX - (CameraSXR>>1)*Camera->Zoom)/CameraGridY) - 1.0;
	FLOAT	End   = (int)((CameraLocX + (CameraSXR>>1)*Camera->Zoom)/CameraGridY) + 1.0;
	int     Dist  = (int)(Camera->SXR * Camera->Zoom / CameraGridY);
	int		i,Color,IncBits,Max,iStart,iEnd;
	FLOAT	Alpha,Ofs;
	//
	IncBits = 0;
	Max     = Camera->SXR >> 2;
	//
	if ((Dist+Dist) >= Max)
		{
		while ((Dist>>IncBits) >= Max) IncBits++;
		Alpha = (FLOAT)Dist / (FLOAT)((1<<IncBits) * Max);
		}
	else Alpha = 0.0;
	//
	iStart  = OurMax((int)Start,-32768/CameraGridY) >> IncBits;
	iEnd    = OurMin((int)End,  +32768/CameraGridY) >> IncBits;
	//
	for (i=iStart; i<iEnd; i++)
		{
		*AX = (i * CameraGridY) << IncBits;
		*BX = (i * CameraGridY) << IncBits;
		//
		if ((i<<IncBits)&7)	Ofs = 6.9f; // Normal
		else				Ofs = 8.9f; // Highlight 8-aligned
		//
		if ((i&1)!=AlphaCase)
			{
			if (i&1) Ofs += Alpha * (4.0-Ofs);
			Color = COLOR(P_GREY,(int)Ofs);
			DrawOrthoLine(Camera,A,B,Color+16,1);
			};
		};
	UNGUARD("FGlobalRender::DrawGridSection");
	};

//
// Draw worldbox and groundplane lines, if desired.
//
void FGlobalRender::DrawWireBackground (ICamera *Camera)
	{
	GUARD;
	//
	if (!GEditor) return;
	//
	FVector	*Origin = &Camera->Coords.Origin;
	FVector B1={ 32768.0, 32767.0, 32767.0}; // Vector defining worldbox lines
	FVector B2={-32768.0, 32767.0, 32767.0};
	FVector B3={ 32768.0,-32767.0, 32767.0};
	FVector B4={-32768.0,-32767.0, 32767.0};
	FVector B5={ 32768.0, 32767.0,-32767.0};
	FVector B6={-32768.0, 32767.0,-32767.0};
	FVector B7={ 32768.0,-32767.0,-32767.0};
	FVector B8={-32768.0,-32767.0,-32767.0};
	FVector A,B;
	int     i,j,Color;
	//
	if (Camera->Camera->IsOrtho())
		{
		if (Camera->ShowFlags & SHOW_Frame)
			{
			//
			// Draw grid:
			//
			for (int AlphaCase=0; AlphaCase<=1; AlphaCase++)
				{
				if (Camera->RendMap==REN_OrthXY)
					{
					A.Y=+32767.0; A.Z=0.0; // Do Y-Axis lines
					B.Y=-32767.0; B.Z=0.0;
					DrawGridSection (Camera,Origin->X,Camera->SXR,GEditor->Constraints.Grid.X,&A,&B,&A.X,&B.X,AlphaCase);
					//
					A.X=+32767.0; A.Z=0.0; // Do X-Axis lines
					B.X=-32767.0; B.Z=0.0;
					DrawGridSection (Camera,Origin->Y,Camera->SYR,GEditor->Constraints.Grid.Y,&A,&B,&A.Y,&B.Y,AlphaCase);
					}
				else if (Camera->RendMap==REN_OrthXZ)
					{
					A.Z=+32767.0; A.Y=0.0; // Do Z-Axis lines
					B.Z=-32767.0; B.Y=0.0;
					DrawGridSection (Camera,Origin->X,Camera->SXR,GEditor->Constraints.Grid.X,&A,&B,&A.X,&B.X,AlphaCase);
					//
					A.X=+32767.0; A.Y=0.0; // Do X-Axis lines
					B.X=-32767.0; B.Y=0.0;
					DrawGridSection (Camera,Origin->Z,Camera->SYR,GEditor->Constraints.Grid.Z,&A,&B,&A.Z,&B.Z,AlphaCase);
					}
				else if (Camera->RendMap==REN_OrthYZ)
					{
					A.Z=+32767.0; A.X=0.0; // Do Z-Axis lines
					B.Z=-32767.0; B.X=0.0;
					DrawGridSection (Camera,Origin->Y,Camera->SXR,GEditor->Constraints.Grid.Y,&A,&B,&A.Y,&B.Y,AlphaCase);
					//
					A.Y=+32767.0; A.X=0.0; // Do Y-Axis lines
					B.Y=-32767.0; B.X=0.0;
					DrawGridSection (Camera,Origin->Z,Camera->SYR,GEditor->Constraints.Grid.Z,&A,&B,&A.Z,&B.Z,AlphaCase);
					};
				};
			//
			// Draw axis lines:
			//
			if (Camera->Camera->IsOrtho())	Color = WireGridAxis;
			else							Color = GroundPlaneHighlight;
			//
			A.X=+32767.0;  A.Y=0; A.Z=0;
			B.X=-32767.0;  B.Y=0; B.Z=0;
        	DrawOrthoLine(Camera,&A,&B,Color,0);
			//
			A.X=0; A.Y=+32767.0; A.Z=0;
			B.X=0; B.Y=-32767.0; B.Z=0;
        	DrawOrthoLine(Camera,&A,&B,Color,0);
			//
			A.X=0; A.Y=0; A.Z=+32767.0;
			B.X=0; B.Y=0; B.Z=-32767.0;
	       	DrawOrthoLine(Camera,&A,&B,Color,0);
			};
		//
		// Draw orthogonal worldframe:
		//
     	DrawOrthoLine(Camera,&B1,&B2,WorldBoxColor,1);
     	DrawOrthoLine(Camera,&B3,&B4,WorldBoxColor,1);
     	DrawOrthoLine(Camera,&B5,&B6,WorldBoxColor,1);
     	DrawOrthoLine(Camera,&B7,&B8,WorldBoxColor,1);
     	DrawOrthoLine(Camera,&B1,&B3,WorldBoxColor,1);
     	DrawOrthoLine(Camera,&B5,&B7,WorldBoxColor,1);
		DrawOrthoLine(Camera,&B2,&B4,WorldBoxColor,1);
     	DrawOrthoLine(Camera,&B6,&B8,WorldBoxColor,1);
     	DrawOrthoLine(Camera,&B1,&B5,WorldBoxColor,1);
     	DrawOrthoLine(Camera,&B2,&B6,WorldBoxColor,1);
     	DrawOrthoLine(Camera,&B3,&B7,WorldBoxColor,1);
     	DrawOrthoLine(Camera,&B4,&B8,WorldBoxColor,1);
		return;
		};
	//
	// Draw worldbox
	//
	if ((Camera->ShowFlags & SHOW_Frame) &&
		!(Camera->ShowFlags & SHOW_Backdrop))
		{
		Draw3DLine(Camera,&B1,&B2,1,WorldBoxColor,1,0);
		Draw3DLine(Camera,&B1,&B2,1,WorldBoxColor,1,0);
		Draw3DLine(Camera,&B3,&B4,1,WorldBoxColor,1,0);
		Draw3DLine(Camera,&B5,&B6,1,WorldBoxColor,1,0);
		Draw3DLine(Camera,&B7,&B8,1,WorldBoxColor,1,0);
		Draw3DLine(Camera,&B1,&B3,1,WorldBoxColor,1,0);
		Draw3DLine(Camera,&B5,&B7,1,WorldBoxColor,1,0);
		Draw3DLine(Camera,&B2,&B4,1,WorldBoxColor,1,0);
		Draw3DLine(Camera,&B6,&B8,1,WorldBoxColor,1,0);
		Draw3DLine(Camera,&B1,&B5,1,WorldBoxColor,1,0);
		Draw3DLine(Camera,&B2,&B6,1,WorldBoxColor,1,0);
		Draw3DLine(Camera,&B3,&B7,1,WorldBoxColor,1,0);
		Draw3DLine(Camera,&B4,&B8,1,WorldBoxColor,1,0);
		//
		j=(63-1)/2; // Index of middle line (axis).
		for (i=0; i<63; i++)
			{
			A.X=32767.0*(-1.0+2.0*i/(63-1));	B.X=A.X;
			A.Y=32767;                          B.Y=-32767.0;
			A.Z=0.0;							B.Z=0.0;
			Draw3DLine(Camera,&A,&B,1,(i==j)?GroundPlaneHighlight:GroundPlaneColor,1,0);
			A.Y=A.X;							B.Y=B.X;
			A.X=32767.0;						B.X=-32767.0;
			Draw3DLine(Camera,&A,&B,1,(i==j)?GroundPlaneHighlight:GroundPlaneColor,1,0);
			};
		};
	UNGUARD("FGlobalRender::DrawWireBackground");
	};
