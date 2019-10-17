/*=============================================================================
	UnEdCam.cpp: Unreal editor camera movement/selection functions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	Globals
-----------------------------------------------------------------------------*/

//
// Flags for cameraClick, stored in Camera header's ClickFlags:
//
enum ECameraClick
	{
	CF_MOVE_BRUSH	= 1,	// Set if the brush has been moved since first click
	CF_MOVE_ACTOR	= 2,	// Set if the actors have been moved since first click
	CF_MOVE_TEXTURE = 4,	// Set if textures have been adjusted since first click
	CF_MOVE_ALL     = (CF_MOVE_BRUSH | CF_MOVE_ACTOR | CF_MOVE_TEXTURE),
	};

//
// Internal declarations:
//
void NoteBrushMovement 		(ICamera *CameraInfo);
void NoteTextureMovement	(ICamera *CameraInfo);
void NoteActorMovement 		(ICamera *CameraInfo);
void constrainSimplePoint	(FVector *Location,FConstraints *Constraints);
int  GetMeshMapFrame		(UResource *MeshMap, int AnimSeq, int AnimOfs);

//
// Global variables (not beautiful, but they do the job)
//
int		GLastScroll		= 0;
int		GFixPanU=0,GFixPanV=0;
int		GFixScale=0;

/*-----------------------------------------------------------------------------
   Selection callbacks
-----------------------------------------------------------------------------*/

//
// Callback for selecting a polygon, transaction-tracked.
//
void SelectPolyFunc (IModel *ModelInfo, INDEX iSurf)
	{
	FBspSurf *Poly = &ModelInfo->BspSurfs[iSurf];
	//
	if (!(Poly->PolyFlags & PF_Selected))
		{
		GTrans->NoteBspSurf (ModelInfo,iSurf,0);
		Poly->PolyFlags |= (PF_Selected);
		};
	};

//
// Callback for deselecting a polygon, transaction-tracked.
//
void DeselectPolyFunc (IModel *ModelInfo, INDEX iSurf)
	{
	FBspSurf *Poly = &ModelInfo->BspSurfs[iSurf];
	//
	if (Poly->PolyFlags & PF_Selected)
		{
		GTrans->NoteBspSurf (ModelInfo,iSurf,0);
		Poly->PolyFlags &= (~PF_Selected);
		};
	};

/*-----------------------------------------------------------------------------
   Routines to calculate various types of movement & rotation
-----------------------------------------------------------------------------*/

//
// Freeform orthogonal movement and rotation.
//
// Limitation: Doesn't handle rotation properly in the XZ and YZ cameras.  Rotation
// there should rotate about the respective axis, but this isn't feasible since brushes
// just have P-Y-R rotations.  Can live with this.
//
void CalcFreeOrthoMoveRot (ICamera *CameraInfo,SWORD MouseX,SWORD MouseY,BYTE Buttons,
	FVector *Delta,FFloatRotation *DeltaRot)
	{
	GUARD;
	FLOAT *OrthoAxis1,*OrthoAxis2,Axis2Sign,Axis1Sign,*OrthoAngle,AngleSign;
	//
	if (CameraInfo->RendMap==REN_OrthXY)
		{
		OrthoAxis1 = &Delta->X;  			Axis1Sign=1.0;
		OrthoAxis2 = &Delta->Y;  			Axis2Sign=1.0;
		OrthoAngle = &DeltaRot->Yaw; 		AngleSign=1.0;
		}
	else if (CameraInfo->RendMap==REN_OrthXZ)
		{
		OrthoAxis1 = &Delta->X; 			Axis1Sign=1.0;
		OrthoAxis2 = &Delta->Z; 			Axis2Sign=-1.0;
		OrthoAngle = &DeltaRot->Pitch; 		AngleSign=1.0; // Should rotate about Y axis
		}
	else if (CameraInfo->RendMap==REN_OrthYZ)
		{
		OrthoAxis1 = &Delta->Y; 			Axis1Sign=1;
		OrthoAxis2 = &Delta->Z; 			Axis2Sign=-1;
		OrthoAngle = &DeltaRot->Roll; 		AngleSign=1.0; // Should rotate about X axis
		}
	else appError("Invalid rendering mode");
	//
	// Special orthogonal view movement controls:
	//
	if (Buttons == BUT_LEFT) // Left button: Move up/down/left/right
		{
		*OrthoAxis1 = CameraInfo->OrthoZoom/30000.0*(FLOAT)MouseX;
		if      ((MouseX<0)&&(*OrthoAxis1==0)) *OrthoAxis1=-Axis1Sign; // Always move when mouse moves
		else if ((MouseX>0)&&(*OrthoAxis1==0)) *OrthoAxis1=Axis1Sign;
		//
		*OrthoAxis2 = Axis2Sign*CameraInfo->OrthoZoom/30000.0*(FLOAT)MouseY;
		if      ((MouseY<0)&&(*OrthoAxis2==0)) *OrthoAxis2=-Axis2Sign;
		else if ((MouseY>0)&&(*OrthoAxis2==0)) *OrthoAxis2=Axis2Sign;
		}
	else if (Buttons == (BUT_LEFT | BUT_RIGHT)) // Both buttons: Zoom in/out
		{
		CameraInfo->OrthoZoom -= CameraInfo->OrthoZoom/200.0 * (FLOAT)MouseY;
		//
		if (CameraInfo->OrthoZoom<500.0)    		CameraInfo->OrthoZoom = 500.0;
		else if (CameraInfo->OrthoZoom>2000000.0) 	CameraInfo->OrthoZoom = 2000000.0;
		}
	else if (Buttons == BUT_RIGHT) // Right button: Rotate
		{
		if (OrthoAngle!=NULL) *OrthoAngle = -AngleSign*8.0*(FLOAT)MouseX;
		};
	UNGUARD("CalcFreeOrthoMoveRot");
	};

//
// Axial orthogonal movement.
//
// Limitation: Doesn't handle rotation properly in the XZ and YZ cameras.  Rotation
// there should rotate about the respective axis, but this isn't feasible since brushes
// just have P-Y-R rotations.  Can live with this.
//
void CalcAxialOrthoMove (ICamera *CameraInfo, SWORD MouseX,SWORD MouseY,BYTE Buttons,
	FVector *Delta,FFloatRotation *DeltaRot)
	{
	FLOAT       *OrthoAxis1,*OrthoAxis2,Axis2Sign,Axis1Sign,*OrthoAngle,AngleSign;
	//
	GUARD;
	if (CameraInfo->RendMap==REN_OrthXY)
		{
		OrthoAxis1 = &Delta->X;  		Axis1Sign=1.0;
		OrthoAxis2 = &Delta->Y;  		Axis2Sign=1.0;
		OrthoAngle = &DeltaRot->Yaw; 	AngleSign=1.0;
		}
	else if (CameraInfo->RendMap==REN_OrthXZ)
		{
		OrthoAxis1 = &Delta->X; 		Axis1Sign=1.0;
		OrthoAxis2 = &Delta->Z; 		Axis2Sign=-1.0;
		OrthoAngle = &DeltaRot->Pitch; 	AngleSign=1.0; // Should rotate about Y axis
		}
	else if (CameraInfo->RendMap==REN_OrthYZ)
		{
		OrthoAxis1 = &Delta->Y; 		Axis1Sign=1;
		OrthoAxis2 = &Delta->Z; 		Axis2Sign=-1;
		OrthoAngle = &DeltaRot->Roll; 	AngleSign=1.0; // Should rotate about X axis
		}
	else appError("Invalid rendering mode");
	//
	// Special orthogonal view movement controls:
	//
	if (Buttons & (BUT_LEFT | BUT_RIGHT)) // Left, right, or both are pressed
		{
		if (Buttons & BUT_LEFT) // Left button: Screen's X-Axis
			{
      		*OrthoAxis1 = CameraInfo->OrthoZoom/30000.0*(FLOAT)MouseX;
      		if      ((MouseX<0)&&(*OrthoAxis1==0)) *OrthoAxis1=-Axis1Sign; // Always move when mouse moves
      		else if ((MouseX>0)&&(*OrthoAxis1==0)) *OrthoAxis1=Axis1Sign;
			};
		if (Buttons & BUT_RIGHT) // Right button: Screen's Y-Axis
			{
      		*OrthoAxis2 = Axis2Sign*CameraInfo->OrthoZoom/30000.0*(FLOAT)MouseY;
      		if      ((MouseY<0)&&(*OrthoAxis2==0)) *OrthoAxis2=-Axis2Sign;
      		else if ((MouseY>0)&&(*OrthoAxis2==0)) *OrthoAxis2=Axis2Sign;
			};
		}
	else if (Buttons == (BUT_MIDDLE)) // Middle button: Zoom in/out
		{
		CameraInfo->OrthoZoom -= CameraInfo->OrthoZoom/200.0 * (FLOAT)MouseY;
		//
		if (CameraInfo->OrthoZoom<500.0)    		CameraInfo->OrthoZoom = 500.0;
		else if (CameraInfo->OrthoZoom>2000000.0) 	CameraInfo->OrthoZoom = 2000000.0;
		};
	UNGUARD("CalcAxialMoveRot");
	};

//
// Freeform perspective movement and rotation.
//
void CalcFreePerspMoveRot (ICamera *CameraInfo, SWORD MouseX,SWORD MouseY,BYTE Buttons,
	FVector *Delta,FFloatRotation *DeltaRot)
	{
	GUARD;
	AActor *Actor = CameraInfo->Actor;
	//
	if (Buttons == BUT_LEFT) // Left button: move ahead and yaw
		{
		Delta->X=(FLOAT)(-MouseY) * GMath.CosTab(Actor->ViewRot.Yaw);
		Delta->Y=(FLOAT)(-MouseY) * GMath.SinTab(Actor->ViewRot.Yaw);
		//
		DeltaRot->Yaw = (64.0 * (FLOAT)MouseX)/20.0;
		}
	else if (Buttons == (BUT_LEFT | BUT_RIGHT)) // Both buttons: Move up and left/right
		{
		Delta->X += (FLOAT)(MouseX) * -GMath.SinTab(Actor->ViewRot.Yaw);
		Delta->Y += (FLOAT)(MouseX) *  GMath.CosTab(Actor->ViewRot.Yaw);
		//
		Delta->Z += (FLOAT)-MouseY;
		}
	else if (Buttons == BUT_RIGHT) // Right button: Pitch and yaw
		{
		DeltaRot->Pitch=(64.0/12.0)*(FLOAT)-MouseY;
		//
		// Disabled Roll -- not useful for a viewing camera (replaced with second yaw):
		//     DeltaRoll=MouseX/10;
		//
		DeltaRot->Yaw=(64.0/20.0) * (FLOAT)MouseX;
		};
	UNGUARD("CalcFreePerspMoveRot");
	};

//
// Axial perspective movement.
//
void CalcAxialPerspMove (ICamera *CameraInfo, SWORD MouseX,SWORD MouseY,BYTE Buttons,
	FVector *Delta,FFloatRotation *DeltaRot)
	{
	GUARD;
	//
	// Do single-axis movement:
	//
	if (Buttons == BUT_LEFT)					Delta->X = +MouseX;
	else if (Buttons == BUT_RIGHT)				Delta->Y = +MouseX;
	else if (Buttons == (BUT_LEFT | BUT_RIGHT)) Delta->Z = -MouseY;
	UNGUARD("CalcAxialPerspMove");
	};

