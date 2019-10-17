/*=============================================================================
FILENAME:     UnSound.cpp
DESCRIPTION:  Implementation of the "USound" class and related routines.
NOTICE:       Copyright 1996 Epic MegaGames, Inc.  This software is a trade
              secret.
TOOLS:        Compiled with Visual C++ 4.0, Calling method=__fastcall
FORMAT:       8 characters per tabstop, 100 characters per line.
HISTORY:
  When      Who                 What
  ----      ---                 ----
  ??/??/96  Tim Sweeney         Create stubs for this module.
  04/18/96  Ammon R. Campbell   Misc. hacks started.
=============================================================================*/

/*
----------------------------
ADBG:  Define this symbol
       for lots of debug
       output.
----------------------------
*/
// #define ADBG

/********************************* INCLUDES ********************************/

#include "Unreal.h"		// Unreal engine includes
#include "SoundEng.h"	// Headers for using SoundEng.lib

/********************************* HEADERS *********************************/

static void audioQueryFamilyForLink(void);
static void audioQuerySoundForLink(char *FamilyName);

/********************************* CONSTANTS *******************************/

/* Maximum number of results during query. */
#define MAX_RESULTS	1024
#define MAX_FAM_RESULTS	256

/********************************* VARIABLES *******************************/

/* Used during sound queries. */
static DWORD GCurResult = 0;
static DWORD GNumResults = 0;
static USound **GTempList = NULL;

/* Used during family queries. */
static DWORD GNumFamResults = 0;
static DWORD GCurFamResult = 0;
static FName *GTempFamList = NULL;

/********************************* FUNCTIONS *******************************/

/*************************************************************************
                     Implementation of USound class
*************************************************************************/

/*
** USound::Register:
** Register the resource type.
*/
void
USound::Register(FResourceType *Type)
{
	GUARD;
#ifdef ADBG
	debugf(LOG_Audio, "USound::Register(Type)\n");
#endif /* ADBG */
	Type->HeaderSize = sizeof (USound);
	Type->RecordSize = 1;
	Type->Version    = 1;
	Type->TypeFlags = RTF_ScriptReferencable;
	strcpy (Type->Descr,"Sound");
	UNGUARD("USound::Register");
};

/*
** USound::InitHeader:
** Initialize the header of the resource.
*/
void
USound::InitHeader(void)
{
	GUARD;
#ifdef ADBG
	debugf(LOG_Audio, "USound::InitHeader()\n");
#endif /* ADBG */

	/* Zero all the variables to known states. */
	DataSize = 0;
	FamilyName = NAME_NONE;
	SoundID = -1;
	mymemset(Pad, 0, sizeof(Pad));

	UNGUARD("USound::InitHeader");
};

/*
** USound::InitData:
**   ???
*/
void
USound::InitData(void)
{
	GUARD;
#ifdef ADBG
	debugf(LOG_Audio, "USound::InitData()\n");
#endif /* ADBG */

	mymemset(Data, 0, DataSize);

	UNGUARD("USound::InitData");
};

/*
** USound::QuerySize:
** Determine size of resource.
**
** Parameters:
**	NONE
**
** Returns:
**	Size of resource in bytes.
*/
int
USound::QuerySize(void)
{
	GUARD;
#ifdef ADBG
	debugf(LOG_Audio, "USound::QuerySize()\n");
#endif /* ADBG */

	return DataSize;

	UNGUARD("USound::QuerySize");
} /* End USound::QuerySize() */

/*
** USound::QueryMinSize:
** Determine size of resource.
**
** Parameters:
**	NONE
**
** Returns:
**	Size of resource in bytes.
*/
int
USound::QueryMinSize(void)
{
	GUARD;
#ifdef ADBG
	debugf(LOG_Audio, "USound::QueryMinSize()\n");
#endif /* ADBG */

	return QuerySize();

	UNGUARD("USound::QueryMinSize");
} /* End USound::QueryMinSize() */

