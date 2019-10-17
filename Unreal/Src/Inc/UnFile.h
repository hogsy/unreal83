/*=============================================================================
	UnFile.h: General-purpose file utilities.

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.
=============================================================================*/

#ifndef _INC_UNFILE // Prevent header from being included multiple times
#define _INC_UNFILE

/*-----------------------------------------------------------------------------
	Platform independent convenience functions
-----------------------------------------------------------------------------*/

//
// File functions
//
UNREAL_API long	fsize 				(const char *fname);
UNREAL_API char *fgetdir			(char *dir);
UNREAL_API int 	fsetdir				(const char *dir);
UNREAL_API int 	fmkdir				(const char *dir);
UNREAL_API int 	fdelete 			(const char *fname);
UNREAL_API int 	frename 			(const char *oldname, const char *newname);
UNREAL_API const char *fext			(const char *fname);
UNREAL_API const char *spc			(int num);

//
// Other functions
//
UNREAL_API char *mystrncpy			(char *dest, const char *src, int maxlen); // Safe strncpy
UNREAL_API char *mystrncat			(char *dest, const char *src, int maxlen); // Safe strcat
UNREAL_API const char *mystrstr		(const char *str, const char *find); // Find string in string, case insensitive, non-alnum lead-in
UNREAL_API unsigned long strhash	(const char *Data, int NumKeys); // Hash value computer
UNREAL_API unsigned long strcrc		(const unsigned char *Data, int Length); // CRC32 computer
UNREAL_API void mymemset			(void *Dest,char c,int Count);
UNREAL_API int mymemeq				(void *P1, void *P2, int n);

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
#endif // _INC_UNFILE

