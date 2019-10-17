/*=============================================================================
	UnSprite.cpp: Unreal sprite rendering functions.

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"
#include "UnRaster.h"

//
// Parameters
//
#define SPRITE_PROJECTION_FORWARD	32.00 /* Move sprite projection planes forward */
#define NOSORT_SIZE					36.0  /* Don't bother sorting meshmaps smaller than this in either axis*/

//
// Addings sprites to per-node list:
//
#define ADD_START 100000.0
#define ADD_END   0.0

/*------------------------------------------------------------------------------
	Dynamic node contents
------------------------------------------------------------------------------*/

//
// Init buffer holding all dynamic contents
//
void dynamicsLock (IModel *ModelInfo)
	{
	GUARD;
	if (GRender.DynamicsLocked) appError ("dynamicsLock: Already locked");
	//
	GRender.NumDynamics 	= 0;
	GRender.NumPostDynamics	= 0;
	GRender.DynamicsLocked	= 1;
	GDynMem.InitPool();
	//
	UNGUARD("dynamicsLock");
	};

//
// Cleanup buffer holding all dynamic contents (call after rendering)
//
void dynamicsUnlock (IModel *ModelInfo)
	{
	GUARD;
	FDynamicsIndex	*DynamicsIndex;
	FBspNode		*Node;
	//
	if (!GRender.DynamicsLocked) appError ("dynamicsUnlocked: Not locked");
	//
	DynamicsIndex = GRender.DynamicsIndex;
	for (INDEX i=0; i<GRender.NumDynamics; i++)
		{
		Node 				= &ModelInfo->BspNodes [DynamicsIndex->iNode];
		Node->iDynamic[0]	= INDEX_NONE;
		Node->iDynamic[1]	= INDEX_NONE;
		DynamicsIndex++;
		};
	GRender.NumDynamics 	= 0;
	GRender.NumPostDynamics = 0;
	//
	GRender.DynamicsLocked  = 0;
	UNGUARD("dynamicsUnlock");
	};

//
// Add an empty entry to dynamic contents and link it up.
// Returns NULL if the dynamics structure is full.
// In this case, the rendering engine must recover gracefully.
//
FDynamicsIndex inline *dynamicsAdd (IModel *ModelInfo, INDEX iNode, int Type,
	FSprite *Sprite, FRasterPoly *Raster, FLOAT Z,int IsBack)
	{
	GUARD;
	FBspNode		*Node = &ModelInfo->BspNodes [iNode];
	FDynamicsIndex	*Index, *TempIndex,*PrevIndex;
	int				iDynamic;
	//
	if (GRender.NumDynamics >= GRender.MaxDynamics) return NULL;
	//
	iDynamic	= GRender.NumDynamics++;
	Index    	= &GRender.DynamicsIndex[iDynamic];
	//
	// Link into dynamics chain:
	//
	if ((Z==ADD_START) || (Node->iDynamic[IsBack] == INDEX_NONE))
		{
		Index->iNext    = Node->iDynamic[IsBack];
		Node->iDynamic[IsBack] = iDynamic;
		}
	else if (Z == ADD_END)
		{
		TempIndex = &GRender.DynamicsIndex [Node->iDynamic[IsBack]];
		while (TempIndex->iNext != INDEX_NONE) TempIndex = &GRender.DynamicsIndex [TempIndex->iNext];
		//
		Index->iNext     = INDEX_NONE;
		TempIndex->iNext = iDynamic;
		}
	else // Drawable dynamics
		{
		//
		// Z sort them down.  Things that must remain on top, such as sprites (not chunks) are
		// assigned a Z value of zero.
		//
		TempIndex = &GRender.DynamicsIndex [Node->iDynamic[IsBack]];
		PrevIndex = NULL;
		//
		while ((TempIndex->iNext != INDEX_NONE) && (TempIndex->Z >= Z))
			{
			PrevIndex = TempIndex;
			TempIndex = &GRender.DynamicsIndex[TempIndex->iNext];
			};
		if (PrevIndex == NULL)
			{
			Index->iNext			= Node->iDynamic[IsBack];
			Node->iDynamic[IsBack]  = iDynamic;
			}
		else
			{
			Index->iNext			= PrevIndex->iNext;
			PrevIndex->iNext		= iDynamic;
			};
		};
	Index->iNode    = iNode;
	Index->Type     = Type;
	Index->Sprite	= Sprite;
	Index->Raster	= Raster;
	Index->Span		= NULL;
	Index->Z		= Z;
	//
	#ifdef STATS
		switch (Type)
			{
			case DY_SPRITE:		GStat.NumSprites++;			break;
			case DY_CHUNK:		GStat.NumChunks++;			break;
			case DY_FINALCHUNK:	GStat.NumFinalChunks++; 	break;
			};
	#endif
	//
	return Index;
	UNGUARD("dynamicsAdd");
	};

//
// Add all actors to dynamic contents, optionally excluding the player actor.
// Pass INDEX_NONE to exclude nothing.
//
void dynamicsSetup (ICamera *Camera, INDEX iExclude)
	{
	GUARD;
	UActorList 		*Actors    = Camera->Level.Actors; 
	IModel 			*ModelInfo = &Camera->Level.ModelInfo;
	FSprite			*Sprite;
	FDynamicsIndex	*Index;
	//
	if ((Camera->ShowFlags & SHOW_Actors) && ModelInfo->NumBspNodes)
		{
		AActor *Actor = &Actors->Element(0);
		for (INDEX iActor=0; iActor<Actors->Max; iActor++)
			{
			if (Actor->Class && (iActor!=iExclude) && 
				!((Camera->Level.State==LEVEL_UpPlay)?Actor->bHidden:Actor->bHiddenEd))
				{
				Sprite  		= (FSprite *)GDynMem.GetFast(sizeof(FSprite));
				Sprite->iActor	= iActor;
				Index    		= dynamicsAdd(ModelInfo,0,DY_SPRITE,Sprite,NULL,ADD_END,0);
				};
			Actor++;
			};
		};
	UNGUARD("dynamicsSetup");
	};

/*------------------------------------------------------------------------------
	Dynamics filtering
------------------------------------------------------------------------------*/

//
// Break a sprite into a chunk and set it up to begin filtering down the Bsp
// during rendering.
//
void inline MakeSpriteChunk (IModel *ModelInfo, ICamera *Camera, INDEX iNode,FDynamicsIndex *Index)
	{
	GUARD;
	FSprite				*Sprite = Index->Sprite;
	FTransform			*Verts  = Sprite->Verts;
	FDynamicsIndex		*NewIndex;
	FRasterPoly			*Raster;
	FRasterLine			*Line;
	FLOAT				PlaneZ,PlaneZRD,PlaneX1,PlaneX2,PlaneY1,PlaneY2;
	FLOAT				FloatX1,FloatX2,FloatY1,FloatY2;
	int					i;
	//
	// Compute four projection-plane points from sprite extents and camera:
	//
	FloatX1 = (FLOAT)Sprite->X1; FloatX2 = (FLOAT)Sprite->X2;
	FloatY1 = (FLOAT)Sprite->Y1; FloatY2 = (FLOAT)Sprite->Y2;
	//
	PlaneZ	 = Sprite->Z - SPRITE_PROJECTION_FORWARD; // Move closer to prevent actors from slipping into floor
	PlaneZRD = PlaneZ * Camera->RProjZ;
	//
	PlaneX1   = PlaneZRD * (FloatX1 - Camera->FSXR2);
	PlaneX2   = PlaneZRD * (FloatX2 - Camera->FSXR2);
	PlaneY1   = PlaneZRD * (FloatY1 - Camera->FSYR2);
	PlaneY2   = PlaneZRD * (FloatY2 - Camera->FSYR2);
	//
	Verts[0].X = PlaneX1; Verts[0].Y = PlaneY1; Verts[0].Z = PlaneZ; Verts[0].ScreenX = FloatX1; Verts[0].ScreenY = FloatY1;
	Verts[1].X = PlaneX2; Verts[1].Y = PlaneY1; Verts[1].Z = PlaneZ; Verts[1].ScreenX = FloatX2; Verts[1].ScreenY = FloatY1;
	Verts[2].X = PlaneX2; Verts[2].Y = PlaneY2; Verts[2].Z = PlaneZ; Verts[2].ScreenX = FloatX2; Verts[2].ScreenY = FloatY2;
	Verts[3].X = PlaneX1; Verts[3].Y = PlaneY2; Verts[3].Z = PlaneZ; Verts[3].ScreenX = FloatX1; Verts[3].ScreenY = FloatY2;
	//
	for (i=0; i<4; i++) Verts[i].TransformPoint(Camera->Uncoords);
	//for (i=0; i<4; i++) rendDraw3DLine  (Camera,&Verts[i].World,&Verts[(i>0)?(i-1):(3)].World,1,3,0);
	//
	Raster			= (FRasterPoly *)GDynMem.Get(sizeof(FRasterPoly) + (Sprite->Y2 - Sprite->Y1)*sizeof(FRasterLine));
	Raster->StartY	= Sprite->Y1;
	Raster->EndY	= Sprite->Y2;
	//
	Line = &Raster->Lines[0];
	for (i=Raster->StartY; i<Raster->EndY; i++)
		{
		Line->Start.X = Sprite->X1;
		Line->End.X   = Sprite->X2;
		//
		Line++;
		};
	//rendDrawFlatRasterPoly (Camera,Raster,2,3);
	//
	// Add first sprite chunk at end of dynamics list so it will be processed in
	// the current node's linked rendering list (which we can assume is being
	// walked when this is called)
	//
	NewIndex = dynamicsAdd (ModelInfo,iNode,DY_CHUNK,Sprite,Raster,ADD_END,0);
	//
	UNGUARD("MakeSpriteChunk");
	};

