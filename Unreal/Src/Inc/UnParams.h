/*=============================================================================
	UnParams.h: Parameter-parsing routines

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNPARAMS
#define _INC_UNPARAMS

/*-----------------------------------------------------------------------------
	Parameter parsing functions (UnParams.cpp)
-----------------------------------------------------------------------------*/

//
// Bogus FStreamParser class, simply contains all stream parsing routines
//
class UNREAL_API FStreamParser
	{
	friend UNREAL_API int GetBYTE 		(const char *Stream, const char *Match,	BYTE *Value);
	friend UNREAL_API int GetSBYTE 		(const char *Stream, const char *Match,	SBYTE *Value);
	friend UNREAL_API int GetWORD 		(const char *Stream, const char *Match,	WORD *Value);
	friend UNREAL_API int GetINDEX		(const char *Stream, const char *Match,	INDEX *Value);
	friend UNREAL_API int GetSWORD 		(const char *Stream, const char *Match,	SWORD *Value);
	friend UNREAL_API int GetFLOAT 		(const char *Stream, const char *Match,	FLOAT *Value);
	friend UNREAL_API int GetFVECTOR 	(const char *Stream, const char *Match,	FVector *Value);
	friend UNREAL_API int GetFVECTOR 	(const char *Stream,					FVector *Value);
	friend UNREAL_API int GetFIXFVECTOR	(const char *Stream,					FVector *Value);
	friend UNREAL_API int GetFSCALE 	(const char *Stream,					FScale *Scale);
	friend UNREAL_API int GetFROTATION	(const char *Stream, const char *Match,	FRotation *Rotation,int ScaleFactor);
	friend UNREAL_API int GetFROTATION	(const char *Stream,					FRotation *Rotation,int ScaleFactor);
	friend UNREAL_API int GetDWORD 		(const char *Stream, const char *Match,	DWORD *Value);
	friend UNREAL_API int GetINT 		(const char *Stream, const char *Match,	INT *Value);
	friend UNREAL_API int GetSTRING 	(const char *Stream, const char *Match,	char *Value,int MaxLen);
	friend UNREAL_API int GetONOFF 		(const char *Stream, const char *Match,	int *OnOff);
	friend UNREAL_API int GetRES 		(const char *Stream, const char *Match,	enum EResourceType ResType, class UResource **Res);
	friend UNREAL_API int GetRESTYPE 	(const char *Stream, const char *Match,	enum EResourceType *ResType);
	friend UNREAL_API int GetNAME		(const char *Stream, const char *Match,	class FName *Name);
	friend UNREAL_API int GetParam		(const char *Stream, const char *Param);
	friend UNREAL_API int GetCMD 		(const char **Stream, const char *Match);
	friend UNREAL_API int GetLINE 		(const char **Stream, char *Result,int MaxLen);
	friend UNREAL_API int GetBEGIN 		(const char **Stream, const char *Match);
	friend UNREAL_API int GetEND		(const char **Stream, const char *Match);
	friend UNREAL_API void GetNEXT		(const char **Stream);
	friend UNREAL_API int GrabSTRING	(const char *&Str,char *Result, int MaxLen);
	friend UNREAL_API int PeekCMD 		(const char *Stream, const char *Match);
	friend UNREAL_API void SkipLINE		(const char **Stream);
	friend UNREAL_API char *SetFVECTOR	(char *Dest,const FVector   *Value);
	friend UNREAL_API char *SetFIXFVECTOR(char *Dest,const FVector   *Value);
	friend UNREAL_API char *SetROTATION	(char *Dest,const FRotation *Rotation);
	friend UNREAL_API char *SetFSCALE	(char *Dest,const FScale	*Scale);
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
#endif // _INC_UNPARAMS

