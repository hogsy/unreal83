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
//class UNREAL_API FStreamParser
//	{
	 UNREAL_API int GetBYTE 		(const char *Stream, const char *Match,	BYTE *Value);
	 UNREAL_API int GetSBYTE 		(const char *Stream, const char *Match,	SBYTE *Value);
	 UNREAL_API int GetWORD 		(const char *Stream, const char *Match,	WORD *Value);
	 UNREAL_API int GetINDEX		(const char *Stream, const char *Match,	INDEX *Value);
	 UNREAL_API int GetSWORD 		(const char *Stream, const char *Match,	SWORD *Value);
	 UNREAL_API int GetFLOAT 		(const char *Stream, const char *Match,	FLOAT *Value);
	 UNREAL_API int GetFVECTOR 	(const char *Stream, const char *Match,	FVector *Value);
	 UNREAL_API int GetFVECTOR 	(const char *Stream,					FVector *Value);
	 UNREAL_API int GetFIXFVECTOR	(const char *Stream,					FVector *Value);
	 UNREAL_API int GetFSCALE 	(const char *Stream,					FScale *Scale);
	 UNREAL_API int GetFROTATION	(const char *Stream, const char *Match,	FRotation *Rotation,int ScaleFactor);
	 UNREAL_API int GetFROTATION	(const char *Stream,					FRotation *Rotation,int ScaleFactor);
	 UNREAL_API int GetDWORD 		(const char *Stream, const char *Match,	DWORD *Value);
	 UNREAL_API int GetINT 		(const char *Stream, const char *Match,	INT *Value);
	 UNREAL_API int GetSTRING 	(const char *Stream, const char *Match,	char *Value,int MaxLen);
	 UNREAL_API int GetONOFF 		(const char *Stream, const char *Match,	int *OnOff);
	 UNREAL_API int GetRES 		(const char *Stream, const char *Match,	enum EResourceType ResType, class UResource **Res);
	 UNREAL_API int GetRESTYPE 	(const char *Stream, const char *Match,	enum EResourceType *ResType);
	 UNREAL_API int GetNAME		(const char *Stream, const char *Match,	class FName *Name);
	 UNREAL_API int GetParam		(const char *Stream, const char *Param);
	 UNREAL_API int GetCMD 		(const char **Stream, const char *Match);
	 UNREAL_API int GetLINE 		(const char **Stream, char *Result,int MaxLen);
	 UNREAL_API int GetBEGIN 		(const char **Stream, const char *Match);
	 UNREAL_API int GetEND		(const char **Stream, const char *Match);
	 UNREAL_API void GetNEXT		(const char **Stream);
	 UNREAL_API int GrabSTRING	(const char *&Str,char *Result, int MaxLen);
	 UNREAL_API int PeekCMD 		(const char *Stream, const char *Match);
	 UNREAL_API void SkipLINE		(const char **Stream);
	 UNREAL_API char *SetFVECTOR	(char *Dest,const FVector   *Value);
	 UNREAL_API char *SetFIXFVECTOR(char *Dest,const FVector   *Value);
	 UNREAL_API char *SetROTATION	(char *Dest,const FRotation *Rotation);
	 UNREAL_API char *SetFSCALE	(char *Dest,const FScale	*Scale);
//	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
#endif // _INC_UNPARAMS

