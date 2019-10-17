/*=============================================================================
FILENAME:     UnFGAud.cpp
DESCRIPTION:  Implementation of the "FGlobalAudio" class and related
              routines.
NOTICE:       Copyright 1996 Epic MegaGames, Inc.  This software is a trade
              secret.
TOOLS:        Compiled with Visual C++ 4.0. Best viewed with Tabs=4.
FORMAT:       8 characters per tabstop, 100 characters per line.
HISTORY:
  When      Who                 What
  --------- ------------------- -------------------------------------------
  07/17/96  Ammon R. Campbell   This rewrite obsoletes the totally
                                different prior versions.
  07/30/96  Ammon R. Campbell   Added function to specify which song should
                                be played, based on an integer song number.
=============================================================================*/

/*
--------------------------------------------
ADBG:  Define this symbol to enable extra
       logfile output for audio debugging
--------------------------------------------
*/
//#define ADBG

/********************************* INCLUDES ********************************/

#include <stdlib.h>		/* For min()/max() */
#include <Windows.h>		/* For GetTickCount(), and required for Galaxy.h */
#include "Unreal.h"		/* Unreal headers (includes "UnFGAud.h") */
#include "SoundEng.h"		/* Declarations for using SoundEng.lib */
#include "UnConfig.h"

/********************************* HEADERS *********************************/

	/* See "UnFGAud.h" for public declarations and prototypes */

/********************************* CONSTANTS *******************************/

/******************************* LOCAL TYPES *******************************/

/******************************* LOCAL CLASSES *****************************/

/********************************* VARIABLES *******************************/

/*
** GAudio:
** The instantiation of the global audio class.
*/
FGlobalAudio GAudio;

/****************************************************
The following variables should be treated as if they
were private member variables of FGlobalAudio.  See
comments at the top of this file.
****************************************************/

/*
** flag_audio_inited:
** Set to 1 after Init() has been called but before Exit() has been
** called.  This just means that the global audio stuff has been
** initialized.  This is not the same as flag_galaxy_inited.  If
** there's no sound card present, flag_audio_inited will initialize
** to 1, but flag_galaxy_inited will stay 0.
*/
static int	flag_audio_inited = 0;

/*
** flag_editor_mode:
** Set to 1 if running in 'editor' mode, which means music
** does not automatically play and sound resources are not
** loaded into galaxy.
*/
static int	flag_editor_mode = 0;

/*
** flag_paused:
** Set to 1 if output is temporarily paused by ::Pause().
** Set back to 0 when output is restored by ::UnPause().
*/
static int	flag_paused = 0;

/*
** num_ticks:
** The number of times FGlobalAudio::Tick() has been
** called.
*/
static unsigned long	num_ticks = 0L;

/********************************* FUNCTIONS *******************************/

/*************************************************************************
                      Functions local to this module
*************************************************************************/

/*
** plog:
** Function used for logging output from the sound engine
** library.
**
** Parameters:
**	Name	Description
**	----	-----------
**	msg	String to be output to log.
**
** Returns:
**	NONE
*/
void __cdecl
plog(char *msg)
{
	if (msg[0] != '\0' && msg[strlen(msg) - 1] != '\n')
		msg[strlen(msg) - 1] = '\0';
	debugf(LOG_Audio, msg);
} /* End plog() */

/*************************************************************************
             Implementation of FGlobalAudio member functions
*************************************************************************/

