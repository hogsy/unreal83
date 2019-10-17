/*=============================================================================
	UnRaster.cpp: Unreal polygon rasterizer

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"
#include "UnRaster.h"

/*-----------------------------------------------------------------------------
	TRasterSetup template implementation
-----------------------------------------------------------------------------*/

//
// Compute screen bounding box of a polygon
//
template <class TRasterSideSetupT,class TRasterLineT,class TRasterPolyT,class TThisT,class TTransformT>
inline void TRasterSetup<TRasterSideSetupT,TRasterLineT,TRasterPolyT,TThisT,TTransformT>::CalcBound
(const TTransformT *Pts, int NumPts, FScreenBounds &BoxBounds)
	{
	SLOW_GUARD;
	const TTransformT *P;
	INT	MinX,MaxX,MinY,MaxY;
	//
	MinY = MaxY = Pts[0].IntY;
	MinX = MaxX = Pts[0].IntX;
	//
	P = &Pts [1];
	for (int i=1; i<NumPts; i++)
		{
		if		(P->IntX < MinX) MinX = P->IntX;
		else if (P->IntX > MaxX) MaxX = P->IntX;
		//
		if		(P->IntY < MinY) MinY = P->IntY;
		else if (P->IntY > MaxY) MaxY = P->IntY;
		P++;
		};
	BoxBounds.MinX = UNFIX(MinX);
	BoxBounds.MaxX = UNFIX(MaxX);
	BoxBounds.MinY = MinY;
	BoxBounds.MaxY = MaxY;
	SLOW_UNGUARD("TRasterSetup::CalcBound");
	};

//
// Setup rasterization for a set of points
//
template <class TRasterSideSetupT,class TRasterLineT,class TRasterPolyT,class TThisT,class TTransformT>
inline void TRasterSetup<TRasterSideSetupT,TRasterLineT,TRasterPolyT,TThisT,TTransformT>::Setup
(const ICamera *Camera, const TTransformT *Pts, int NumPts, FMemPool *MemPool)
	{
	GUARD;
	BEGINTIME(Setup);
	//
	const TTransformT	*P1,*P2;
	int					iTop,iBottom,TopY,BottomY,Y,i,j;
	TRasterSideSetupT	*Side,**SidePtr;
	//
	Mem    = MemPool;
	MemTop = Mem->GetFast(0);
	//
	// Find top/bottom:
	//
	TopY    = Pts[0].IntY; iTop    = 0;
	BottomY = Pts[0].IntY; iBottom = 0;
	//
	P1 = &Pts [1];
	for (i=1; i<NumPts; i++)
		{
		if		(P1->IntY < TopY   ) {TopY    = P1->IntY; iTop    = i;}
		else if (P1->IntY > BottomY) {BottomY = P1->IntY; iBottom = i;};
		P1++;
		};
	StartY = TopY;
	EndY   = BottomY;
	//
	// Rasterize left side
	//
	i       = iTop;
	P1      = &Pts[i]; 
	Y       = StartY;
	SidePtr = &LeftSide;
	while (i != iBottom)
		{
		j			= (i>=1) ? (i-1) : (NumPts-1);
		P2			= &Pts[j];
		Side		= (TRasterSideSetupT *)Mem->GetFast(sizeof(TRasterSideSetupT));
		*SidePtr	= Side;
		SidePtr		= &Side->Next;
		Side->DY	= ftoi(P2->ScreenY-0.5) - Y;
		Side->SetupSide (P1,P2,Y);
		Y		   += Side->DY;
		P1			= P2;
		i			= j;
		};
	*SidePtr = NULL;
	//
	// Rasterize right side
	//
	i		= iTop;
	P1		= &Pts[i]; 
	Y       = StartY;
	SidePtr = &RightSide;
	while (i != iBottom)
		{
		j        = ((i+1)<NumPts) ? (i+1) : 0;
		P2       = &Pts[j];
		Side     = (TRasterSideSetupT *)Mem->GetFast(sizeof(TRasterSideSetupT));
		*SidePtr = Side;
		SidePtr  = &Side->Next;
		Side->DY = ftoi(P2->ScreenY-0.5) - Y;
		Side->SetupSide (P1,P2,Y);
		P1		 = P2;
		Y       += Side->DY;
		i		 = j;
		};
	*SidePtr = NULL;
	//
	STAT(GStat.NumSides += NumPts;)
	STAT(GStat.NumRasterPolys ++;)
	//
	ENDTIME(Setup);
	UNGUARD("TRasterSetup::Setup");
	};

