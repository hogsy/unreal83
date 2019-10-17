/*==========================================================================
FILENAME:     UnFGAud.h
DESCRIPTION:  Declarations of the "FGlobalAudio" class and related
              routines.
NOTICE:       Copyright 1996 Epic MegaGames, Inc. This software is a
              trade secret.
TOOLS:        Compiled with Visual C++ 4.0, Calling method=__fastcall
FORMAT:       8 characters per tabstop, 100 characters per line.
HISTORY:
  When      Who                 What
  ----      ---                 ----
  ??/??/96  Tim Sweeney         Created stubs for this module.
  04/18/96  Ammon R. Campbell   Misc. hacks started.
  05/12/96  Ammon R. Campbell   Added volume get/set routines.
==========================================================================*/

#ifndef _INC_UNFGAUD /* Prevent header from being included multiple times */
#define _INC_UNFGAUD

/******************************* CONSTANTS *****************************/

/*
** Highest possible volume level for music,
** for MusicVolumeSet/Get calls.
*/
#define MAX_MUSIC_VOLUME	127

/*
** Highest possible volume level for sound effects,
** for SfxVolumeSet/Get calls.
*/
#define MAX_SFX_VOLUME		127

/***************************** TYPES/CLASSES ***************************/

/*
** FGlobalAudio:
** The class that contains the functions called by
** the Unreal engine to initialize, drive, and shut
** down the sound module.
*/
class UNREAL_API FGlobalAudio
{
public:
	/*
	** Member functions:
	*/

	/* Performs once-per-instance initialization of sound stuff. */
	int Init(int Active);

	/* Performs once-per-instace shutdown of sound stuff. */
	void Exit(void);

	/* Initialize prior to playing a map. */
//	int InitLevel(ULevel *Level);
	int InitLevel(int MaxIndices);

	/* Clean up after playing a map. */
	void ExitLevel(void);

	/* Called about 35 times per second to update sound stuff. */
	void Tick(ILevel *Level);

	/* Called by ULevel::Tick() to specify current player location. */
	void SetOrigin(const FVector *Where, const FRotation *Angles);

	/* Called to play a sound effect (see "UnFGAud.cpp"). */
	/* Note: SoundRadius==0 means the radius parameter should be ignored */
	INT PlaySfxOrigined(const FVector *Source, USound *usnd, const FLOAT SoundRadius=0.f);
	INT PlaySfxPrimitive(USound *usnd);
	INT PlaySfxLocated(const FVector *Source, USound *usnd, const INT iActor, const FLOAT SoundRadius=0.f);

	/* Called to modify or stop sound effects. */
	void SfxStop(INT iPlay);
	void SfxStopActor(const INT iActor);
	void SfxMoveActor(const INT iActor, const FVector *Where);

	/* Volume get/set routines. */
	INT MusicVolumeGet(void);
	INT MusicVolumeSet(INT NewVol);
	INT SfxVolumeGet(void);
	INT SfxVolumeSet(INT NewVol);

	/* Enable/disable DirectSound in Galaxy. */
	INT DirectSoundFlagGet(void);
	INT DirectSoundFlagSet(INT val);

	/* Specify which song to play during game. */
	void SpecifySong(int iSong);

	/* Temporarily stop/start sound during playback. */
	void Pause(void);
	void UnPause(void);

/*
private:
See notes in "UnFGAud.cpp" for psuedo-private stuff
*/
};