//
// Filter a chunk from the current Bsp node down to its children.  If this is a leaf,
// leave the chunk here.
//
void inline FilterChunk (IModel *ModelInfo, ICamera *Camera, INDEX iNode, FDynamicsIndex *Index,int Outside)
	{
	FSprite				*Sprite 		= Index->Sprite;
	FRasterPoly			*Raster 		= Index->Raster;
	FTransform			*Verts  		= Sprite->Verts;
	FTransform			*V1,*V2,Intersect[4],*I;
	FRasterPoly			*FrontRaster,*BackRaster;
	FRasterPoly			*TopRaster,*BottomRaster,*LeftRaster,*RightRaster;
	FRasterLine			*SourceLine,*Line,*LeftLine,*RightLine;
	FBspNode			*Node;
	FBspSurf			*Poly;
	FVector				*Normal,*Base;
	FLOAT				Dist[4],*D1,*D2,Alpha;
	FLOAT				FloatYAdjust,FloatFixDX;
	INT					FixX,FixDX,X;
	int					i,Front,Back,NumInt,Y0,Y1,ClippedY0,ClippedY1,Size,CSG;
	//
	Node   = &ModelInfo->BspNodes [iNode];
	Poly   = &ModelInfo->BspSurfs [Node->iSurf];
	Normal = &ModelInfo->FVectors [Poly->vNormal];
	Base   = &ModelInfo->FPoints  [Poly->pBase];
	//
	// Find point-to-plane distances for all four vertices (side-of-plane classifications):
	//
	Front  = 0;
	Back   = 0;
	V1     = &Verts [0];
	D1     = &Dist  [0];
	//
	for (i=0; i<4; i++)
		{
		*D1 = FPointPlaneDist (*V1,*Base,*Normal);
		if      (*D1 >  +0.01)  Front = 1;
		else if (*D1 <  -0.01)  Back  = 1; // Note: Both Front and Back can be zero
		//
		V1++;
		D1++;
		};
	if (Front && Back) // Must split the rasterization:
		{
		//
		// Find intersection points:
		//
		V1     = &Verts[3]; D1 = &Dist[3];
		V2     = &Verts[0]; D2 = &Dist[0];
		I      = &Intersect [0];
		NumInt = 0;
		//
		for (i=0; i<4; i++)
			{
			if (((*D1) * (*D2)) < 0.0) // At intersection point
				{
				Alpha = *D1 / (*D1 - *D2);
				//
				I->ScreenX = V1->ScreenX + Alpha * (V2->ScreenX - V1->ScreenX);
				I->ScreenY = V1->ScreenY + Alpha * (V2->ScreenY - V1->ScreenY);
				//
				I++;
				NumInt++;
				};			
			V1 = V2; V2++;
			D1 = D2; D2++;
			};
		if (NumInt<2) goto NoSplit;
		//
		// Allocate front and back rasters:
		//
		Size        = sizeof (FRasterPoly) + (Raster->EndY - Raster->StartY) * sizeof (FRasterLine);
		FrontRaster = (FRasterPoly *)GDynMem.Get(Size);
		BackRaster  = (FRasterPoly *)GDynMem.Get(Size);
		//
		// Make sure that first intersection point is on top:
		//
		if (Intersect[0].ScreenY > Intersect[1].ScreenY)
			{
			Intersect[2] = Intersect[0];
			Intersect[0] = Intersect[1];
			Intersect[1] = Intersect[2];
			};
		Y0 = ftoi(Intersect[0].ScreenY);
		Y1 = ftoi(Intersect[1].ScreenY);
		//
		// Figure out how to split up this raster into two new rasters and copy the right
		// data to each (top/bottom rasters plus left/right rasters)
		//
		if (Y0 > Raster->StartY) // Need to find TopRaster
			{
			if (Dist[0] >= 0) TopRaster = FrontRaster;
			else              TopRaster = BackRaster;
			}
		else TopRaster = NULL; // No TopRaster
		//
		if (Y1 < Raster->EndY) // Need to find BottomRaster
			{
			ClippedY1 = Y1;
			if (Dist[2] >= 0) BottomRaster = FrontRaster;
			else              BottomRaster = BackRaster;
			}
		else BottomRaster = NULL; // No BottomRaster
		//
		if (Intersect[1].ScreenX >= Intersect[0].ScreenX)
			{
			if (Dist[1] >= 0.0) {LeftRaster = BackRaster;  RightRaster = FrontRaster;}
			else	   			{LeftRaster = FrontRaster; RightRaster = BackRaster; };
			}
		else // Intersect[1].ScreenX < Intersect[0].ScreenX
			{
			if (Dist[0] >= 0.0) {LeftRaster = FrontRaster; RightRaster = BackRaster; }
			else                {LeftRaster = BackRaster;  RightRaster = FrontRaster;};
			};
		ClippedY0 = OurMax(Raster->StartY,OurMin(Y0,Raster->EndY));
		ClippedY1 = OurMax(Raster->StartY,OurMin(Y1,Raster->EndY));
		//
		// Set left and right raster defaults (may be overwritten by TopRaster or BottomRaster):
		//
		LeftRaster->StartY = ClippedY0; RightRaster->StartY = ClippedY0;
		LeftRaster->EndY   = ClippedY1; RightRaster->EndY   = ClippedY1;
		//
		// Copy TopRaster section:
		//
		if (TopRaster)
			{
			TopRaster->StartY = Raster->StartY;
			//
			SourceLine        = &Raster->Lines    [0];
			Line              = &TopRaster->Lines [0];
			//
			for (i=TopRaster->StartY; i<ClippedY0; i++)
				{
				*(Line++) = *(SourceLine++);
				};
			};
		//
		// Copy BottomRaster section:
		//
		if (BottomRaster)
			{
			BottomRaster->EndY = Raster->EndY;
			//
			SourceLine         = &Raster->Lines       [ClippedY1 - Raster->StartY];
			Line               = &BottomRaster->Lines [ClippedY1 - BottomRaster->StartY];
			//
			for (i=ClippedY1; i<BottomRaster->EndY; i++)
				*(Line++) = *(SourceLine++);
			};
		//
		// Split middle raster section:
		//
		if (Y1 != Y0)
			{
			FloatYAdjust = (FLOAT)Y0 + 1.0 - Intersect[0].ScreenY;
			FloatFixDX 	 = 65536.0 * (Intersect[1].ScreenX - Intersect[0].ScreenX) / (Intersect[1].ScreenY - Intersect[0].ScreenY);
			FixDX      	 = FloatFixDX;
			FixX       	 = 65536.0 * Intersect[0].ScreenX + FloatFixDX * FloatYAdjust;
			//
			SourceLine = &Raster->Lines      [Y0 - Raster->StartY];
			LeftLine   = &LeftRaster->Lines  [Y0 - LeftRaster->StartY];
			RightLine  = &RightRaster->Lines [Y0 - RightRaster->StartY];
			//
			for (i=Y0; i<Y1; i++)
				{
				if ((i >= Raster->StartY) && (i < Raster->EndY)) // Can skip-in faster !!
					{
					*LeftLine  = *SourceLine;
					*RightLine = *SourceLine;
					//
					X = UNFIX (FixX);
					//
					if (X < LeftLine->End.X)    LeftLine->End.X    = X;
					if (X > RightLine->Start.X) RightLine->Start.X = X;
					};
				FixX += FixDX;
				//
				SourceLine ++;
				LeftLine   ++;
				RightLine  ++;
				};
			};
		if (BackRaster ->EndY <= BackRaster ->StartY) BackRaster  = NULL;
		if (FrontRaster->EndY <= FrontRaster->StartY) FrontRaster = NULL;
		}
	else // Don't have to split the rasterization
		{
		NoSplit:
		FrontRaster = Raster;
		BackRaster  = Raster;
		};
	//
	// Filter:
	//
	CSG = Node->IsCsg();
	//
	if (Front && FrontRaster)
		{
		if (Node->iFront != INDEX_NONE)	dynamicsAdd (ModelInfo,Node->iFront,DY_CHUNK,Sprite,FrontRaster,ADD_START,0);
		else if (Outside || CSG)		dynamicsAdd (ModelInfo,iNode,DY_FINALCHUNK,Sprite,FrontRaster,Sprite->Z,0);
		};
	if (Back && BackRaster) // Only add if this isn't a back leaf
		{
		if (Node->iBack != INDEX_NONE)	dynamicsAdd (ModelInfo,Node->iBack,DY_CHUNK,Sprite,BackRaster,ADD_START,0);
		else if (Outside && !CSG)		dynamicsAdd (ModelInfo,iNode,DY_FINALCHUNK,Sprite,BackRaster,Sprite->Z,1);
		};
	};

