/*=============================================================================
	UnEdCnst.cpp: Functions related to movement constraints

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	What's happening: When the Visual Basic level editor is being used,
	this code exchanges messages with Visual Basic.  This lets Visual Basic
	affect the world, and it gives us a way of sending world information back
	to Visual Basic.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/
	 
#include "Unreal.h"

//
// To do: Convert to a C++ class.
// Genericise to allow for other types of constraints later on.
//

/*------------------------------------------------------------------------------
	Movement constraints
------------------------------------------------------------------------------*/

//
// Force a location and rotation corresponding to a brush to be constrained within
// a level.  Affects *Location and *Rotation, not actual brush properties.
// Handles grid alignment and vertex snapping.
//
// Returns 1 if snapped to a vertex.
// Returns -1 if brush location is invalid, i.e. partially off map. Not implemented.
//
int FEditor::constraintApply(IModel *LevelModelInfo, IModel *BrushInfo,
	FVector *Location, FRotation *Rotation,FConstraints *Constraints)
	{
	FVector  SourcePoint;
	FVector	 DestPoint;
	INDEX	 Temp;
	int		 Snapped	= 0;
	//
	GUARD;
	if ((Constraints->RotGridEnabled) && Rotation)
		{
		*Rotation = Rotation->GridSnap(Constraints->RotGrid);
		};
	if ((LevelModelInfo != NULL) && (BrushInfo != NULL) && (Constraints->SnapVertex))
		{
		SourcePoint = *Location + BrushInfo->PostPivot;
		if (LevelModelInfo->FindNearestVertex (&SourcePoint,&DestPoint,Constraints->SnapDist,&Temp) >= 0.0)
			{
			*Location = DestPoint - BrushInfo->PostPivot;
			Snapped   = 1;
			};
		};
	if ((Constraints->GridEnabled) && (!Snapped))
		{
		*Location = Location->GridSnap(Constraints->Grid);
		};
	return Snapped;
	UNGUARD("FEditor::constraintApply");
	};

void constrainSimplePoint(FVector *Location,FConstraints *Constraints)
	{
	GUARD;
	if (Constraints->GridEnabled) *Location = Location->GridSnap(Constraints->Grid);
	UNGUARD("constrainSimplePoint");
	};

//
// Finish snapping a brush
//
void FEditor::constraintFinishSnap(ILevel *LevelInfo,UModel *Brush)
	{
	GUARD;
	IModel BrushInfo;
	//
	if (Brush->ModelFlags & MF_ShouldSnap) // Snap it
		{
		Brush->Lock(&BrushInfo,LOCK_Trans);
		constraintApply (&LevelInfo->ModelInfo,&BrushInfo,&BrushInfo.Location,&BrushInfo.Rotation,&Constraints);
		Brush->Unlock(&BrushInfo);
		Brush->ModelFlags &= ~MF_ShouldSnap;
		};
	UNGUARD("FEditor::constraintFinishSnap");
	};

//
// Finish snapping all brushes in a level
//
void FEditor::constraintFinishAllSnaps(ULevel *Level)
	{
	GUARD;
	//
	IModel		BrushInfo;
	ILevel		LevelInfo;
	UModel			*Brush;
	int 			i,n;
	//
	Level->Lock (&LevelInfo,LOCK_Trans);
	//
	//	Constrain active brush:
	// 
	Brush = LevelInfo.BrushArray->Element(0);
	//
	Brush->Lock   		(&BrushInfo,LOCK_Trans);
	constraintApply 	(&LevelInfo.ModelInfo,&BrushInfo,&BrushInfo.Location,&BrushInfo.Rotation,&Constraints);
	Brush->Unlock 		(&BrushInfo);
	//
	// Constrain level brushes:
	//
	if (MapEdit)
		{
		n = LevelInfo.BrushArray->Num;
		for (i=1; i<n; i++)
			{
			constraintFinishSnap(&LevelInfo,LevelInfo.BrushArray->Element(i));
			};
		};
	Level->Unlock(&LevelInfo);
	UNGUARD("FEditor::constraintFinishAllSnaps");
	};

void FEditor::constraintInit(FConstraints *Const)
	{
	GUARD;
	mymemset (Const,0,sizeof (FConstraints));
	//
	Const->Grid					= GMath.ZeroVector;
	Const->GridBase				= GMath.ZeroVector;
	Const->RotGrid				= GMath.ZeroRotation;
	Const->SnapDist				= 0.0;
	//
	Const->Flags				= 0;
	Const->GridEnabled			= 0;
	Const->RotGridEnabled		= 0;
	Const->SnapVertex			= 1;
	UNGUARD("FEditor::constraintInit");
	};

/*------------------------------------------------------------------------------
	The end
------------------------------------------------------------------------------*/
