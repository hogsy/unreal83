/*=============================================================================
	UnAudio.cpp: Null interface to replace SoundEng

	Copyright 2019 Mark E Sowden <markelswo@gmail.com>

	Revision history:
		* Created by Mark Sowden
=============================================================================*/

#include "Unreal.h"		// Unreal engine includes
#include "SoundEng.h"	// Headers for using SoundEng.lib

int	SoundEngGlobalInit(int enable) { return 1; }
void SoundEngGlobalDeinit(void) {}
int SoundEngLocalInit(void) { return 1; }
void SoundEngLocalDeinit(void) {}
void SoundEngLogSetup(void(*logfn)(char*)) {}
void SoundEngLogDetail(int dbglvl) {}
int	SoundEngUpdate(void) { return 1; }
int	SoundEngSoundRegister(void* sound) { return 1; }
int	SoundEngSoundPlay(int sid, int volume, int pan) { return 1; }
void SoundEngSoundStop(int pid) {}
void SoundEngSoundChange(int pid, int newvol, int newpan) {}
void SoundEngSetOrigin(double x, double y, double z, int yaw, int pitch, int roll) {}
int	SoundEngSoundPlayLocated(int sid, int actid, double x, double y, double z) { return 1; }
void SoundEngSoundMoveLocated(int pid, double x, double y, double z) {}
void SoundEngActorMove(int actid, double x, double y, double z) {}
void SoundEngActorStop(int actid) {}
int	SoundEngMusicSpecifySong(char* name) { return 1; }
int	SoundEngMusicVolumeSet(int newvol) { return 1; }
int	SoundEngMusicVolumeGet(void) { return 1; }
int	SoundEngSfxVolumeSet(int newvol) { return 1; }
int	SoundEngSfxVolumeGet(void) { return 1; }
int	SoundEngDirectSoundFlagSet(int val) { return 1; }
int	SoundEngDirectSoundFlagGet(void) { return 1; }
DWORD SoundEngMixingRateSet(DWORD val) { return 1; }
int	SoundEngNumChannelsSet(int val) { return 1; }
void SoundEngPause(void) {}
void SoundEngUnPause(void) {}