//
// If there are any dynamics things in this node that need to be filtered down further,
// process them.  This is called while walking the Bsp tree during rendering _before_
// this node's children are walked.
//
// This routine should not draw anything or save the span buffer  The span buffer is not in
// the proper state to draw stuff because it may contain more holes now (since the front
// hasn't been drawn).  However, the span buffer can be used to reject stuff here.
//
// If FilterDown=0, the node should be treated as if it has no children so the contents are
// processed but not filtered further down.  This occurs when a Bsp node's polygons are
// Bound rejected and thus they can't possibly affect the visible results of drawing the actors.
// This saves lots of tree-walking time.
//
void dynamicsFilter (ICamera *Camera, INDEX iNode,int FilterDown,int Outside)
	{
	GUARD;
	FDynamicsIndex 			*Index;
	IModel 					*ModelInfo	= &Camera->Level.ModelInfo;
	FBspNode				*Node		= &ModelInfo->BspNodes[iNode];
	INDEX					iDynamic	= Node->iDynamic[0];
	FSprite					*Sprite;
	INDEX					iActor;
	//
	while (iDynamic != INDEX_NONE)
		{
		Index = &GRender.DynamicsIndex [iDynamic];
		switch (Index->Type)
			{
			case DY_SPRITE:
				Sprite = Index->Sprite;
				iActor = Sprite->iActor;
				if (GRender.SetActorSprite (Camera,Sprite,iActor))
					{
					MakeSpriteChunk (ModelInfo,Camera,iNode,Index); // May set Index->iNext
					};
				break;
			case DY_CHUNK:
				if (FilterDown) FilterChunk (ModelInfo,Camera,iNode,Index,Outside);
				else            Index->Type = DY_FINALCHUNK;
				break;
			case DY_FINALCHUNK:
				break;
			};
		iDynamic = Index->iNext;
		};
	UNGUARD("dynamicsFilter");
	};

/*------------------------------------------------------------------------------
	Dynamics rendering and prerendering
------------------------------------------------------------------------------*/

//
// This is called for a node's dynamic contents when the contents should be drawn.
// At this instant in time, the span buffer is set up properly for front-to-back rendering.
//
// * Any dynamic contents that can be drawn with span-buffered front-to-back rendering
//   should be drawn now.
//
// * For any dynamic contents that must be drawn transparently (using masking or
//   transparency), the span buffer should be saved.  The dynamic contents can later be
//   drawn back-to-front with the restored span buffer.
//
void dynamicsPreRender (ICamera *Camera, FSpanBuffer *SpanBuffer,
	INDEX iNode, int IsBack)
	{
	GUARD;
	IModel 			*ModelInfo	= &Camera->Level.ModelInfo;
	FBspNode		*Node 		= &ModelInfo->BspNodes[iNode];
	INDEX			iDynamic	= Node->iDynamic[IsBack];
	FSpanBuffer		*Span;
	FDynamicsIndex 	*Index;
	//
	if (iDynamic != INDEX_NONE)
		{
		if (GRender.NumPostDynamics < GRender.MaxPostDynamics)
			GRender.PostDynamics[GRender.NumPostDynamics++] = iNode;
		};
	while (iDynamic != INDEX_NONE)
		{
		Index = &GRender.DynamicsIndex [iDynamic];
		//
		switch (Index->Type)
			{
			case DY_FINALCHUNK:
				Span = (FSpanBuffer *)GDynMem.Get(sizeof(FSpanBuffer));
				Span->AllocIndex(Index->Raster->StartY,Index->Raster->EndY,&GDynMem);
				if (Span->CopyFromRaster(*SpanBuffer,*Index->Raster))
					{
					Index->Span = Span;
					STAT(GStat.ChunksDrawn++);
					}
				else Span->Release(); // Chunk is occluded
				break;
			case DY_CHUNK:
				break;
			case DY_SPRITE:
				break;
			};
		iDynamic = Index->iNext;
		};
	UNGUARD("dynamicsPreRender");
	};

/*------------------------------------------------------------------------------
	Dynamics postrendering
------------------------------------------------------------------------------*/

void dynamicsFinalize (ICamera *Camera, int SpanRender)
	{
	GUARD;
	IModel 		*ModelInfo = &Camera->Level.ModelInfo;
	FDynamicsIndex	*Index;
	FBspNode		*Node;
	INDEX			*iNodePtr,iDynamic;
	//
	if (ModelInfo->NumBspNodes==0) // Can't manage dynamic contents if no nodes
		{
		GRender.DrawLevelActors (Camera,INDEX_NONE);
		return;
		};
	iNodePtr = &GRender.PostDynamics[GRender.NumPostDynamics-1];
	for (INDEX i=0; i<GRender.NumPostDynamics; i++)
		{
		GApp->Poll();
		//
		Node = &ModelInfo->BspNodes [*iNodePtr];
		for (INDEX j=0; j<2; j++)
			{
			iDynamic = Node->iDynamic[j];
			while (iDynamic != INDEX_NONE)
				{
				Index = &GRender.DynamicsIndex  [iDynamic];
				switch (Index->Type)
					{
					case DY_SPRITE:
						break;
					case DY_CHUNK:
						break;
					case DY_FINALCHUNK:
						if (Index->Span != NULL)
							{
							if (GRender.ShowChunks) Index->Raster->DrawFlat(Camera,2,5); // For debugging
							GRender.DrawActorChunk (Camera,Index->Sprite,Index->Span);
							};
						break;
					};
				iDynamic = Index->iNext;
				};
			};
		iNodePtr--;
		};
	UNGUARD("dynamicsFinalize");
	};

/*-----------------------------------------------------------------------------
	Sprites
-----------------------------------------------------------------------------*/

void inline CalcSpriteExtent (ICamera *Camera, UTexture *Texture, int ScreenX, int ScreenY, int Center,
	int *X1, int *Y1, int *X2, int *Y2) // Must agree exactly with rendDrawSprite
	{
	if (Center)
		{
		ScreenX -= Texture->USize >> 1;
		ScreenY -= Texture->VSize >> 1;
		};
	*X1 = ScreenX;
	*Y1 = ScreenY;
	//
	*X2    = *X1 + Texture->USize;
	*Y2    = *Y1 + Texture->VSize;
	};

#define SPRITE_SPAN_LOOP(CMD)\
	while (YL--)\
		{\
		Span = *(SpanIndex++);\
		while (Span && (Span->End <= X1)) Span=Span->Next;\
		if (Span)\
			{\
			if (Span->Start <= X1) /* Draw a span that's not left-clipped */ \
				{\
				Source = Source1;\
				Dest   = Dest1;\
				X      = OurMin (X2,Span->End) - X1;\
				\
				if (!Highlight)	while (X-- > 0) {B = *Source++; CMD; Dest++;}\
				else			while (X-- > 0) {B = *Source++; B=GGfx.ShadeData[B+0x2f00]; CMD; Dest++;};\
				\
				Span=Span->Next;\
				};\
			while (Span && (Span->Start < X2)) /* Draw spans that are left-clipped */ \
				{\
				Offset = Span->Start - X1;\
				Source = Source1            + Offset;\
				Dest   = Dest1              + Offset;\
				X      = OurMin (X2,Span->End) - Span->Start;\
				\
				while (X-- > 0) {B = *Source++; CMD; Dest++;};\
				\
				Span=Span->Next;\
				};\
			};\
		Source1 += USize;\
		Dest1   += Camera->SXStride;\
		};\

#define SPRITE_LOOP(CMD)\
	while (YL--)\
		{\
		Source   = Source1;\
		Dest     = Dest1; X = XL;\
		\
		Source1 += USize;\
		Dest1   += Camera->SXStride;\
		\
		if (!Highlight)	while (X-- > 0) {B = *Source++; CMD; Dest++;}\
		else			while (X-- > 0) {B = *Source++; B=GGfx.ShadeData[B+0x2f00]; CMD; Dest++;};\
		};\

