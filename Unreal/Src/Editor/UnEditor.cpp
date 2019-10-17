/*=============================================================================
	UnEditor.cpp: Unreal editor main file

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

/*---------------------------------------------------------------------------------------
	Globals
---------------------------------------------------------------------------------------*/

UNEDITOR_API FEditor GUnrealEditor;

/*-----------------------------------------------------------------------------
	Init & Exit
-----------------------------------------------------------------------------*/

//
// Init the editor:
//
void FEditor::Init(void)
	{
	GUARD;
	//
	// Init misc:
	//
	GGfx.DefaultCameraFlags  = SHOW_Frame | SHOW_MovingBrushes | SHOW_Actors | SHOW_Brush | SHOW_Menu;
	GGfx.DefaultRendMap      = REN_PlainTex;
	//
	MacroRecBuffer = NULL;
	//
	// Allocate resources needed by editor:
	//
	TempModel = new("Temp",CREATE_Unique)UModel(1);
	//
	// Allocate editor resource array and fill it:
	//
	EditorArray = new("Editor",CREATE_Unique)UArray(64);
	EditorArray->Add(TempModel);
	//
	// Keep all classes active:
	//
	EditorArray->Add(GClasses.Camera);
	EditorArray->Add(GClasses.Player);
	EditorArray->Add(GClasses.Light);
	//
	// Transaction tracking system:
	//
	GTrans 					= new("Undo",CREATE_Unique)UTransBuffer;
	GTrans->MaxTrans		= GDefaults.MaxTrans;
	GTrans->MaxChanges		= GDefaults.MaxChanges;
	GTrans->MaxDataOffset	= GDefaults.MaxDataOffset;
	GTrans->Locked 			= 0;
	GTrans->NumTrans		= 0;
	GTrans->NumChanges		= 0;
	GTrans->Locked			= 0;
	GTrans->Overflow		= 0;
	GTrans->TransCount		= 0;
	GTrans->UndoTransCount	= 0;
	strcpy (GTrans->ResetAction,"startup");
	//
	// Allocate data for resource (includes space for transaction index, change log, and data):
	//
	GTrans->AllocData(0);
	//
	// Add Undo resource to editor resource array:
	//
	GUnrealEditor.EditorArray->Add(GTrans);
	debug (LOG_Init,"Transaction tracking system initialized");
	//
	// Other editor info:
	//
	CurrentTexture = NULL;
	//
	// Settings:
	//
	Mode			= EM_None;
	MovementSpeed	= 4.0;
	MapEdit         = 0;
	ShowVertices	= 0;
	//
	// Constraints:
	//
	constraintInit (&Constraints);
	//
	// Set editor mode:
	//
	edcamSetMode (EM_CameraMove);
	//
	// Add editor array to root:
	//
	GRes.Root->Add(EditorArray);
	debug(LOG_Init,"Editor initialized");
	UNGUARD("FEditor::Init");
	};

void FEditor::Exit(void)
	{
	GUARD;
	//
	// Shut down transaction tracking system:
	//
	if (GTrans)
		{
		if (GTrans->Locked) debug (LOG_Trans,"Warning: A transaction is active");
		GTrans->Reset ("shutdown"); // Purge any unused resources held in undo-limbo
		//
		GUnrealEditor.EditorArray->Delete(GTrans);
		GTrans->Kill();
		GTrans=NULL;
		//
		debug (LOG_Exit,"Transaction tracking system closed");
		};
	//
	// Remove editor array from root:
	//
	GRes.Root->Delete(EditorArray);
	debug(LOG_Exit,"Editor closed");
	//
	UNGUARD("FEditor::Exit");
	};

/*-----------------------------------------------------------------------------
	Macro recording
-----------------------------------------------------------------------------*/

void FEditor::NoteMacroCommand(const char *Cmd)
	{
	GUARD;
	char *Text;
	int	 Length;
	//
	if (MacroRecBuffer)
		{
		Text   = MacroRecBuffer->GetData();
		Length = strlen(Cmd);
		//
		if ((Length + MacroRecBuffer->Num + 3) < MacroRecBuffer->Max)
			{
			if (MacroRecBuffer->Num<1) MacroRecBuffer->Num = 1;
			//
			strcpy (Text + MacroRecBuffer->Num          - 1,Cmd);
			strcpy (Text + MacroRecBuffer->Num + Length - 1,"\r\n");
			MacroRecBuffer->Num += Length + 2;
			}
		else
			{
			debug (LOG_Ed,"Macro recording buffer is full");
			};
		};
	UNGUARD("FEditor::NoteMacroCommand");
	};

/*---------------------------------------------------------------------------------------
	The End
---------------------------------------------------------------------------------------*/