/*
** USound::Import:
** Import a sound effect from a buffer in memory.
**
** Parameters:
**	Name		Description
**	----		-----------
**	Buffer		Pointer to data containing resource to
**			be imported.
**	BufferEnd	Pointer to first byte following resource
**			to be imported.
**	FileType	???
**
** Returns:
**	Value	Meaning
**	-----	-------
**	NULL	Error occured.
**	other	Same as BufferEnd if successful.
*/
const char *
USound::Import(const char *Buffer, const char *BufferEnd,const char *FileType)
{
	GUARD;

#ifdef ADBG
	debugf(LOG_Audio, "USound::Import(Buffer, BufferEnd, FileType)\n");
#endif /* ADBG */

	/* If resource is a WAV, turn it into a one-shot UFX */
	/* NOT CODED YET */

	/* Determine size of resource in bytes. */
	DataSize = (int)(BufferEnd - Buffer);

	/* Make space for resource. */
	Realloc();

	/* Copy import buffer to our data buffer. */
	memcpy(Data, Buffer, DataSize);

	return BufferEnd;

	UNGUARD("USound::Import");
} /* End USound::Import() */

/*
** USound::Export:
** Export a sound effect to a buffer in memory.
**
** Parameters:
**	Name		Description
**	----		-----------
**	Buffer		Pointer to data containing resource to
**			be imported.
**	FileType	Type of file, i.e. WAV
**	Ident		???
**
** Returns:
**	Value	Meaning
**	-----	-------
**	any	Same as BufferEnd if successful.
*/
char *
USound::Export(char *Buffer, const char *FileType, int Indent)
{
	GUARD;

#ifdef ADBG
	debugf(LOG_Audio, "USound::Export(Buffer, BufferEnd, FileType)\n");
#endif /* ADBG */

	/* Copy from our data buffer to export buffer. */
	memcpy(Buffer, Data, DataSize);

	return Buffer + DataSize;

	UNGUARD("USound::Export");
} /* End USound::Export() */

/*
** USound::QueryHeaderReferences:
**   ???
*/
void
USound::QueryHeaderReferences(FResourceCallback &Callback)
{
	GUARD;
#ifdef ADBG
	debugf(LOG_Audio, "USound::QueryHeaderReferences(&Callback)\n");
#endif /* ADBG */

	Callback.Name(this, &FamilyName, 0);

	UNGUARD("USound::QueryHeaderReferences");
} /* End USound::QueryHeaderReferences() */

/*
** USound::QueryDataReferences:
**   ???
*/
void
USound::QueryDataReferences(FResourceCallback &Callback)
{
	GUARD;
#ifdef ADBG
	debugf(LOG_Audio, "USound::QueryDataReferences(&Callback)\n");
#endif /* ADBG */

	// Doesn't reference any other resources

	UNGUARD("USound::QueryDataReferences");
} /* End USound::QueryDataReferences() */

AUTOREGISTER_RESOURCE(RES_Sound,USound,0xB2D90875,0xCCD211cf,0x91360000,0xC028B992);

/*************************************************************************
                      Unreal editor hooks for audio
*************************************************************************/

/*---------------------------------------------------------------------------
	Audio command link
---------------------------------------------------------------------------*/