//
// Draw an unscaled sprite.  Call with Center=1 if you're specifying the
// center point, or Center=0 if you're specifying the top left corner.
// If sprite must be clipped, this takes care of clipping.
//
// Does not apply illumination (assumes source is prelit)
//
void FGlobalRender::DrawSprite(ICamera *Camera, BYTE *Texture, int USize, int VSize,
	int X1, int Y1, int Center, EBlitType BlitType, FSpanBuffer *SpanBuffer,
	int Highlight)
	{
	GUARD;
	FSpan		*Span,**SpanIndex;
	BYTE		*Source,*Dest,*Source1,*Dest1;
	BYTE		B;
	int 		X2,Y2,XL,YL,U1,V1,X,Offset;
	//
	if (Camera->ColorBytes!=1) return;
	//
	if (Texture==NULL)
		{
		debug (LOG_Rend,"rendDrawSprite: NULL");
		return;
		};
	if (Center)
		{
		X1 -= USize >> 1;
		Y1 -= VSize >> 1;
		};
	X2    = X1 + USize;
	Y2    = Y1 + VSize;
	//
	// Clip:
	//
	U1 = 0; V1 = 0;
	//
	if (X1<0) {U1 -= X1; X1=0;};
	if (X2 >= Camera->SXR) X2 = Camera->SXR-1;
	//
	if (SpanBuffer)
		{
		if (Y1 < SpanBuffer->StartY) 	{V1 += (SpanBuffer->StartY - Y1); Y1=SpanBuffer->StartY;};
		if (Y2 > SpanBuffer->EndY)  	{Y2 = SpanBuffer->EndY;};
		}
	else
		{
		if (Y1 < 0) 					{V1 -= Y1; Y1=0;};
		if (Y2 > Camera->SYR) 			{Y2 = Camera->SYR;};
		};
	XL = X2-X1;	if (XL <= 0) return;
	YL = Y2-Y1; if (YL <= 0) return;
	//
	Source1 = Texture + U1 + V1 * USize;
	Dest1   = Camera->Screen + X1 + Y1*Camera->SXStride;
	//
	if (SpanBuffer)
		{
		SpanIndex = &SpanBuffer->Index [Y1 - SpanBuffer->StartY];
		switch (BlitType)
			{
			case BT_None:
			case BT_Normal:
			case BT_Fuzzy:
				SPRITE_SPAN_LOOP(if (B!=0) *Dest = B); 
				break;
			case BT_Transparent:
				SPRITE_SPAN_LOOP(if (B!=0) *Dest = GGfx.Blenders[BLEND_Transparent][B+((int)*Dest<<8)]);
				break;
			case BT_Ghost:
				SPRITE_SPAN_LOOP(if (B!=0) *Dest = GGfx.Blenders[BLEND_Ghost][B+((int)*Dest<<8)]);
				break;
			case BT_Glow:
				SPRITE_SPAN_LOOP(if (B!=0) *Dest = GGfx.Blenders[BLEND_Glow][B+((int)*Dest<<8)]);
				break;
			};
		}
	else
		{
		switch (BlitType)
			{
			case BT_None:
			case BT_Normal:
			case BT_Fuzzy:
				SPRITE_LOOP(if (B!=0) *Dest = B); 
				break;
			case BT_Transparent:
				SPRITE_LOOP(if (B!=0) *Dest = GGfx.Blenders[BLEND_Transparent][B+((int)*Dest<<8)]); 
				break;
			case BT_Ghost:
				SPRITE_LOOP(if (B!=0) *Dest = GGfx.Blenders[BLEND_Ghost][B+((int)*Dest<<8)]); 
				break;
			case BT_Glow:
				SPRITE_LOOP(if (B!=0) *Dest = GGfx.Blenders[BLEND_Glow][B+((int)*Dest<<8)]); 
				break;
			};
		};
	UNGUARD("FGlobalRender::DrawSprite");
	};

/*-----------------------------------------------------------------------------
	Scaled sprites
-----------------------------------------------------------------------------*/

//
// Compute the extent (X1,Y1)-(X2,Y2) of a scaled sprite.
//
void inline CalcScaledSpriteExtent
	(
	ICamera *Camera,
	FLOAT ScreenX, FLOAT ScreenY,
	FLOAT XSize, FLOAT YSize,
	int *X1, int *Y1, int *X2, int *Y2
	) 
	{
	ScreenX -= XSize * 0.5;
	ScreenY -= YSize * 0.5;
	//
	// Find correctly rounded X and Y:
	//
	*X1 = ftoi(ceil(ScreenX));
	*X2 = ftoi(ceil(ScreenX+XSize));
	if (*X1 < 0)
		{
		*X1 = 0;
		if (*X2 < 0) *X2 = 0;
		};
	if (*X2 > Camera->SXR)
		{
		*X2 = Camera->SXR;
		if (*X1 > Camera->SXR) *X1 = Camera->SXR;
		};
	*Y1 = ftoi(ceil(ScreenY));
	*Y2 = ftoi(ceil(ScreenY+YSize));
	if (*Y1 < 0)
		{
		*Y1 = 0;
		if (*Y2 < 0) *Y2 = 0;
		};
	if (*Y2 > Camera->SYR)
		{
		*Y2 = Camera->SYR;
		if (*Y1 > Camera->SYR) *Y1 = Camera->SYR;
		};
	};

//
// Draw a scaled sprite.  Takes care of clipping.
// XSize and YSize are in pixels.
//
void FGlobalRender::DrawScaledSprite
	(
	ICamera *Camera, UTexture *Texture,
	FLOAT ScreenX, FLOAT ScreenY, 
	FLOAT XSize, FLOAT YSize, 
	EBlitType BlitType,FSpanBuffer *SpanBuffer,
	int Center,int Highlight
	)
	{
	FLOAT X1,X2,Y1,Y2;
	//
	GUARD;
	if (!Texture) appError("Null texture");
	//
	if (Center)
		{
		ScreenX -= XSize * 0.5;
		ScreenY -= YSize * 0.5;
		};
	X1 = ScreenX; X2 = ScreenX + XSize;
	Y1 = ScreenY; Y2 = ScreenY + YSize;	
	//
	// Clip:
	//
	if (X1 < 0.0) 			X1 = 0.0;
	if (X2 > Camera->FSXR) 	X2 = Camera->FSXR;
	if (X2<=X1) return;
	//
	if (SpanBuffer)
		{
		if (Y1 < SpanBuffer->StartY) Y1 = SpanBuffer->StartY;
		if (Y2 > SpanBuffer->EndY  ) Y2 = SpanBuffer->EndY;
		}
	else
		{
		if (Y1 < 0.0) 					Y1 = 0.0;
		if (Y2 > Camera->FSYR) 			Y2 = Camera->FSYR;
		};
	if (Y2<=Y1) return;
	//
	// Find correctly sampled U and V start and end values:
	//
	FLOAT UScale = 65536.0 * (FLOAT)Texture->USize / XSize;
	FLOAT VScale = 65536.0 * (FLOAT)Texture->VSize / YSize;
	//
	FLOAT U1 = (X1 - ScreenX) * UScale - 0.5;
	FLOAT V1 = (Y1 - ScreenY) * VScale - 0.5;
	//
	FLOAT U2 = (X2 - ScreenX) * UScale - 0.5;
	FLOAT V2 = (Y2 - ScreenY) * VScale - 0.5;
	//
	FLOAT G = Highlight ? (56.0 * 256.0) : (26.0 * 256.0);
	//
	// Build and draw poly:
	//
	FRasterTexPoly *RasterTexPoly = (FRasterTexPoly	*)GMem.Get
		(
		sizeof(FRasterTexPoly)+FGlobalRaster::MAX_RASTER_LINES*sizeof(FRasterTexLine)
		);
	FTransTex P[4];
	P[0].ScreenX = X1; P[0].ScreenY = Y2; P[0].U = U1; P[0].V = V2; P[0].G = G; ftoi(P[0].IntX,P[0].ScreenX-0.5); ftoi(P[0].IntY,P[0].ScreenY-0.5);
	P[1].ScreenX = X1; P[1].ScreenY = Y1; P[1].U = U1; P[1].V = V1; P[1].G = G; ftoi(P[1].IntX,P[1].ScreenX-0.5); ftoi(P[1].IntY,P[1].ScreenY-0.5);
	P[2].ScreenX = X2; P[2].ScreenY = Y1; P[2].U = U2; P[2].V = V1; P[2].G = G; ftoi(P[2].IntX,P[2].ScreenX-0.5); ftoi(P[2].IntY,P[2].ScreenY-0.5);
	P[3].ScreenX = X2; P[3].ScreenY = Y2; P[3].U = U2; P[3].V = V2; P[3].G = G; ftoi(P[3].IntX,P[3].ScreenX-0.5); ftoi(P[3].IntY,P[2].ScreenY-0.5);
	//
	FRasterTexSetup RasterTexSetup;
	RasterTexSetup.Setup	(Camera,P,4,&GMem);
	RasterTexSetup.Generate	(RasterTexPoly);
	//
	switch (BlitType)
		{
		case BT_None:			GBlit.Setup(Camera,Texture,PF_Masked | PF_NoSmooth,0);		break;
		case BT_Normal:			GBlit.Setup(Camera,Texture,PF_Masked | 0,0);				break;
		case BT_Transparent:	GBlit.Setup(Camera,Texture,PF_Masked | PF_Transparent,0);	break;
		case BT_Ghost:			GBlit.Setup(Camera,Texture,PF_Masked | PF_Ghost,0);			break;
		case BT_Glow:			GBlit.Setup(Camera,Texture,PF_Masked | PF_Glow,0);			break;
		case BT_Fuzzy:			GBlit.Setup(Camera,Texture,PF_Masked | 0,0);				break; /* Unused */
		};
	RasterTexPoly->Draw(Camera);
	//
	GMem.Release(RasterTexPoly);
	//
	UNGUARD_BEGIN;
	UNGUARD_MSGF("FGlobalRender::DrawScaledSprite (Y1=%f Y2=%f)",Y1,Y2);
	UNGUARD_END;
	};

