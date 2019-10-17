/*=============================================================================
	UnCamMgr.cpp: Unreal camera manager, generic implementation

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

/*-----------------------------------------------------------------------------
	FCameraManager generic implementation
-----------------------------------------------------------------------------*/

//
// Redraw all cameras looking at the level.
//
void FCameraManager::RedrawLevel(ULevel *Level)
	{
	GUARD;
	for (int i=0; i<CameraArray->Num; i++)
		{
		UCamera *Camera = CameraArray->Element(i);
	    if ((Camera->Level == Level) &&
	     	(!Camera->OnHold) &&
			Camera->SXR && Camera->SYR)
			{
			Camera->Draw(0);
			};
		};
	UNGUARD("FCameraManager::RedrawLevel");
	};

//
// Close all cameras that are child windows of a specified window.
//
void FCameraManager::CloseWindowChildren(DWORD ParentWindow)
	{
	GUARD;
	char TempName[NAME_SIZE];
	//
	for (int i=CameraArray->Num-1; i>=0; i--)
	   	{
		UCamera *Camera = CameraArray->Element(i);
		strcpy (TempName,Camera->Name); strupr(TempName);
		//
		if ((ParentWindow==0) ||
			(Camera->ParentWindow==ParentWindow) ||
			((ParentWindow==MAXDWORD)&&!strstr(TempName,"STANDARD"))
			)
			{
			Camera->Kill();
			};
		};
	UNGUARD("FCameraManager::CloseWindowChildren");
	};

/*-----------------------------------------------------------------------------
	Globals to reconcile actors and cameras after loading or creating a new level
-----------------------------------------------------------------------------*/

//
// Update all actor Camera references in the world based on the Cameras
// controlling them.
//
void FCameraManager::UpdateActorUsers(void)
	{
	GUARD;
	//
	// Dissociate all Cameras and cameras from all actors:
	//
	int i;
	for (i=0; i<GServer.Levels->Num; i++)
		{
		GServer.Levels->Element(i)->DissociateActors();
		};
	//
	// Hook all cameras up to their corresponding actors:
	//
	for (i=0; i<GCameraManager->CameraArray->Num; i++)
		{
		UCamera *Camera = GCameraManager->CameraArray->Element(i);
		AActor  *Actor  = &Camera->GetActor();
		if (Actor->Class == NULL)
			{
			debugf (LOG_Ed,"cameraUpdateActorCameras: Bad actor association");
			Camera->Kill();
			}
		else Actor->Camera = Camera;
		};
	//
	// Hook all logged-in Cameras up to their corresponding actors:
	//
	/* Not yet implemented */
	//
	UNGUARD("FCameraManager::UpdateActorUsers");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
