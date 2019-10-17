/*==========================================================================
FILENAME:     UnSound.h
DESCRIPTION:  Declarations of the "USound" class and related routines.
NOTICE:       Copyright 1996 Epic MegaGames, Inc. This software is a
              trade secret.
TOOLS:        Compiled with Visual C++ 4.0, Calling method=__fastcall
FORMAT:       8 characters per tabstop, 100 characters per line.
HISTORY:
  When      Who                 What
  ----      ---                 ----
  ??/??/96  Tim Sweeney         Created stubs for this module.
  04/18/96  Ammon R. Campbell   Misc. hacks started.
  05/03/96  Ammon R. Campbell   More or less working now.
==========================================================================*/

#ifndef _INC_UNSOUND /* Prevent header from being included multiple times */
#define _INC_UNSOUND

/*
** USound:
** Class used to store information about each sound effect resource.
** This is derived from the UResource class.
*/
class UNREAL_API USound : public UResource
	{
	RESOURCE_CLASS(USound,BYTE,RES_Sound)
	/*
	** Variables.
	*/

	/* Size of WAV data in bytes. */
	INT	DataSize;

	/* Name of the sound effect family to which this sound belongs. */
	FName	FamilyName;

	/* Sound ID of this sound when it is registered with the sound drivers. */
	INT	SoundID;

	/* For future expansion. */
	BYTE	Pad[256];	/* For future expansion. */

	/*
	** Resource function overriden from UResource.
	*/
	void Register(FResourceType *Type);
	void InitHeader(void);
	void InitData(void);
	int  QuerySize(void);
	int  QueryMinSize(void);
	const char *Import(const char *Buffer, const char *BufferEnd, const char *FileType);
	char *Export(char *Buffer,const char *FileType,int Indent);
	void QueryHeaderReferences(FResourceCallback &Callback);
	void QueryDataReferences(FResourceCallback &Callback);
	};

#endif // _INC_UNSOUND

/*
==========================================================================
End UnSound.h
==========================================================================
*/