/*-----------------------------------------------------------------------------
	Texture block drawing
-----------------------------------------------------------------------------*/

//
// Draw a block of texture on the screen.
//
void FGlobalRender::DrawTiledTextureBlock(ICamera *Camera,UTexture *Texture,
	int X, int XL, int Y, int YL,int U,int V)
	{
	GUARD;
	//
	int		MipLevel=0,USize,VSize;
	//
	BYTE	*Data	= Texture->GetData(&MipLevel,Camera->ColorBytes,&USize,&VSize);
	//
	int UAnd		= USize-1;
	int VAnd		= VSize-1;
	BYTE VShift		= Texture->UBits;
	BYTE *Dest1		= &Camera->Screen[(X + Y*Camera->SXStride)*Camera->ColorBytes];
	//
	int XE			= X+XL+U;
	int YE			= Y+YL+V;
	X+=U; Y+=V;
	//
	if (Camera->ColorBytes==1)
		{
		FColor *Colors = &Texture->Palette->Element(0);
		for (V=Y; V<YE; V++)
			{
			BYTE *Src  = &Data[(V & VAnd) << VShift];
			BYTE *Dest = Dest1;
			for (int U=X; U<XE; U++) *Dest++ = Src[U&UAnd];
			Dest1 += Camera->SXStride;
			}
		}
	else if (Camera->ColorBytes==2)
		{
		WORD *HiColors = (WORD *)GGfx.GetPaletteTable(Camera->Texture,Texture->Palette);
		for (V=Y; V<YE; V++)
			{
			BYTE *Src  = &Data[(V & VAnd) << VShift];
			WORD *Dest = (WORD *)Dest1;
			for (int U=X; U<XE; U++) *Dest++ = HiColors[Src[U&UAnd]];
			Dest1 += Camera->SXStride*2;
			}
		}
	else if (Camera->ColorBytes==4)
		{
		DWORD *TrueColors = (DWORD *)GGfx.GetPaletteTable(Camera->Texture,Texture->Palette);
		for (V=Y; V<YE; V++)
			{
			BYTE  *Src  = &Data[(V & VAnd) << VShift];
			DWORD *Dest = (DWORD *)Dest1;
			for (U=X; U<XE; U++) *Dest++ = TrueColors[Src[U&UAnd]];
			Dest1 += Camera->SXStride*4;
			}
		}
	else appError("Invalid color bytes");
	UNGUARD("FGlobalRender::DrawTiledTextureBlock");
	};

/*-----------------------------------------------------------------------------
	Actor drawing (sprites and chunks)
-----------------------------------------------------------------------------*/

//
// Draw a Sprite (sprite or scaled sprite) with no span clipping
//
void FGlobalRender::DrawActorSprite (ICamera *Camera,FSprite *Sprite)
	{
	GUARD;
	AActorDraw *Actor = Sprite->Actor;
	int ModeClass = GEditor ? GEditor->edcamModeClass(GEditor->Mode) : EMC_None;
	//
	if (GEditor && GEditor->Scan.Active) GEditor->Scan.PreScan();
	//
	if ((Actor->DrawType==DT_Sprite) || (Actor->DrawType==DT_Brush) || 
		(Camera->ShowFlags & SHOW_ActorIcons))
		{
		if (Sprite->Texture)
			{
			DrawScaledSprite
				(
				Camera,Sprite->Texture,
				Sprite->ScreenX,Sprite->ScreenY,
				Sprite->Scale * Actor->Texture->USize, Sprite->Scale * Actor->Texture->VSize,
				Sprite->BlitType,0,1,Actor->bSelected
				);
			if (Camera->Camera->IsOrtho())
				{
				if (Camera->ShowFlags & SHOW_ActorRadii)
					{
					if (Actor->bCollideActors)
						{
						DrawCircle(Camera,Actor->Location,Actor->CollisionRadius,ActorArrowColor,1);
						}
					if ((Actor->LightType!=LT_None) && Actor->bSelected)
						{
						DrawCircle(Camera,Actor->Location,Actor->WorldLightRadius(),ScaleBoxColor,1);
						}
					}
				if ((Actor->bDirectional) && ((ModeClass==EMC_Actor) || (Actor->Class==GClasses.Camera)))
					{
					GGfx.ArrowBrush->Location = Actor->Location;
					GGfx.ArrowBrush->Rotation = Actor->DrawRot;
					GGfx.ArrowBrush->Bound[1].Init(); // Don't bound-reject!
					GRender.DrawBrushPolys (Camera,GGfx.ArrowBrush,ActorArrowColor,0,NULL,1,0,Actor->bSelected,0);
					};
				};
			};
		}
	else if ((Actor->DrawType==DT_MeshMap) && (Actor->MeshMap))
		{
		DrawMeshMap
			(
			Camera,Actor->MeshMap,&Actor->Location,&Actor->DrawRot,
			Actor->bSelected,Sprite,0,
			!Actor->bUnlit
			);
		};
	if (GEditor && GEditor->Scan.Active) GEditor->Scan.PostScan(EDSCAN_Actor,Sprite->iActor,0,0,NULL);
	//
	UNGUARD("FGlobalRender::DrawActorSprite");
	};

//
// Draw an actor clipped to a span buffer representing the portion of
// the actor that falls into a particular Bsp leaf.
//
void FGlobalRender::DrawActorChunk (ICamera *Camera,FSprite *Sprite,FSpanBuffer *SpanBuffer)
	{
	GUARD;
	AActor *Actor = &Camera->Level.Actors->Element(Sprite->iActor);
	//
	if (GEditor && GEditor->Scan.Active) GEditor->Scan.PreScan();
	//
	if ((Actor->DrawType==DT_Sprite) || (Actor->DrawType==DT_Brush) ||
		(Camera->ShowFlags & SHOW_ActorIcons))
		{
		if (Sprite->Texture) DrawScaledSprite
			(
			Camera,Sprite->Texture,
			Sprite->ScreenX,Sprite->ScreenY,
			Sprite->Scale * Actor->Texture->USize, Sprite->Scale * Actor->Texture->VSize,
			Sprite->BlitType,SpanBuffer,1,Actor->bSelected
			);
		}
	else if ((Actor->DrawType==DT_MeshMap) && (Actor->MeshMap))
		{
		if ((Sprite->Buffer==NULL) && (Sprite->BlitType!=BT_None)) // Build sprite from mesh
			{
			//
			// Draw mesh sprite to buffer:
			//
			if (!DrawMeshMap
				(
				Camera,Actor->MeshMap,&Actor->Location,&Actor->DrawRot,
				Actor->bSelected,Sprite,1,!Actor->bUnlit
				)) Sprite->BlitType = BT_None; // Actor is 100% occluded
			};
		if (Sprite->BlitType != BT_None) // Blit pre-drawn sprite
			{
			DrawSprite
				(
				Camera,Sprite->Buffer,
				Sprite->USize, Sprite->VSize,
				Sprite->X1, Sprite->Y1, 0,
				Sprite->BlitType,
				SpanBuffer,Actor->bSelected
				);
			};
		};
	if (GEditor && GEditor->Scan.Active) GEditor->Scan.PostScan(EDSCAN_Actor,Sprite->iActor,0,0,NULL);
	UNGUARD("FGlobalRender::DrawActorChunk");
	};

/*-----------------------------------------------------------------------------
	Sprite information
-----------------------------------------------------------------------------*/