/*
** AudioCmdLine:
** Receives audio-related commands from the Unreal editor.
** Tim's comment:
**   See the huge function in UnEdSrv.cpp and the parsers in UnParams.cpp
**   for an example of how to do this.
**
** Parameters:
**	Name	Description
**	----	-----------
**	Str	Remainder of command line after first token is removed.
**
** Returns:
**	NONE
*/
void
UNREAL_API AudioCmdLine(const char *Str)
{
	GUARD;

	char TempStr[256];

	debugf(LOG_Audio, "AudioCmdLine(\"%s\")\n", Str);

	/*
	** Do the right thing, depending on what editor
	** command was just given to us.
	*/
	if (GetCMD(&Str, "LOADFAMILY")) // AUDIO LOAD FILE=...
	{
		char Fname[80];

		/*
		** Load the specified audio resources file
		** into memory.
		*/
		if (GetSTRING(Str,"FILE=",Fname,80))
		{
			/* Add the specified resource file. */
			GRes.AddFile(Fname);
		}
	}
	else if (GetCMD(&Str, "SAVEFAMILY"))
	{
		FName	Name;			/* Family name. */
		char	TempFname[80];		/* Filename. */

		/*
		** Save the specified audio resources
		** to a file.
		*/
		if (GetCMD(&Str,"ALL") && GetSTRING(Str,"FILE=",TempFname,79))
		{
			/*
			** Save all sound resources to specified file.
			*/

			USound * Sound;		/* Temporary sound resource. */

			GRes.UntagAll();
			FOR_ALL_TYPED_RES(Sound,RES_Sound,USound)
			{
				if (!Sound->FamilyName.IsNone())
				{
					Sound->Flags |= RF_TagExp;
				}
			}
			END_FOR_ALL_TYPED_RES;
			GRes.SaveDependentTagged(TempFname, 0);
		}
		else if ((GetSTRING(Str,"FILE=",TempFname,79)) &&
			GetNAME(Str,"FAMILY=",&Name))
		{
			/*
			** Save sound resources from specified family
			** to specified file.
			*/

			GRes.UntagAll();
			if (GRes.TagAllReferencingName(Name, RES_Sound) == 0)
			{
				debug(LOG_Audio,
					"AUDIO SAVEFAMILY:  Unknown family specified");
			}
			else
			{
				GRes.SaveDependentTagged(TempFname, 0);
			}
		}
	}
	else if (GetCMD(&Str, "IMPORT"))
	{
		char	TempFName[256];		/* Filename. */
		char	TempName[256];		/* Name of resource. */

		/*
		** Import a WAV file as a new resource.
		*/

		/* Get parameters from command line. */
		if (!GetSTRING(Str, "FILE=", TempFName, 256))
		{
			debugf(LOG_Audio, "AUDIO IMPORT:  No FILE= specified!\n");
			return;
		}
		if (!GetSTRING(Str, "NAME=", TempName, 256))
		{
			debugf(LOG_Audio, "AUDIO IMPORT:  No NAME= specified!\n");
			return;
		}

		/* Create the sound resource. */
		USound *Sound = new(TempName, TempFName, IMPORT_Replace)USound;
		if (Sound == NULL)
		{
			debugf(LOG_Audio, "AUDIO IMPORT:  Failed creating new resource!\n");
			return;
		}

		GetNAME(Str, "FAMILY=", &Sound->FamilyName);
		debugf(LOG_Audio, "AUDIO IMPORT:  Sound resource created.\n");
	}
#if 0
	else if (GetCMD(&Str, "EXPORT"))
	{
		/*
		** Export a sound resource to a WAV file.
		*/
		/* Not coded yet */
	}
#endif
	else if (GetCMD(&Str, "TEST"))
	{
		/*
		** Plays a sound resource.  This happens
		** when the user presses the "test sound"
		** button in the Unreal editor.
		*/
		/* Not coded yet */
	}
	else if (GetCMD(&Str, "KILL"))
	{
		/*
		** ???
		*/
		USound *Sound;
		if (GetUSound(Str, "NAME=", &Sound))
		{
			Sound->FamilyName = NAME_NONE;
		}
	}
	else if (GetCMD (&Str,"QUERY"))
	{
		/* Command:  AUDIO QUERY [FAMILY=xxx] */

		if (GetSTRING(Str, "FAMILY=", TempStr, NAME_SIZE))
		{
			/* Return list of sounds in family. */
			audioQuerySoundForLink(TempStr);
		}
		else
		{
			/* Return list of sound families. */
			audioQueryFamilyForLink();
		}
	}
	else if (GetCMD (&Str,"DEBUG"))
	{
		/*
		** Output list of audio resources to log window,
		** for debugging.  This is called by issuing the
		** "AUDIO DEBUG" command to the UnrealEd console.
		*/
		USound	*Sound;
		int	i;

		/* Loop through all the sound resources. */
		i = 0;	
		FOR_ALL_TYPED_RES(Sound, RES_Sound, USound)
		{
			debugf("USound %d:\n", i);
			debugf("  Name == \"%s\"\n",
				Sound->Name);
			if (!Sound->FamilyName.IsNone())
			{
				debugf("  FamilyName == \"%s\"\n",
					Sound->FamilyName.Name());
			}
			else
			{
				debugf("  Sound has no family name!\n");
			}
			i++;
		}
		END_FOR_ALL_TYPED_RES;

		/* If there were no sound resources, say so. */
		if (i < 1)
		{
			debugf("No sound resources found.\n");
		}
	}
	else
	{
		/* Add more commands as necessary... */
	}

	UNGUARD("AudioCmdLine");
} /* End AudioCmdLine() */

/*-----------------------------------------------------------------------------
	Audio link topic functions
-----------------------------------------------------------------------------*/