//
// See if a scale is within acceptable bounds:
//
int ScaleIsWithinBounds (FVector *V, FLOAT Min, FLOAT Max)
	{
	FLOAT Temp;
	//
	GUARD;
	//
	Temp = OurAbs (V->X);
	if ((Temp<Min) || (Temp>Max)) return 0;
	//
	Temp = OurAbs (V->Y);
	if ((Temp<Min) || (Temp>Max)) return 0;
	//
	Temp = OurAbs (V->Z);
	if ((Temp<Min) || (Temp>Max)) return 0;
	//
	return 1;
	UNGUARD("ScaleIsWithinBounds");
	};

/*-----------------------------------------------------------------------------
   Camera movement computation
-----------------------------------------------------------------------------*/

//
// Move and rotate camera freely.
//
void CameraMoveRot (ICamera *CameraInfo,FVector *Delta,FFloatRotation *DeltaRot)
	{
	GUARD;
	CameraInfo->Actor->ViewRot.AddBounded   (DeltaRot->Pitch,DeltaRot->Yaw,DeltaRot->Roll);
	CameraInfo->Actor->Location.MoveBounded (*Delta);
	UNGUARD("CameraMoveRot");
	};

//
// Bound a vector to a cube.
//
void FBound (FVector *V, FLOAT Max)
	{
	if		(V->X > Max) 	V->X = Max;
	else if (V->X < -Max)	V->X = -Max;
	//
	if 		(V->Y > Max) 	V->Y = Max;
	else if (V->Y < -Max)	V->Y = -Max;
	//
	if 		(V->Z > Max) 	V->Z = Max;
	else if (V->Z < -Max)	V->Z = -Max;
	};

//
// Move and rotate camera using gravity and collision where appropriate.
//
void CameraMoveRotWithPhysics (ICamera *CameraInfo,FVector *Delta,FFloatRotation *DeltaRot)
	{
	GUARD;
	//
	// Update rotation
	//
	CameraInfo->Actor->ViewRot.AddBounded (4.0*DeltaRot->Pitch,4.0*DeltaRot->Yaw,4.0*DeltaRot->Roll);
	//
	//	Bound velocity and add it to delta:
	//
	FBound (&CameraInfo->Actor->Velocity,40.0);
	*Delta += CameraInfo->Actor->Velocity;
	//
	if ((CameraInfo->ShowFlags & SHOW_PlayerCtrl) &&
		(!CameraInfo->Camera->IsOrtho()) && 
		(!(CameraInfo->Level.ModelInfo.ModelFlags & MF_InvalidBsp)))
		{
		if (CameraInfo->Level.ModelInfo.PointClass(&CameraInfo->Actor->Location,NULL) != 0)
			{
			//
			// Move with collision; player is outside and collision is enabled:
			//
			if (CameraInfo->Level.ModelInfo.SphereMove(&CameraInfo->Actor->Location,Delta,20.0,1)==0)
				{
				if (!(CameraInfo->ShowFlags&SHOW_PlayerCtrl)) CameraInfo->Actor->Velocity = GMath.ZeroVector;
				};
			}
		else CameraInfo->Actor->Location.MoveBounded(*Delta); // Camera is trapped inside a wall, move freely
		}
	else CameraInfo->Actor->Location.MoveBounded(*Delta); // Move without collision
	UNGUARD("CameraMoveRotWithPhysics");
	};

//
// Move the camera so that it's facing the rotated/moved brush in exactly the
// same way as before.  Takes grid effects into account:
//
void CameraTrackBrush (ICamera *CameraInfo,FVector *Delta,FFloatRotation *DeltaRot)
	{
	IModel	BrushInfo,*ModelInfo;
	FVector		StartLocation,EndLocation;
	FRotation	StartRotation,EndRotation;
	//
	GUARD;
	//
	ModelInfo = &CameraInfo->Level.ModelInfo;
	CameraInfo->Level.Brush->Lock (&BrushInfo,LOCK_Read);
	//
	StartLocation = BrushInfo.Location; EndLocation = StartLocation;
	StartRotation = BrushInfo.Rotation; EndRotation = StartRotation;
	//
	EndLocation.MoveBounded (*Delta);
	EndRotation.AddBounded  (DeltaRot->Pitch,DeltaRot->Yaw,DeltaRot->Roll);
	//
	GUnrealEditor.constraintApply (ModelInfo,&BrushInfo,&StartLocation,&StartRotation,&GUnrealEditor.Constraints);
	GUnrealEditor.constraintApply (ModelInfo,&BrushInfo,&EndLocation  ,&EndRotation,  &GUnrealEditor.Constraints);
	//
	// Now move camera accordinly:
	//
	CameraInfo->Actor->Location += EndLocation;
	CameraInfo->Actor->Location -= StartLocation;
	//
	CameraInfo->Level.Brush->Unlock(&BrushInfo);
	UNGUARD("CameraMoveRotWithPhysics");
	};

//
// If this is the first time the brush has moved since the user first
// pressed a mouse button, save the brush position transactionally so it can
// be undone/redone:
//
// Implicityly assumes that the selection set can't be changed between
// NoteBrushMovement/FinishBrushMovement pairs.
//
void NoteBrushMovement (ICamera *CameraInfo)
	{
	FVector			WorldPivotLocation;
	UModel			*Brush;
	int 			i,n;
	//
	GUARD;
	if ((!GTrans->Locked) && (!(CameraInfo->Camera->ClickFlags & CF_MOVE_BRUSH)))
		{
		GTrans->Begin (CameraInfo->Level.Level,"Brush movement");
		//
		if (GUnrealEditor.MapEdit)
			{
			n = CameraInfo->Level.BrushArray->Num;
			for (i=0; i<n; i++)
				{
				Brush = CameraInfo->Level.BrushArray->Element(i);
				if ((Brush->ModelFlags & MF_Selected) || (i==0))
					{
					GTrans->NoteResHeader (Brush);
					Brush->ModelFlags |= MF_ShouldSnap;
					if (i!=0)
						{
						Brush->SetPivotPoint(&WorldPivotLocation,0);
						}
					else // i==0, first iteration
						{
						GUnrealEditor.constraintApply (NULL,NULL,&Brush->Location,&Brush->Rotation,&GUnrealEditor.Constraints);
						WorldPivotLocation = Brush->Location + Brush->PostPivot;
						};
					Brush->Bound[1].Min.iTransform=0; // Note that transformed bounds are invalid
					};
				};
			}
		else
			{
			GTrans->NoteResHeader (CameraInfo->Level.Brush);
			CameraInfo->Level.Brush->Bound[1].Min.iTransform=0;
			};
		GTrans->End();
		//
		CameraInfo->Camera->ClickFlags |= CF_MOVE_BRUSH;
		};
	UNGUARD("NoteBrushMovement");
	};

//
// Finish any snaps that were applied after NoteBrushMovement:
//
void FinishBrushMovement (ICamera *CameraInfo)
	{
	GUARD;
	//
	IModel		BrushInfo;
	UModel		*Brush;
	//
	if (GUnrealEditor.MapEdit)
		{
		int n = CameraInfo->Level.BrushArray->Num;
		for (int i=0; i<n; i++)
			{
			Brush = CameraInfo->Level.BrushArray->Element(i);
			if ((i==0) || (Brush->ModelFlags & MF_ShouldSnap))
				{
				Brush->Lock(&BrushInfo,LOCK_Trans);
				GUnrealEditor.constraintApply (&CameraInfo->Level.ModelInfo,&BrushInfo,&BrushInfo.Location,&BrushInfo.Rotation,&GUnrealEditor.Constraints);
				BrushInfo.ModelFlags &= ~MF_ShouldSnap;
				Brush->Unlock(&BrushInfo);
				};
			if (Brush->ModelFlags & MF_Selected) Brush->BuildBound(1);
			};
		}
	else
		{
		CameraInfo->Level.Brush->Lock(&BrushInfo,LOCK_Trans);
		GUnrealEditor.constraintApply (&CameraInfo->Level.ModelInfo,&BrushInfo,&BrushInfo.Location,&BrushInfo.Rotation,&GUnrealEditor.Constraints);
		CameraInfo->Level.Brush->Unlock(&BrushInfo);
		//
		CameraInfo->Level.Brush->BuildBound(1);
		};
	UNGUARD("FinishBrushMovement");
	};

//
// If this is the first time called since first click, note all selected
// brush polys:
//
void NoteActorMovement (ICamera *CameraInfo)
	{
	AActor	*Actor;
	INDEX	i,Found;
	//
	GUARD;
	Found = 0;
	if ((!GTrans->Locked) && (!(CameraInfo->Camera->ClickFlags & CF_MOVE_ACTOR)))
		{
		Actor = &CameraInfo->Level.Actors->Element(0);
		for (i=0; i<CameraInfo->Level.Actors->Max; i++)
			{			
			if (Actor->bSelected)
				{
				if (!Found) GTrans->Begin (CameraInfo->Level.Level,"Actor movement");
				GTrans->NoteActor (CameraInfo->Level.Actors,i);
				if (Actor->Brush) GTrans->NoteResHeader (Actor->Brush);
				Found++;
				};
			Actor++;
			};
		if (Found) GTrans->End ();
		//
		CameraInfo->Camera->ClickFlags |= CF_MOVE_ACTOR;
		};
	UNGUARD("NoteActorMovement");
	};

//
// If this is the first time textures have been adjusted since the user first
// pressed a mouse button, save selected polygons transactionally so this can
// be undone/redone:
//
void NoteTextureMovement (ICamera *CameraInfo)
	{
	GUARD;
	if ((!GTrans->Locked) && 
		(CameraInfo->Level.ModelInfo.Trans) && 
		(!(CameraInfo->Camera->ClickFlags & CF_MOVE_TEXTURE)))
		{
		GTrans->Begin (CameraInfo->Level.Level,"Texture movement");
		GTrans->NoteSelectedBspSurfs (&CameraInfo->Level.ModelInfo,1);
		GTrans->End ();
		//
		CameraInfo->Camera->ClickFlags |= CF_MOVE_TEXTURE;
		};
	UNGUARD("NoteTextureMovement");
	};

//
// Move and rotate brush.
//
void BrushMoveRot (ICamera *CameraInfo,FVector *Delta,FFloatRotation *DeltaRot)
	{
	IModel		BrushInfo;
	UModel			*Brush;
	int				i,n,Moved;
	//
	GUARD;
	NoteBrushMovement (CameraInfo);
	//
	Moved = 0;
	if (GUnrealEditor.MapEdit)
		{
		n = CameraInfo->Level.BrushArray->Num;
		for (i=0; i<n; i++)
			{
			Brush = CameraInfo->Level.BrushArray->Element(i);
			if (Brush->ModelFlags & MF_Selected)
				{
		   		Brush->Lock (&BrushInfo,LOCK_Trans);
		   		BrushInfo.Location.MoveBounded(*Delta);
				BrushInfo.Rotation.Add        (DeltaRot->Pitch,DeltaRot->Yaw,DeltaRot->Roll);
		   		Brush->Unlock (&BrushInfo);
				//
				Moved = 1;
				};
			};
		};
	if (!Moved)
		{
   		CameraInfo->Level.Brush->Lock   (&BrushInfo,LOCK_Trans);
   		BrushInfo.Location.MoveBounded	(*Delta);
		BrushInfo.Rotation.Add			(DeltaRot->Pitch,DeltaRot->Yaw,DeltaRot->Roll);
   		CameraInfo->Level.Brush->Unlock (&BrushInfo);
		};
	UNGUARD("BrushMoveRot");
	};