/*
** FGlobalAudio::Init:
** Performs tasks necessary to start up the audio system
** cleanly.  Called once when the application starts up.
**
** Parameters:
**	Name		Description
**	----		-----------
**	MakeActive	Flag; 1 if unreal game is starting,
**			or 0 if unreal editor is starting.
**
** Returns:
**	Value	Meaning
**	-----	-------
**	1	No error.
**	0	Fatal error occured.
*/
int
FGlobalAudio::Init(int MakeActive)
{
	GUARD;

	/* Connect sound engine log output to Unreal log. */
	SoundEngLogSetup(plog);
	SoundEngLogDetail(1);
	plog("FGlobalAudio::Init()\n");

	/* Initialize sound engine. */
	if (!SoundEngGlobalInit(MakeActive))
	{
		/* Init failed. */
		return 0;
	}
	SpecifySong(1);

    // Stuff added by Mark: to be checked by Ammon...
    {
        //todo: Clean-up - these strings are both here and in unconfig.cpp...
        MusicVolumeSet( GConfiguration.GetInteger( FConfiguration::AudioSection, "MusicVolume", 42 ) );        
        SfxVolumeSet( GConfiguration.GetInteger( FConfiguration::AudioSection, "SoundVolume", 127 ) );        
        DirectSoundFlagSet( GConfiguration.GetBoolean( FConfiguration::AudioSection, "UseDirectSound", TRUE ) );
        //todo: Sampling rate? (New profile string must be added to unconfig.cpp)
    }

	return 1;
	UNGUARD("FGlobalAudio::Init");
} /* End FGlobalAudio::Init() */

/*
** FGlobalAudio::Exit:
** Performs tasks necessary to shut down the audio system
** cleanly.  Called once just before Unreal terminates.
**
** Parameters:
**	NONE
**
** Returns:
**	NONE
*/
void
FGlobalAudio::Exit(void)
{
	GUARD;

	plog("FGlobalAudio::Exit()\n");

	SoundEngGlobalDeinit();

	UNGUARD("FGlobalAudio::Exit");
} /* End FGlobalAudio::Exit() */

/*
** FGlobalAudio::InitLevel:
** Performs initializations specific to a map.
** Cannot be called until after FGlobalAudio::Init()
** is called.  FGlobalAudio::ExitLevel() should be
** called between subsequent calls to this function.
**
** Parameters:
**	To be determined
**
** Returns:
**	Value	Meaning
**	-----	-------
**	1	No error.
**	0	Error occured.
*/
int
//FGlobalAudio::InitLevel(ULevel *Level)
FGlobalAudio::InitLevel(int MaxIndices)
{
	GUARD;

	USound *Sound;	/* Used for resource enumeration loop. */
	int	count;	/* Count of resources. */

	plog("FGlobalAudio::InitLevel()\n");

	/* Register all sound effects with sound engine. */
	/* Loop through all USound resources. */
	count = 0;
	FOR_ALL_TYPED_RES(Sound, RES_Sound, USound)
	{
		/*
		** Load this sound into sound engine, saving
		** the sound ID returned by the sound engine.
		*/
//		bug("Registering %s",Sound->Name);
		Sound->SoundID = SoundEngSoundRegister(Sound->Data);
		if (Sound->SoundID == -1)
		{
			appErrorf("Failed registering %s",Sound->Name);
		}
//		bug("Registered %s",Sound->Name);
		count++;
	}
	END_FOR_ALL_TYPED_RES;

	/* Perform local init of sound engine. */
	if (!SoundEngLocalInit())
	{
		/* Failed init. */
		return 0;
	}

	return 1;
	UNGUARD("FGlobalAudio::InitLevel()");
} /* End FGlobalAudio::InitLevel() */

/*
** FGlobalAudio::ExitLevel:
** Performs clean up tasks necessary after playing
** a map.  Should not be called unless a prior call
** was made to FGlobalAudio::InitLevel().
**
** Parameters:
**	NONE
**
** Returns:
**	NONE
*/
void
FGlobalAudio::ExitLevel(void)
{
	GUARD;
	plog("FGlobalAudio::ExitLevel()\n");

	USound *Sound;	/* Used for resource enumeration loop. */
	int	count;	/* Count of resources. */

	/* Perform local shutdown of sound engine. */
	SoundEngLocalDeinit();

	/* Mark all sound resources as no longer registered. */
	count = 0;
	FOR_ALL_TYPED_RES(Sound, RES_Sound, USound)
	{
		/* Mark this sound resource as no longer registered. */
		Sound->SoundID = -1;
		count++;
	}
	END_FOR_ALL_TYPED_RES;

	UNGUARD("FGlobalAudio::ExitLevel()");
} /* End FGlobalAudio::ExitLevel() */