//
// Query a list of sounds.  Call with sound family's name,
// or "All" for all families.
//
static void
audioQuerySoundForLink(char *FamilyName)
{
	GUARD;

	FName	Name;
	int	All = !stricmp(FamilyName,"All");
	USound	*Sound;

#ifdef ADBG
	debugf(LOG_Audio, "audioQuerySoundForLink(\"%s\")\n", FamilyName);
#endif /* ADBG */

	Name.Add(FamilyName);
	if (GTempList==NULL)
		GTempList = (USound **)appMalloc(MAX_RESULTS * sizeof(USound *),"Ammon");
	GCurResult  = 0;
	GNumResults = 0;

	FOR_ALL_TYPED_RES(Sound,RES_Sound,USound)
	{
		if ((GNumResults < MAX_RESULTS) &&
			((Sound->FamilyName == Name) ||
			(All && (!Sound->FamilyName.IsNone()))))
		{
			GTempList[GNumResults++] = Sound;
		}
	}
	END_FOR_ALL_TYPED_RES;

	UNGUARD("audioQuerySoundForLink");
} /* End audioQuerySoundForLink() */

//
// Query a list of sound families.
//
static void
audioQueryFamilyForLink(void)
{
	GUARD;

	USound	*Sound;
	FName	ListName;

#ifdef ADBG
	debugf(LOG_Audio, "audioQueryFamilyForLink()\n");
#endif /* ABG */

	if (GTempFamList == NULL)
	{
		GTempFamList = (FName *)appMalloc(MAX_FAM_RESULTS * sizeof (FName),"Ammon"); // Must free !!
	}

	GCurFamResult  = 0;
	GNumFamResults = 0;

	GTempFamList [GNumFamResults++].Add("All");
	FOR_ALL_TYPED_RES(Sound,RES_Sound,USound)
	{
		ListName = Sound->FamilyName;
		if (!ListName.IsNone())
		{
			DWORD j;
			for (j = 0; j < GNumFamResults; j++)
			{
				if (GTempFamList[j] == ListName)
					break;
			}
			if ((j >= GNumFamResults) && (GNumFamResults < MAX_FAM_RESULTS))
			{
				GTempFamList [GNumFamResults++] = ListName;
			}
		}
	}
	END_FOR_ALL_TYPED_RES;

	UNGUARD("audioQueryFamilyForLink");
} /* End audioQueryFamilyForLink() */

//
// Communicates information between Unreal and UnrealEd.
//
AUTOREGISTER_TOPIC("Audio",AudioTopicHandler);

/*
** AudioTopicHandler::Get
** Gets called when Ed.Server.GetProp("Audio", "{item}") is
** called from the unreal editor in Visual Basic.
*/
void
AudioTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
{
	GUARD;
	USound	*Sound;
	FName	FamilyName;

#ifdef ADBG
	debugf(LOG_Audio, "AudioTopicHandler::Get(Level, Topic==\"%s\", Item==\"%s\", Data)\n",
			Topic, Item);
#endif /* ADBG */

	if ((stricmp(Item,"QUERYAUDIO")==0) && (GCurResult < GNumResults))
	{
		Sound = GTempList[GCurResult];
		//
		sprintf(Data, "%s", Sound->Name);
		GCurResult++;
	}
	else if ((stricmp(Item,"QUERYFAM")==0) && (GCurFamResult < GNumFamResults))
	{
		FamilyName = GTempFamList [GCurFamResult];
		sprintf(Data, "%s", FamilyName.Name());
		GCurFamResult++;
	}
#if 0
	else if ((stricmp(Item, "VibratoDepth") == 0)
	{
	}
	else if ((stricmp(Item, "VibratoSpeed") == 0)
	{
	}
	else if ((stricmp(Item, "TremoloDepth") == 0)
	{
	}
	else if ((stricmp(Item, "TremoloSpeed") == 0)
	{
	}
#endif
	UNGUARD("AudioTopicHandler::Get");
} /* End AudioTopicHandler::Get() */

/*
** AudioTopicHandler::Set
** Gets called when Ed.Server.SetProp("Audio", "{item}", "{data}") is
** called from the unreal editor in Visual Basic.
*/
void
AudioTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
{
	GUARD;

#ifdef ADBG
	debugf(LOG_Audio, "AudioTopicHandler::Set(Level, Topic==\"%s\", Item==\"%s\", Data)\n",
			Topic, Item);
#endif /* ADBG */

	if (stricmp(Item,"YourParameter")==0)
	{
		// Set some parameter based on the contents of *Data
	};
	UNGUARD("AudioTopicHandler::Set");
} /* End AudioTopicHandler::Set() */

/*
=============================================================================
End UnSound.cpp
=============================================================================
*/
