/*=========================================================================
FILENAME:     soundeng.h
DESCRIPTION:  Include file shared between Unreal sound engine and the
              Unreal sound maker utility.
AUTHOR:       Ammon R. Campbell
COPYRIGHT:    (C) Copyright 1996 Epic MegaGames, Inc.
NOTICE:       This source code contains trade secrets and/or proprietary
              information of Epic MegaGames, Inc., and may not be
              disclosed to third parties without express written consent.
TOOLS:        Microsoft Visual C++ version 4.0
FORMAT:       100 characters per line, 8 characters per tabstop.
=========================================================================
NOTES:

When stored in memory, a csound occupies a single block of memory that
contains, in this order, the csound header, the ssound headers, and the
WAV data for the samples for each of the ssounds.

When stored in a file, the same format is used.
=========================================================================*/

#ifndef _SOUNDHDR_H
#define _SOUNDHDR_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************** CONSTANTS *****************************/

/* Maximum length for the name of an ssound: */
#define SSOUND_NAME_SIZE	128

/* Possible values for the 'Mode' field of a csound: */
#define CSMODE_REGULAR_ONESHOT	0	/* Normal one-shot sound effect. */
#define CSMODE_RANDOM_ONESHOT	1	/* Play one randomly selected ssound. */
#define CSMODE_LOOP_CONTINUOUS	2	/* All Ssounds loop continuously. */
#define CSMODE_LOOP_INTERVAL	3	/* Ssounds are repeated in a sequence */
					/* with an interval between each.     */

/* Four byte signature found at the beginning of a csound. */
#define CSOUND_SIGNATURE	"CSNX"

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

/*
** Maximum volume level for playing a sound effect.
** This is dependent on Galaxy.
*/
#define	CSOUND_MAX_VOLUME	63

/*
** Extents of pan positions for sound effects.
** These must match the LEFT, CENTER, and RIGHT
** #defines in galaxy.h
*/
#define CSOUND_PAN_LEFT		0
#define CSOUND_PAN_CENTER	32
#define CSOUND_PAN_RIGHT	64

/****************************** MACROS ********************************/

/****************************** TYPES *********************************/

#pragma pack(1)

/*
** Data structure to describe a sample (SSound).  A CSound
** contains one or more of these samples.
*/
typedef struct
{
	/* Size of this struct in bytes. */
	DWORD	HdrSize;

	/* Name of the WAV file that this ssound was created from. */
	unsigned char Name[SSOUND_NAME_SIZE];

	/* Size in bytes of .WAV data associate with this sound. */
	DWORD	DataSize;

	/* Galaxy sound effect handle for the .WAV associated with
	** this sound.  Only valid between calls to LocalInit and
	** LocalDeinit. */
	int	SFXHandle;

	/* How much random pitch shift to apply to the sound. */
	/* A value from 0 to 100. */
	DWORD	RandomPitchAmount;

	/* Tremolo effect properties. */
	DWORD	TremoloDepth;	/* Depth from 0 to 100. */
	DWORD	TremoloSpeed;	/* Envelope length in miliseconds. */

	/* Vibrato effect properties. */
	DWORD	VibratoDepth;	/* Depth from 0 to 100. */
	DWORD	VibratoSpeed;	/* Envelope length in miliseconds. */

	/* Reserved for future expansion. */
	unsigned char reserved[32];
} SSOUNDHDR;

/*
** Data structure to describe a complex sound (CSound).
*/
typedef struct
{
	/* Four byte signature, to identify the file as a csound. */
	unsigned char	Signature[4];

	/* Size of this struct in bytes. */
	DWORD		HdrSize;

	/* Number of ssounds used in this csound. */
	DWORD		nSounds;

	/* Mode specifying how this csound is played. */
	DWORD		Mode;

	/*
	** Flag; nonzero if the simple sounds are to be played
	** in random order instead of sequential order.
	*/
	BOOL		bRandomSequence;

	/*
	** Minimum and maximum time intervals in miliseconds
	** between playing of the ssounds in this csound.
	*/
	DWORD		IntervalMin;
	DWORD		IntervalMax;

	/* Reserved for future expansion. */
	unsigned char reserved[32];
} CSOUNDHDR;

#pragma pack()

/****************************** FUNCTIONS *****************************/

int	SoundEngGlobalInit(int enable);
void	SoundEngGlobalDeinit(void);
int	SoundEngLocalInit(void);
void	SoundEngLocalDeinit(void);

void	SoundEngLogSetup(void (* logfn)(char *));
void	SoundEngLogDetail(int dbglvl);

int	SoundEngUpdate(void);

int	SoundEngSoundRegister(void *sound);

int	SoundEngSoundPlay(int sid, int volume, int pan);
void	SoundEngSoundStop(int pid);
void	SoundEngSoundChange(int pid, int newvol, int newpan);

void	SoundEngSetOrigin(double x, double y, double z,
				int yaw, int pitch, int roll);

int	SoundEngSoundPlayLocated(int sid, int actid,
				double x, double y, double z);
void	SoundEngSoundMoveLocated(int pid,
				double x, double y, double z);
void	SoundEngActorMove(int actid, double x, double y, double z);
void	SoundEngActorStop(int actid);

int	SoundEngMusicSpecifySong(char *name);

int	SoundEngMusicVolumeSet(int newvol);
int	SoundEngMusicVolumeGet(void);
int	SoundEngSfxVolumeSet(int newvol);
int	SoundEngSfxVolumeGet(void);

int	SoundEngDirectSoundFlagSet(int val);
int	SoundEngDirectSoundFlagGet(void);

DWORD	SoundEngMixingRateSet(DWORD val);
int	SoundEngNumChannelsSet(int val);

void	SoundEngPause(void);
void	SoundEngUnPause(void);

#ifdef __cplusplus
};
#endif /* __cplusplus */
#endif /* _SOUNDHDR_H */

/*
=========================================================================
End soundeng.h
=========================================================================
*/