/*
** FGlobalAudio::Tick:
** Called about 35 times per second to update the state
** of the sound system.
**
** Parameters:
**	To be determined
**
** Returns:
**	NONE
*/
void
FGlobalAudio::Tick(ILevel *Level)
{
	GUARD;

// Don't log here unless necessary for debugging,
// as this is called *very* often.
//	plog("FGlobalAudio::Tick()\n");

	/* Update count of times called. */
	num_ticks++;

	/* Force sound engine update. */
	SoundEngUpdate();

	UNGUARD("FGlobalAudio::Tick()");
} /* End FGlobalAudio::Tick() */

/*
** FGlobalAudio::SetOrigin:
** Sets the current player position and rotation that is used for
** calculating volume levels and pan positions for game sound
** effects.
**
** Parameters:
**	Name	Description
**	----	-----------
**	Where	Location of player in map.
**	Angles	Which direction the player is facing.
**
** Returns:
**	NONE
*/
void
FGlobalAudio::SetOrigin(const FVector *Where, const FRotation *Angles)
{
	GUARD;

	SoundEngSetOrigin(Where->X, Where->Y, Where->Z,
			Angles->Yaw, Angles->Pitch, Angles->Roll);

	UNGUARD("FGlobalAudio::SetOrigin");
} /* End FGlobalAudio::SetOrigin() */

/*
** FGlobalAudio::PlaySfxOrigined:
** Plays a sound effect, calculating the volume and pan
** position based on the sound source location provided
** as an argument, and the last known player location and
** angles that were set by the most recent call to
** FGlobalAudio::SetOrigin().
** NOTE that this function has been superseded by
** ::PlaySfxLocated(), but remains for compatibility
** with prior versions of source.
**
** Parameters:
**	Name	Description
**	----	-----------
**	Source	Position of sound source in map.
**	usnd	Pointer to USound resource to be played.
**
** Returns:
**	Value	Meaning
**	-----	-------
**	any	Playback ID number that can be used
**		in calls to SfxStop(), etc.
*/
INT
FGlobalAudio::PlaySfxOrigined(const FVector *Source, USound *usnd, const FLOAT SoundRadius)
{
	GUARD;
	INT	pid;

	/* Check for bogus arguments. */
	if (usnd == NULL || Source == NULL)
	{
		return -1;
	}

	/* Play the sound. */
	pid = SoundEngSoundPlayLocated(
				usnd->SoundID,	/* Registered ID of sound */
				1,		/* actor ID */
				Source->X,	/* World location */
				Source->Y,
				Source->Z);

	return pid;

	UNGUARD("FGlobalAudio::PlaySfxOrigined");
} /* End FGlobalAudio::PlaySfxOrigined() */

/*
** FGlobalAudio::PlaySfxLocated:
** Plays a sound effect, calculating the volume and pan
** position based on the sound source location provided
** as an argument, and the last known player location and
** angles that were set by the most recent call to
** FGlobalAudio::SetOrigin().
**
** Parameters:
**	Name	Description
**	----	-----------
**	Source	Position of sound source in map.
**	usnd	Pointer to USound resource to be played.
**	iActor	Unique number (usually actor index) that
**		identifies the actor/thing/object/etc
**		that made this sound.  May be -1 for
**		sounds that are not associated with
**		anything in particular.
**
** Returns:
**	Value	Meaning
**	-----	-------
**	any	Playback ID number that can be used
**		in calls to SfxStop(), etc.
*/
INT
FGlobalAudio::PlaySfxLocated(const FVector *Source, USound *usnd,
				const INT iActor, const FLOAT SoundRadius)
{
	GUARD;
	INT	pid;

	/* Check for bogus arguments. */
	if (usnd == NULL || Source == NULL)
	{
		return -1;
	}

	/* Play the sound. */
	pid = SoundEngSoundPlayLocated(
				usnd->SoundID,	/* Registered ID of sound */
				iActor,		/* actor ID */
				Source->X,	/* World location */
				Source->Y,
				Source->Z);

	return pid;

	UNGUARD("FGlobalAudio::PlaySfxOrigined");
} /* End FGlobalAudio::PlaySfxLocated() */