//
// Setup rasterization for a set of points, utilizing a side setup cache.
// Note that we don't cache Side->DY values, since cache entries 0-3
// are reserved for automatic setup of sides clipped by the view frustrum
// planes.
//
template <class TRasterSideSetupT,class TRasterLineT,class TRasterPolyT,class TThisT,class TTransformT>
inline void TRasterSetup<TRasterSideSetupT,TRasterLineT,TRasterPolyT,TThisT,TTransformT>::SetupCached
(const ICamera *Camera, const TTransformT *Pts, int NumPts, FMemPool *TempPool, FMemPool *CachePool, TRasterSideSetupT **Cache)
	{
	GUARD;
	BEGINTIME(SetupCached);
	//
	const TTransformT	*P1,*P2;
	int					iTop,iBottom,Y,i,j,TopY,BottomY;
	TRasterSideSetupT	*Side,**SidePtr;
	//
	Mem    = TempPool;
	MemTop = Mem->GetFast(0);
	//
	// Find top/bottom:
	//
	TopY = BottomY = Pts[0].IntY;
	iTop = iBottom = 0;
	//
	P1       = &Pts [1];
	for (i=1; i<NumPts; i++)
		{
		if (P1->IntY < TopY   ) {TopY    = P1->IntY; iTop    = i;};
		if (P1->IntY > BottomY) {BottomY = P1->IntY; iBottom = i;};
		P1++;
		};
	StartY = TopY;
	EndY   = BottomY;
	//
	// Rasterize left side
	//
	Y       = TopY;
	i       = iTop;
	P1      = &Pts[i]; 
	SidePtr = &LeftSide;
	while (i != iBottom)
		{
		j			= (i>=1) ? (i-1) : (NumPts-1);
		P2			= &Pts[j];
		//
		TRasterSideSetupT **CachePtr = &Cache[P1->iSide];
		if ((P1->iSide==INDEX_NONE) || (*CachePtr == NULL))
			{
			if (P1->iSide==INDEX_NONE)
				{
				Side = (TRasterSideSetupT *)TempPool->GetFast(sizeof(TRasterSideSetupT));
				}
			else
				{
				Side = (TRasterSideSetupT *)CachePool->GetFast(sizeof(TRasterSideSetupT));
				*CachePtr=Side;
				};
			Side->DY = P2->IntY - Y;
			Side->SetupSide (P1,P2,Y);
			}
		else
			{
			Side		= *CachePtr;
			Side->DY	= P2->IntY - Y;
			STAT(GStat.NumSidesCached++);
			};
		*SidePtr	= Side;
		SidePtr		= &Side->Next;
		Y		   += Side->DY;
		P1			= P2;
		i			= j;
		};
	*SidePtr = NULL;
	//
	// Rasterize right side
	//
	Y       = TopY;
	i		= iTop;
	P1		= &Pts[i]; 
	SidePtr = &RightSide;
	while (i != iBottom)
		{
		j        = ((i+1)<NumPts) ? (i+1) : 0;
		P2       = &Pts[j];
		//
		TRasterSideSetupT **CachePtr = &Cache[P2->iSide];
		if ((P2->iSide==INDEX_NONE) || (*CachePtr == NULL))
			{
			if (P2->iSide==INDEX_NONE)
				{
				Side = (TRasterSideSetupT *)TempPool->GetFast(sizeof(TRasterSideSetupT));
				}
			else
				{
				Side = (TRasterSideSetupT *)CachePool->GetFast(sizeof(TRasterSideSetupT));
				*CachePtr=Side;
				};
			Side->DY = P2->IntY - Y;
			Side->SetupSide (P1,P2,Y);
			}
		else
			{
			Side		= *CachePtr;
			Side->DY	= P2->IntY - Y;
			STAT(GStat.NumSidesCached++);
			};
		*SidePtr = Side;
		SidePtr  = &Side->Next;
		P1		 = P2;
		Y       += Side->DY;
		i		 = j;
		};
	*SidePtr = NULL;
	//
	STAT(GStat.NumSides += NumPts;)
	STAT(GStat.NumRasterPolys ++;)
	//
	ENDTIME(SetupCached);
	UNGUARD("TRasterSetup::SetupCached");
	};