/*-----------------------------------------------------------------------------
   Editor camera movement
-----------------------------------------------------------------------------*/

//
// Move the edit-camera.
//
void FEditor::edcamMove (UCamera *Camera, BYTE Buttons, SWORD MouseX, SWORD MouseY, int Shift, int Ctrl)
	{
	ICamera 		CameraInfo;
	IModel  		BrushInfo;
	FVector     	Delta,Vector,SnapMin,SnapMax,DeltaMin,DeltaMax,DeltaFree;
	FFloatRotation	DeltaRot;
	FLOAT			TempFloat,TempU,TempV,Speed;
	int				Temp;
	static int		ForceXSnap=0,ForceYSnap=0,ForceZSnap=0;
	static FLOAT	TextureAngle=0.0;
	static INDEX	*OriginalUVectors = NULL,*OriginalVVectors = NULL,OrigNumVectors=0;
	//
	GUARD;
	if (Camera->IsBrowser()) return;
	if (!Camera->Lock(&CameraInfo)) return;
	//
	Delta.X    		= 0.0;  Delta.Y  		= 0.0;  Delta.Z   		= 0.0;
	DeltaRot.Pitch	= 0.0;  DeltaRot.Yaw	= 0.0;  DeltaRot.Roll	= 0.0;
	//
	if (Buttons & BUT_FIRSTHIT)
		{
		//
		// Reset flags that last for the duration of the click:
		//
		CameraInfo.Camera->ClickFlags &= ~(CF_MOVE_ALL);
		//
		CameraInfo.Level.Brush->Lock (&BrushInfo,LOCK_Trans);
		if (GUnrealEditor.Mode==EM_BrushSnap)
			{
			if (!(BrushInfo.ModelFlags & MF_PostScale))
				{
				BrushInfo.ModelFlags |= MF_PostScale;
				BrushInfo.PostScale   = GMath.UnitScale;
				};
			BrushInfo.TempScale = BrushInfo.PostScale;
			ForceXSnap = 0;
			ForceYSnap = 0;
			ForceZSnap = 0;
			}
		else if (GUnrealEditor.Mode==EM_TextureRotate)
			{
			//
			// Guarantee that each texture u and v vector on each selected polygon
			// is unique in the world.
			//
			if (OriginalUVectors) appFree(OriginalUVectors);
			if (OriginalVVectors) appFree(OriginalVVectors);
			//
			int Size = CameraInfo.Level.ModelInfo.NumBspSurfs * sizeof(INDEX);
			OriginalUVectors = (INDEX *)appMalloc(Size,"OriginalUVectors");
			OriginalVVectors = (INDEX *)appMalloc(Size,"OriginalVVectors");
			//
			OrigNumVectors = CameraInfo.Level.ModelInfo.NumVectors;
			//
			for (int i=0; i<CameraInfo.Level.ModelInfo.NumBspSurfs; i++)
				{
				FBspSurf *Surf = &CameraInfo.Level.ModelInfo.BspSurfs[i];
				OriginalUVectors[i] = Surf->vTextureU;
				OriginalVVectors[i] = Surf->vTextureV;
				//
				if (Surf->PolyFlags & PF_Selected)
					{
					if ((CameraInfo.Level.ModelInfo.NumVectors+2)<CameraInfo.Level.ModelInfo.MaxVectors)
						{
						int n			= CameraInfo.Level.ModelInfo.NumVectors++;
						FVector *V		= &CameraInfo.Level.ModelInfo.FVectors[n];
						*V				= CameraInfo.Level.ModelInfo.FVectors[Surf->vTextureU];
						Surf->vTextureU = n;
						//
						n				= CameraInfo.Level.ModelInfo.NumVectors++;
						V				= &CameraInfo.Level.ModelInfo.FVectors[n];
						*V				= CameraInfo.Level.ModelInfo.FVectors[Surf->vTextureV];
						Surf->vTextureV = n;
						//
						Surf->iLightMesh = INDEX_NONE; // Invalidate lighting mesh
						}
					else Surf->PolyFlags &= ~PF_Selected; // Overflow; prevent further rotation
					};
				};
			TextureAngle = 0.0;
			};
		CameraInfo.Level.Brush->Unlock(&BrushInfo);
		};
	if (Buttons & BUT_LASTRELEASE)
		{
		FinishBrushMovement(&CameraInfo);
		//
		if (OriginalUVectors)
			{
			//
			// Finishing up texture rotate mode.  Go through and minimize the set of
			// vectors we've been adjusting by merging the new vectors in and eliminating
			// duplicates:
			//
			int Size = CameraInfo.Level.ModelInfo.NumVectors * sizeof(FVector);
			FVector *AllVectors = (FVector *)GMem.Get(Size);
			memcpy(AllVectors,CameraInfo.Level.ModelInfo.FVectors,Size);
			//
			CameraInfo.Level.ModelInfo.NumVectors = OrigNumVectors;
			//
			for (int i=0; i<CameraInfo.Level.ModelInfo.NumBspSurfs; i++)
				{
				FBspSurf *Surf = &CameraInfo.Level.ModelInfo.BspSurfs[i];
				if (Surf->PolyFlags & PF_Selected)
					{
					// Update master texture coordinates but not base:
					polyUpdateMaster (&CameraInfo.Level.ModelInfo,i,1,0);
					// Add this poly's vectors, merging with the level's existing vectors:
					Surf->vTextureU = bspAddVector(&CameraInfo.Level.ModelInfo,&AllVectors[Surf->vTextureU],0);
					Surf->vTextureV = bspAddVector(&CameraInfo.Level.ModelInfo,&AllVectors[Surf->vTextureV],0);
					};
				};
			GMem.Release(AllVectors);
			appFree(OriginalUVectors); OriginalUVectors=NULL;
			appFree(OriginalVVectors); OriginalVVectors=NULL;
			//
			// Force an entire Bsp refresh to remove unneeded vectors:
			//
			bspRefresh(&CameraInfo.Level.ModelInfo,1);
			};
		};
	switch (GUnrealEditor.Mode)
		{
		case EM_None: /* Editor disabled */
			//
			debug (LOG_Ed,"Editor is disabled");
			//
			break;
		case EM_CameraMove: /* Move camera normally */
			//
			CameraMove:
			//
			if (Buttons & (BUT_FIRSTHIT | BUT_LASTRELEASE | BUT_SETMODE | BUT_EXITMODE))
				{
				CameraInfo.Actor->Velocity = GMath.ZeroVector;
				}
			else
				{
   				if (!(Ctrl||Shift))
					{
					Speed = 0.30*GUnrealEditor.MovementSpeed;
					if (CameraInfo.Camera->IsOrtho())
						{
						if (Buttons == BUT_RIGHT)
							{
							Buttons = BUT_LEFT;
							Speed   = 0.60*GUnrealEditor.MovementSpeed;
							};
						CalcFreeOrthoMoveRot (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
						}
					else
						{
						CalcFreePerspMoveRot (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
						};
					Delta *= Speed;
					CameraMoveRotWithPhysics (&CameraInfo,&Delta,&DeltaRot); // Move camera
					}
				else
					{
					if (CameraInfo.Camera->IsOrtho())	CalcFreeOrthoMoveRot  (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
					else                				CalcAxialPerspMove    (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
					//
					Delta *= GUnrealEditor.MovementSpeed*0.25;
					//
					if (Shift) CameraTrackBrush (&CameraInfo,&Delta,&DeltaRot);
   					BrushMoveRot				(&CameraInfo,&Delta,&DeltaRot); // Move brush
					};
				};
			break;
		case EM_CameraZoom:		/* Move camera with acceleration */
			//
			if (Buttons&(BUT_FIRSTHIT | BUT_LASTRELEASE | BUT_SETMODE | BUT_EXITMODE))
				{
				CameraInfo.Actor->Velocity = GMath.ZeroVector;
				}
			else
				{
				if (!(Ctrl||Shift))
					{
					if (CameraInfo.Camera->IsOrtho())	CalcFreeOrthoMoveRot (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
					else                				CalcFreePerspMoveRot (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
					}
				else
					{
					if (CameraInfo.Camera->IsOrtho())	CalcFreeOrthoMoveRot  (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
					else                				CalcAxialPerspMove    (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
					};
				Delta *= GUnrealEditor.MovementSpeed * 0.025;
				//
				CameraInfo.Actor->Velocity += Delta;
				Delta = CameraInfo.Actor->Velocity;
				//
   				if (!(Ctrl||Shift))
	   				{
					CameraMoveRotWithPhysics	(&CameraInfo,&Delta,&DeltaRot); // Move camera
					}
				else
					{
					if (Shift) CameraTrackBrush (&CameraInfo,&Delta,&DeltaRot);
					BrushMoveRot				(&CameraInfo,&Delta,&DeltaRot); // Move brush
					};
				};
			break;
		case EM_BrushFree:		/* Move brush free-form */
			//
			if (Buttons & (BUT_FIRSTHIT | BUT_LASTRELEASE | BUT_SETMODE | BUT_EXITMODE))
				{
				CameraInfo.Actor->Velocity = GMath.ZeroVector;
				}
			else
				{
				if (Ctrl && !Shift)
					{
					Temp 	= Shift;
					Shift = !Ctrl;
					Ctrl	= Temp;
					goto CameraMove; // Just want to move camera and not brush!
					};
				if (CameraInfo.Camera->IsOrtho())	CalcFreeOrthoMoveRot (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
				else                				CalcFreePerspMoveRot (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
				//
				Delta *= GUnrealEditor.MovementSpeed * 0.25;
				//
				if (Shift || Ctrl) 		CameraMoveRotWithPhysics(&CameraInfo,&Delta,&DeltaRot); // Move camera
   				if (1)				 	BrushMoveRot			(&CameraInfo,&Delta,&DeltaRot); // Move brush
				};
			break;
		case EM_BrushMove:		/* Move brush along one axis at a time */
			//
			if (Buttons & (BUT_FIRSTHIT | BUT_LASTRELEASE | BUT_SETMODE | BUT_EXITMODE))
				{
				CameraInfo.Actor->Velocity = GMath.ZeroVector;
				}
			else if (GApp->KeyDown[K_ALT])
				{
				goto TextureSet;
				}
			else
				{
				if (Ctrl && !Shift)
					{
					Temp 	= Shift;
					Shift = !Ctrl;
					Ctrl	= Temp;
					goto CameraMove; // Just want to move camera and not brush!
					};
				if (CameraInfo.Camera->IsOrtho())	CalcAxialOrthoMove  (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
				else                				CalcAxialPerspMove	(&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
				//
				Delta *= GUnrealEditor.MovementSpeed*0.25;
				//
				if (Shift || Ctrl)	CameraMoveRotWithPhysics(&CameraInfo,&Delta,&DeltaRot); // Move camera
   				if (1)				BrushMoveRot			(&CameraInfo,&Delta,&DeltaRot); // Move brush
				};
			break;
		case EM_BrushRotate:		/* Rotate brush */
			//
			if (GApp->KeyDown[K_ALT]) goto TextureSet;
			else if (!Ctrl) goto CameraMove;
			//
			NoteBrushMovement			(&CameraInfo);
   			CameraInfo.Level.Brush->Lock(&BrushInfo,LOCK_Trans);
			//
			CalcAxialPerspMove			(&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
			//
			BrushInfo.Rotation.Pitch += Delta.X * 4.0;
			BrushInfo.Rotation.Yaw   += Delta.Y * 4.0;
			BrushInfo.Rotation.Roll  += Delta.Z * 4.0;
			//
			CameraInfo.Level.Brush->Unlock(&BrushInfo);
			break;
		case EM_BrushSheer: /* Sheer brush */
			//
			if (GApp->KeyDown[K_ALT]) goto TextureSet;
			else if (!Ctrl) goto CameraMove;
			//
			NoteBrushMovement(&CameraInfo);
   			CameraInfo.Level.Brush->Lock (&BrushInfo,LOCK_Trans);
			BrushInfo.Scale.SheerRate = OurMax(-4.0,OurMin(4.0,BrushInfo.Scale.SheerRate + (FLOAT)(-MouseY) / 240.0));
			CameraInfo.Level.Brush->Unlock (&BrushInfo);
			break;
		case EM_BrushScale:	/* Scale brush (proportionally along all axes) */
			//			  
			if (GApp->KeyDown[K_ALT]) goto TextureSet;
			else if (!Ctrl) goto CameraMove;
			//
			NoteBrushMovement(&CameraInfo);
   			CameraInfo.Level.Brush->Lock(&BrushInfo,LOCK_Trans);
			Vector  = BrushInfo.Scale.Scale * (1 + (FLOAT)(-MouseY) / 256.0);
			//
			if (ScaleIsWithinBounds(&Vector,0.05,400.0)) BrushInfo.Scale.Scale = Vector;
			CameraInfo.Level.Brush->Unlock (&BrushInfo);
			break;
		case EM_BrushStretch: /* Stretch brush axially */
			//
			if (GApp->KeyDown[K_ALT]) goto TextureSet;
			else if (!Ctrl) goto CameraMove;
			//
			NoteBrushMovement(&CameraInfo);
   			CameraInfo.Level.Brush->Lock (&BrushInfo,LOCK_Trans);
			//
			if (CameraInfo.Camera->IsOrtho()) CalcAxialOrthoMove (&CameraInfo,MouseX,-MouseY,Buttons,&Delta,&DeltaRot);
			else CalcAxialPerspMove (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
			//
			Vector = BrushInfo.Scale.Scale;
			Vector.X *= (1 + Delta.X / 256.0);
			Vector.Y *= (1 + Delta.Y / 256.0);
			Vector.Z *= (1 + Delta.Z / 256.0);
			//
			if (ScaleIsWithinBounds(&Vector,0.05,400.0)) BrushInfo.Scale.Scale = Vector;
			CameraInfo.Level.Brush->Unlock (&BrushInfo);
			break;
		case EM_BrushSnap: /* Scale brush snapped to grid */
			if (GApp->KeyDown[K_ALT]) goto TextureSet;
			else if (!Ctrl) goto CameraMove;
			//
			NoteBrushMovement(&CameraInfo);
   			CameraInfo.Level.Brush->Lock (&BrushInfo,LOCK_Trans);
			//
			if (CameraInfo.Camera->IsOrtho()) CalcAxialOrthoMove (&CameraInfo,MouseX,-MouseY,Buttons,&Delta,&DeltaRot);
			else CalcAxialPerspMove (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
			//
			Vector = BrushInfo.TempScale.Scale;
			Vector.X *= (1 + Delta.X / 400.0);
			Vector.Y *= (1 + Delta.Y / 400.0);
			Vector.Z *= (1 + Delta.Z / 400.0);
			//
			if (ScaleIsWithinBounds(&Vector,0.05,400.0))
				{
				BrushInfo.TempScale.Scale = Vector;
				BrushInfo.PostScale.Scale = Vector;
				//
				if (CameraInfo.Level.Brush->Polys->Num==0) break;
				//
   				CameraInfo.Level.Brush->Unlock(&BrushInfo);
				CameraInfo.Level.Brush->BuildBound(1);
				FVector BoxMin=CameraInfo.Level.Brush->Bound[1].Min;
				FVector BoxMax=CameraInfo.Level.Brush->Bound[1].Max;
   				CameraInfo.Level.Brush->Lock (&BrushInfo,LOCK_Trans);
				//
				SnapMin   = BoxMin; constrainSimplePoint (&SnapMin,&GUnrealEditor.Constraints);
				DeltaMin  = BrushInfo.Location + BrushInfo.PostPivot - SnapMin;
				DeltaFree = BrushInfo.Location + BrushInfo.PostPivot - BoxMin;
				SnapMin.X = BrushInfo.PostScale.Scale.X * DeltaMin.X/DeltaFree.X;
				SnapMin.Y = BrushInfo.PostScale.Scale.Y * DeltaMin.Y/DeltaFree.Y;
				SnapMin.Z = BrushInfo.PostScale.Scale.Z * DeltaMin.Z/DeltaFree.Z;
				//
				SnapMax   = BoxMax; constrainSimplePoint (&SnapMax,&GUnrealEditor.Constraints);
				DeltaMax  = BrushInfo.Location + BrushInfo.PostPivot - SnapMax;
				DeltaFree = BrushInfo.Location + BrushInfo.PostPivot - BoxMax;
				SnapMax.X = BrushInfo.PostScale.Scale.X * DeltaMax.X/DeltaFree.X;
				SnapMax.Y = BrushInfo.PostScale.Scale.Y * DeltaMax.Y/DeltaFree.Y;
				SnapMax.Z = BrushInfo.PostScale.Scale.Z * DeltaMax.Z/DeltaFree.Z;
				//
				// Set PostScale so brush extents are gridsnapped
				// in all directions of movement:
				//
				if (ForceXSnap || (Delta.X!=0))
					{
					ForceXSnap = 1;
					if ((SnapMin.X>0.05) &&
						((SnapMax.X<=0.05) ||
						(OurAbs(SnapMin.X-BrushInfo.PostScale.Scale.X) < OurAbs(SnapMax.X-BrushInfo.PostScale.Scale.X))))
						BrushInfo.PostScale.Scale.X = SnapMin.X;
					else if (SnapMax.X>0.05)
						BrushInfo.PostScale.Scale.X = SnapMax.X;
					};
				if (ForceYSnap || (Delta.Y!=0))
					{
					ForceYSnap = 1;
					if ((SnapMin.Y>0.05) &&
						((SnapMax.Y<=0.05) ||
						(OurAbs(SnapMin.Y-BrushInfo.PostScale.Scale.Y) < OurAbs(SnapMax.Y-BrushInfo.PostScale.Scale.Y))))
						BrushInfo.PostScale.Scale.Y = SnapMin.Y;
					else if (SnapMax.Y>0.05)
						BrushInfo.PostScale.Scale.Y = SnapMax.Y;
					};
				if (ForceZSnap || (Delta.Z!=0))
					{
					ForceZSnap = 1;
					if ((SnapMin.Z>0.05) &&
						((SnapMax.Z<=0.05) ||
						(OurAbs(SnapMin.Z-BrushInfo.PostScale.Scale.Z) < OurAbs(SnapMax.Z-BrushInfo.PostScale.Scale.Z))))
						BrushInfo.PostScale.Scale.Z = SnapMin.Z;
					else if (SnapMax.Z>0.05)
						BrushInfo.PostScale.Scale.Z = SnapMax.Z;
					};
				};
			CameraInfo.Level.Brush->Unlock (&BrushInfo);
			break;
		case EM_MoveActor:		/* Move actor/light */
		case EM_AddActor:		/* Add actor/light */
			if (Buttons & (BUT_FIRSTHIT | BUT_LASTRELEASE | BUT_SETMODE | BUT_EXITMODE))
				{
				CameraInfo.Actor->Velocity = GMath.ZeroVector;
				}
			else
				{
   				if (!Ctrl)
					{
					if (CameraInfo.Camera->IsOrtho())	CalcFreeOrthoMoveRot (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
					else                				CalcFreePerspMoveRot (&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
					//
					Delta *= GUnrealEditor.MovementSpeed * 0.25;
					//
					CameraMoveRotWithPhysics (&CameraInfo,&Delta,&DeltaRot); // Move camera
					}
				else if (MouseX||MouseY)
					{
					NoteActorMovement (&CameraInfo);
					//
					if (CameraInfo.Camera->IsOrtho())	CalcFreeOrthoMoveRot	(&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
					else                				CalcAxialPerspMove  	(&CameraInfo,MouseX,MouseY,Buttons,&Delta,&DeltaRot);
					//
					Delta *= GUnrealEditor.MovementSpeed * 0.25;
					//
					DeltaRot.Pitch	*= 2;
					DeltaRot.Yaw	*= 2;
					DeltaRot.Roll	*= 2;
					//
					if (Shift) CameraMoveRot(&CameraInfo,&Delta,&DeltaRot);
   					GUnrealEditor.edactMoveSelected(&CameraInfo.Level,&Delta,&DeltaRot); // Move actor
					};
				};
			break;
		case EM_TexturePan:		/* Pan/scale textures */
			//
			if (!Ctrl) goto CameraMove;
			//
			NoteTextureMovement (&CameraInfo);
			//
			if (Buttons == (BUT_LEFT | BUT_RIGHT)) // Scale
				{
				GFixScale += FIX(MouseY)/64;
				//
				TempFloat = 1.0;
				Temp = UNFIX (GFixScale); 
				while (Temp>0) {TempFloat *= 0.5; Temp--;};
				while (Temp<0) {TempFloat *= 2.0; Temp++;};
				//
				if (Buttons & BUT_LEFT)		TempU = TempFloat; else TempU = 1.0;
				if (Buttons & BUT_RIGHT)	TempV = TempFloat; else TempV = 1.0;
				//
				if ((TempU != 1.0) || (TempV != 1.0))
					{
					GUnrealEditor.polyTexScale (&CameraInfo.Level.ModelInfo,TempU,0.0,0.0,TempV,0);
					};
				GFixScale &= 0xffff;
				}
			else if (Buttons == BUT_LEFT)
				{
				GFixPanU += FIX(MouseX)/16;  GFixPanV += FIX(MouseY)/16;
				GUnrealEditor.polyTexPan (&CameraInfo.Level.ModelInfo,UNFIX(GFixPanU),UNFIX(0),0);
				GFixPanU &= 0xffff; GFixPanV &= 0xffff;
				}
			else // Right
				{
				GFixPanU += FIX(MouseX)/16;  GFixPanV += FIX(MouseY)/16;
				GUnrealEditor.polyTexPan (&CameraInfo.Level.ModelInfo,UNFIX(0),UNFIX(GFixPanV),0);
				GFixPanU &= 0xffff; GFixPanV &= 0xffff;
				}
			break;
		case EM_TextureSet:		/* Set textures */
			//
			TextureSet:
			goto CameraMove;
			//
			break;
		case EM_TextureRotate:	/* Rotate textures */
			{
			if (!Ctrl) goto CameraMove;
			NoteTextureMovement (&CameraInfo);
			//
			TextureAngle += (FLOAT)MouseX / 256.0;
			//
			for (int i=0; i<CameraInfo.Level.ModelInfo.NumBspSurfs; i++)
				{
				FBspSurf *Surf = &CameraInfo.Level.ModelInfo.BspSurfs[i];
				if (Surf->PolyFlags & PF_Selected)
					{
					FVector U		= CameraInfo.Level.ModelInfo.FVectors[OriginalUVectors[i]];
					FVector V		= CameraInfo.Level.ModelInfo.FVectors[OriginalVVectors[i]];
					//
					FVector *NewU	= &CameraInfo.Level.ModelInfo.FVectors[Surf->vTextureU];
					FVector *NewV	= &CameraInfo.Level.ModelInfo.FVectors[Surf->vTextureV];
					//
					*NewU			= U * cos(TextureAngle) + V * sin(TextureAngle);
					*NewV			= V * cos(TextureAngle) - U * sin(TextureAngle);
					//
					NewU->Align		= FVA_None;
					NewU->iTransform= INDEX_NONE;
					NewU->Flags		= 0;
					//
					NewV->Align		= FVA_None;
					NewV->iTransform= INDEX_NONE;
					NewV->Flags		= 0;
					};
				};
			};
			break;
		case EM_BrushWarp:		/* Move brush verts */
			//
			goto CameraMove;
			//
			break;
		case EM_Terraform:		/* Terrain editing */
			//
			goto CameraMove;
			//
			break;
		default:
			//
			debugf (LOG_Ed,"Unknown editor mode %i",GUnrealEditor.Mode);
			goto CameraMove;
			break;
		};
	if (CameraInfo.RendMap != REN_MeshView) CameraInfo.Actor->DrawRot = CameraInfo.Actor->ViewRot;
	Camera->Unlock(&CameraInfo,0);
	//
	UNGUARD_BEGIN
	UNGUARD_MSGF("edcamMove(Mode=%i)",GUnrealEditor.Mode);
	UNGUARD_END
	};

/*-----------------------------------------------------------------------------
   Keypress handling
-----------------------------------------------------------------------------*/

//
// Handle a regular ASCII key that was pressed in UnrealEd.
// Returns 1 if proceesed, 0 if not.
//
int FEditor::edcamKey (UCamera *Camera, int Key)
	{
	GUARD;
	ICamera CameraInfo;
	ULevel		*Level;
	int			ModeClass;
	int 		Processed=0;
	//
	if (Camera->IsBrowser()) return 0;
	//
	ModeClass = GUnrealEditor.edcamModeClass(GUnrealEditor.edcamMode(Camera));
	Level     = Camera->Level;
	//
	if (GApp->KeyDown[K_SHIFT] || (Key==K_DELETE))  // Selection functions
		{
		if (ModeClass==EMC_Actor) switch (toupper(Key))
			{
			case 'A': GUnrealEditor.Exec  ("ACTOR SELECT ALL");		return 1;
			case 'N': GUnrealEditor.Exec  ("ACTOR SELECT NONE");	return 1;
			case 'Z': GUnrealEditor.Exec  ("ACTOR SELECT NONE");	return 1;
			case 'Y': GUnrealEditor.Exec  ("ACTOR DELETE");			return 1;
			case 'D': GUnrealEditor.Exec  ("ACTOR DUPLICATE");		return 1;
			case K_DELETE: GUnrealEditor.Exec("ACTOR DELETE");		return 1;			
			}
		else if (GUnrealEditor.MapEdit) switch (toupper(Key))
			{
			case 'A': GUnrealEditor.Exec  ("MAP SELECT ALL");		return 1;
			case 'N': GUnrealEditor.Exec  ("MAP SELECT NONE");		return 1;
			case 'Z': GUnrealEditor.Exec  ("MAP SELECT NONE");		return 1;
			case 'Y': GUnrealEditor.Exec  ("MAP DELETE");			return 1;
			case 'D': GUnrealEditor.Exec  ("MAP DUPLICATE");		return 1;
			case 'F': GUnrealEditor.Exec  ("MAP SELECT FIRST");		return 1;
			case 'L': GUnrealEditor.Exec  ("MAP SELECT LAST");		return 1;
			case 'G': GUnrealEditor.Exec  ("MAP BRUSH GET");		return 1;
			case 'P': GUnrealEditor.Exec  ("MAP BRUSH PUT");		return 1;
			case 'S': GUnrealEditor.Exec  ("MAP SELECT NEXT");		return 1;
			case 'X': GUnrealEditor.Exec  ("MAP SELECT PREVIOUS"); 	return 1;
			case K_DELETE: GUnrealEditor.Exec  ("MAP DELETE");		return 1;
			}
		else switch (toupper(Key))
			{
			case 'A': GUnrealEditor.Exec  ("POLY SELECT ALL"); 				return 1;
			case 'N': GUnrealEditor.Exec  ("POLY SELECT NONE"); 			return 1;
			case 'Z': GUnrealEditor.Exec  ("POLY SELECT NONE"); 			return 1;
			case 'G': GUnrealEditor.Exec  ("POLY SELECT MATCHING GROUPS"); 	return 1;
			case 'I': GUnrealEditor.Exec  ("POLY SELECT MATCHING ITEMS"); 	return 1;
			case 'C': GUnrealEditor.Exec  ("POLY SELECT ADJACENT COPLANARS"); return 1;
			case 'J': GUnrealEditor.Exec  ("POLY SELECT ADJACENT ALL"); 	return 1;
			case 'W': GUnrealEditor.Exec  ("POLY SELECT ADJACENT WALLS"); 	return 1;
			case 'F': GUnrealEditor.Exec  ("POLY SELECT ADJACENT FLOORS"); 	return 1;
			case 'S': GUnrealEditor.Exec  ("POLY SELECT ADJACENT SLANTS"); 	return 1;
			case 'B': GUnrealEditor.Exec  ("POLY SELECT MATCHING BRUSH"); 	return 1;
			case 'T': GUnrealEditor.Exec  ("POLY SELECT MATCHING TEXTURE"); return 1;
			case 'Q': GUnrealEditor.Exec  ("POLY SELECT REVERSE");			return 1;
			case 'M': GUnrealEditor.Exec  ("POLY SELECT MEMORY SET"); 		return 1;
			case 'R': GUnrealEditor.Exec  ("POLY SELECT MEMORY RECALL"); 	return 1;
			case 'X': GUnrealEditor.Exec  ("POLY SELECT MEMORY XOR"); 		return 1;
			case 'U': GUnrealEditor.Exec  ("POLY SELECT MEMORY UNION"); 	return 1;
			case 'O': GUnrealEditor.Exec  ("POLY SELECT MEMORY INTERSECT"); return 1;
			};
		}
	else if (!GApp->KeyDown[K_ALT])
		{
		if (Camera->Lock(&CameraInfo))
			{
			switch (toupper(Key))
				{
				case 'B': 		// Toggle brush visibility
					CameraInfo.ShowFlags ^= SHOW_Brush;
					Processed=1;
					break;
				case 'K': 		// Toggle backdrop
					CameraInfo.ShowFlags ^= SHOW_Backdrop;
					Processed=1;
					break;
				case 'P':		// Toggle player controls
					CameraInfo.ShowFlags ^= SHOW_PlayerCtrl;
					Processed=1;
					GCameraManager->ResetModes();
					break;
				case 'U':		// Undo
					Camera->Unlock(&CameraInfo,0);
					if (GTrans->Undo ()) GCameraManager->RedrawLevel (Level);
					Camera->Lock(&CameraInfo);
					Processed=1;
					break;
				case 'R':		// Redo
					Camera->Unlock(&CameraInfo,0);
					if (GTrans->Redo()) GCameraManager->RedrawLevel (Level);
					Camera->Lock(&CameraInfo);
					Processed=1;
					break;
				case 'L':		// Look ahead
		        	CameraInfo.Actor->ViewRot.Pitch = 0;
		        	CameraInfo.Actor->ViewRot.Roll  = 0;
					Processed=1;
					break;
				case '1':	  // Movement speed
					GUnrealEditor.MovementSpeed = 1;
					Processed=1;
					break;
				case '2':	  // Movement speed
					GUnrealEditor.MovementSpeed = 4;
					Processed=1;
					break;
				case '3':  	  // Movement speed
					GUnrealEditor.MovementSpeed = 16;
					Processed=1;
					break;
				case K_ESCAPE:   // Escape, update screen
					Processed=1;
					break;
				case K_DELETE:
					Camera->Unlock(&CameraInfo,0);
					if (ModeClass==EMC_Actor) GUnrealEditor.Exec  ("ACTOR DELETE");
					Camera->Lock(&CameraInfo);
					Processed=1;
					break;
				};
			Camera->Unlock(&CameraInfo,0);
			};
		};
	return Processed;
	UNGUARD("edcamKey");
	};

/*-----------------------------------------------------------------------------
   Coordinates
-----------------------------------------------------------------------------*/

void cameraShowCoords (ICamera *CameraInfo)
	{
	FVector			Location;
	FRotation		Rotation;
	UModel			*Brush;
	IModel		BrushInfo;
	AActor			*Actor;
	int				ModeClass,X,Y,State,i,n;
	char			XStr[40],YStr[40],ZStr[40],Descr[40],Fail[40];
	//
	GUARD;
	ModeClass 	= GUnrealEditor.edcamModeClass (GUnrealEditor.Mode);
	State       = CameraInfo->Level.State;
	Fail[0]     = 0;
	//
	if ((CameraInfo->ShowFlags&SHOW_PlayerCtrl) || (State==LEVEL_UpPlay))
		{
		sprintf (Descr,"%s",CameraInfo->Actor->Class->Name);
		if (!CameraInfo->Actor->Name.IsNone())
			{
			sprintf (Descr+strlen(Descr)," %s",CameraInfo->Actor->Name.Name());
			};
		Location = CameraInfo->Actor->Location;
		}
	else if (ModeClass==EMC_Actor) // Location of first selected actor
		{
		Actor = &CameraInfo->Level.Actors->Element(0);
		n     = CameraInfo->Level.Actors->Max;
		//
		for (i=0; i<n; i++)
			{
			if (Actor->Class && Actor->bSelected)
				{
				sprintf (Descr,"%s",Actor->Class->Name);
				if (!Actor->Name.IsNone())
					{
					sprintf (Descr+strlen(Descr)," %s",Actor->Name.Name());
					};
				Location = Actor->Location;
				Rotation = Actor->DrawRot;
				goto Found;
				};
			Actor++;
			};
		strcpy (Descr,"Actor");
		strcpy (Fail, "None selected");
		}
	else  
		{ 
		if (GUnrealEditor.MapEdit) // Location of first selected brush
			{
			n = CameraInfo->Level.BrushArray->Num;
			//
			for (i=0; i<n; i++)
				{
				Brush = CameraInfo->Level.BrushArray->Element(i);
				if (Brush->ModelFlags & MF_Selected)
					{
					Location = Brush->Location;
					Rotation = Brush->Rotation;
					strcpy (Descr,GUnrealEditor.csgGetName (Brush->CsgOper));
					goto Found;
					};
				};
			};
		//
		// Location of default brush:
		//
		CameraInfo->Level.Brush->Lock (&BrushInfo,LOCK_Read);
		strcpy (Descr,GUnrealEditor.csgGetName (BrushInfo.CsgOper));
		//
		Location = BrushInfo.Location;
		Rotation = BrushInfo.Rotation;
		GUnrealEditor.constraintApply (&CameraInfo->Level.ModelInfo,&BrushInfo,&Location,&Rotation,&GUnrealEditor.Constraints);
		//
		CameraInfo->Level.Brush->Unlock (&BrushInfo);
		};
	Found:
	//
	sprintf (XStr,"X=%05i",(int)Location.X);
	sprintf (YStr,"Y=%05i",(int)Location.Y);
	sprintf (ZStr,"Z=%05i",(int)Location.Z);
	//
	X = 12;
	Y = CameraInfo->SYR - 24;
	//
	GGfx.Printf (CameraInfo->Texture,X+ 0 ,Y,0,GGfx.MedFont,3,Descr);
	//
	if (Fail[0]!=0)
		{
		GGfx.Printf (CameraInfo->Texture,X+100,Y,0,GGfx.MedFont,3,Fail);
		}
	else
		{
		GGfx.Printf (CameraInfo->Texture,X+105,Y,0,GGfx.MedFont,3,XStr);
		GGfx.Printf (CameraInfo->Texture,X+170,Y,0,GGfx.MedFont,3,YStr);
		GGfx.Printf (CameraInfo->Texture,X+235,Y,0,GGfx.MedFont,3,ZStr);
		};
	UNGUARD("cameraShowCoords");
	};

/*-----------------------------------------------------------------------------
   Texture browser routines
-----------------------------------------------------------------------------*/

void DrawViewerBackground(ICamera *CameraInfo)
	{
	GUARD;
	UTexture		*Texture		= GGfx.BkgndTexture;
	FColor			*Palette		= Texture->Palette->GetData();
	BYTE			*Dest1			= CameraInfo->Screen;
	BYTE			*TextureBits;
	BYTE			*Src,*Dest;
	int				USize,VSize,UMask,VMask,U,V,MipLevel=0;
	//
	TextureBits = GGfx.BkgndTexture->GetOriginalData(&MipLevel,&USize,&VSize);
	UMask		= USize-1;
	VMask		= VSize-1;
	for (V=0; V<CameraInfo->SYR; V++)
		{
		Src  = &TextureBits[(V&VMask)*Texture->USize];
		Dest = Dest1;
		for (U=0; U<CameraInfo->SXR; U++)
			{
			*Dest++ = Palette[Src[U&UMask]].RemapIndex-0x10;
			};
		Dest1 += CameraInfo->SXStride;
		};
	UNGUARD("DrawViewerBackground");
	};

void DrawTextureRect(ICamera *CameraInfo,UTexture *Texture,int X,int Y,int Size,int DoText,int Highlight)
	{
	GUARD;
	if (!Texture->Palette) appError("No palette");
	FColor			*Palette = Texture->Palette->GetData();
	BYTE			VShift,*Dest1,*Dest,*TextureBits,B;
	int				OrigY=Y,U,U1=1,V=0,XL=Size,YL=Size,i,j,VOfs,USize,VSize,MipLevel=0,UMask,VMask,D;
	char			Temp[80];
	//
	if (Texture->bNoTile)
		{
		DrawTextureRect(CameraInfo,GGfx.BadTexture,X,Y,Size,0,0);
		}
	else
		{
		TextureBits = Texture->GetOriginalData(&MipLevel,&USize,&VSize);
		VMask		= VSize-1;
		UMask		= USize-1;
		VShift		= FLogTwo(USize);
		D			= FIX(OurMax(USize,VSize))/Size;
		//
		if (Y<0) {V -=Y*D; YL+=Y; Y=0;};
		if (X<0) {U1-=X*D; XL+=X; X=0;};
		if ((Y+YL)>CameraInfo->SYR) YL = CameraInfo->SYR-Y;
		if ((X+XL)>CameraInfo->SXR) XL = CameraInfo->SXR-X;
		//
		Dest1 = &CameraInfo->Screen[X + Y*CameraInfo->SXR];
		for (i=0; i<YL; i++)
			{
			Dest = Dest1;
			U    = U1;
			VOfs = (UNFIX(V) & VMask) << VShift;
			if (!(Texture->PolyFlags & PF_Masked)) for (j=0; j<XL; j++)
				{
				*(Dest++) = Palette[TextureBits [VOfs + (UNFIX(U)&UMask)]].RemapIndex;
				U += D;
				}
			else for (j=0; j<XL; j++)
				{
				B=TextureBits [VOfs + (UNFIX(U)&UMask)];
				if (B) *Dest = Palette[B].RemapIndex;
				Dest++;
				U += D;
				};
			V     += D;
			Dest1 += CameraInfo->SXR;
			};
		};
	if (DoText)
		{
		strcpy(Temp,Texture->Name);
		if (Size>=128) sprintf(Temp+strlen(Temp)," (%ix%i)",Texture->USize,Texture->VSize);
		GGfx.StrLen (&XL,&YL,(Size>=128)?-1:0,0,GGfx.MedFont,Temp);
		X = X+(Size-XL)/2;
		GGfx.Printf (CameraInfo->Texture,X,OrigY+Size+1,-1,GGfx.MedFont,3,Temp);
		};
	UNGUARD("DrawTextureRect");
	};

int __cdecl ResNameCompare(const void *A, const void *B)
	{
	return stricmp((*(UResource **)A)->Name,(*(UResource **)B)->Name);
	};

void DrawTextureBrowser(ICamera *CameraInfo)
	{
	GUARD;
	UTexture *Texture,**List;
	int i,n,Size,PerRow,Space,VSkip,YOfs,YL,All,X,Y;
	//
	if (CameraInfo->Camera->MiscName.IsNone()) return;
	All = !stricmp(CameraInfo->Camera->MiscName.Name(),"All");
	//
	List   = (UTexture **)GMem.Get(32768*sizeof(UTexture *));
	Size   = CameraInfo->Actor->CameraStatus.Misc1;
	PerRow = CameraInfo->SXR/Size;
	Space  = (CameraInfo->SXR - Size*PerRow)/(PerRow+1);
	if (Size>=64) VSkip=10;
	else VSkip=0;
	//
	n=0;
	FOR_ALL_TYPED_RES(Texture,RES_Texture,UTexture)
		{
		if ((All && (!Texture->FamilyName.IsNone())) || (Texture->FamilyName==CameraInfo->Camera->MiscName))
			{
			List[n++] = Texture;
			};
		}
	END_FOR_ALL_TYPED_RES;
	//
	qsort(&List[0],n,sizeof(UTexture *),ResNameCompare);
	//
	YL = Space+(Size+Space+VSkip)*((n+PerRow-1)/PerRow);
	if (YL>0)
		{
		YOfs = -((CameraInfo->Actor->CameraStatus.Misc2*CameraInfo->SYR)/512);
		for (i=0; i<n; i++)
			{
			X = (Size+Space)*(i%PerRow);
			Y = (Size+Space+VSkip)*(i/PerRow)+YOfs;
			//
			if (((Y+Size+Space+VSkip)>0) && (Y<CameraInfo->SYR))
				{
				if (GUnrealEditor.Scan.Active) GUnrealEditor.Scan.PreScan();
				//
				if (List[i]==GUnrealEditor.CurrentTexture)
					{
					GGfx.BurnRect(CameraInfo->Texture,X+1,X+Size+Space*2-2,Y+1,Y+Size+Space*2+VSkip-2,1);
					};
				DrawTextureRect(CameraInfo,List[i],X+Space,Y+Space,Size,(Size>=64),0);
				//
				if (GUnrealEditor.Scan.Active) GUnrealEditor.Scan.PostScan(EDSCAN_BrowserTex,(int)List[i],0,0,&GMath.ZeroVector);
				};
			};
		};
	GMem.Release(List);
	//
	GLastScroll = OurMax(0,(512*(YL-CameraInfo->SYR))/CameraInfo->SYR);
	UNGUARD("DrawTextureBrowser");
	};

/*-----------------------------------------------------------------------------
   Camera frame drawing
-----------------------------------------------------------------------------*/

//
// Draw an onscreen mouseable button.
//
int DrawButton (ICamera *CameraInfo,UTexture *Texture,int X, int Y, int ScanCode)
	{
	BYTE *Data;
	int USize,VSize,MipLevel=0;
	//
	if (GUnrealEditor.Scan.Active) GUnrealEditor.Scan.PreScan();
	Data = Texture->GetOriginalData(&MipLevel,&USize,&VSize);
	GRend->DrawSprite (CameraInfo,Data,USize,VSize,X,Y,0,BT_Normal,NULL,0);
	if (GUnrealEditor.Scan.Active) GUnrealEditor.Scan.PostScan(EDSCAN_UIElement,ScanCode,0,0,&GMath.ZeroVector);
	//
	return USize+2;
	};

//
// Draw the camera view:
//
void FEditor::edcamDraw (UCamera *Camera, int DoScan)
	{
	AActor				*Actor;
	ICamera 			CameraInfo;
	FVector				OriginalLocation;
	FRotation			OriginalRotation;
	PCalcView			ViewInfo;
	IMeshMap			MeshMapInfo;
	DWORD				ShowFlags;
	char				Temp[80];
	int					InvalidBsp,ButtonX,Mode,ModeClass,RealClass;
	static DWORD		LastTicks=0;
	GUARD;
	//
	// Lock the camera:
	//
	if (!Camera->Lock(&CameraInfo))
		{
		debug (LOG_Ed,"Couldn't lock camera for drawing");
		return;
		};
	//
	// Init scanner if desired:
	//
	if (DoScan) Scan.Init(&CameraInfo); // Caller must set Scan.X, Scan.Y.
	else Scan.Active = 0;
	//
	GRend->PreRender(&CameraInfo);
	CameraInfo.Camera->Console->PreRender(&CameraInfo);
	GGfx.PreRender(&CameraInfo);
	//
	GRend->RendIter++;
	//
	Actor					= CameraInfo.Actor;
	ShowFlags				= Actor->CameraStatus.ShowFlags;
	//
	switch (Actor->CameraStatus.RendMap)
		{
		case REN_TexView:
			Actor->bHiddenEd = 1;
			Actor->bHidden   = 1;
			DrawViewerBackground(&CameraInfo);
			DrawTextureRect(&CameraInfo,(UTexture *)CameraInfo.Camera->MiscRes,4,4,128,128,1);
			goto Out;
		case REN_TexBrowser:
			Actor->bHiddenEd = 1;
			Actor->bHidden   = 1;
			DrawViewerBackground(&CameraInfo);
			DrawTextureBrowser(&CameraInfo);
			goto Out;
		case REN_MeshView:
			{
			GGfx.Clearscreen(&CameraInfo,0xe8);
			//
			Actor->Location = CameraInfo.Coords.ZAxis * (-Actor->Location.Size());
			CameraInfo.BuildCoords();
			//
			OriginalLocation	= Actor->Location;
			OriginalRotation	= Actor->ViewRot;
			Actor->Location		= GMath.ZeroVector;		
			Actor->bHiddenEd	= 1;
			Actor->bHidden		= 1;
			Actor->DrawType		= DT_MeshMap;
			Actor->MeshMap		= (UMeshMap *)CameraInfo.Camera->MiscRes;
			Actor->AnimSeq		= Actor->CameraStatus.Misc1;
			Actor->bCollideWorld = 0;
			Actor->bCollideActors = 0;
			//
			UMeshMap *MeshMap = (UMeshMap*)CameraInfo.Camera->MiscRes;
			MeshMap->Lock(&MeshMapInfo);
			//
			if (ShowFlags&SHOW_Brush) Actor->DrawRot.Yaw+=OurMin(4,(int)(GServer.Ticks-LastTicks))*256;
			LastTicks=GServer.Ticks;
			//
			if (ShowFlags&SHOW_Backdrop) Actor->AnimBase=(GServer.Ticks/2) % MeshMapInfo.AnimSeqs[Actor->AnimSeq].SeqNumFrames;
			else Actor->AnimBase=Actor->CameraStatus.Misc2;
			//
			if		(ShowFlags&SHOW_Frame)		CameraInfo.RendMap = REN_Wire;
			else if	(ShowFlags&SHOW_Coords)		CameraInfo.RendMap = REN_Polys;
			else								CameraInfo.RendMap = REN_PlainTex;
			//
			MeshMap->Unlock(&MeshMapInfo);
			//
			GRend->DrawActor (&CameraInfo,CameraInfo.iActor);
			sprintf
				(
				Temp,"%s, Seq %i, Frame %i",
				CameraInfo.Camera->MiscRes->Name,CameraInfo.Actor->CameraStatus.Misc1,
				(int)Actor->AnimBase
				);
			GGfx.Printf (CameraInfo.Texture,4,CameraInfo.SYR-12,-1,GGfx.MedFont,3,Temp);
			//
			CameraInfo.RendMap	= REN_MeshView;
			Actor->Location		= OriginalLocation;
			Actor->DrawType		= DT_None;
			goto Out;
			};
		case REN_MeshBrowser:
			DrawViewerBackground(&CameraInfo);
			goto Out;
		};
	Mode 		= edcamMode(Camera);
	ModeClass	= edcamModeClass(Mode);
	RealClass   = edcamModeClass(Mode);
	//
	if (CameraInfo.Camera->IsOrtho()||(Actor->CameraStatus.ShowFlags & SHOW_NoCapture)) Actor->bHiddenEd=1;
	else Actor->bHiddenEd=0;
	//
	if (CameraInfo.ShowFlags & SHOW_PlayerCtrl)
		{
		OriginalLocation = Actor->Location;
		OriginalRotation = Actor->ViewRot;
		//
		ViewInfo.Coords			= &CameraInfo.Coords;
		ViewInfo.Uncoords		= &CameraInfo.Uncoords;
		ViewInfo.ViewLocation	= Actor->Location;
		ViewInfo.ViewRotation	= Actor->ViewRot;
		CameraInfo.Level.SendMessage(CameraInfo.iActor,ACTOR_PlayerCalcView,&ViewInfo);
		Actor->Location			= ViewInfo.ViewLocation;
		Actor->ViewRot			= ViewInfo.ViewRotation;
		//
		// Rebuild coordinate system:
		//
		CameraInfo.BuildCoords();
		//
		Actor->Location = OriginalLocation;
		Actor->ViewRot  = OriginalRotation;
		};
	//
	// Handle case if Bsp is invalid:
	//
	InvalidBsp = (CameraInfo.Level.ModelInfo.ModelFlags & MF_InvalidBsp) &&
		!(CameraInfo.Camera->IsOrtho() || CameraInfo.Camera->IsRealWire());
	//
	// Init stats:
	//
	#ifdef STATS
	mymemset (GRend->Stat,0,sizeof(FRenderStats));
	GRend->Stat->LastEndTime   = GRend->Stat->ThisEndTime;
	GRend->Stat->ThisStartTime = GApp->TimeMSec();
	#endif
	//
	// Draw background:
	//
	if (CameraInfo.Camera->IsOrtho() || CameraInfo.Camera->IsWire() || 
		CameraInfo.Camera->IsInvalidBsp() || !(CameraInfo.ShowFlags & SHOW_Backdrop))
		{
		if (CameraInfo.Camera->IsOrtho())	GGfx.Clearscreen(&CameraInfo,WireBackground);
		else								GGfx.Clearscreen(&CameraInfo,BlackColor);
		GRend->DrawWireBackground(&CameraInfo);
		};
	//
	// Draw the level:
	//
	if (CameraInfo.Camera->IsOrtho() || CameraInfo.Camera->IsWire() || InvalidBsp)
		{
		GRend->DrawLevelBrushes(&CameraInfo);
		if (CameraInfo.ShowFlags & SHOW_Actors) GRend->DrawLevelActors (&CameraInfo,CameraInfo.iActor);
		if (CameraInfo.ShowFlags & SHOW_MovingBrushes) GRend->DrawMovingBrushWires(&CameraInfo);
		}
	else GRend->DrawWorld(&CameraInfo);
	//
	if (CameraInfo.ShowFlags & SHOW_Brush)
		{
		if ((ModeClass==EMC_Camera)||(ModeClass==EMC_Brush))
			{
			GRend->DrawActiveBrush(&CameraInfo);
			if (GUnrealEditor.Mode==EM_BrushSnap)
				{
				if (!CameraInfo.Level.Brush->Bound[1].Min.iTransform)
					{
					CameraInfo.Level.Brush->BuildBound(1);
					};
				constrainSimplePoint(&CameraInfo.Level.Brush->Bound[1].Min,&Constraints);
				constrainSimplePoint(&CameraInfo.Level.Brush->Bound[1].Max,&Constraints);
				//
				GRend->DrawBoundingVolume(&CameraInfo,&CameraInfo.Level.Brush->Bound[1]);
				};
			};
		};
	//
	// Draw status:
	//
	if (CameraInfo.ShowFlags & SHOW_Coords) cameraShowCoords(&CameraInfo);
	if (InvalidBsp)
		{
		GGfx.Printf (CameraInfo.Texture,8,20,0,GGfx.MedFont,3,"Map view - Geometry must be rebuilt");
		};
	ButtonX=2;
	if ((CameraInfo.Level.Level->GetState()==LEVEL_UpEdit) &&
		(!GCameraManager->FullscreenCamera) &&
		(!(CameraInfo.ShowFlags & SHOW_NoButtons)))
		{
		ButtonX +=
			DrawButton(&CameraInfo,(CameraInfo.ShowFlags&SHOW_Menu)?GGfx.MenuUp:GGfx.MenuDn,ButtonX,2,1);
		if (!CameraInfo.Camera->IsOrtho()) ButtonX +=
			DrawButton (&CameraInfo, (CameraInfo.ShowFlags&SHOW_PlayerCtrl)?GGfx.PlyrOn:GGfx.PlyrOff,
			ButtonX,2,2);
		};
	Out:
	//
	GGfx.PostRender(&CameraInfo);
	CameraInfo.Camera->Console->PostRender(&CameraInfo,ButtonX);
	GRend->PostRender(&CameraInfo);
	//
	Scan.Exit();
	Camera->Unlock (&CameraInfo,!DoScan);
	//
	UNGUARD_BEGIN
	UNGUARD_MSGF("edcamDraw(Cam=%s,Mode=%i,Flags=%i",Camera->Name,Mode,ShowFlags);
	UNGUARD_END
	};

/*-----------------------------------------------------------------------------
   Brush pivot setting
-----------------------------------------------------------------------------*/

//
// Set the brush's pivot point to a location specified in world coordinates.
// This transforms the point into the brush's coordinate system and takes
// care of all the thorny details.
//
void SetWorldPivotPoint (ILevel *LevelInfo,UModel *Brush,FVector *PivotLocation,int SnapPivotToGrid)
	{
	GUARD;
	if (GUnrealEditor.MapEdit)
		{
		int n = LevelInfo->BrushArray->Num;
		for (int i=0; i<n; i++)
			{
			Brush = LevelInfo->BrushArray->Element(i);
			if ((Brush->ModelFlags&MF_Selected) || (i==0))
				{
				Brush->SetPivotPoint (PivotLocation,SnapPivotToGrid);
				};
			};
		}
	else if (Brush)
		{
		Brush->SetPivotPoint (PivotLocation,SnapPivotToGrid);
		}
	else LevelInfo->Brush->SetPivotPoint (PivotLocation,SnapPivotToGrid);
	UNGUARD("SetWorldPivotPoint");
	};

/*-----------------------------------------------------------------------------
   Camera mouse click handling
-----------------------------------------------------------------------------*/

//
// Handle a mouse click in the camera window
//
void FEditor::edcamClick (UCamera *Camera, BYTE Buttons, SWORD MouseX, SWORD MouseY,
	int Shift, int Ctrl)
	{
	GUARD;
	POLY_CALLBACK	Callback;
	ICamera 		CameraInfo;
	IModel 			*ModelInfo;
	UModel			*Brush;
	FBspNode	  	*Node;
	FBspSurf		*Poly;
	FVector			V;
	FPoly			*MasterFPoly;
	AActor			*Actor;
	INDEX			iActor;
	int				i,UpdateWindow=0;
	int				Mode,ModeClass;
	char			Temp[80];
	//
	GUnrealEditor.Scan.X = MouseX;
	GUnrealEditor.Scan.Y = MouseY;
	//
	Camera->Draw (1);
	//
	Mode      = GUnrealEditor.edcamMode(Camera);
	ModeClass = GUnrealEditor.edcamModeClass(Mode);
	//
	if (!Camera->Lock(&CameraInfo)) return;
	ModelInfo = &CameraInfo.Level.ModelInfo;
	//
	// Check scan results:
	//
	if ((Mode == EM_AddActor) && (Buttons == BUT_LEFT) && (GUnrealEditor.Scan.Type != EDSCAN_Actor) && GClasses.AddClass)
		{
		if (GClasses.AddClass->DefaultActor.bTemplateClass)
			{
			char Temp[256];
			sprintf(Temp,"Class %s is a base class.  You can't add actors of this type to the world.",GClasses.AddClass->Name);
			GApp->MessageBox(Temp,"Can't add actor",0);
			}
		else if (GRend->Deproject(&CameraInfo,MouseX,MouseY,&V,1,GClasses.AddClass->DefaultActor.CollisionRadius))
			{
			GTrans->Begin         (CameraInfo.Level.Level,"Add Actor");
			GTrans->NoteResHeader (CameraInfo.Level.Actors);
			//
			if (!GClasses.AddClass) GClasses.AddClass = GClasses.Light;
			iActor = CameraInfo.Level.SpawnActor(GClasses.AddClass,NAME_NONE,&V);
			//
			if (iActor != INDEX_NONE)
				{
				AActor *Actor				= &CameraInfo.Level.Actors->Element(iActor);
				Actor->bTempDynamicLight	= 1;
				if (CameraInfo.Level.FarMoveActor(iActor,&V))
					{
					debug (LOG_Ed,"Added actor successfully");
					}
				else
					{
					debug (LOG_Ed,"Actor doesn't fit there");
					CameraInfo.Level.DestroyActor(iActor);
					};
				if (GClasses.AddClass->DefaultActor.Brush)
					{
					Actor->Brush = csgDuplicateBrush(Camera->Level,GClasses.AddClass->DefaultActor.Brush,0,0);
					Actor->UpdateBrushPosition(&CameraInfo.Level,iActor,1);
					};
				};
			GTrans->End();
			};
		}
	else switch (GUnrealEditor.Scan.Type)
		{
		case EDSCAN_BspNodePoly:
			GTrans->Begin (CameraInfo.Level.Level,"Poly Click");
			//
			Node 			= &ModelInfo->BspNodes [GUnrealEditor.Scan.Index];
			Poly 			= &ModelInfo->BspSurfs [Node->iSurf];
			MasterFPoly		= GUnrealEditor.polyFindMaster (ModelInfo,Node->iSurf);
			//
			if (Mode == EM_TextureSet)
				{
				if (Buttons == BUT_RIGHT) // Get
					{
					GUnrealEditor.CurrentTexture = Poly->Texture;
					GApp->EdCallback(EDC_CurTexChange,0);
					}
				else // Set
					{
					if (GApp->KeyDown[K_SHIFT]) // Set to all selected
						{
						GTrans->NoteSelectedBspSurfs (ModelInfo,1);
						for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
							{
							if (ModelInfo->BspSurfs[i].PolyFlags & PF_Selected)
								{
								ModelInfo->BspSurfs[i].Texture = GUnrealEditor.CurrentTexture;
								GUnrealEditor.polyUpdateMaster (ModelInfo,i,0,0);
								};
							};
						}
					else // Set to the one polygon clicked on
						{
						GTrans->NoteBspSurf 	(ModelInfo,Node->iSurf,0);
						Poly->Texture = GUnrealEditor.CurrentTexture;
						GUnrealEditor.polyUpdateMaster(ModelInfo,Node->iSurf,0,0);
						};
					};
				}
			else
				{
				if (Buttons==BUT_RIGHT) 
					{
					// If nothing is selected, select just this:
					if (!(Poly->PolyFlags & PF_Selected)) GUnrealEditor.polySetAndClearPolyFlags (ModelInfo, 0, PF_Selected,0,0);
					GUnrealEditor.polyFindByBrush(ModelInfo,Poly->Brush,Poly->iBrushPoly,SelectPolyFunc);
					// Tell editor to bring up the polygon properties dialog:
					GApp->EdCallback(EDC_SelPolyChange,0);
					GApp->EdCallback(EDC_RtClickPoly,0);
					}
				else
					{
					if (Poly->PolyFlags & PF_Selected)  Callback = DeselectPolyFunc;
					else								Callback = SelectPolyFunc;
					//
					if (!GApp->KeyDown[K_CTRL]) GUnrealEditor.polySetAndClearPolyFlags (ModelInfo, 0, PF_Selected,0,0);
					GUnrealEditor.polyFindByBrush(ModelInfo,Poly->Brush,Poly->iBrushPoly,Callback);
					//
					GApp->EdCallback(EDC_SelPolyChange,0);
					};
				}
			GTrans->End ();
			break;
		case EDSCAN_BspNodeSide:
			break;
		case EDSCAN_BspNodeVertex:
			break;
		case EDSCAN_BrushPoly:
		case EDSCAN_BrushSide:
			if (GUnrealEditor.MapEdit)
				{
				GTrans->Begin  (CameraInfo.Level.Level,"Select brush");
				//
				if (Buttons==BUT_RIGHT)
					{
					for (i=0; i<CameraInfo.Level.BrushArray->Num; i++)
						{
						Brush = CameraInfo.Level.BrushArray->Element(i);
						if (Brush->ModelFlags & MF_Selected)
							{
							GTrans->NoteResHeader(Brush);
							Brush->ModelFlags &= ~MF_Selected;
							};
						};
					};
				Brush = (UModel *)GUnrealEditor.Scan.Index;
				GTrans->NoteResHeader (Brush);
				//
				Brush->ModelFlags ^= MF_Selected;
				//
				GTrans->End ();
				};
			break;
		case EDSCAN_BrushVertex:
			{
			UModel *Brush = (UModel *)GUnrealEditor.Scan.Index;
			GTrans->Begin		(CameraInfo.Level.Level,"Brush Vertex Selection");
			SetWorldPivotPoint	(&CameraInfo.Level,Brush,&GUnrealEditor.Scan.V,(Buttons==BUT_RIGHT));
			GTrans->End			();
			//
			// If this was a moving brush, update its actor accordingly:
			//
			AActor *Actor = &CameraInfo.Level.Actors->Element(0);
			for (int i=0; i<CameraInfo.Level.Actors->Max; i++)
				{
				if (Actor->Brush==Brush) Actor->UpdateBrushPosition(&CameraInfo.Level,i,1);
				Actor++;
				};
			};
			break;
		case EDSCAN_Actor:
			iActor	= GUnrealEditor.Scan.Index;
			Actor	= &CameraInfo.Level.Actors->Element(iActor);
			//
			if (Actor->Class != GClasses.Camera)
				{
				GTrans->Begin(CameraInfo.Level.Level,"clicking on actors");
				if (Buttons==BUT_RIGHT)
					{
					if (!Actor->bSelected) GUnrealEditor.edactSelectNone(&CameraInfo.Level);
					GTrans->NoteActor (CameraInfo.Level.Actors,iActor);
					Actor->bSelected=1;
					GApp->EdCallback(EDC_SelActorChange,0);
					GApp->EdCallback(EDC_RtClickActor,0);
					}
				else
					{
					if (GApp->KeyDown[K_ALT]) // Set default add-class
						{
						GClasses.AddClass = Actor->Class;
						GApp->EdCallback(EDC_CurClassChange,0);
						}
					else // Select/deselect actor
						{
						if (!GApp->KeyDown[K_CTRL]) GUnrealEditor.edactSelectNone(&CameraInfo.Level);
						GTrans->NoteActor (CameraInfo.Level.Actors,iActor);
						Actor->bSelected ^= 1;
						GApp->EdCallback(EDC_SelActorChange,0);
						};
					};
				GTrans->End();
				};
			break;
		case EDSCAN_UIElement:
			switch(GUnrealEditor.Scan.Index)
				{
				case 1: // Menu on/off button
					CameraInfo.ShowFlags ^= SHOW_Menu;
					UpdateWindow=1;
					break;
				case 2: // Player controls
					CameraInfo.ShowFlags ^= SHOW_PlayerCtrl;
					CameraInfo.Camera->Console->Logf(LOG_Info,"Player controls are %s",
						CameraInfo.ShowFlags&SHOW_PlayerCtrl ? "On" : "Off");
					GCameraManager->ResetModes();
					break;
				}; 
			break;
		case EDSCAN_BrowserTex:
			Camera->Unlock(&CameraInfo,0);
			if (Buttons==BUT_LEFT)
				{
				strcpy(Temp,"POLY DEFAULT TEXTURE="); strcat(Temp,((UResource *)GUnrealEditor.Scan.Index)->Name);
				GUnrealEditor.Exec (Temp);
				strcpy(Temp,"POLY SET TEXTURE="); strcat(Temp,((UResource *)GUnrealEditor.Scan.Index)->Name);
				GUnrealEditor.Exec (Temp);
				GApp->EdCallback(EDC_CurTexChange,0);
				}
			else if (Buttons==BUT_RIGHT)
				{
				GUnrealEditor.CurrentTexture=(UTexture *)GUnrealEditor.Scan.Index;
				GApp->EdCallback(EDC_RtClickTexture,0);
				};
			if (!Camera->Lock(&CameraInfo)) return;
			break;
		case EDSCAN_None:
			if (Buttons==BUT_RIGHT) GApp->EdCallback(EDC_RtClickWindow,0);
			break;
		};
	Camera->Unlock(&CameraInfo,0);
	//
	if (UpdateWindow) Camera->UpdateWindow();
	//
	UNGUARD_BEGIN
	UNGUARD_MSGF("edcamClick(Mode=%i)",GUnrealEditor.Mode);
	UNGUARD_END
	};

/*-----------------------------------------------------------------------------
   Editor camera mode
-----------------------------------------------------------------------------*/

//
// Set the editor mode.
//
void FEditor::edcamSetMode (int Mode)
	{
	int i;
	//
	// Clear old mode:
	//
	GUARD;
	if (GUnrealEditor.Mode != EM_None)
		{
		for (i=0; i<GCameraManager->CameraArray->Num; i++)
			{
			edcamMove (GCameraManager->CameraArray->Element(i),BUT_EXITMODE,0,0,0,0);
			};
		};
	//
	// Set new mode:
	//
	GUnrealEditor.Mode = Mode;
	if (GUnrealEditor.Mode != EM_None)
		{
		for (i=0; i<GCameraManager->CameraArray->Num; i++)
			{
			edcamMove (GCameraManager->CameraArray->Element(i),BUT_SETMODE,0,0,0,0);
			};
		};
	UNGUARD("edcamSetMode");
	};

//
// Return editor camera mode given GUnrealEditor.Mode and state of keys.
// This handlers special keyboard mode overrides which should
// affect the appearance of the mouse cursor, etc.
//
int FEditor::edcamMode (UCamera *Camera)
	{
	GUARD;
	if (Camera) switch (Camera->GetActor().CameraStatus.RendMap)
		{
		case REN_TexView:							return EM_TexView;
		case REN_TexBrowser:						return EM_TexBrowser;
		case REN_MeshView:							return EM_MeshView;
		case REN_MeshBrowser:						return EM_MeshBrowser;
		};
	switch (GUnrealEditor.Mode)
		{
		case EM_None:
			return GUnrealEditor.Mode;
		case EM_CameraMove:
		case EM_CameraZoom:
			if (GApp->KeyDown[K_ALT]) 				return EM_TextureSet;
			else if (GApp->KeyDown[K_CTRL])			return EM_BrushMove;
			else if (GApp->KeyDown[K_SHIFT])		return EM_BrushMove;
			else									return GUnrealEditor.Mode;
			break;
		case EM_BrushFree:
		case EM_BrushMove:
		case EM_BrushRotate:
		case EM_BrushSheer:
		case EM_BrushScale:
		case EM_BrushStretch:
		case EM_BrushWarp:
		case EM_BrushSnap:
			if		(GApp->KeyDown[K_ALT])  		return EM_TextureSet;
			else if (GApp->KeyDown[K_CTRL])			return GUnrealEditor.Mode;
			else if (GApp->KeyDown[K_SHIFT])		return EM_BrushMove;
			else									return EM_CameraMove;
			break;
		case EM_AddActor:
		case EM_MoveActor:
			return GUnrealEditor.Mode;
			break;
		case EM_TextureSet:
			if (GApp->KeyDown[K_CTRL])				return EM_CameraMove;
			else									return GUnrealEditor.Mode;
		case EM_TexturePan:
		case EM_TextureRotate:
		case EM_TextureScale:
			if (GApp->KeyDown[K_ALT])  				return EM_TextureSet;
			else									return GUnrealEditor.Mode;
			break;
		case EM_Terraform:
			return GUnrealEditor.Mode;
			break;
		default:
			return GUnrealEditor.Mode;
		};
	UNGUARD("edcamMode");
	};

//
// Return classification of current mode:
//
int FEditor::edcamModeClass (int Mode)
	{
	switch (Mode)
		{
		case EM_CameraMove:
		case EM_CameraZoom:
			return EMC_Camera;
		case EM_BrushFree:
		case EM_BrushMove:
		case EM_BrushRotate:
		case EM_BrushSheer:
		case EM_BrushScale:
		case EM_BrushStretch:
		case EM_BrushSnap:
		case EM_BrushWarp:
			return EMC_Brush;
		case EM_AddActor:
		case EM_MoveActor:
			return EMC_Actor;
		case EM_TextureSet:
		case EM_TexturePan:
		case EM_TextureRotate:
		case EM_TextureScale:
			return EMC_Texture;
		case EM_Terraform:
			return EMC_Terrain;
		case EM_None:
			default:
			return EMC_None;
		};
	};

/*-----------------------------------------------------------------------------
	Ed link topic function
-----------------------------------------------------------------------------*/

AUTOREGISTER_TOPIC("Ed",EdTopicHandler);
void EdTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	GUARD;
	//
	if		(!stricmp(Item,"LASTSCROLL"))	itoa (GLastScroll,Data,10);
	else if (!stricmp(Item,"CURTEX"))		strcpy(Data,GUnrealEditor.CurrentTexture ? GUnrealEditor.CurrentTexture->Name : "None");
	else if (!stricmp(Item,"CURCLASS"))		strcpy(Data,GClasses.AddClass ? GClasses.AddClass->Name : "None");
	//
	UNGUARD("EdTopicHandler::Get");
	};
void EdTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
	{
	GUARD;
	UNGUARD("EdTopicHandler::Set");
	};

/*-----------------------------------------------------------------------------
	Ed link topic function
-----------------------------------------------------------------------------*/