/*
** FGlobalAudio::PlaySfxPrimitive:
** Plays a sound effect.  This is similar to PlaySfx(), but
** allows the caller to specify a fixed volume and pan
** position.  This would generally be used for menu or
** error message beeps, not in-game sound effects.
**
** Parameters:
**	Name	Description
**	----	-----------
**	usnd	Pointer to the USound to be played.
**
** Returns:
**	Value	Meaning
**	-----	-------
**	any	Playback ID number that can be used
**		in calls to SfxStop(), etc.
*/
INT
FGlobalAudio::PlaySfxPrimitive(USound *usnd)
{
	GUARD;
	INT	pid;

	/* Check for bogus arguments. */
	if (usnd == NULL)
	{
		return -1;
	}

	/* Play the sound. */
	pid = SoundEngSoundPlay(usnd->SoundID,
			CSOUND_MAX_VOLUME,
			CSOUND_PAN_CENTER);

	return pid;

	UNGUARD("FGlobalAudio::PlaySfxPrimitive");
} /* End FGlobalAudio::PlaySfxPrimitive() */

/*
** FGlobalAudio::SfxStop:
** Stops playing a sound effect that was started
** by a previous call to ::PlaySfxPrimitive,
** ::PlaySfxOrigined, or ::PlaySfxLocated.
**
** Parameters:
**	Name	Description
**	----	-----------
**	iPlay	Playback ID number from previous
**		::PlaySfx... call.
**
** Returns:
**	NONE
*/
void
FGlobalAudio::SfxStop(INT iPlay)
{
	SoundEngSoundStop(iPlay);
} /* End FGlobalAudio::SfxStop() */

/*
** FGlobalAudio::SfxStopActor:
** Stops playback of any sound effects that were
** started by a prior call to ::PlaySfxLocated()
** with a particular iActor argument.
**
** Parameters:
**	Name	Description
**	----	-----------
**	iActor	Actor index or other identifying
**		number as supplied to a prior call
**		to ::PlaySfxLocated().
**
** Returns:
**	NONE
*/
void
FGlobalAudio::SfxStopActor(const INT iActor)
{
	SoundEngActorStop(iActor);
} /* End FGlobalAudio::SfxStopActor() */

/*
** FGlobalAudio::SfxMoveActor:
** Moves any sound effects associated with a particular
** actor in the virtual listening space.  This should be
** called periodically for each actor to keep the sounds
** moving along with the actor's movement.
**
** Parameters:
**	Name	Description
**	----	-----------
**	iActor	Actor index or other identifying
**		number as supplied to a prior call
**		to ::PlaySfxLocated().
**	Where	Position of actor in map.
**
** Returns:
**	NONE
*/
void
FGlobalAudio::SfxMoveActor(const INT iActor, const FVector *Where)
{
	SoundEngActorMove(iActor,
			Where->X,
			Where->Y,
			Where->Z);
} /* End FGlobalAudio::SfxMoveActor() */

/*
** FGlobalAudio::MusicVolumeSet:
** Changes the volume level for music playback.
**
** Parameters:
**	Name	Description
**	----	-----------
**	NewVol	New volume level (0..MAX_MUSIC_VOLUME)
**
** Returns:
**	Value	Meaning
**	-----	-------
**	any	Previous volume level for music playback.
*/
INT
FGlobalAudio::MusicVolumeSet(INT NewVol)
{
	GUARD;

	return SoundEngMusicVolumeSet(NewVol);

	UNGUARD("FGlobalAudio::MusicVolumeSet");
} /* End FGlobalAudio::MusicVolumeSet() */

/*
** FGlobalAudio::MusicVolumeGet:
** Retrieves the current volume level for music playback.
**
** Parameters:
**	NONE
**
** Returns:
**	Value	Meaning
**	-----	-------
**	any	Current volume level for music playback.
*/
INT
FGlobalAudio::MusicVolumeGet(void)
{
	GUARD;

	return SoundEngMusicVolumeGet();

	UNGUARD("FGlobalAudio::MusicVolumeGet");
} /* End FGlobalAudio::MusicVolumeGet() */