//
// Generate a rasterization that has been set up already
//
template <class TRasterSideSetupT,class TRasterLineT,class TRasterPolyT,class TThisT,class TTransformT>
inline void TRasterSetup<TRasterSideSetupT,TRasterLineT,TRasterPolyT,TThisT,TTransformT>::Generate
(TRasterPolyT *Raster) const
	{
	GUARD;
	BEGINTIME(Generate);
	//
	Raster->StartY = StartY;
	Raster->EndY   = EndY;
	//
	TRasterSideSetupT *Side = LeftSide;
	for (int c=0; c<2; c++)
		{
		TRasterLineT *Line = &Raster->Lines[0];
		while(Side)
			{
			Side->GenerateSide(Line,c,Side->DY);
			Line += Side->DY;
			Side  = Side->Next;
			};
		Side = RightSide;
		};
	ENDTIME(Generate);
	//
	UNGUARD_BEGIN;
	UNGUARD_MSGF("TRasterSetup::Generate (Start=%i, End=%i)",StartY,EndY);
	UNGUARD_END;
	};

/*-----------------------------------------------------------------------------
	TRasterPoly template implementation
-----------------------------------------------------------------------------*/

//
// Force a rasterized polygon to be forward-faced.  If you don't call this, a
// rasterized backfaced polygon will have its end before its start and won't be drawn.
//
template <class TRasterLineT,class TRasterPointT>
inline void TRasterPoly<TRasterLineT,TRasterPointT>::ForceForwardFace 
(void)
	{
	SLOW_GUARD;
	TRasterLineT *Line = &Lines[0];
	for (int i=StartY; i<EndY; i++)
		{
		if (Line->IsBackwards())
			{
			TRasterPointT Temp	= Line->Start;
			Line->Start			= Line->End;
			Line->End 			= Temp;
			};
		Line++;
		};
	SLOW_UNGUARD("TRasterPoly::ForceForwardFace");
	};

template <class TRasterLineT,class TRasterPointT>
inline void TRasterPoly<TRasterLineT,TRasterPointT>::DrawFlat
(const ICamera *Camera,BYTE Color,BYTE BorderColor)
	{
	GUARD;
	TRasterLineT	*Line,*Prev;
	BYTE			*Dest;
	int				Pixels,LeftBorderStart,LeftBorderEnd,RightBorderStart,RightBorderEnd;
	//
	GBlit.Line	= StartY;
	Prev		= &Lines [0];
	Line   		= &Lines [0];
	Dest   		= Camera->Screen + StartY * Camera->SXStride;
	//
	while (GBlit.Line < EndY)
		{
		Pixels = Line->End.GetX() - Line->Start.GetX();
		if (Pixels>0)
			{
			mymemset(Dest + Line->Start.GetX(), Color, Pixels);
			if ((BorderColor!=0)&&(GBlit.Line==StartY))
				{
				mymemset(Dest + Line->Start.GetX(), BorderColor, Pixels);
				}
			else if (BorderColor!=0)
				{
				LeftBorderStart =		(Prev->Start.GetX()                  );
				LeftBorderStart = OurMin(Line->Start.GetX(),LeftBorderStart  );
				//
				LeftBorderEnd   = OurMax(Prev->Start.GetX(),LeftBorderStart+1);
				LeftBorderEnd   = OurMax(Line->Start.GetX(),LeftBorderEnd    );
				//
				RightBorderStart =		 (Prev->End.GetX()                    );
				RightBorderStart = OurMin(Line->End.GetX(),RightBorderStart   );
				//
				RightBorderEnd   = OurMax(Prev->End.GetX(),RightBorderStart+1 );
				RightBorderEnd   = OurMax(Line->End.GetX(),RightBorderEnd     );
				//
				mymemset (Dest + LeftBorderStart,  BorderColor, LeftBorderEnd-LeftBorderStart);
				mymemset (Dest + RightBorderStart, BorderColor, RightBorderEnd-RightBorderStart);
				//
				if ((GBlit.Line==(EndY-1)) && ((GBlit.Line+1)<Camera->SYR))
					{
					// Draw bottom:
					Dest += Camera->SXStride;
					mymemset (Dest + LeftBorderEnd, BorderColor, OurMax(0,RightBorderStart-LeftBorderEnd));
					return;
					};
				};
			};
		Dest 		+= Camera->SXStride;
		Prev		 = Line;
		GBlit.Line	++;
		Line		++;
		};
	UNGUARD("TRasterPoly::DrawFlat");
	};

