/*=============================================================================
	UnEdSrv.cpp: FEditor implementation, the Unreal editing server

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
#include "UnRender.h"

#pragma DISABLE_OPTIMIZATION /* Not performance-critical */

//
// Global hacks:
//
void UNREAL_API AudioCmdLine(const char *Str);
void UNREAL_API resQueryForLink(UResource *Res,EResourceType Type); // Resource ID or MAXWORD-2=Root, MAXWORD-1=All
void UNREAL_API classQueryForLink(UResource *Res); // Class ID or NULL-2=Root, NULL-1=All
void texQueryTextureForLink(char *FamilyName);
void texQueryFamilyForLink(int All);

/*-----------------------------------------------------------------------------
	UnrealEd command line
-----------------------------------------------------------------------------*/

//
// Process an incoming network message meant for the editor server
//
int FEditor::Exec(const char *Stream,FOutputDevice *Out)
	{
	char ErrorTemp[256]="Setup: ";
	//
	GUARD;
	ULevel			*Level;
	ILevel			LevelInfo;
	IModel 			ModelInfo;
	FVector 		TempVector;
	FRotation		TempRotation;
	UModel			*Brush,*LevelModel;
	WORD	 		Word1,Word2,Word3,Word4;
	INDEX			Index1;
	ETexAlign		TexAlign;
	EResourceType	ResType;
	ECsgOper		CsgType;
	char	 		TempStr[256],TempStr1[256],TempFname[256],TempName[256];
	int				SaveAsMacro,Mode,ModeClass,DWord1,DWord2;
	int				Processed=0;
	//
	if (strlen(Stream)<200) strcat(ErrorTemp,Stream);
	//
	UPlayer *Player = new("Editor",FIND_Optional)UPlayer;
	if (Player) Level = Player->Level;
	else if (GServer.Levels->Num) Level = GServer.Levels->Element(0);
	else Level=NULL;
	//
	if (Level)
		{
		Brush		= Level->BrushArray->Element(0);
		LevelModel	= Level->Model;
		};
	Mode			= edcamMode(NULL);
	ModeClass		= edcamModeClass(Mode);
	//
	SaveAsMacro = (MacroRecBuffer != NULL);
	//
	char Temp[256];
	mystrncpy(Temp,Stream,256);
	const char *Str = &Temp[0];
	if (strchr(Str,';')) *strchr(Str,';') = 0; // Kill comments
	//
	strncpy(ErrorTemp,Str,79);
	ErrorTemp[79]=0;
	//
	//------------------------------------------------------------------------------------
	// STATUS
	//
	if (GetCMD(&Str,"STATUS"))
		{
		if (GetCMD(&Str,"EDITOR") || !Str[0])
			{
			Out->Logf("   ED - Ok");
			Processed = Str[0]!=0;
			}
		else return 0;
		}
	//------------------------------------------------------------------------------------
	// BRUSH
	//
	else if (GetCMD(&Str,"BRUSH"))
		{
		if (GetCMD(&Str,"SET")) // BRUSH SET
			{
			GTrans->Begin  		(Level,"Brush Set");
			Brush->Lock			(&ModelInfo,LOCK_Trans);
			GTrans->NoteFPoly	(&ModelInfo,INDEX_NONE); // Save all old brush polys
			Brush->Unlock		(&ModelInfo);
			Brush->Init			(0);
			NoteMacroCommand	(Stream);
			Brush->Polys->ParseFPolys(&Stream,0,1);
			bspValidateBrush	(Brush,1,1);
			GTrans->End			();
			GCameraManager->RedrawLevel(Level);
			SaveAsMacro = 0;
			Processed = 1;
			}
		else if (GetCMD(&Str,"MORE")) // BRUSH MORE
			{
			GTrans->Continue 	(); // Continue previous (assumed "BRUSH SET") transaction
			Brush->Lock			(&ModelInfo,LOCK_Trans);
			GTrans->NoteFPoly	(&ModelInfo,INDEX_NONE); // Save all old brush polys
			Brush->Unlock		(&ModelInfo);
			NoteMacroCommand	(Stream);
			Brush->Polys->ParseFPolys(&Stream,1,1);
			bspValidateBrush	(Brush,1,1);
			GTrans->End			();	
			GCameraManager->RedrawLevel(Level);
			SaveAsMacro = 0;
			Processed = 1;
			}
		else if (GetCMD(&Str,"RESET")) // BRUSH RESET
			{
			GTrans->Begin  		(Level,"Brush Reset");
			Brush->Init 		(1);
			GTrans->NoteResHeader(Brush);
			Brush->BuildBound	(1);
			GTrans->End			();
			GCameraManager->RedrawLevel(Level);
			Processed = 1;
			}
		else if (GetCMD(&Str,"MIRROR"))
			{
			GTrans->Begin		(Level,"Brush Mirror");
			Brush->Lock			(&ModelInfo,LOCK_Trans);
			if (GetCMD(&Str,"X")) ModelInfo.Scale.Scale.X *= -1.0;
			if (GetCMD(&Str,"Y")) ModelInfo.Scale.Scale.Y *= -1.0;
			if (GetCMD(&Str,"Z")) ModelInfo.Scale.Scale.Z *= -1.0;
			Brush->Unlock		(&ModelInfo);
			Brush->BuildBound	(1);
			GTrans->End			();
			GCameraManager->RedrawLevel(Level);
			Processed = 1;
			}
		else if (GetCMD(&Str,"SCALE")) // BRUSH SCALE [X=f] [Y=f] [Z=f] [SHEER=f] [SHEERAXIS=XX]
			{
			GTrans->Begin		(Level,"Brush Scale");
			Brush->Lock			(&ModelInfo,LOCK_Trans);
			//
			if (GetCMD(&Str,"RESET"))
				{
				ModelInfo.Scale     = GMath.UnitScale;
				ModelInfo.PostScale = GMath.UnitScale;
				}
			else
				{
				GetFVECTOR 		(Str,&ModelInfo.Scale.Scale);
				GetFLOAT   		(Str,"SHEER=",&ModelInfo.Scale.SheerRate);
				//
				if (GetSTRING (Str,"SHEERAXIS=",TempStr,255))
					{
					if      (stricmp(TempStr,"XY")==0)	ModelInfo.Scale.SheerAxis = SHEER_XY;
					else if (stricmp(TempStr,"XZ")==0)	ModelInfo.Scale.SheerAxis = SHEER_XZ;
					else if (stricmp(TempStr,"YX")==0)	ModelInfo.Scale.SheerAxis = SHEER_YX;
					else if (stricmp(TempStr,"YZ")==0)	ModelInfo.Scale.SheerAxis = SHEER_YZ;
					else if (stricmp(TempStr,"ZX")==0)	ModelInfo.Scale.SheerAxis = SHEER_ZX;
					else if (stricmp(TempStr,"ZY")==0)	ModelInfo.Scale.SheerAxis = SHEER_ZY;
					else								ModelInfo.Scale.SheerAxis = SHEER_None;
					}
				};
			Brush->Unlock		(&ModelInfo);
			Brush->BuildBound	(1);
			GTrans->End			();
			GCameraManager->RedrawLevel(Level);
			Processed = 1;
			}
		else if (GetCMD(&Str,"APPLYTRANSFORM")) // BRUSH APPLYTRANSFORM
			{
			GTrans->Begin 		(Level,"Brush ApplyTransform");
			Brush->Transform	();
			Brush->BuildBound	(1);
			GTrans->End			();
			GCameraManager->RedrawLevel(Level);
			Processed = 1;
			}
		else if (GetCMD(&Str,"ROTATETO")) // BRUSH ROTATETO [PITCH=int] [YAW=int] [ROLL=int]
			{
			GTrans->Begin		(Level,"Brush RotateTo");
			Brush->Lock			(&ModelInfo,LOCK_Trans);
			GetFROTATION 		(Str,&ModelInfo.Rotation,256);
			Brush->Unlock		(&ModelInfo);
			Brush->BuildBound	(1);
			GTrans->End			();
			GCameraManager->RedrawLevel(Level);
			Processed = 1;
			}
		else if (GetCMD(&Str,"ROTATEREL")) // BRUSH ROTATEREL [PITCH=int] [YAW=int] [ROLL=int]
			{
			GTrans->Begin 		(Level,"Brush RotateRel");
			Brush->Lock			(&ModelInfo,LOCK_Trans);
			//
			TempRotation = GMath.ZeroRotation;
			GetFROTATION (Str,&TempRotation,256);
			//
			ModelInfo.Rotation.Pitch += TempRotation.Pitch;
			ModelInfo.Rotation.Yaw	 += TempRotation.Yaw;
			ModelInfo.Rotation.Roll  += TempRotation.Roll;
			//
			Brush->Unlock		(&ModelInfo);
			Brush->BuildBound	(1);
			GTrans->End			();
			GCameraManager->RedrawLevel(Level);
			Processed = 1;
			}
		else if (GetCMD(&Str,"MOVETO")) // BRUSH MOVETO [X=int] [Y=int] [Z=int]
			{
			GTrans->Begin 		(Level,"Brush MoveTo");
			Brush->Lock			(&ModelInfo,LOCK_Trans);
			GetFVECTOR 			(Str,&ModelInfo.Location);
			Brush->Unlock		(&ModelInfo);
			Brush->BuildBound	(1);
			GTrans->End			();
			GCameraManager->RedrawLevel(Level);
			Processed = 1;
			}
		else if (GetCMD(&Str,"MOVEREL")) // BRUSH MOVEREL [X=int] [Y=int] [Z=int]
			{
			GTrans->Begin		(Level,"Brush MoveRel");
			Brush->Lock			(&ModelInfo,LOCK_Trans);
			//
			TempVector = GMath.ZeroVector;
			GetFVECTOR   (Str,&TempVector);
			ModelInfo.Location.MoveBounded(TempVector);
			//
			Brush->Unlock		(&ModelInfo);
			Brush->BuildBound	(1);
			GTrans->End			();
			GCameraManager->RedrawLevel(Level);
			Processed = 1;
			}
		else if (GetCMD(&Str,"ADD")) // BRUSH ADD
			{
			GTrans->Begin           		(Level,"Brush Add");
			constraintFinishAllSnaps	(Level);
			//
			DWord1=0; GetINT(Str,"FLAGS=",&DWord1);
			//
			if 		(GetCMD(&Str,"NOCUT"	))	CsgType = CSG_NoCut;
			else if (GetCMD(&Str,"NOTERRAIN"))	CsgType = CSG_NoTerrain;
			else if (GetCMD(&Str,"CUTAWAY"))	CsgType = CSG_Cutaway;
			else								CsgType = CSG_Add;
			//
			UModel *TempModel = csgAddOperation (Brush,Level,DWord1,CsgType,0);
			if (TempModel && !MapEdit)
				{
				bspBrushCSG (TempModel,LevelModel,DWord1,CsgType,1);
				};
			GTrans->End();
			GCameraManager->RedrawLevel(Level);
			GApp->EdCallback(EDC_MapChange,0);
			Processed = 1;
			}
		else if (GetCMD(&Str,"ADDMOVER")) // BRUSH ADDMOVER
			{
			GTrans->Begin           	(Level,"Brush AddMover");
			GTrans->NoteResHeader		(Level->ActorList);
			constraintFinishAllSnaps	(Level);
			Level->Lock					(&LevelInfo,LOCK_Trans);
			//
			UClass *MoverClass	= new("Mover",FIND_Existing)UClass;
			INDEX iActor		= LevelInfo.SpawnActor(MoverClass,NAME_NONE,&Brush->Location);
			AActor *Actor		= &LevelInfo.Actors->Element(iActor);
			Actor->DrawRot		= Brush->Rotation;
			Actor->Brush		= csgDuplicateBrush(Level,Brush,0,0);
			//
			Level->Unlock				(&LevelInfo);
			GTrans->End					();
			GCameraManager->RedrawLevel	(Level);
			Processed = 1;
			}
		else if (GetCMD(&Str,"SUBTRACT")) // BRUSH SUBTRACT
			{
			GTrans->Begin				(Level,"Brush Subtract");
			constraintFinishAllSnaps(Level);
			//
			UModel *TempModel = csgAddOperation(Brush,Level,0,CSG_Subtract,0); // Layer
			if (TempModel && !MapEdit)
				{
				bspBrushCSG (TempModel,LevelModel,0,CSG_Subtract,1);
				};
			GTrans->End();
			GCameraManager->RedrawLevel(Level);
			GApp->EdCallback(EDC_MapChange,0);
			Processed = 1;
			}
		else if (GetCMD(&Str,"FROM")) // BRUSH FROM ACTOR/INTERSECTION/DEINTERSECTION
			{
			if (GetCMD(&Str,"INTERSECTION"))
				{
				Out->Log		("Brush from intersection");
				//
				GTrans->Begin	(Level,"Brush From Intersection");
				Brush->Lock		(&ModelInfo,LOCK_Trans);
				GTrans->NoteFPoly(&ModelInfo,INDEX_NONE); // Save all old brush polys
				Brush->Unlock	(&ModelInfo);
				//
				constraintFinishAllSnaps (Level);
				//
				if (!MapEdit) bspBrushCSG (Brush,LevelModel,0,CSG_Intersect,0);
				//
				GTrans->End			();
				GCameraManager->RedrawLevel(Level);
				Processed = 1;
				}
			else if (GetCMD(&Str,"DEINTERSECTION"))
				{
				Out->Log		("Brush from deintersection");
				//
				GTrans->Begin	(Level,"Brush From Deintersection");
				Brush->Lock		(&ModelInfo,LOCK_Trans);
				GTrans->NoteFPoly(&ModelInfo,INDEX_NONE); // Save all old brush polys
				Brush->Unlock	(&ModelInfo);
				//
				constraintFinishAllSnaps (Level);
				//
				if (!MapEdit) bspBrushCSG (Brush,LevelModel,0,CSG_Deintersect,0);
				GTrans->End				();
				GCameraManager->RedrawLevel(Level);
				Processed = 1;
				};
			}
		else if (GetCMD (&Str,"NEW"))
			{
			GTrans->Begin		(Level,"Brush New");
			Brush->Lock			(&ModelInfo,LOCK_Trans);
			GTrans->NoteFPoly	(&ModelInfo,INDEX_NONE); // Save all old brush polys
			//
			ModelInfo.NumFPolys=0;
			//
			Brush->Unlock		(&ModelInfo);
			GTrans->End			();
			GCameraManager->RedrawLevel(Level);
			//
			Processed = 1;
			}
		else if (GetCMD (&Str,"LOAD"))
			{
			if (GetSTRING(Str,"FILE=",TempFname,79))
				{
				GTrans->Begin			(Level,"Brush Load");
				//
				Brush->Lock				(&ModelInfo,LOCK_Trans);
				TempVector   = ModelInfo.Location;
				TempRotation = ModelInfo.Rotation;
				GTrans->NoteFPoly		(&ModelInfo,INDEX_NONE); // Save all old brush polys
				Brush->Unlock			(&ModelInfo);
				//
				GRes.AddFile 			(TempFname);
				//
				Brush->Lock				(&ModelInfo,LOCK_Trans);
				ModelInfo.Location = TempVector;
				ModelInfo.Rotation = TempRotation;
				Brush->Unlock			(&ModelInfo);
				//
				bspValidateBrush		(Brush,0,1);
				GTrans->End				();
				GCameraManager->RedrawLevel(Level);
				Processed = 1;
				};
			}
		else if (GetCMD (&Str,"SAVE"))
			{
			if (GetSTRING(Str,"FILE=",TempFname,79))
				{
				Out->Logf("Saving %s",TempFname);
				GRes.SaveDependent (Brush,TempFname,FILE_NONE);
				}
			else Out->Log(LOG_ExecError,"Missing filename");
			Processed = 1;
			}
		else if (GetCMD (&Str,"IMPORT"))
			{
			if (GetSTRING(Str,"FILE=",TempFname,79))
				{
				GApp->BeginSlowTask	("Importing brush",1,0);
				GTrans->Begin		(Level,"Brush Import");
				//
				if (!GetSTRING(Str,"NAME=", TempName,NAME_SIZE)) strcpy(TempName,"Brush");
				if (!GetONOFF (Str, "MERGE=",&DWord2)) DWord2=1;
				if (!GetINT   (Str, "FLAGS=",&DWord1)) DWord1=0;
				//
				UModel *TempModel = new(TempName,FIND_Optional)UModel;
				if (!TempModel)
					{
					TempModel = new(TempName,CREATE_Unique)UModel;
					TempModel->ModelFlags &= ~MF_Linked;
					TempModel->Polys = new(TempName,TempFname,IMPORT_Replace)UPolys;
					if (!TempModel->Polys)
						{
						Out->Log(LOG_ExecError,"Brush import failed");
						TempModel->Kill();
						goto ImportBrushError;
						};
					}
				else
					{
					TempModel->ModelFlags &= ~MF_Linked;
					TempModel->Lock		(&ModelInfo,LOCK_Trans);
					GTrans->NoteFPoly	(&ModelInfo,INDEX_NONE); // Save all old brush polys
					TempModel->Unlock	(&ModelInfo);
					//
					if (!TempModel->Polys->ImportFromFile(TempFname))
						{
						GTrans->Rollback();
						goto ImportBrushError;
						};
					};
				if (DWord1) // Set flags for all imported EdPolys
					{
					for (Word2=0; Word2<TempModel->Polys->Num; Word2++)
						{
						TempModel->Polys->Element(Word2).PolyFlags |= DWord1;
						};
					};
				if (DWord2) bspMergeCoplanars (TempModel,0);
				bspValidateBrush	(TempModel,0,1);
				GTrans->End			();
				//
				ImportBrushError:
				//
				GApp->EndSlowTask	();
				GCameraManager->RedrawLevel(Level);
				}
			else Out->Log(LOG_ExecError,"Missing filename");
			Processed=1;
			}
		else if (GetCMD (&Str,"EXPORT"))
			{
			if (GetSTRING(Str,"FILE=",TempFname,79))
				{
				GApp->BeginSlowTask	("Exporting brush",1,0);
				Brush->Polys->ExportToFile(TempFname); // Only exports polys
				GApp->EndSlowTask();
				}
			else Out->Log(LOG_ExecError,"Missing filename");
			Processed=1;
			};
		}
	//------------------------------------------------------------------------------------
	// Bsp
	//
	else if (GetCMD(&Str,"BSP"))
		{
		if (GetCMD(&Str,"REBUILD")) // Bsp REBUILD [LAME/GOOD/OPTIMAL] [BALANCE=0-100] [LIGHTS] [MAPS] [REJECT]
			{
			GTrans->Reset("rebuilding Bsp"); // Not tracked transactionally
			Out->Log("Bsp Rebuild");
			EBspOptimization BspOpt;
			//
			if      (GetCMD(&Str,"LAME")) 		BspOpt=BSP_Lame;
			else if (GetCMD(&Str,"GOOD"))		BspOpt=BSP_Good;
			else if (GetCMD(&Str,"OPTIMAL"))	BspOpt=BSP_Optimal;
			else								BspOpt=BSP_Good;
			//
			if (!GetWORD(Str,"BALANCE=",&Word2)) Word2=50; // Equal tendency to balance vs. minimize splits
			//
			GApp->BeginSlowTask	("Rebuilding Bsp",1,0);
			//
			GApp->StatusUpdate  ("Building polygons",0,0);
			bspBuildFPolys		(LevelModel,1);
			//
			GApp->StatusUpdate  ("Merging planars",0,0);
			bspMergeCoplanars	(LevelModel,0);
			//
			GApp->StatusUpdate  ("Partitioning",0,0);
			bspBuild			(LevelModel,BspOpt,Word2,0);
			//
			if (GetSTRING(Str,"ZONES",TempStr,1))
				{
				GApp->StatusUpdate("Building visibility zones",0,0);
				visBuild (Level);
				};
			if (GetSTRING(Str,"OPTGEOM",TempStr,1))
				{
				GApp->StatusUpdate("Optimizing geometry",0,0);
				bspOptGeom		(LevelModel);
				};
			GApp->StatusUpdate("Building unique planes",0,0);
			bspBuildUniquePlanes(LevelModel);
			//
			// Empty EdPolys:
			//
			LevelModel->Lock	(&ModelInfo,LOCK_NoTrans);
			ModelInfo.NumFPolys = 0;
			LevelModel->Unlock	(&ModelInfo);
			//
			GApp->EndSlowTask	();
			GCameraManager->RedrawLevel(Level);
			GApp->EdCallback	(EDC_MapChange,0);
			//
			Processed=1;
			};
		}
	//------------------------------------------------------------------------------------
	// ZONE
	//
	else if (GetCMD(&Str,"ZONE"))
		{
		GApp->BeginSlowTask	("Computing visibilty",1,0);
		GApp->StatusUpdate	("Building visibility zones",0,0);
		visBuild			(Level);
		GApp->EndSlowTask	();
		GCameraManager->RedrawLevel(Level);
		GApp->EdCallback	(EDC_MapChange,0);
		Processed=1;
		}
	//------------------------------------------------------------------------------------
	// LIGHT
	//
	else if (GetCMD(&Str,"LIGHT"))
		{
		if (GetCMD(&Str,"APPLY")) // LIGHT APPLY [MESH=..] [SELECTED=..] [SMOOTH=..] [RADIOSITY=..]
			{
			DWord1 = 0; GetONOFF (Str,"SELECTED=",  &DWord1); // Light selected lights only
			//
			shadowIlluminateBsp (Level,DWord1);
			GCameraManager->RedrawLevel (Level);
			//
			Processed=1;
			};
		}
	//------------------------------------------------------------------------------------
	// MAP
	//
	else if (GetCMD(&Str,"MAP"))
		{
		//
		// Parameters:
		//
		if (GetONOFF (Str,"EDIT=", &MapEdit))
			{
			constraintFinishAllSnaps (Level);
			if (MapEdit)
				{
				GTrans->Reset ("map editing"); // Can't be transaction-tracked
				csgInvalidateBsp (Level);
				};
			mapSelectNone		(Level);
			GCameraManager->RedrawLevel(Level);
			//
			Processed=1;
			};
		//
		// Commands:
		//
		if (GetCMD(&Str,"GRID")) // MAP GRID [SHOW3D=ON/OFF] [SHOW2D=ON/OFF] [X=..] [Y=..] [Z=..]
			{
			//
			// Before changing grid, force editor to current grid position to avoid jerking:
			//
			constraintFinishAllSnaps (Level);
			//
			Word1  = GetONOFF   (Str,"SHOW2D=",&Show2DGrid);
			Word1 |= GetONOFF   (Str,"SHOW3D=",&Show3DGrid);
			Word1 |= GetFVECTOR (Str,&Constraints.Grid);
			//
			if (Word1) GCameraManager->RedrawLevel(Level);
			Processed=1;
			}
		else if (GetCMD(&Str,"ROTGRID")) // MAP ROTGRID [PITCH=..] [YAW=..] [ROLL=..]
			{
			constraintFinishAllSnaps (Level);
			if (GetFROTATION (Str,&Constraints.RotGrid,256)) GCameraManager->RedrawLevel(Level);
			Processed=1;
			}
		else if (GetCMD(&Str,"SELECT")) // MAP SELECT ALL/NONE/INVERSE/NAMEstr
			{
			GTrans->Begin (Level,"Select");
			//
			if 		(GetCMD(&Str,"ALL"))		mapSelectAll		(Level);
			else if (GetCMD(&Str,"NONE"))		mapSelectNone		(Level);
			else if (GetCMD(&Str,"ADDS"))		mapSelectOperation	(Level,CSG_Add);
			else if (GetCMD(&Str,"SUBTRACTS"))	mapSelectOperation	(Level,CSG_Subtract);
			else if (GetCMD(&Str,"SEMISOLIDS"))	mapSelectFlags		(Level,PF_Semisolid);
			else if (GetCMD(&Str,"NONSOLIDS"))	mapSelectFlags		(Level,PF_NotSolid);
			else if (GetCMD(&Str,"PREVIOUS"))	mapSelectPrevious	(Level);
			else if (GetCMD(&Str,"NEXT"))		mapSelectNext		(Level);
			else if (GetCMD(&Str,"FIRST"))		mapSelectFirst		(Level);
			else if (GetCMD(&Str,"LAST"))		mapSelectLast		(Level);
			//
			GTrans->End 			();
			GCameraManager->RedrawLevel	(Level);
			Processed=1;
			}
		else if (GetCMD(&Str,"DELETE")) // MAP DELETE
			{
			GTrans->Begin		(Level,"Map Delete");
			mapDelete 			(Level);
			GTrans->End			();
			GCameraManager->RedrawLevel	(Level);
			Processed=1;
			}
		else if (GetCMD(&Str,"DUPLICATE")) // MAP DUPLICATE
			{
			GTrans->Begin		(Level,"Map Duplicate");
			mapDuplicate		(Level);
			GTrans->End			();
			GCameraManager->RedrawLevel	(Level);
			Processed=1;
			}
		else if (GetCMD(&Str,"BRUSH")) // MAP BRUSH GET/PUT
			{
			if (GetCMD (&Str,"GET"))
				{
				GTrans->Begin		(Level,"Brush Get");
				mapBrushGet			(Level);
				GTrans->End			();
				GCameraManager->RedrawLevel	(Level);
				Processed=1;
				}
			else if (GetCMD (&Str,"PUT"))
				{
				GTrans->Begin		(Level,"Brush Put");
				mapBrushPut			(Level);
				GTrans->End			();
				GCameraManager->RedrawLevel	(Level);
				Processed=1;
				};
			}
		else if (GetCMD(&Str,"SENDTO")) // MAP SENDTO FRONT/BACK
			{
			if (GetCMD(&Str,"FIRST"))
				{
				GTrans->Begin		(Level,"Map SendTo Front");
				mapSendToFirst		(Level);
				GTrans->End			();
				GCameraManager->RedrawLevel	(Level);
				Processed=1;
				}
			else if (GetCMD(&Str,"LAST"))
				{
				GTrans->Begin		(Level,"Map SendTo Back");
				mapSendToLast		(Level);
				GTrans->End			();
				GCameraManager->RedrawLevel	(Level);
				Processed=1;
				};
			}
		else if (GetCMD(&Str,"REBUILD")) // MAP REBUILD
			{
			GTrans->Reset		("rebuilding map"); 	// Can't be transaction-tracked
			csgRebuild			(Level);				// Revalidates the Bsp
			GCameraManager->RedrawLevel	(Level);
			GApp->EdCallback	(EDC_MapChange,0);
			Processed=1;
			}
		else if (GetCMD (&Str,"NEW")) // MAP NEW
			{
			GTrans->Reset			("clearing map");
			Level->RememberActors	();
			Level->Empty			();
			Level->ReconcileActors	(1);				
			Level->SetState 		(LEVEL_UpEdit,NULL);
			//
			GCameraManager->RedrawLevel		(Level);
			//
			GApp->EdCallback		(EDC_MapChange,0);
			GApp->EdCallback		(EDC_SelPolyChange,0);
			GApp->EdCallback		(EDC_SelActorChange,0);
			//
			GRes.Purge(0);
			//
			Processed=1;
			}
		else if (GetCMD (&Str,"LOAD")) // MAP LOAD
			{
			if (GetSTRING(Str,"FILE=",TempFname,79))
				{
				GTrans->Reset			("loading map"); // Can't be transaction tracked
				//
				GApp->BeginSlowTask		("Loading map",1,0);
				Level->RememberActors	();
				GRes.AddFile 			(TempFname);
				Level->ReconcileActors	(1); // Match camera actors to existing cameras
				Level->SetState 		(LEVEL_UpEdit,NULL);
				bspValidateBrush		(Brush,0,1);
				GApp->EndSlowTask   	();
				GCache.Flush			();
				//
				DWord1=1; GetONOFF(Str,"REDRAW=",&DWord1);
				if (DWord1) GCameraManager->RedrawLevel(Level);
				//
				GApp->EdCallback	(EDC_MapChange,0);
				GApp->EdCallback	(EDC_SelPolyChange,0);
				GApp->EdCallback	(EDC_SelActorChange,0);
				//
				GRes.Purge(0);
				}
			else Out->Log(LOG_ExecError,"Missing filename");
			Processed=1;
			}
		else if (GetCMD (&Str,"SAVE"))
			{
			if (GetSTRING(Str,"FILE=",TempFname,79))
				{
				GApp->BeginSlowTask	("Saving map",1,0);
				GRes.SaveDependent	(Level,TempFname,FILE_NONE);
				GApp->EndSlowTask	();
				}
			else Out->Log(LOG_ExecError,"Missing filename");
			Processed=1;
			}
		else if (GetCMD (&Str,"IMPORT"))
			{
			Word1=1;
			DoImportMap:
			if (GetSTRING(Str,"FILE=",TempFname,79))
				{
				GTrans->Reset			("importing map"); 		// Can't be transaction tracked
				//
				GApp->BeginSlowTask		("Importing map",1,0);
				Level->RememberActors	();
				if (Word1) Level->Empty	();
				Level->ImportFromFile	(TempFname);			// Import it and replace existing
				GCache.Flush			();
				csgInvalidateBsp 		(Level);
				Level->SetState 		(LEVEL_UpEdit,NULL);	// Set as editing for reconciliation
				Level->ReconcileActors 	(1);					// Match camera actors to existing cameras
				if (Word1) mapSelectNone(Level);
				//
				GApp->EndSlowTask   	();
				GCameraManager->RedrawLevel		(Level);
				GApp->EdCallback		(EDC_MapChange,0);
				GApp->EdCallback		(EDC_SelPolyChange,0);
				GApp->EdCallback		(EDC_SelActorChange,0);
				}
			else Out->Log(LOG_ExecError,"Missing filename");
			Processed=1;
			}
		else if (GetCMD (&Str,"IMPORTADD"))
			{
			Word1=0;
			mapSelectNone (Level);
			goto DoImportMap; // Import without emptying level
			}
		else if (GetCMD (&Str,"EXPORT"))
			{
			if (GetSTRING(Str,"FILE=",TempFname,79))
				{
				GApp->BeginSlowTask	("Exporting map",1,0);
				GRes.UntagAll();
				Level->ExportToFile(TempFname);
				GApp->EndSlowTask();
				}
			else Out->Log(LOG_ExecError,"Missing filename");
			Processed=1;
			}
		else if (GetCMD (&Str,"SETBRUSH")) // MAP SETBRUSH (set properties of all selected brushes)
			{
			GTrans->Begin		(Level,"Set Brush Properties");
			//
			Word1  = 0;  // Properties mask
			DWord1 = 0;  // Set flags
			DWord2 = 0;  // Clear flags
			//
			FName GroupName=NAME_NONE;
			if (GetWORD (Str,"COLOR=",&Word2))			Word1 |= MSB_BrushColor;
			if (GetNAME (Str,"GROUP=",&GroupName))		Word1 |= MSB_Group;
			if (GetINT  (Str,"SETFLAGS=",&DWord1))		Word1 |= MSB_PolyFlags;
			if (GetINT  (Str,"CLEARFLAGS=",&DWord2))	Word1 |= MSB_PolyFlags;
			//
			mapSetBrush(Level,(EMapSetBrushFlags)Word1,Word2,GroupName,DWord1,DWord2);
			//
			GTrans->End			();
			GCameraManager->RedrawLevel	(Level);
			//
			Processed=1;
			}
		else if (GetCMD (&Str,"SAVEPOLYS"))
			{
			if (GetSTRING(Str,"FILE=",TempFname,79))
				{
				GApp->BeginSlowTask	("Exporting map polys",1,0);
				GApp->StatusUpdate  ("Building polygons",0,0);
				bspBuildFPolys		(LevelModel,0);
				//
				GApp->StatusUpdate  ("Merging planars",0,0);
				bspMergeCoplanars	(LevelModel,0);
				//
				LevelModel->Lock	(&ModelInfo,LOCK_NoTrans); 
				ModelInfo.PolysResource->ExportToFile(TempFname);
				ModelInfo.NumFPolys = 0; // Empty edpolys
				LevelModel->Unlock	(&ModelInfo);
				//
				GApp->EndSlowTask 	();
				GCameraManager->RedrawLevel	(Level);
				}
			else Out->Log(LOG_ExecError,"Missing filename");
			Processed=1;
			};
		}
	//------------------------------------------------------------------------------------
	// RESOURCE
	//
	else if (GetCMD(&Str,"RESOURCE"))
		{
		if (GetCMD(&Str,"QUERY"))
			{
			SaveAsMacro = 0;
			if (GetRESTYPE(Str,"TYPE=",&ResType))
				{
				UResource *Res;
				if (GetRES(Str,"NAME=",ResType,&Res)) resQueryForLink(Res,ResType);
				else resQueryForLink(NULL,ResType);
				}
			else Out->Log(LOG_ExecError,"Missing resource type");
			Processed=1;
			}
		else if (GetCMD(&Str,"DEBUG")) // Build list of unclaimed resources
			{
			GRes.FindUnclaimed();
			Processed=1;
			}
		else if (GetCMD(&Str,"PURGE")) // Purge unclaimed resources
			{
			GRes.KillUnclaimed();
			Processed=1;
			};
		}
	//------------------------------------------------------------------------------------
	// SELECT: Rerouted to mode-specific command
	//
	else if (GetCMD(&Str,"SELECT"))
		{
		strcpy (TempStr,"SELECT");
		//
		ModeSpecificReroute:
		//
		switch (ModeClass)
			{
			case EMC_Camera:
			case EMC_Player:
			case EMC_Brush:
			case EMC_Texture:
				if (MapEdit)
					{
					sprintf (TempStr1,"MAP %s %s ",TempStr,Str);
					Exec(TempStr1,Out);
					}
				else
					{
					sprintf (TempStr1,"POLY %s %s ",TempStr,Str);
					Exec(TempStr1,Out);
					};
			case EMC_Actor:
				sprintf (TempStr1,"ACTOR %s %s ",TempStr,Str);
				Exec(TempStr1,Out);
			default:
			case EMC_None:
				// No applicable reroute
				break;
			};
		Processed=1;
		}
	//------------------------------------------------------------------------------------
	// DELETE: Rerouted to mode-specific command
	//
	else if (GetCMD(&Str,"DELETE"))
		{
		strcpy (TempStr,"DELETE");
		goto ModeSpecificReroute;
		}
	//------------------------------------------------------------------------------------
	// DUPLICATE: Rerouted to mode-specific command
	//
	else if (GetCMD(&Str,"DUPLICATE"))
		{
		strcpy (TempStr,"DUPLICATE");
		goto ModeSpecificReroute;
		}
	//------------------------------------------------------------------------------------
	// ACTOR: Actor-related functions
	//
	else if (GetCMD(&Str,"ACTOR"))
		{
		if (GetCMD(&Str,"SELECT")) // ACTOR SELECT
			{
			if (GetCMD(&Str,"NONE")) // ACTOR SELECT NONE
				{
				GTrans->Begin			(Level,"Select None");
				//
				Level->Lock				(&LevelInfo,LOCK_Trans);
				edactSelectNone 		(&LevelInfo);
				Level->Unlock			(&LevelInfo);
				//
				GTrans->End				();
				GCameraManager->RedrawLevel		(Level);
				//
				GApp->EdCallback(EDC_SelActorChange,0);
				Processed=1;
				}
			else if (GetCMD(&Str,"ALL")) // ACTOR SELECT ALL
				{
				GTrans->Begin		(Level,"Select All");
				//
				Level->Lock			(&LevelInfo,LOCK_Trans);
				edactSelectAll 		(&LevelInfo);
				Level->Unlock		(&LevelInfo);
				//
				GTrans->End			();
				GCameraManager->RedrawLevel	(Level);
				//
				GApp->EdCallback(EDC_SelActorChange,0);
				Processed=1;
				}
			else if (GetCMD(&Str,"OFCLASS")) // ACTOR SELECT OFCLASS CLASS=..
				{
				UClass *Class;
				if (GetUClass(Str,"CLASS=",&Class))
					{
					GTrans->Begin		(Level,"Select of class");
					//
					Level->Lock			(&LevelInfo,LOCK_Trans);
					edactSelectOfClass	(&LevelInfo,Class);
					Level->Unlock		(&LevelInfo);
					//
					GTrans->End			();
					GCameraManager->RedrawLevel	(Level);
					//
					GApp->EdCallback(EDC_SelActorChange,0);
					}
				else Out->Log(LOG_ExecError,"Missing class");
				Processed=1;
				};
			}
		else if (GetCMD(&Str,"SET")) // ACTOR SET [ADDCLASS=class]
			{
			GetUClass(Str,"ADDCLASS=",&GClasses.AddClass);
			Processed=1;
			}
		else if (GetCMD(&Str,"RESET")) // ACTOR RESET
			{
			GTrans->Begin		(Level,"Actor Reset");
			//
			Level->Lock			(&LevelInfo,LOCK_Trans);
			edactResetSelected	(&LevelInfo);
			Level->Unlock		(&LevelInfo);
			//
			GTrans->End			();
			GCameraManager->RedrawLevel	(Level);
			//
			GApp->EdCallback(EDC_SelActorChange,0);
			Processed=1;
			}
		else if (GetCMD(&Str,"DELETE")) // ACTOR DELETE (selected)
			{
			GTrans->Begin			(Level,"Delete Actors");
			//
			Level->Lock				(&LevelInfo,LOCK_Trans);
			edactDeleteSelected		(&LevelInfo);
			Level->Unlock			(&LevelInfo);
			//
			GTrans->End				();
			GCameraManager->RedrawLevel		(Level);
			//
			GApp->EdCallback(EDC_SelActorChange,0);
			Processed=1;
			}
		else if (GetCMD(&Str,"DUPLICATE")) // ACTOR DUPLICATE (selected)
			{
			GTrans->Begin				(Level,"Duplicate Actors");
			//
			Level->Lock					(&LevelInfo,LOCK_Trans);
			edactDuplicateSelected		(&LevelInfo);
			Level->Unlock				(&LevelInfo);
			//
			GTrans->End					();
			GCameraManager->RedrawLevel			(Level);
			//
			GApp->EdCallback(EDC_SelActorChange,0);
			Processed=1;
			};
		}
	//------------------------------------------------------------------------------------
	// POLY: Polygon adjustment and mapping
	//
	else if (GetCMD(&Str,"POLY"))
		{
		if (GetCMD(&Str,"SELECT")) // POLY SELECT [ALL/NONE/INVERSE] FROM [LEVEL/SOLID/GROUP/ITEM/ADJACENT/MATCHING]
			{
			sprintf    (TempStr,"POLY SELECT %s",Str);
			GTrans->Begin (Level,TempStr);
			LevelModel->Lock  (&ModelInfo,LOCK_Trans);
			//
			if (GetCMD(&Str,"ALL"))
				{
				polySelectAll (&ModelInfo);
				GApp->EdCallback(EDC_SelPolyChange,0);
				Processed=1;
				}
			else if (GetCMD(&Str,"NONE"))
				{
				polySelectNone (&ModelInfo);
				GApp->EdCallback(EDC_SelPolyChange,0);
				Processed=1;
				}
			else if (GetCMD(&Str,"REVERSE"))
				{
				polySelectReverse (&ModelInfo);
				GApp->EdCallback(EDC_SelPolyChange,0);
				Processed=1;
				}
			else if (GetCMD(&Str,"MATCHING"))
				{
				if 		(GetCMD(&Str,"GROUPS"))		polySelectMatchingGroups 	(&ModelInfo);
				else if (GetCMD(&Str,"ITEMS"))		polySelectMatchingItems 	(&ModelInfo);
				else if (GetCMD(&Str,"BRUSH"))		polySelectMatchingBrush 	(&ModelInfo);
				else if (GetCMD(&Str,"TEXTURE"))	polySelectMatchingTexture 	(&ModelInfo);
				GApp->EdCallback(EDC_SelPolyChange,0);
				Processed=1;
				}
			else if (GetCMD(&Str,"ADJACENT"))
				{
				if 	  (GetCMD(&Str,"ALL"))			polySelectAdjacents 		(&ModelInfo);
				else if (GetCMD(&Str,"COPLANARS"))	polySelectCoplanars 		(&ModelInfo);
				else if (GetCMD(&Str,"WALLS"))		polySelectAdjacentWalls 	(&ModelInfo);
				else if (GetCMD(&Str,"FLOORS"))		polySelectAdjacentFloors 	(&ModelInfo);
				else if (GetCMD(&Str,"CEILINGS"))	polySelectAdjacentFloors 	(&ModelInfo);
				else if (GetCMD(&Str,"SLANTS"))		polySelectAdjacentSlants 	(&ModelInfo);
				GApp->EdCallback(EDC_SelPolyChange,0);
				Processed=1;
				}
			else if (GetCMD(&Str,"MEMORY"))
				{
				if 		(GetCMD(&Str,"SET"))		polyMemorizeSet 			(&ModelInfo);
				else if (GetCMD(&Str,"RECALL"))		polyRememberSet 			(&ModelInfo);
				else if (GetCMD(&Str,"UNION"))		polyUnionSet 				(&ModelInfo);
				else if (GetCMD(&Str,"INTERSECT"))	polyIntersectSet 			(&ModelInfo);
				else if (GetCMD(&Str,"XOR"))		polyXorSet 					(&ModelInfo);
				GApp->EdCallback(EDC_SelPolyChange,0);
				Processed=1;
				};
			LevelModel->Unlock	(&ModelInfo);
			GTrans->End			();
			GCameraManager->RedrawLevel	(Level);
			}
		else if (GetCMD(&Str,"DEFAULT")) // POLY DEFAULT <variable>=<value>...
			{
			if (!GetUTexture(Str,"TEXTURE=",&CurrentTexture))
				{
				Out->Log(LOG_ExecError,"Missing texture");
				};
			Processed=1;
			}
		else if (GetCMD(&Str,"SET")) // POLY SET <variable>=<value>...
			{
			//
			// Options: TEXTURE=name SETFLAGS=value CLEARFLAGS=value
			//          UPAN=value VPAN=value ROTATION=value XSCALE=value YSCALE=value
			//
			GTrans->Begin				(Level,"Poly Set");
			LevelModel->Lock			(&ModelInfo,LOCK_Trans);
			GTrans->NoteSelectedBspSurfs (&ModelInfo,1);
			//
			UTexture *Texture;
			if (GetUTexture(Str,"TEXTURE=",&Texture))
				{
				for (Index1=0; Index1<ModelInfo.NumBspSurfs; Index1++)
					{
					if (ModelInfo.BspSurfs[Index1].PolyFlags & PF_Selected)
						{
						ModelInfo.BspSurfs[Index1].Texture  = Texture;
						polyUpdateMaster (&ModelInfo,Index1,0,0);
						};
					};
				};
			Word4  = 0;
			DWord1 = 0;
			DWord2 = 0;
			if (GetINT(Str,"SETFLAGS=",&DWord1))   Word4=1;
			if (GetINT(Str,"CLEARFLAGS=",&DWord2)) Word4=1;
			if (Word4)  polySetAndClearPolyFlags (&ModelInfo,DWord1,DWord2,1,1); // Update selected polys' flags
			//
			FName ItemName;
			if (GetNAME (Str,"ITEM=",&ItemName)) polySetItemNames(&ModelInfo,ItemName);
			//
			LevelModel->Unlock	(&ModelInfo);
			GTrans->End			();
			GCameraManager->RedrawLevel	(Level);			
			Processed=1;
			}
		else if (GetCMD(&Str,"TEXSCALE")) // POLY TEXSCALE [U=..] [V=..] [UV=..] [VU=..]
			{
			GTrans->Begin 				(Level,"Poly Texscale");
			LevelModel->Lock			(&ModelInfo,LOCK_Trans);
			GTrans->NoteSelectedBspSurfs	(&ModelInfo,1);
			//
			Word2 = 1; // Scale absolute
			//
			TexScale:
			//
			FLOAT UU,UV,VU,VV;
			UU=1.0; GetFLOAT (Str,"UU=",&UU);
			UV=0.0; GetFLOAT (Str,"UV=",&UV);
			VU=0.0; GetFLOAT (Str,"VU=",&VU);
			VV=1.0; GetFLOAT (Str,"VV=",&VV);
			//
			polyTexScale (&ModelInfo,UU,UV,VU,VV,Word2);
			//
			LevelModel->Unlock	(&ModelInfo);
			GTrans->End			();
			GCameraManager->RedrawLevel	(Level);
			Processed=1;
			}
		else if (GetCMD(&Str,"TEXMULT")) // POLY TEXMULT [U=..] [V=..]
			{
			GTrans->Begin 				(Level,"Poly Texmult");
			LevelModel->Lock			(&ModelInfo,LOCK_Trans);
			GTrans->NoteSelectedBspSurfs (&ModelInfo,1);
			//
			Word2 = 0; // Scale relative;
			//
			goto TexScale;
			}
		else if (GetCMD(&Str,"TEXPAN")) // POLY TEXPAN [RESET] [U=..] [V=..]
			{
			GTrans->Begin 				(Level,"Poly Texpan");
			LevelModel->Lock			(&ModelInfo,LOCK_Trans);
			GTrans->NoteSelectedBspSurfs (&ModelInfo,1);
			//
			if (GetCMD (&Str,"RESET")) polyTexPan  (&ModelInfo,0,0,1);
			//
			Word1 = 0; GetWORD (Str,"U=",&Word1);
			Word2 = 0; GetWORD (Str,"V=",&Word2);
			polyTexPan (&ModelInfo,Word1,Word2,0);
			//
			LevelModel->Unlock	(&ModelInfo);
			GTrans->End			();
			GCameraManager->RedrawLevel	(Level);
			Processed=1;
			}
		else if (GetCMD(&Str,"TEXALIGN")) // POLY TEXALIGN [FLOOR/GRADE/WALL/NONE]
			{
			if		(GetCMD (&Str,"DEFAULT"))	TexAlign = TEXALIGN_Default;
			else if (GetCMD (&Str,"FLOOR"))		TexAlign = TEXALIGN_Floor;
			else if (GetCMD (&Str,"WALLDIR"))	TexAlign = TEXALIGN_WallDir;
			else if (GetCMD (&Str,"WALLPAN"))	TexAlign = TEXALIGN_WallPan;
			else if (GetCMD (&Str,"WALLCOLUMN"))TexAlign = TEXALIGN_WallColumn;
			else if (GetCMD (&Str,"ONETILE"))	TexAlign = TEXALIGN_OneTile;
			else								goto Skipt;
			//
			if (!GetINT(Str,"TEXELS=",&DWord1)) DWord1=0;
			//
			GTrans->Begin				(Level,"Poly Texalign");
			LevelModel->Lock			(&ModelInfo,LOCK_Trans);
			GTrans->NoteSelectedBspSurfs(&ModelInfo,1);
			//
			polyTexAlign				(&ModelInfo,TexAlign,DWord1);
			//
			LevelModel->Unlock			(&ModelInfo);
			GTrans->End					();
			GCameraManager->RedrawLevel	(Level);
			Processed=1;
			//
			Skipt:;
			};
		}
	//------------------------------------------------------------------------------------
	// PALETTE management:
	//
	else if (GetCMD(&Str,"PALETTE"))
		{
		if (GetCMD(&Str,"IMPORT")) // PALETTE IMPORT FILE=.. NAME=.. SMOOTH=on/off
			{
			if ((GetSTRING (Str, "FILE=",  TempFname, 128)) &&
				(GetSTRING (Str, "NAME=",  TempName,  NAME_SIZE)))
				{
				GApp->BeginSlowTask ("Importing palette",1,0);
				//
				DWord1=0; GetONOFF (Str,"SMOOTH=",&DWord1);
				UPalette *Palette = new(TempName,TempFname,IMPORT_Replace)UPalette;
				if (Palette)
					{
					if (DWord1) Palette->Smooth();
					GCache.Flush();
					GCameraManager->RedrawLevel(Level);
					};
				GApp->EndSlowTask();
				}
			else Out->Log(LOG_ExecError,"Missing file or name");
			Processed=1;
			};
		}
	//------------------------------------------------------------------------------------
	// TEXTURE management (not mapping):
	//
	else if (GetCMD(&Str,"TEXTURE"))
		{
		if (GetCMD(&Str,"IMPORT")) // TEXTURE IMPORT FILE=.. NAME=.. REMAP=on/off
			{
			if ((GetSTRING (Str, "FILE=",  TempFname, 128)) &&
				(GetSTRING (Str, "NAME=",  TempName,  NAME_SIZE)))
				{
				GApp->BeginSlowTask ("Importing texture",1,0);
				//
				UPalette		*Palette;
				Palette=NULL;	GetUPalette (Str,"PALETTE=",&Palette);
				DWord1=1;		GetONOFF	(Str,"MIPS=", &DWord1);
				DWord2=1;		GetONOFF	(Str,"REMAP=",&DWord2);
				//
				UTexture *Texture = new(TempName,TempFname,IMPORT_Replace)UTexture;
				if (Texture)
					{
					GetDWORD(Str,"FLAGS=",			&Texture->PolyFlags);
					GetNAME (Str,"FAMILY=",			&Texture->FamilyName);
					GetFLOAT(Str,"PALDIFFUSION=",	&Texture->PalDiffusionC);
					//
					if ((!Palette) && DWord2) // Import palette
						{
						Palette = new(TempName,TempFname,IMPORT_Replace)UPalette;
						if (Palette)
							{
							Palette->BuildPaletteRemapIndex(Texture->PolyFlags & PF_Masked);
							Palette = Palette->ReplaceWithExisting();
							};
						}
					else if (Palette) // Use someone else's palette
						{
						UPalette *TempPalette = new("Temp",TempFname,IMPORT_Replace)UPalette;
						if (TempPalette)
							{
							Texture->Remap(TempPalette,Palette);
							TempPalette->Kill();
							};
						}
					else Texture->Fixup();
					//
					Texture->Palette = Palette;
					Texture->CreateMips(DWord1);
					//
					GCache.Flush		();
					GCameraManager->RedrawLevel	(Level);
					}
				else Out->Logf(LOG_ExecError,"Import texture %s from %s failed",TempName,TempFname);
				GApp->EndSlowTask();
				}
			else Out->Log(LOG_ExecError,"Missing file or name");
			Processed=1;
			}
		else if (GetCMD(&Str,"KILL")) // TEXTURE KILL [NAME=..]
			{
			UTexture *Texture;
			if (GetUTexture(Str,"NAME=",&Texture)) Texture->FamilyName=NAME_NONE;
			else Out->Log(LOG_ExecError,"Missing name");
			Processed=1;
			}
		else if (GetCMD (&Str,"QUERY")) // TEXTURE QUERY [ALL] [FAMILY=..]
			{
			SaveAsMacro = 0;
			if (GetSTRING (Str,"FAMILY=",TempStr,NAME_SIZE))
				{
				texQueryTextureForLink (TempStr); // Return list of textures in family
				}
			else if (GetCMD(&Str,"ALL"))
				{
				texQueryFamilyForLink(1); // Return list of all families including special
				}
			else
				{
				texQueryFamilyForLink(0); // Return list of all normal families
				};
			Processed=1;
			}
		else if (GetCMD(&Str,"LOADFAMILY")) // TEXTURE SAVEFAMILY FAMILY=.. FILE=..
			{
			if (GetSTRING(Str,"FILE=",TempFname,79))
				{
				GApp->BeginSlowTask ("Loading textures",1,0);
				GRes.AddFile(TempFname);
				GCache.Flush();
				GApp->EndSlowTask();
				}
			else Out->Log(LOG_ExecError,"Missing filename");
			Processed=1;
			}
		else if (GetCMD(&Str,"SAVEFAMILY")) // TEXTURE SAVEFAMILY FAMILY=.. FILE=..
			{
			FName Name;
			if (GetCMD(&Str,"ALL") && GetSTRING(Str,"FILE=",TempFname,79))
				{
				GRes.UntagAll();
				//
				UTexture *Texture;
				FOR_ALL_TYPED_RES(Texture,RES_Texture,UTexture)
					{
					if (!Texture->FamilyName.IsNone()) Texture->Flags |= RF_TagExp;
					}
				END_FOR_ALL_TYPED_RES;
				//
				GApp->BeginSlowTask("Saving textures",1,0);
				GRes.SaveDependentTagged (TempFname,0);
				GApp->EndSlowTask();
				}
			else if ((GetSTRING(Str,"FILE=",TempFname,79)) && GetNAME(Str,"FAMILY=",&Name))
				{
				GRes.UntagAll();
				if (GRes.TagAllReferencingName (Name, RES_Texture)==0)
					{
					Out->Log(LOG_ExecError,"TEXTURE SAVEFAMILY: Unknown family");
					}
				else
					{
					GApp->BeginSlowTask("Saving textures",1,0);
					GRes.SaveDependentTagged (TempFname,0);
					GApp->EndSlowTask();
					};
				}
			else Out->Log(LOG_ExecError,"Missing filename");
			Processed=1;
			}
		}
	//------------------------------------------------------------------------------------
	// FONT management
	//
	else if (GetCMD(&Str,"FONT"))
		{
		if (GetCMD(&Str,"BUILD"))
			{
			UTexture *Texture;
			if (GetUTexture(Str,"TEXTURE=",&Texture)) GGfx.MakeFontFromTexture (Texture);
			else Out->Log(LOG_ExecError,"Missing texture");
			Processed=1;
			};
		}
	//------------------------------------------------------------------------------------
	// MODE management (Global EDITOR mode):
	//
	else if (GetCMD(&Str,"MODE"))
		{
		Word1 = GUnrealEditor.Mode;  // To see if we should redraw
		Word2 = GUnrealEditor.Mode;  // Destination mode to set
		//
		if (GetONOFF (Str,"GRID=", &DWord1))
			{
			//
			// Before changing grid, force editor to current grid position to avoid jerking:
			//
			constraintFinishAllSnaps (Level);
			Constraints.GridEnabled = DWord1;
			Word1=MAXWORD;
			};
		if (GetONOFF (Str,"ROTGRID=", &DWord1))
			{
			constraintFinishAllSnaps (Level);
			Constraints.RotGridEnabled=DWord1;
			Word1=MAXWORD;
			};
		if (GetONOFF (Str,"SNAPVERTEX=", &DWord1))
			{
			constraintFinishAllSnaps (Level);
			Constraints.SnapVertex=DWord1;
			Word1=MAXWORD;
			};
		if (GetONOFF (Str,"SHOWVERTICES=", &ShowVertices))
			{
			Word1=MAXWORD;
			};
		GetFLOAT (Str,"SPEED=",    &MovementSpeed);
		GetFLOAT (Str,"SNAPDIST=",	&Constraints.SnapDist);
		//
		// Major modes:
		//
		if 		(GetCMD(&Str,"CAMERAMOVE"))		Word2 = EM_CameraMove;
		else if	(GetCMD(&Str,"CAMERAZOOM"))		Word2 = EM_CameraZoom;
		else if	(GetCMD(&Str,"BRUSHFREE"))		Word2 = EM_BrushFree;
		else if	(GetCMD(&Str,"BRUSHMOVE"))		Word2 = EM_BrushMove;
		else if	(GetCMD(&Str,"BRUSHROTATE"))	Word2 = EM_BrushRotate;
		else if	(GetCMD(&Str,"BRUSHSHEER"))		Word2 = EM_BrushSheer;
		else if	(GetCMD(&Str,"BRUSHSCALE"))		Word2 = EM_BrushScale;
		else if	(GetCMD(&Str,"BRUSHSTRETCH"))	Word2 = EM_BrushStretch;
		else if	(GetCMD(&Str,"BRUSHSNAP")) 		Word2 = EM_BrushSnap;
		else if	(GetCMD(&Str,"ADDACTOR"))		Word2 = EM_AddActor;
		else if	(GetCMD(&Str,"MOVEACTOR"))		Word2 = EM_MoveActor;
		else if	(GetCMD(&Str,"TEXTUREPAN"))		Word2 = EM_TexturePan;
		else if	(GetCMD(&Str,"TEXTURESET"))		Word2 = EM_TextureSet;
		else if	(GetCMD(&Str,"TEXTUREROTATE"))	Word2 = EM_TextureRotate;
		else if	(GetCMD(&Str,"TEXTURESCALE")) 	Word2 = EM_TextureScale;
		else if	(GetCMD(&Str,"BRUSHWARP")) 		Word2 = EM_BrushWarp;
		else if	(GetCMD(&Str,"TERRAFORM")) 		Word2 = EM_Terraform;
		//
		if (Word2 != Word1)
			{
			edcamSetMode(Word2);
			GCameraManager->RedrawLevel(Level);
			};
		Processed=1;
		}
	//------------------------------------------------------------------------------------
	// Transaction tracking and control
	//
	else if (GetCMD(&Str,"TRANSACTION"))
		{
		if (GetCMD(&Str,"UNDO"))
			{
			if (GTrans->Undo ()) GCameraManager->RedrawLevel (Level);
			Processed=1;
			}
		else if (GetCMD(&Str,"REDO"))
			{
			if (GTrans->Redo()) GCameraManager->RedrawLevel (Level);
			Processed=1;
			}
		else if (GetCMD(&Str,"REDOALL"))
			{
			if (GTrans->RedoAll()) GCameraManager->RedrawLevel (Level);
			Processed=1;
			};
		GApp->EdCallback(EDC_SelActorChange,0);
		GApp->EdCallback(EDC_SelPolyChange,0);
		GApp->EdCallback(EDC_MapChange,0);
		}
	//------------------------------------------------------------------------------------
	// RES (General resources)
	//
	else if (GetCMD(&Str,"RES"))
		{
		if (GetCMD(&Str,"IMPORT")) // RES IMPORT TYPE=.. NAME=.. FILE=..
			{
			if (GetRESTYPE (Str,"TYPE=",&ResType) &&
				GetSTRING  (Str,"FILE=",TempFname,80) &&
				GetSTRING  (Str,"NAME=",TempName,NAME_SIZE))
				{
				//GRes.Import (ResType,TempFname,TempName,1);
				}
			else Out->Log(LOG_ExecError,"Missing file, name, or type");
			Processed=1;
			}
		else if (GetCMD(&Str,"EXPORT")) // RES EXPORT TYPE=.. NAME=.. FILE=..
			{
			UResource *Res;
			if (GetRESTYPE (Str,"TYPE=",&ResType) &&
				GetSTRING (Str,"FILE=",TempFname,80) &&
				GetRES(Str,"NAME=",ResType,&Res))
				{
				GRes.UntagAll();
				Res->ExportToFile(TempFname);
				}
			else Out->Log(LOG_ExecError,"Missing file, name, or type");
			Processed=1;
			}
		else if (GetCMD(&Str,"LOAD")) // RES LOAD FILE=..
			{
			if (GetSTRING (Str,"FILE=",TempFname,80))
				{
				Level->RememberActors	();
				GRes.AddFile			(TempFname);
				GCache.Flush			();
				Level->ReconcileActors	(1);
				GCameraManager->RedrawLevel(Level);
				}
			else Out->Log(LOG_ExecError,"Missing filename");
			Processed=1;
			}
		else if (GetCMD(&Str,"SAVE")) // RES SAVE TYPE=.. NAME=.. FILE=..
			{
			UResource *Res;
			if (GetRESTYPE (Str,"TYPE=",&ResType) &&
				GetSTRING (Str,"FILE=",TempFname,80) &&
				GetRES(Str,"NAME=",ResType,&Res))
				{
				GRes.Save (Res,TempFname,FILE_NONE);
				}
			else Out->Log(LOG_ExecError,"Missing file, name, or type");
			Processed=1;
			}
		else if (GetCMD(&Str,"KILL")) // RES KILL TYPE=.. NAME=..
			{
			UResource *Res;
			if (GetRESTYPE (Str,"TYPE=",&ResType) &&
				GetRES(Str,"NAME=",ResType,&Res)) 
				{
				Res->Kill();
				}
			else Out->Log(LOG_ExecError,"Missing type or name");
			Processed=1;
			}
		else if (GetCMD(&Str,"ARRAYADD")) // RES ARRAYADD TYPE=.. NAME=.. ARRAY=..
			{
			UResource	*Res;
			UArray		*Array;
			if (GetRESTYPE (Str,"TYPE=",&ResType) &&
				GetRES(Str,"NAME=",ResType,&Res) &&
				GetUArray(Str,"ARRAY=",&Array))
				{
				Array->Add(Res);
				}
			else Out->Log(LOG_ExecError,"Missing file, name, or type");
			Processed=1;
			}
		}
	//------------------------------------------------------------------------------------
	// TEXT resources
	//
	else if (GetCMD(&Str,"TEXT"))
		{
		if (GetCMD(&Str,"IMPORT")) // TEXT IMPORT NAME=.. FILE=..
			{
			if (GetSTRING(Str,"FILE=",TempFname,79) && GetSTRING(Str,"NAME=",TempName,79))
				{
				new(TempName,TempFname,IMPORT_Replace)UTextBuffer;
				}
			else Out->Log(LOG_ExecError,"Missing file or name");
			Processed=1;
			}
		else if (GetCMD(&Str,"EXPORT")) // TEXT EXPORT NAME=.. FILE=..
			{
			UTextBuffer *Text;
			if (GetSTRING(Str,"FILE=",TempFname,79) && GetUTextBuffer(Str,"NAME=",&Text))
				{
				Text->ExportToFile(TempFname);
				}
			else Out->Log(LOG_ExecError,"Missing file or name");
			Processed=1;
			}
		}
	//------------------------------------------------------------------------------------
	// CLASS functions
	//
	else if (GetCMD(&Str,"CLASS"))
		{
		SaveAsMacro = 0;
		if (GetCMD(&Str,"QUERY")) // CLASS QUERY
			{
			SaveAsMacro=0;
			UClass *Class;
			if (GetUClass(Str,"PARENT=",&Class))
				{
				classQueryForLink (Class); // Query root classes
				}
			else if (GetCMD(&Str,"ALL"))
				{
				classQueryForLink (NULL); // Query all classes
				}
			else Out->Log(LOG_ExecError,"Missing class");
			Processed=1;
			}
		else if (GetCMD(&Str,"SAVEBELOW")) // CLASS SAVEBELOW
			{
			UClass *Class;
			if (GetSTRING (Str,"FILE=",TempFname,80) &&
				GetUClass (Str,"NAME=",&Class))
				{
				GRes.UntagAll				();
				Class->SetFlags				(RF_TagExp);
				GRes.TagReferencingTagged	(RES_Class);
				//
				DoSaveClass:
				//
				// Snub out ParentClass to prevent tagging all parent classes above:
				//
				UClass *Parent     = Class->ParentClass;
				Class->ParentClass = NULL;
				GRes.SaveTagAllDependents();
				Class->ParentClass = Parent;
				//
				if (mystrstr(TempFname,"H") || mystrstr(TempFname,"TCX")) // Save as C++ header
					{
					Class->ExportToFile(TempFname);
					}
				else // Save as Unreal resource
					{
					GRes.SaveTagged(TempFname,FILE_NONE);
					};
				}
			else Out->Log(LOG_ExecError,"Missing file or name");
			Processed=1;
			}
		else if (GetCMD(&Str,"SAVE")) // CLASS SAVE
			{
			UClass *Class;
			if (GetSTRING (Str,"FILE=",TempFname,80) &&
				GetUClass (Str,"NAME=",&Class))
				{
				GRes.UntagAll();
				Class->Flags |= RF_TagExp;
				GRes.SaveTagAllDependents();
				goto DoSaveClass;
				}
			else Out->Log(LOG_ExecError,"Missing file or name");
			Processed=1;
			}
		else if (GetCMD(&Str,"SET")) // CLASS SET
			{
			UClass *Class;
			if (GetUClass(Str,"CLASS=",&Class) &&
				GetUMeshMap(Str,"MESHMAP=",&Class->DefaultActor.MeshMap))
				{
				Class->DefaultActor.DrawType = DT_MeshMap;
				}
			else Out->Log(LOG_ExecError,"Missing class or meshmap");
			Processed=1;
			}
		else if (GetCMD(&Str,"LOAD")) // CLASS LOAD FILE=..
			{
			if (GetSTRING (Str,"FILE=",TempFname,80))
				{
				if (mystrstr(TempFname,"TCX")) // Import from text file
					{
					// TempClass is a dummy class; it's import routine imports multiple, named classes
					UClass *TempClass = new("Temp",TempFname,IMPORT_Replace)UClass;
					TempClass->Kill();
					}
				else if (mystrstr(TempFname,"UCX"))// Load from resource file
					{
					GRes.AddFile (TempFname);
					}
				else Out->Log(LOG_ExecError,"Unrecognized file type");
				}
			else Out->Log(LOG_ExecError,"Missing filename");
			Processed=1;
			}
		else if (GetCMD(&Str,"NEW")) // CLASS NEW
			{
			UClass *Parent;
			if (GetUClass(Str,"PARENT=",&Parent) && 
				GetSTRING(Str,"NAME=",TempStr,NAME_SIZE))
				{
				UClass *Class = new(TempStr,CREATE_Replace)UClass(Parent);
				if (Class)
					{
					Class->ScriptText=new(TempStr,CREATE_Replace)UTextBuffer(0);
					}
				else Out->Log(LOG_ExecError,"Class not found");
				};
			Processed=1;
			}
		else if (GetCMD(&Str,"DELETE"))
			{
			UClass *Class;
			if (GetUClass(Str,"NAME=",&Class))
				{
				GTrans->Reset			("deleting actor class"); // Not tracked transactionally
				Level->Lock				(&LevelInfo,LOCK_Trans);
				edactDeleteDependentsOf (&LevelInfo,Class);
				Class->Delete			();
				Level->Unlock			(&LevelInfo);
				//
				GApp->EdCallback(EDC_SelActorChange,0);
				}
			else Out->Log(LOG_ExecError,"Missing name");
			Processed=1;
			};
		}
	//------------------------------------------------------------------------------------
	// MACRO functions
	//
	else if (GetCMD(&Str,"MACRO"))
		{
		if (GetCMD(&Str,"PLAY")) // MACRO PLAY [NAME=..] [FILE=..]
			{
			Word1 = GetSTRING (Str,"FILE=",TempFname,79);
			Word2 = GetSTRING (Str,"NAME=",TempName,79);
			//
			if (!Word2) strcpy (TempName,"MACRO");
			//
			UTextBuffer *Text;
			if (Word1) Text = new(TempName,TempFname,IMPORT_Replace)UTextBuffer;
			else       Text = new(TempName,FIND_Optional)UTextBuffer;
			//
			if (Text)
				{
				char Temp[256];
				const char *Data = Text->GetData();
				while (GetLINE (&Data,Temp,256)==0) Exec(Temp);
				}
			else Out->Log(LOG_ExecError,"Macro not found for playing");
			Processed=1;
			}
		else if (GetCMD(&Str,"RECORD")) // MACRO RECORD NAME=..
			{
			if (!GetSTRING (Str,"NAME=",TempName,79)) strcpy (TempName,"MACRO");
			//
			MacroRecBuffer = new(TempName,CREATE_Replace)UTextBuffer(MACRO_TEXT_REC_SIZE);
			SaveAsMacro = 0;
			//
			Out->Log(LOG_ExecError,"Macro record begin");
			Processed=1;
			}
		else if (GetCMD(&Str,"ENDRECORD")) // MACRO ENDRECORD
			{
			MacroRecBuffer = NULL;
			SaveAsMacro = 0;
			//
			Out->Log(LOG_ExecError,"Macro record ended");
			Processed=1;
			}
		else if (GetCMD(&Str,"LOAD")) // MACRO LOAD FILE=..
			{
			if (!GetSTRING (Str,"NAME=",TempName,79)) strcpy (TempName,"MACRO");
			if (GetSTRING(Str,"FILE=",TempFname,79))
				{
				new(TempName,TempFname,IMPORT_Replace)UTextBuffer;
				}
			else Out->Log(LOG_ExecError,"Missing filename");
			Processed=1;
			}
		else if (GetCMD(&Str,"SAVE")) // MACRO SAVE [NAME=..] FILE=..
			{
			if (!GetSTRING (Str,"NAME=",TempName,79)) strcpy (TempName,"MACRO");
			UTextBuffer *Text = new(TempName,FIND_Optional)UTextBuffer;
			//
			if (!Text) 										Out->Log(LOG_ExecError,"Macro not found for saving");
			else if (GetSTRING(Str,"FILE=",TempFname,79))	Text->ExportToFile(TempFname);
			else											Out->Log(LOG_ExecError,"Missing filename");
			Processed=1;
			}
		}
	//------------------------------------------------------------------------------------
	// MESH functions
	//
	else if (GetCMD(&Str,"MESH"))
		{
		if (GetCMD(&Str,"IMPORT")) // MESH IMPORT MESH=.. ANIVFILE=.. DATAFILE=..
			{
			if (GetSTRING(Str,   "MESH=",TempName,79) &&
				GetSTRING(Str,"ANIVFILE=",TempStr,79) &&
				GetSTRING(Str,"DATAFILE=",TempStr1,79))
				{
				meshImport (TempName,TempStr,TempStr1);
				}
			else Out->Log(LOG_ExecError,"Missing mesh, anivfile, or datafile");
			Processed=1;
			}
		else if (GetCMD(&Str,"ORIGIN")) // MESH ORIGIN X=.. Y=.. Z=..
			{
			UMesh *Mesh;
			if (GetUMesh(Str,"MESH=",&Mesh))
				{
				TempVector = GMath.UnitScaleVect; GetFVECTOR(Str,&TempVector);
				Mesh->Origin.X = TempVector.X;
				Mesh->Origin.Y = TempVector.Y;
				Mesh->Origin.Z = TempVector.Z;
				Mesh->RotOrigin = GMath.ZeroRotation;
				GetFROTATION (Str,&Mesh->RotOrigin,256);
				}
			else Out->Log(LOG_ExecError,"Missing mesh");
			Processed=1;
			}
		else if (GetCMD(&Str,"SEQUENCE")) // MESH SEQUENCE MESH=.. SEQ=.. STARTFRAME=.. NUMFRAMES=..
			{
			UMesh *Mesh;
			if (GetUMesh	(Str,      "MESH=",&Mesh) &&
				GetSTRING   (Str,       "SEQ=",TempStr  ,79) &&			
				GetWORD		(Str,"STARTFRAME=",&Word1      ) &&
				GetWORD		(Str, "NUMFRAMES=",&Word2      ))
				{
				Mesh->SetSequence(TempStr,Word1,Word2);
				}
			else Out->Log(LOG_ExecError,"Missing mesh, sequence, startframe, or numframes");
			Processed=1;
			}
		else if (GetCMD(&Str,"SAVE")) // MESH SAVE MESH=.. FILE=..
			{
			UMesh *Mesh;
			if (GetUMesh   (Str,"MESH=",&Mesh) &&
				GetSTRING  (Str,"FILE=",TempFname,79    ))
				{
				Out->Log("Saving mesh");
				GRes.Save (Mesh,TempFname,FILE_NONE);
				}
			else Out->Log(LOG_ExecError,"Missing mesh or filename");
			Processed=1;
			}
		}
	//------------------------------------------------------------------------------------
	// MESHMAP functions
	//
	else if (GetCMD(&Str,"MESHMAP"))
		{
		if (GetCMD(&Str,"NEW")) // MESHMAP NEW MESHMAP=.. MESH=..
			{
			UMesh *Mesh;
			if (GetSTRING (Str,"MESHMAP=",TempName,NAME_SIZE) &&
				GetUMesh  (Str,  "MESH=",&Mesh))
				{
				UMeshMap *MeshMap = new(TempName,CREATE_Replace)UMeshMap;
				//
				Word2 = MAXWORD; GetWORD (Str,"AND=",&Word2);
				Word3 = 0;       GetWORD (Str, "OR=",&Word3);
				//
				MeshMap->Mesh	     = Mesh;
				MeshMap->MaxTextures = 16;
				MeshMap->AndFlags    = Word2;
				MeshMap->OrFlags     = Word3;
				//
				MeshMap->AllocData(1);
				}
			else Out->Log(LOG_ExecError,"Missing meshmap or mesh");
			Processed=1;
			}
		else if (GetCMD(&Str,"SCALE")) // MESHMAP SCALE X=.. Y=.. Z=..
			{
			UMeshMap *MeshMap;
			if (GetUMeshMap(Str,"MESHMAP=",&MeshMap))
				{
				TempVector = GMath.UnitScaleVect; GetFVECTOR(Str,&TempVector);
				MeshMap->Scale = TempVector;
				}
			else Out->Log(LOG_ExecError,"Missing meshmap");
			Processed=1;
			}
		else if (GetCMD(&Str,"SETTEXTURE")) // MESHMAP SETTEXTURE MESHMAP=.. NUM=.. TEXTURE=..
			{
			UMeshMap *MeshMap;
			UTexture *Texture;
			if (GetUMeshMap(Str, "MESHMAP=",&MeshMap) &&
			    GetUTexture(Str,"TEXTURE=", &Texture) &&
				GetWORD    (Str,    "NUM=",&Word2))
				{
				if (Word2 >= MeshMap->MaxTextures) Out->Log(LOG_ExecError,"Texture number exceeds maximum");
				else MeshMap->GetData()[Word2] = Texture;
				}
			else Out->Log(LOG_ExecError,"Missing meshmap, texture, or num");
			Processed=1;
			}
		else if (GetCMD(&Str,"SAVE")) // MESHMAP SAVE MESHMAP=.. FILE=..
			{
			UMeshMap *MeshMap;
			if (GetUMeshMap(Str,"MESHMAP=",&MeshMap) &&
				GetSTRING  (Str,"FILE=",TempFname,79    ))
				{
				GRes.SaveDependent (MeshMap,TempFname,FILE_NONE);
				}
			else Out->Log(LOG_ExecError,"Missing meshmap or file");
			Processed=1;
			}
		}
	//------------------------------------------------------------------------------------
	// SENDACTORS: Send some text command to all selected actors
	//
	else if (GetCMD(&Str,"SENDACTORS"))
		{
		Level->Lock(&LevelInfo,LOCK_Trans);
		//
		PExec ExecInfo;
		ExecInfo.iSourceActor = INDEX_NONE;
		mystrncpy(ExecInfo.Arg,Str,TEXTMSG_LENGTH);
		//
		AActor *Actor = &Level->ActorList->Element(0);
		for (int i=0; i<Level->ActorList->Max; i++)
			{
			if (Actor->Class && Actor->bSelected)
				{
				GTrans->NoteActor(Level->ActorList,i);
				LevelInfo.SendMessage(i,ACTOR_Exec,&ExecInfo);
				};
			Actor++;
			};
		Level->Unlock(&LevelInfo);
		Processed=1;
		}
	//------------------------------------------------------------------------------------
	// DEBUG: Misc debugging
	//
	else if (GetCMD(&Str,"DEBUG"))
		{
		if (GetCMD(&Str,"CRASH"))
			{
			appError ("Unreal crashed at your request");
			Processed=1;
			}
		else if (GetCMD(&Str,"GPF"))
			{
			Out->Log("Unreal crashing with voluntary GPF");
			*(int *)NULL = 123;
			Processed=1;
			}
		else if (GetCMD(&Str,"EATMEM"))
			{
			Out->Log("Eating up all available memory");
			while (1)
				{
				void *Eat = GApp->Malloc(65536,"EatMem");
				memset(Eat,0,65536);
				};
			Processed=1;
			};
		}
	//------------------------------------------------------------------------------------
	// SCRIPT: script compiler
	//
	else if (GetCMD(&Str,"SCRIPT"))
		{
		if (GetCMD(&Str,"COMPILE")) // Compile one script
			{
			UClass *Class;
			if (GetUClass(Str,"CLASS=",&Class))
				{
				CompileScript(Class,1);
				}
			else Out->Log(LOG_ExecError,"Missing class");
			Processed=1;
			};
		if (GetCMD(&Str,"MAKE")) // Make all scripts
			{
			if (GetCMD(&Str,"ALL")) MakeScripts(GClasses.Root,1); // Make all
			else					MakeScripts(GClasses.Root,0); // Make changed scripts
			Processed=1;
			};
		}
	//------------------------------------------------------------------------------------
	// LEVEL: Level functions
	//
	else if (GetCMD(&Str,"LEVEL"))
		{
		if (GetCMD(&Str,"REDRAW"))
			{
			GCameraManager->RedrawLevel(Level);
			Processed=1;
			}
		}
	//------------------------------------------------------------------------------------
	// AUDIO: audio functions
	//
	else if (GetCMD(&Str,"AUDIO"))
		{
		AudioCmdLine(Str);
		Processed=1;
		}
	//------------------------------------------------------------------------------------
	// CUTAWAY: cut-away areas for overhead view
	//
	else if (GetCMD(&Str,"CUTAWAY"))
		{
		if (GetCMD(&Str,"SHOW"))
			{
			Word1 = 1; // Show
			ShowOrHideCutaway:;
			//
			Word2=0;
			if (GetCMD(&Str,"ALL")) Word2=1;
			else if (GetCMD(&Str,"SELECTED")) Word2=2;
			//
			if (Word2)
				{
				// Cutaway logic goes here
				};
			Processed=1;
			}
		else if (GetCMD(&Str,"HIDE"))
			{
			Word1 = 0; // Hide
			goto ShowOrHideCutaway;
			};
		}
	//------------------------------------------------------------------------------------
	// TASK: slow task status
	//
	else if (GetCMD(&Str,"TASK"))
		{
		if (GetCMD(&Str,"BEGIN"))
			{
			char Temp[256];
			if (!GetSTRING(Str,"MESSAGE=",Temp,256)) strcpy(Temp,"Please wait...");
			GApp->BeginSlowTask(Temp,1,0);
			Processed=1;
			}
		else if (GetCMD(&Str,"END"))
			{
			GApp->EndSlowTask();
			Processed=1;
			};
		}
	//------------------------------------------------------------------------------------
	// Done with this command.  Now note it (if we're recording a macro) and go to
	// next command:
	//
	if (SaveAsMacro) NoteMacroCommand (Stream);
	return Processed;
	//
	UNGUARD_BEGIN
	UNGUARD_MSGF("FEditor::Exec(%s)%s",ErrorTemp,(strlen(ErrorTemp)>=69)?"..":"");
	UNGUARD_END
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