/*-----------------------------------------------------------------------------
(From Tim):
Implementation notes:

1.	May as well use a global class (GAudio) for audio.  Unreal can shut it down
	and restart it as the player switches levels.  This keeps everything simple,
	and it's in tune with the single-task nature of Galaxy.

2.	Internally, you'll probably want to cleanly separate all of your generic
	code from all code that deals with Unreal-specific stuff (the actor list
	and level).  This will insulate most of your code from possible changes.

3.	ExitLevel() will always be called after InitLevel() and before subsequent
	calls to InitLevel.  In other words, the audio subsystem will never be
	aware of more than one level at a time, even though the Unreal client or
	server might actually have several levels running in the background.

4.	Note that your AudioInit() function might fail if a particular computer doesn't
	have a working sound card.  This is not cause for Unreal to fail.  If AudioInit()
	fails, all of the Audio() functions should return gracefully without doing anything.
	In other words, Unreal doesn't need to know/care whether audio is working.

Interfacing to UnrealEd:

	This is entirely your job... I have created severa stub functions so that
	you can achieve this without ripping apart the C code very badly. These are:

	SoundCmd: This is called by the code in UnEdSrv.cpp, which processes the
	UnrealEd command line sent from Visual Basic to C.  Check our UnEdSrv.cpp
	and UnParams.cpp to see the kind of things you can do in the command line, 
	and how to parse parameters easily.

	SoundLinkTopicFunc: This is a way of exchanging two-way between Unreal and
	UnrealEd.  Check out the code in UnClass.cpp for an example of a link topic
	function that processes data between Unreal and UnrealEd.  Also check out the
	Visual Basic forms BrClass form.

	I have created two stub forms in Visual Basic" BrSound and BrAmbient.
	These are intended to be browsers for audio related stuff,
	much as BrClass contains browser code for actor classes.  You'll need to
	expand these in many ways, adding load/save/import/export functionality,
	as well as other dialogs to facilitate editing these items.

	I recommend using a "family" based approach for browsing through all audio
	types, so that designers can group sounds together functionally, and save
	time looking for stuff in a huge list.

	Please use the ".uax" extension for all audio-related files (both sounds
	and ambient sounds) and make sure they end up in the designer's "Audio"
	directory.  Check out UnEdApp.cls (in VB) to see how directory names
	are managed.  Also check out Dirs.frm (in VB) to see how I set up common
	dialogs.  You can just copy/paste/modify one of the existing common dialogs
	for your purposes.

Thoughts:

1.	USound: This should thinly encapsulate a sound effect.  It should contain
	the minimum parameters needed to play the sound properly, such as length
	and sampling rate.  Whatever is needed to get the point across to Galaxy.

2.	UAmbient: This is the complex structure you're going to invent, which
	contains references to other sound effects and possibly music in its header.
	If UAmbient contains data (which is optional), it should contain a song
	in any format that Galaxy supports.  Note that you can just stick all of the
	raw data from the music file in at import time, and Galaxy will be able to
	parse it.
	
	UAmbient will need parameters defining how to mix the music (if any) together
	with the pure USound sound effects it references (if any), in order to achieve
	a continuous environmental sound.  Some possible functionality includes:

	- Looping sounds over and over, such as a torch burning constantly.
	- Playing sounds with a random delay between them, such as water dripping into
	  a puddle, or thunder, in cases where these events aren't synchronized with
	  anything in the game.
	- Random pitch variation, such as water flowing with a nice little
	  variation to hide the looping.
	- Possibly multiple instances of the sound effect at the same time, such
	  as two torch-burning sounds intermingled at different rates, which would
	  totally hide the looping.

4.	Synchroinzation of sounds with events in the game: This will be entirely
    the responsibility of the scripting language, which may fire off sound effect
	events at will.  For example, a script could be written to control lighting,
	which would encapsulate several different things: a sound effect or several, 
	a change in the brightness of an outdoors light, etc.  The sound system will
	never fire events for Unreal.

5.  Note that the Ambient member of actors may change at any
	time, under script control.  You'll probably be maintaining some kind of list
	of active ambient sounds so that you can update them on calls to Tick().
	You'll have to detect changes to Actors' AmbientSound members in calls to 
	Tick().

6.	ULevel vs. ILevel: ULevel pointers are simple pointers to Level resources.
	ILevel pointers are pointers to a special structure that Unreal uses
	to hold all level information in a convenient, easy-to-access form.  Check
	out UnLevel.cpp for ULevel::Lock and ULevel::Unlock to get a feeling for
	what's happening.

7.	Use the physSoundDamping function to determine the amount of sound damping
    that occurs between two points (the player and an actor emitting a sound).
	The returns a value from 0.0 (silence) to 1.0 (full volume) based solely
	on level geometry.  Note that you must do the following calculations
	yourself, which physSoundDamping doesn't consider:

	- Distance attenuation (far off sounds are quieter)
	- Stereo panning based on the direction the listener is facing

-----------------------------------------------------------------------------*/
#endif // _INC_UNFGAUD

/*
==========================================================================
End UnFGAud.h
==========================================================================
*/