template <class TRasterLineT,class TRasterPointT>
inline void TRasterPoly<TRasterLineT,TRasterPointT>::Draw
(const ICamera *Camera)
	{
	GUARD;
	const TRasterLineT *Line;
	BYTE *Dest;
	//
	GBlit.Line	= StartY;
	Line   		= &Lines [0];
	Dest   		= Camera ->Screen + StartY * Camera->SXStride;
	while (GBlit.Line < EndY)
		{
		Line->DrawSpan(Camera,Dest);
		Dest += Camera->SXStride;
		GBlit.Line++;
		Line++;
		};
	UNGUARD("TRasterPoly::Draw");
	};

/*-----------------------------------------------------------------------------
	Forced template instantiation
-----------------------------------------------------------------------------*/

template void TRasterPoly
	<
	class FRasterLine,
	class FRasterPoint
	>::ForceForwardFace(void);
template void TRasterPoly
	<
	class FRasterLine,
	class FRasterPoint
	>::DrawFlat(const ICamera *Camera,BYTE Color,BYTE BorderColor);
template void TRasterPoly
	<
	class FRasterLine,
	class FRasterPoint
	>::Draw(const ICamera *Camera);
template void TRasterSetup
	<
	class FRasterSideSetup,
	class FRasterLine,
	class FRasterPoly,
	class FRasterSetup,
	class FTransform
	>::Setup(const ICamera *Camera,const FTransform *Pts, int NumPts, FMemPool *Mem);
template void TRasterSetup
	<
	class FRasterSideSetup,
	class FRasterLine,
	class FRasterPoly,
	class FRasterSetup,
	class FTransform
	>::SetupCached(const ICamera *Camera,const FTransform *Pts, int NumPts, FMemPool *TempPool, FMemPool *CachePool, FRasterSideSetup **Cache);
template void TRasterSetup
	<
	class FRasterSideSetup,
	class FRasterLine,
	class FRasterPoly,
	class FRasterSetup,
	class FTransform
	>::CalcBound(const FTransform *Pts, int NumPts, FScreenBounds &Bounds);
template void TRasterSetup
	<
	class FRasterSideSetup,
	class FRasterLine,
	class FRasterPoly,
	class FRasterSetup,
	class FTransform
	>::Generate(FRasterPoly *Raster) const;

template void TRasterPoly
	<
	class FRasterTexLine,
	class FRasterTexPoint
	>::ForceForwardFace(void);
template void TRasterPoly
	<
	class FRasterTexLine,
	class FRasterTexPoint
	>::DrawFlat(const ICamera *Camera,BYTE Color,BYTE BorderColor);
template void TRasterPoly
	<
	class FRasterTexLine,
	class FRasterTexPoint
	>::Draw(const ICamera *Camera);
template void TRasterSetup
	<
	class FRasterTexSideSetup,
	class FRasterTexLine,
	class FRasterTexPoly,
	class FRasterTexSetup,
	class FTransTex
	>::Setup(const ICamera *Camera,const FTransTex *Pts, int NumPts, FMemPool *Mem);
template void TRasterSetup
	<
	class FRasterTexSideSetup,
	class FRasterTexLine,
	class FRasterTexPoly,
	class FRasterTexSetup,
	class FTransTex
	>::CalcBound(const FTransTex *Pts, int NumPts, FScreenBounds &Bounds);
template void TRasterSetup
	<
	class FRasterTexSideSetup,
	class FRasterTexLine,
	class FRasterTexPoly,
	class FRasterTexSetup,
	class FTransTex
	>::Generate(FRasterTexPoly *Raster) const;

/*-----------------------------------------------------------------------------
	Rasterizer globals
-----------------------------------------------------------------------------*/

void FGlobalRaster::Init(void)
	{
	GUARD;
	//
	Raster = (FRasterPoly *)appMalloc(sizeof(FRasterPoly)+FGlobalRaster::MAX_RASTER_LINES*sizeof (FRasterLine),"GRaster");
	debug(LOG_Init,"Rasterizer initialized");
	//
	UNGUARD("FGlobalRaster::Init");
	};

void FGlobalRaster::Exit(void)
	{
	GUARD;
	appFree(Raster);
	debug(LOG_Exit,"Rasterizer closed");
	UNGUARD("FGlobalRaster::Exit");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