//
// Set the Sprite structure corresponding to an actor;  returns 1 if the actor
// is visible, or 0 if the actor is occluded.  If occluded, the Sprite isn't
// visible and should be ignored.
//
int FGlobalRender::SetActorSprite(ICamera *Camera, FSprite *Sprite, INDEX iActor)
	{
	GUARD;
	AActorDraw			*Actor  = &Camera->Level.Actors->Element(iActor);
	UClass				*Class	= Actor->Class;
	FBoundingVolume		Bounds;
	FScreenBounds		ScreenBounds;
	FVector				Location;
	int					MipLevel=0;
	//
	Sprite->BlitType    = (EBlitType)Actor->BlitType;
	Location			= Actor->Location - Camera->Coords.Origin;
	Sprite->Z			= Location | Camera->Coords.ZAxis;
	Sprite->iActor		= iActor;
	Sprite->Actor		= Actor;
	//
	if ((Sprite->Z < -SPRITE_PROJECTION_FORWARD) && !Camera->Camera->IsWire())
		{
		return 0;
		}
	else if ((Actor->DrawType==DT_Sprite) || (Actor->DrawType==DT_Brush) || 
		(Camera->ShowFlags & SHOW_ActorIcons))
		{
		if ((Actor->DrawType!=DT_Brush) || (GEditor))
			{
			if (!Actor->Texture) return 0; // Nothing is available to draw
			if (!GRender.Project (Camera,&Actor->Location,&Sprite->ScreenX,&Sprite->ScreenY,&Sprite->Scale)) return 0; // Occluded
			//
			Sprite->Scale	*= Actor->DrawScale;
			Sprite->Texture  = Actor->Texture;
			Sprite->Buffer   = Actor->Texture->GetData(&MipLevel,Camera->ColorBytes,&Sprite->USize,&Sprite->VSize);
			//
			CalcScaledSpriteExtent
				(
				Camera,
				Sprite->ScreenX,Sprite->ScreenY,
				Sprite->Scale * Actor->Texture->USize,Sprite->Scale * Actor->Texture->VSize,
				&Sprite->X1,&Sprite->Y1,&Sprite->X2,&Sprite->Y2
				);
			};
		return ((Sprite->Y1 < Camera->SYR) && (Sprite->Y2 > 0));
		}
	else if ((Actor->DrawType==DT_MeshMap) && Actor->MeshMap)
		{
		Sprite->Buffer		= NULL; // Must generate upon first draw
		UMeshMap *MeshMap	= Actor->MeshMap;
		UMesh *Mesh			= MeshMap->Mesh;
		Location			= Actor->Location;
		//
		IMesh MeshInfo;
		Mesh->GetInfo(&MeshInfo);
		//
		if (Actor->AnimSeq>=Mesh->NumAnimSeqs)
			{
			Sprite->iFrame=0;
			}
		else
			{
			Sprite->iFrame = 
				MeshInfo.AnimSeqs[Actor->AnimSeq].SeqStartFrame + 
				((int)Actor->AnimBase%(MeshInfo.AnimSeqs[Actor->AnimSeq].SeqNumFrames));
			};
		if (Sprite->iFrame>=Mesh->NumAnimFrames) Sprite->iFrame=0;
		if (Camera->Camera->IsWire())
			{
			if (!GRender.Project (Camera,&Location,&Sprite->ScreenX,&Sprite->ScreenY,NULL)) return 0;
			//
			Sprite->Scale = Actor->DrawScale;
			Sprite->X1	  = Sprite->ScreenX; Sprite->X2 = Sprite->ScreenX;
			Sprite->Y1	  = Sprite->ScreenY; Sprite->Y2 = Sprite->ScreenY;
			}
		else
			{
			Sprite->Scale = Actor->DrawScale;
			//
			// Get pointer to mesh's per-frame bounding Bound:
			//
			Bounds = MeshInfo.Bound [Sprite->iFrame];
			//
			// Transform Bound by scale and origin: 
			//
			Bounds.Min.X = -1.0 + Location.X + MeshMap->Scale.X * (Bounds.Min.X - Mesh->Origin.X) * Actor->DrawScale;
			Bounds.Min.Y = -1.0 + Location.Y + MeshMap->Scale.Y * (Bounds.Min.Y - Mesh->Origin.Y) * Actor->DrawScale;
			Bounds.Min.Z = -1.0 + Location.Z + MeshMap->Scale.Z * (Bounds.Min.Z - Mesh->Origin.Z) * Actor->DrawScale;
			//
			Bounds.Max.X = +1.0 + Location.X + MeshMap->Scale.X * (Bounds.Max.X - Mesh->Origin.X) * Actor->DrawScale;
			Bounds.Max.Y = +1.0 + Location.Y + MeshMap->Scale.Y * (Bounds.Max.Y - Mesh->Origin.Y) * Actor->DrawScale;
			Bounds.Max.Z = +1.0 + Location.Z + MeshMap->Scale.Z * (Bounds.Max.Z - Mesh->Origin.Z) * Actor->DrawScale;
			//
			FCoords BoxCoords = GMath.UnitCoords;
			BoxCoords.DeTransformByRotation(Actor->DrawRot);
			BoxCoords.DeTransformByRotation(Mesh->RotOrigin);
			BoxCoords.Origin = Location;
			//
			if (!GRender.BoundVisible (Camera,&Bounds,NULL,&ScreenBounds,&BoxCoords)) return 0;
			if (ScreenBounds.Valid)
				{
				Sprite->X1 = OurMax (0,ScreenBounds.MinX); Sprite->X2 = OurMin (Camera->SXR,ScreenBounds.MaxX);
				Sprite->Y1 = OurMax (0,ScreenBounds.MinY); Sprite->Y2 = OurMin (Camera->SYR,ScreenBounds.MaxY);
				}
			else
				{
				Sprite->X1 = 0; Sprite->X2 = Camera->SXR;
				Sprite->Y1 = 0; Sprite->Y2 = Camera->SYR;
				};
			};
		return 1;
		}
	else return 0;
	//
	UNGUARD("FGlobalRender::SetActorSprite");
	};


/*-----------------------------------------------------------------------------
	Wireframe view drawing
-----------------------------------------------------------------------------*/

void FGlobalRender::DrawMovingBrushWires(ICamera *Camera)
	{
	GUARD;
	//
	AActor *Actor = &Camera->Level.Actors->Element(0);
	for (int i=0; i<Camera->Level.Actors->Max; i++)
		{
		if (Actor->Class && Actor->IsMovingBrush())
			{
			if (GEditor && GEditor->Scan.Active) GEditor->Scan.PreScan();
			//
			if (!GEditor) Actor->UpdateBrushPosition(&Camera->Level,i,0);
			DrawBrushPolys(Camera,Actor->Brush,MoverColor,0,NULL,Actor->bSelected,Actor->bSelected,Actor->bSelected,0);
			//
			if (GEditor && GEditor->Scan.Active) GEditor->Scan.PostScan(EDSCAN_Actor,i,0,0,NULL);
			};
		Actor++;
		};
	UNGUARD("FGlobalRender::DrawMovingBrushWires");
	};

//
// Just draw an actor, no span occlusion
//
void FGlobalRender::DrawActor(ICamera *Camera,INDEX iActor)
	{
	GUARD;
	FSprite	Sprite;
	if (SetActorSprite (Camera,&Sprite,iActor))
		{
		DrawActorSprite (Camera,&Sprite);
		};
	UNGUARD("FGlobalRender::DrawActor");
	};

//
// Draw all actors in a level without doing any occlusion checking or sorting.
// For wireframe views.
//
void FGlobalRender::DrawLevelActors(ICamera *Camera, INDEX iExclude)
	{
	GUARD;
	UActorList *Actors = Camera->Level.Actors;
	//
	AActor *Actor = &Actors->Element(0);
	for (int i=0; i<Actors->Max; i++)
		{
		if (Actor->Class && (i!=iExclude) &&
			!((Camera->Level.State==LEVEL_UpPlay)?Actor->bHidden:Actor->bHiddenEd))
			{
			//
			// If this actor is an event source, draw event lines connecting it to
			// all corresponding event sinks:
			//
			if ((Actor->EventName!=NAME_NONE) && GEditor && 
				(GEditor->edcamModeClass(GEditor->Mode)==EMC_Actor))
				{
				AActor *OtherActor = &Actors->Element(0);
				for (int j=0; j<Actors->Max; j++)
					{
					if (OtherActor->Class &&
						(OtherActor->Name==Actor->EventName) &&
						!((Camera->Level.State==LEVEL_UpPlay)?Actor->bHidden:Actor->bHiddenEd))
						{
						GRender.Draw3DLine
							(
							Camera,
							&Actor->Location, 
							&OtherActor->Location,
							1,ActorArrowColor,0,1
							);
						};
					OtherActor++;
					};
				};
			//
			// Draw this actor:
			//
			FSprite Sprite;
			if (SetActorSprite (Camera,&Sprite,i)) DrawActorSprite (Camera,&Sprite);
			};
		Actor++;
		};
	UNGUARD("FGlobalRender::DrawLevelActors");
	};

/*------------------------------------------------------------------------------
	Mesh sprites
------------------------------------------------------------------------------*/

class FMeshTriSort // Structure used by rendDrawMeshMap for sorting triangles
	{
	public:
	FLOAT	MinZ;
	int		Index;
	int		iFirstPoint;
	int		NumPts;
	};

class FMeshPivotRec
	{
	public:
	int				Num;
	FMeshTriSort	*TriList;
	FMeshPivotRec 	*Next;
	};