/*
** FGlobalAudio::SfxVolumeSet:
** Changes the volume level for Sfx playback.
**
** Parameters:
**	Name	Description
**	----	-----------
**	NewVol	New volume level (0..MAX_Sfx_VOLUME)
**
** Returns:
**	Value	Meaning
**	-----	-------
**	any	Previous volume level for Sfx playback.
*/
INT
FGlobalAudio::SfxVolumeSet(INT NewVol)
{
	GUARD;

	return SoundEngSfxVolumeSet(NewVol);

	UNGUARD("FGlobalAudio::SfxVolumeSet");
} /* End FGlobalAudio::SfxVolumeSet() */

/*
** FGlobalAudio::SfxVolumeGet:
** Retrieves the current volume level for Sfx playback.
**
** Parameters:
**	NONE
**
** Returns:
**	Value	Meaning
**	-----	-------
**	any	Current volume level for Sfx playback.
*/
INT
FGlobalAudio::SfxVolumeGet(void)
{
	GUARD;

	return SoundEngSfxVolumeGet();

	UNGUARD("FGlobalAudio::SfxVolumeGet");
} /* End FGlobalAudio::SfxVolumeGet() */

/*
** FGlobalAudio::DirectSoundFlagGet:
** Retrieves the current setting of the DirectSound enable
** flag.
**
** Parameters:
**	NONE
**
** Returns:
**	Value	Meaning
**	-----	-------
**	1	Unreal will use DirectSound for playback.
**	0	Unreal will not use DirectSound.
*/
INT
FGlobalAudio::DirectSoundFlagGet(void)
{
	GUARD;

	return SoundEngDirectSoundFlagGet();

	UNGUARD("FGlobalAudio::DirectSoundFlagGet");
} /* End FGlobalAudio::DirectSoundFlagGet() */

/*
** FGlobalAudio::DirectSoundFlagSet:
** Changes the DirectSound enable flag.  The change
** will not take effect until ::Init() is called.
**
** Parameters:
**	Name	Description
**	----	-----------
**	val	New enable setting.
**		0 means don't use DirectSound.
**		Other means use DirectSound.
**
** Returns:
**	Value	Meaning
**	-----	-------
**	any	Previous setting of DirectSound flag.
*/
INT
FGlobalAudio::DirectSoundFlagSet(INT val)
{
	GUARD;

	return SoundEngDirectSoundFlagSet(val);

	UNGUARD("FGlobalAudio::DirectSoundFlagSet");
} /* End FGlobalAudio::DirectSoundFlagSet() */

/*
** FGlobalAudio::Pause:
** Pauses the sound output (by calling Galaxy's StopOutput(), if
** Galaxy is running).
**
** Parameters:
**	NONE
**
** Returns:
**	NONE
*/
void
FGlobalAudio::Pause(void)
{
	GUARD;

	SoundEngPause();

	UNGUARD("FGlobalAudio::Pause");
} /* End FGlobalAudio::Pause() */

/*
** FGlobalAudio::UnPause:
** Resumes sound output after a previous call to ::Pause().
**
** Parameters:
**	NONE
**
** Returns:
**	NONE
*/
void
FGlobalAudio::UnPause(void)
{
	GUARD;

	SoundEngUnPause();

	UNGUARD("FGlobalAudio::UnPause");
} /* End FGlobalAudio::UnPause() */

/*
** FGlobalAudio::SpecifySong:
** Specifies which song will be played the next time ::InitLevel()
** is called.
**
** Parameters:
**	Name	Description
**	----	-----------
**	iSong	Song number of song to be played.
**		The song number becomes part of the song
**		filename, i.e. SONGx.S3M, where 'x' is
**		the specified number.
**
** Returns:
**	NONE
*/
void
FGlobalAudio::SpecifySong(int iSong)
{
	GUARD;

	char	stmp[256];

	/* Hack until songs are stored as resources. */
	sprintf(stmp, "..\\Music\\Song%d.s3m", iSong);
	SoundEngMusicSpecifySong(stmp);
	plog("Selecting music:");
	plog(stmp);

	UNGUARD("FGlobalAudio::SpecifySong");
} /* End FGlobalAudio::SpecifySong() */

/*
=============================================================================
End UnFGAud.cpp
=============================================================================
*/