int FGlobalRender::DrawMeshMap
	(
	ICamera *Camera, UMeshMap *MeshMap,
	FVector *Location, FRotation *Rotation, int Highlight, FSprite *Sprite,
	int DrawToBuffer, int DrawLit
	)
	{
	GUARD;
	ICamera				TempCamera;
	IMeshMap			MeshMapInfo;
	FMeshTriangle		*Triangle,*OtherTriangle;
	FMeshVertex			*MeshVertex;
	FVector				TempVector,TempNormal,VertexNormal,Side1,Side2,*FVectPool;
	FMeshPivotRec		*PivotRecs,*CurPivotRec,*TopPivotRec,*FrontRec,*BackRec,*MiddleRec;
	FPoly				EdPoly;
	FCoords				Coords;
	FTransTex			*Pts,*PtsPool,*P;
	FTransform			*Verts,*Vert,*V1,*V2,*V3;
	FMeshTriSort 		*TriPool,*TriPtr,*TriSort,*TriSortPtr,*TriTop;
	FMeshVertLinkIndex	*VertLinkIndex;
	FRasterTexPoly		*RasterTexPoly;
	FRasterTexSetup		RasterTexSetup;
	FLOAT				ThisMinZ;
	FLOAT				MinX,MinY,MaxX,MaxY;
	INT					*VertexShade;
	WORD				Color,*VertLink;
	BYTE				Outcode = FVF_OutReject;
	int					*SortOrder;
	int					i,j,k,n,Pivot,FrameOffset,NumPts,VisibleTriangles;
	int					Temp=0,TotalPts,iVertex,Result=0;
	//
	if (Camera->ColorBytes!=1) return 0;
	if (!MeshMap) return 0;
	//
	MeshMap->Lock(&MeshMapInfo);
	//
	// Grab all memory needed for drawing the sprite:
	//
	SortOrder	  = (int			*)GMem.Get(2048 * sizeof (int));
	Verts 		  = (FTransform		*)GMem.Get(MeshMapInfo.NumVertices * sizeof  (FTransform));
	VertexShade	  = (INT			*)GMem.Get(MeshMapInfo.NumVertices * sizeof  (INT));
	TriSort		  = (FMeshTriSort	*)GMem.Get(MeshMapInfo.NumTriangles * sizeof (FMeshTriSort));
	PivotRecs	  = (FMeshPivotRec	*)GMem.Get(4096 * sizeof (FMeshPivotRec));
	TriPool		  = (FMeshTriSort	*)GMem.Get(64 * MeshMapInfo.NumTriangles * sizeof (FMeshTriSort)); // Way overestimate
	Pts 		  = (FTransTex		*)GMem.Get(32 * sizeof (FTransTex));
	RasterTexPoly = (FRasterTexPoly	*)GMem.Get(sizeof(FRasterTexPoly)+FGlobalRaster::MAX_RASTER_LINES*sizeof(FRasterTexLine));
	PtsPool		  = (FTransTex		*)GMem.Get(0); // Will be expanded	
	//
	FVector Scale = MeshMapInfo.Scale * Sprite->Scale;
	//
	if (Camera->Camera->IsWire() || Camera->Camera->IsOrtho()) // Wireframe view
		{
		Coords      = GMath.UnitCoords;
		Coords.TransformByRotation (*Rotation);
		Coords.TransformByRotation (MeshMapInfo.RotOrigin);
		//
		Triangle	= &MeshMapInfo.Triangles[0]; 
		FrameOffset	= Sprite->iFrame * MeshMapInfo.NumVertices;
		//
		if (Highlight)	Color = ActorHiWireColor;
		else			Color = ActorWireColor;
		//
		FVectPool = (FVector *)GMem.Get(MeshMapInfo.NumVertices*sizeof(FVector));
		for (i=0; i<MeshMapInfo.NumVertices; i++) // Transform all points
			{
			MeshVertex		= &MeshMapInfo.Vertex [FrameOffset + i];
			//
			FVectPool[i].X	= Scale.X * ((FLOAT)MeshVertex->X - MeshMapInfo.Origin.X);
			FVectPool[i].Y	= Scale.Y * ((FLOAT)MeshVertex->Y - MeshMapInfo.Origin.Y);
			FVectPool[i].Z	= Scale.Z * ((FLOAT)MeshVertex->Z - MeshMapInfo.Origin.Z);
			//
			FVectPool[i].TransformVector(Coords);	 
			FVectPool[i] += *Location;
			};
		for (i=0; i<MeshMapInfo.NumTriangles; i++) // Build all EdPolys
			{
			EdPoly.Init();
			EdPoly.NumVertices = 3;
			for (j=0; j<3; j++) EdPoly.Vertex[j] = FVectPool[Triangle->iVertex[j]];
			if (EdPoly.Finalize(1)==0)
				{
				if (Triangle->Type==MT_Masked) EdPoly.PolyFlags |= PF_Masked | PF_NotSolid;
				else if (Triangle->Type==MT_Transparent) EdPoly.PolyFlags |= PF_Transparent | PF_NotSolid;
				//
				GRender.DrawFPoly(Camera,&EdPoly,Color,1,0);
				};
			Triangle++;
			};
		goto Out;
		};
	//
	// 3D textured view: Build coordinate system to get creature into screenspace:
	//
	Coords = Camera->Coords;
	Coords.TransformByRotation	(*Rotation);
	Coords.TransformByRotation	(MeshMapInfo.RotOrigin);
	TempVector = *Location - Camera->Coords.Origin;
	TempVector.TransformVector (Camera->Coords);
	//
	// Transform all points into screenspace and compute outcodes:
	//
	FrameOffset	= Sprite->iFrame * MeshMapInfo.NumVertices;
	MeshVertex  = &MeshMapInfo.Vertex	[FrameOffset];
	Vert        = &Verts 				[0];
	//
	for (i=0; i<MeshMapInfo.NumVertices; i++)
		{
		Vert->X = Scale.X * ((FLOAT)MeshVertex->X - MeshMapInfo.Origin.X);
		Vert->Y = Scale.Y * ((FLOAT)MeshVertex->Y - MeshMapInfo.Origin.Y);
		Vert->Z = Scale.Z * ((FLOAT)MeshVertex->Z - MeshMapInfo.Origin.Z);
		//
		Vert->TransformVector(Coords);
		*Vert += TempVector;
		//
		Vert->ComputeOutcode(Camera);
		Outcode &= Vert->Flags;
		//
		MeshVertex++;
		Vert++;
		};
	if (Outcode) goto Out; // Everything is rejected
	//
	// Set up list for triangle sorting, adding all non-backfaced triangles:
	//
	Triangle			= &MeshMapInfo.Triangles [0]; 		
	TriTop   			= &TriPool               [0];
	VisibleTriangles	= 0;
	for (i=0; i<MeshMapInfo.NumTriangles; i++)
		{
		V1 = &Verts [Triangle->iVertex[0]];
		V2 = &Verts [Triangle->iVertex[1]];
		V3 = &Verts [Triangle->iVertex[2]];
		//
		// Try to outcode-reject the triangle:
		//
		if (!(V1->Flags & V2->Flags & V3->Flags))
			{
			//
			// See if this triangle is either two-sided or front-faced:
			//
			if ((Triangle->Type==MT_Masked)			||
				(Triangle->Type==MT_Transparent)    ||
				((*V1 | (*V2 ^ *V3))>0.0))
				{
				TriTop->Index = i;
				TriTop->MinZ =
					Verts [Triangle->iVertex[0]].Z +
					Verts [Triangle->iVertex[1]].Z +
					Verts [Triangle->iVertex[2]].Z;
				VisibleTriangles ++;
				TriTop ++;
				};
			};
		Triangle++;
		};
	if (VisibleTriangles==0) goto Out;
	//
	// If creature is small on screen, don't bother sorting:
	//
	if (((Sprite->X2-Sprite->X1)<NOSORT_SIZE) || ((Sprite->Y2-Sprite->Y1)<NOSORT_SIZE))
		{
		//
		// Far away, don't bother sorting (unnoticeable visually, but a huge speedup!)
		//
		TriSort = &TriPool[0];
		}
	else // Near, must sort
		{
		//
		// Pivot sort triangles in back-to-front order
		//
		PivotRecs[0].Num		= VisibleTriangles;	// Set first record to pool with all triangles
		PivotRecs[0].TriList	= &TriPool   [0];
		PivotRecs[0].Next		= NULL;
		CurPivotRec				= &PivotRecs [0];	// Current node in linked list
		TopPivotRec				= &PivotRecs [1];	// Top record, used for adding new records
		TriSortPtr				= &TriSort   [0];	// Destination of final sorted triangles
		//
		while (CurPivotRec != NULL)
			{
			if (CurPivotRec->Num > 1)
				{
				//
				// Pick a new pivot point:
				//
				Pivot      			= CurPivotRec->Num / 2; // Pick pivot halfway through list
				//
				//	Create new back and front pools:
				//
				BackRec				= TopPivotRec++; // Back pool
				BackRec->Num 		= 0;
				BackRec->TriList	= TriTop;
				TriTop 			   += CurPivotRec->Num;
				//
				FrontRec			= TopPivotRec++; // Front pool
				FrontRec->Num 		= 0;
				FrontRec->TriList	= TriTop;
				TriTop 			   += CurPivotRec->Num;
				//
				MiddleRec			= TopPivotRec++; // Middle pool (pivot only)
				MiddleRec->Num 		= 1;
				MiddleRec->TriList	= &CurPivotRec->TriList [Pivot];
				//
				ThisMinZ = CurPivotRec->TriList [Pivot].MinZ; // Remember pivot's Z location
				//
				// Update links so that we do the back immediately after this, followed
				// by the front, followed by our next link:
				//
				FrontRec->Next    = CurPivotRec->Next;
				MiddleRec->Next   = FrontRec;
				BackRec->Next     = MiddleRec;
				//
				// Go through all triangles in this pool and add to either front or back:
				//
				TriPtr 	= CurPivotRec->TriList;
				//
				for (i=0; i<CurPivotRec->Num; i++)
					{
					if (TriPtr->MinZ < ThisMinZ)	FrontRec->TriList [FrontRec->Num++] = *TriPtr;
					else if (i != Pivot)			BackRec->TriList  [BackRec->Num++ ] = *TriPtr;
					else							{}; // Ignore the pivot itself
					TriPtr++;
					};
				CurPivotRec = BackRec; // Go to next entry in list
				}
			else if (CurPivotRec->Num==1) // PivotRec->Num == 1 (should also do special-case 2, 3)
				{
				*(TriSortPtr++) = CurPivotRec->TriList [0];
				Temp++;
				//
				CurPivotRec = CurPivotRec->Next; // Go to next entry in list
				}
			else // Empty record (this can happen when a pivot happens to be picked with no front or back)
				{
				CurPivotRec = CurPivotRec->Next; // Go to next entry in list
				};
			};
		};
	//
	// Build list of all incident lights on the creature.
	// Considers only the entire creature and does not
	// differentiate between individual creature polys.
	//
	GLightManager->SetupForActor(Camera,Sprite->iActor);
	//
	for (i=0; i<MeshMapInfo.NumVertices; i++) VertexShade[i] = 0; // Tag as uncomputed
	//
	TriPtr    = &TriSort     [0];
	for (i=0; i<VisibleTriangles; i++)
		{
		Triangle = &MeshMapInfo.Triangles[TriPtr->Index];
		//
		for (j=0; j<3; j++)
			{
			iVertex = Triangle->iVertex[j];
			if (VertexShade[iVertex]==0)
				{
				FTransform *P = &Verts[iVertex];
				if (!P->Flags)
					{
					P->iTransform=0; // Transformation is done, don't generate again
					FLOAT Factor = Camera->ProjZ / P->Z;
					P->ScreenX  = P->X * Factor + Camera->FSXR15;
					P->ScreenY  = P->Y * Factor + Camera->FSYR15;
					}
				else P->iTransform=INDEX_NONE; // Must transform after clipping
				//
				VertexNormal 	= GMath.ZeroVector;
				//
				VertLinkIndex	= &MeshMapInfo.VertLinkIndex [iVertex];
				VertLink     	= &MeshMapInfo.VertLinks     [VertLinkIndex->TriangleListOffset];
				n            	= VertLinkIndex->NumVertTriangles;
				//
				for (k=0; k<n; k++)
					{
					OtherTriangle = &MeshMapInfo.Triangles [*VertLink++];
					//
					Side1      = Verts[OtherTriangle->iVertex[1]] - Verts[OtherTriangle->iVertex[0]];
					Side2      = Verts[OtherTriangle->iVertex[2]] - Verts[OtherTriangle->iVertex[0]];
					TempNormal = Side2 ^ Side1;
					TempNormal.Normalize();
					//
					VertexNormal += TempNormal;
					};
				VertexNormal.Normalize();
				//
				//	Compute effect of each lightsource on this vertex:
				//
				FTexLattice Lattice;
				if( DrawLit )
					{
					// Temporarily disabled:
					//GRender.LightList.CalcRectSample(1,0,*P,VertexNormal,Lattice,(Camera->RendMap==REN_DynLight)?2:0);				
					Lattice.G = 32*256.0 * (1.001 - OurMin(1.0,0.00035*P->Z));
					}
				else
					{
					Lattice.G = 32*256.0 * (1.001 - OurMin(1.0,0.00035*P->Z));
					};
				VertexShade[iVertex] = OurMin(256*61,Lattice.G + (Sprite->Actor->InherentBrightness<<6));
				};
			};
		TriPtr++;
		};
	//
	// Clip the triangles:
	//
	TriPtr   = &TriSort [0];
	TotalPts = 0;
	for (i=0; i<VisibleTriangles; i++)
		{
		Triangle = &MeshMapInfo.Triangles[TriPtr->Index];
		//
		P = &Pts [0];
		for (j=0; j<3; j++)
			{
			*(FTransform *)P = Verts[Triangle->iVertex[j]];
			//
			P->U = Triangle->Tex[j].U * 65536.0;
			P->V = Triangle->Tex[j].V * 65536.0;
			P->G = VertexShade [Triangle->iVertex[j]];
			//
			P++;
			};
		NumPts = GRender.ClipTexPoints(Camera,Pts,&PtsPool[TotalPts],3);
		//
		TriPtr->iFirstPoint  = TotalPts;
		TriPtr->NumPts       = NumPts;
		//
		TotalPts             += NumPts;
		TriPtr				 ++;
		};
	GMem.Get(TotalPts * sizeof(FTransTex)); // Reserve memory for PtsPool
	//
	// Find min/max and reserve memory for sprite:
	//
	if (DrawToBuffer)
		{
		P    = &PtsPool[0];
		MinX = P->ScreenX; MaxX = P->ScreenX;
		MinY = P->ScreenY; MaxY = P->ScreenY;
		P    ++;
		//
		for (i=1; i<TotalPts; i++)
			{
			if (P->ScreenX < MinX) MinX = P->ScreenX;
			if (P->ScreenX > MaxX) MaxX = P->ScreenX;
			if (P->ScreenY < MinY) MinY = P->ScreenY;
			if (P->ScreenY > MaxY) MaxY = P->ScreenY;
			//
			P++;
			};
		//
		// Fake camera parameters:
		//
		TempCamera		 = *Camera;
		//
		Sprite->X1		 = (ftoi(MinX)    ) & ~3;
		Sprite->Y1		 = (ftoi(MinY)    ) & ~3;
		Sprite->X2       = (ftoi(MaxX) + 4) & ~3;
		Sprite->Y2		 = (ftoi(MaxY) + 4) & ~3;
		//
		Sprite->USize	 = (Sprite->X2 - Sprite->X1);
		Sprite->VSize	 = (Sprite->Y2 - Sprite->Y1);
		//
		Sprite->Buffer   = (BYTE *)GDynMem.Get(Sprite->USize * Sprite->VSize * Camera->ColorBytes);
		//
		Camera->Screen	 = Sprite->Buffer - (Sprite->X1 + Sprite->Y1 * Sprite->USize)*Camera->ColorBytes;
		Camera->SXR      = Sprite->USize;
		Camera->SXStride = Sprite->USize;
		//
		mymemset (Sprite->Buffer,0,Sprite->USize * Sprite->VSize * Camera->ColorBytes);
		};
	//
	// Draw all marked triangles:
	//
	TriPtr = &TriSort [0];
	for (i=0; i<VisibleTriangles; i++)
		{
		Triangle = &MeshMapInfo.Triangles[TriPtr->Index];
		NumPts   = TriPtr->NumPts;
		//
		if (NumPts>0)
			{
			P = &PtsPool [TriPtr->iFirstPoint];
			//
			RasterTexSetup.Setup	(Camera,P,NumPts,&GMem);
			RasterTexSetup.Generate	(RasterTexPoly);
			if ((Triangle->Type==MT_Masked)||(Triangle->Type==MT_Transparent))
				{
				RasterTexPoly->ForceForwardFace();
				};
			if ((Camera->RendMap!=REN_Polys)&&(Camera->RendMap!=REN_PolyCuts)&&
				(Camera->RendMap!=REN_Zones)&&(Triangle->Type!=MT_Flat)&&(Camera->RendMap!=REN_Wire))
				{
				GBlit.Setup
					(
					Camera,
					MeshMapInfo.Textures[Triangle->TextureNum],
					(Triangle->Type==MT_Masked) ? PF_Masked : 0,
					PF_Masked
					);
				RasterTexPoly->Draw(Camera);
				}
			else if (Triangle->Type==MT_Flat)
				{
				RasterTexPoly->DrawFlat(Camera,BlackColor,BlackColor);
				}
			else 
				{
				RasterTexPoly->DrawFlat(Camera,ActorFillColor,ActorWireColor);
				};
			RasterTexSetup.Release();
			};
		TriPtr++;
		};
	if (DrawToBuffer) *Camera = TempCamera;
	Result=1;
	//
	Out:
	STAT(GStat.MeshMapsDrawn++);
	MeshMap->Unlock	(&MeshMapInfo);
	GMem.Release	(SortOrder); // Release first element allocated (releases all later memory)
	//
	return Result;
	//
	UNGUARD_BEGIN
	UNGUARD_MSGF("FGlobalRender::DrawMeshMap(%s)",MeshMap->Name);
	UNGUARD_END
	};

/*------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------*/
