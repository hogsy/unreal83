/*=============================================================================
	UnParams.cpp: Functions to help parse commands.

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	What's happening: When the Visual Basic level editor is being used,
	this code exchanges messages with Visual Basic.  This lets Visual Basic
	affect the world, and it gives us a way of sending world information back
	to Visual Basic.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

/*-----------------------------------------------------------------------------
	Getters
	All of these functions return 1 if the appropriate item was
	fetched, or 0 if not.
-----------------------------------------------------------------------------*/

//
// Get a byte (0-255)
//
int UNREAL_API GetBYTE (const char *Stream, const char *Match,BYTE *Value)
	{
	GUARD;
	const char *Temp=mystrstr(Stream,Match);
	if (Temp==NULL) return 0; // didn't match
	*Value=(BYTE)atoi(Temp+strlen(Match));
	return 1;
	UNGUARD("GetBYTE");
	};

//
// Get a signed byte (-128 to 127)
//
int UNREAL_API GetSBYTE (const char *Stream, const char *Match,SBYTE *Value)
	{
	GUARD;
	const char *Temp=mystrstr(Stream,Match);
	if (Temp==NULL) return 0; // didn't match
	*Value=(SBYTE)atoi(Temp+strlen(Match));
	return 1;
	UNGUARD("GetSBYTE");
	};

//
// Get a word (0-65536)
//
int UNREAL_API GetWORD (const char *Stream, const char *Match,WORD *Value)
	{
	GUARD;
	const char *Temp=mystrstr(Stream,Match);
	if (Temp==NULL) return 0; // didn't match
	*Value=(WORD)atol(Temp+strlen(Match));
	return 1;
	UNGUARD("GetWORD");
	};

//
// Get an INDEX (0-65536, or None)
//
int UNREAL_API GetINDEX (const char *Stream, const char *Match,INDEX *Value)
	{
	GUARD;
	const char *Temp=mystrstr(Stream,Match);
	if (Temp==NULL) return 0; // didn't match
	//
	Temp += strlen(Match);
	if (!strnicmp(Temp,"NONE",4)) *Value = INDEX_NONE;
	else *Value=(WORD)atol(Temp);
	//
	return 1;
	UNGUARD("GetINDEX");
	};

//
// Get a signed word (-32768 to 32767)
//
int UNREAL_API GetSWORD (const char *Stream, const char *Match,SWORD *Value)
	{
	GUARD;
	const char *Temp=mystrstr(Stream,Match);
	if (Temp==NULL) return 0; // didn't match
	*Value=(SWORD)atol(Temp+strlen(Match));
	return 1;
	UNGUARD("GetSWORD");
	};

//
// Get a floating-point number
//
int UNREAL_API GetFLOAT (const char *Stream, const char *Match,FLOAT *Value)
	{
	GUARD;
	const char *Temp=mystrstr(Stream,Match);
	if (Temp==NULL) return 0; // didn't match
	*Value=(FLOAT) atof (Temp+strlen(Match));
	return 1;
	UNGUARD("GetFLOAT");
	};

//
// Get a floating-point vector (X=, Y=, Z=), return number of components parsed (0-3)
//
int UNREAL_API GetFVECTOR (const char *Stream, FVector *Value)
	{
	GUARD;
	int NumVects = 0;
	//
	// Support for old format:
	//
	NumVects += GetFLOAT (Stream,"X=",&Value->X);
	NumVects += GetFLOAT (Stream,"Y=",&Value->Y);
	NumVects += GetFLOAT (Stream,"Z=",&Value->Z);
	//
	// New format:
	//
	if (NumVects==0)
		{
		Value->X = atof(Stream);
		//
		Stream = strchr(Stream,',');
		if (!Stream) return 0;
		Value->Y = atof(++Stream);
		//
		Stream = strchr(Stream,',');
		if (!Stream) return 0;
		Value->Z = atof(++Stream);
		//
		NumVects=3;
		};
	return NumVects;
	UNGUARD("GetFVECTOR");
	};

//
// Get a string enclosed in parenthesis
//
int UNREAL_API GetSUBSTRING (const char *Stream, const char *Match,char *Value,int MaxLen)
	{
	GUARD;
	const char *Found = mystrstr(Stream,Match);
	const char *Start;
	//
	if (Found==NULL) return 0; // didn't match
	//
	Start = Found+strlen(Match);
	if (*Start != '(') return 0;
	//
	strncpy (Value,Start+1,MaxLen);
	Value[MaxLen-1]=0;
	char *Temp=strchr(Value,')');
	if (Temp!=NULL) *Temp=0;
	return 1;
	//
	UNGUARD("GetSUBSTRING");
	};

//
// Get a floating-point vector (X=, Y=, Z=), return number of components parsed (0-3)
//
int UNREAL_API GetFVECTOR (const char *Stream, const char *Match, FVector *Value)
	{
	GUARD;
	char Temp[80];
	if (!GetSUBSTRING(Stream,Match,Temp,80)) return 0;
	return GetFVECTOR(Temp,Value);
	UNGUARD("GetFVECTOR");
	};

//
// Get a floating-point vector (X=, Y=, Z=), return number of components parsed (0-3)
//
int UNREAL_API GetFIXFVECTOR (const char *Stream, FVector *Value)
	{
	GUARD;
	int NumVects = GetFVECTOR(Stream,Value);
	*Value *= 65536.0;
	return NumVects;
	UNGUARD("GetFIXFVECTOR");
	};

//
// Get a floating-point scale value
//
int UNREAL_API GetFSCALE (const char *Stream, FScale *FScale)
	{
	GUARD;
	if (GetFVECTOR 	(Stream,&FScale->Scale)!=3) 				return 0;
	if (!GetFLOAT  	(Stream,"S=",&FScale->SheerRate)) 			return 0;
	if (!GetINT     (Stream,"AXIS=",(int *)&FScale->SheerAxis)) return 0;
	return 1;
	UNGUARD("GetFSCALE");
	};

//
// Get a set of rotations (PITCH=, YAW=, ROLL=), return number of components parsed (0-3)
//
int UNREAL_API GetFROTATION (const char *Stream, FRotation *Rotation,int ScaleFactor)
	{
	GUARD;
	FLOAT	Temp;
	int 	N = 0;
	//
	// Old format:
	//
	if (GetFLOAT (Stream,"PITCH=",&Temp)) {Rotation->Pitch = Temp * ScaleFactor; N++;};
	if (GetFLOAT (Stream,"YAW=",  &Temp)) {Rotation->Yaw   = Temp * ScaleFactor; N++;};
	if (GetFLOAT (Stream,"ROLL=", &Temp)) {Rotation->Roll  = Temp * ScaleFactor; N++;};
	//
	// New format:
	//
	if (N==0)
		{
		Rotation->Pitch = atof(Stream) * ScaleFactor;
		//
		Stream = strchr(Stream,',');
		if (!Stream) return 0;
		Rotation->Yaw = atof(++Stream) * ScaleFactor;
		//
		Stream = strchr(Stream,',');
		if (!Stream) return 0;
		Rotation->Roll = atof(++Stream) * ScaleFactor;
		//
		N=3;
		};
	return N;
	UNGUARD("GetROTATION");
	};

//
// Get a rotation value, return number of components parsed (0-3)
//
int UNREAL_API GetFROTATION (const char *Stream, const char *Match, FRotation *Value,int ScaleFactor)
	{
	GUARD;
	char Temp[80];
	if (!GetSUBSTRING(Stream,Match,Temp,80)) return 0;
	return GetFROTATION(Temp,Value,ScaleFactor);
	UNGUARD("GetFROTATION");
	};

//
// Get a double word (4 bytes)
//
int UNREAL_API GetDWORD (const char *Stream, const char *Match,DWORD *Value)
	{
	GUARD;
	const char *Temp=mystrstr(Stream,Match);
	if (Temp==NULL) return 0; // didn't match
	*Value=(DWORD)atol(Temp+strlen(Match));
	return 1;
	UNGUARD("GetDWORD");
	};

//
// Get a signed double word (4 bytes)
//
int UNREAL_API GetINT (const char *Stream, const char *Match,INT *Value)
	{
	GUARD;
	const char *Temp=mystrstr(Stream,Match);
	if (Temp==NULL) return 0; // didn't match
	*Value=atoi(Temp+strlen(Match));
	return 1;
	UNGUARD("GetINT");
	};

//
// Get a string
//
int UNREAL_API GetSTRING (const char *Stream, const char *Match,char *Value,int MaxLen)
	{
	GUARD;
	int i=strlen(Stream);
	const char *Found = mystrstr(Stream,Match);
	const char *Start;
	//
	if (Found)
		{
		Start = Found+strlen(Match);
		if (*Start == '\x22') // Quoted string with spaces
			{
			strncpy (Value,Start+1,MaxLen);
			Value[MaxLen-1]=0;
			char *Temp=strchr(Value,'\x22');
			if (Temp!=NULL) *Temp=0;
			}
		else // Non-quoted string without spaces
			{
			strncpy(Value,Start,MaxLen);
			Value[MaxLen-1]=0;
			char *Temp;
			Temp=strchr(Value,' ' ); if (Temp) *Temp=0;
			Temp=strchr(Value,'\r'); if (Temp) *Temp=0;
			Temp=strchr(Value,'\n'); if (Temp) *Temp=0;
			};
		return 1;
		}
	else return 0;
	//
	UNGUARD("GetSTRING");
	};

//
// Sees if Stream starts with the named command.  If it does,
// skips through the command and blanks past it.  Returns 1 of match,
// 0 if not.
//
int UNREAL_API GetCMD (const char **Stream, const char *Match)
	{
	GUARD;
	while ((**Stream==' ')||(**Stream==9)) (*Stream)++;
	if (strnicmp(*Stream,Match,strlen(Match))==0)
		{
		*Stream += strlen(Match);
		if (!isalnum(**Stream))
			{
			while ((**Stream==' ')||(**Stream==9)) (*Stream)++;
			return 1; // Success
			}
		else
			{
			*Stream -= strlen(Match);
			return 0; // Only found partial match
			};
		}
	else return 0; // No match
	UNGUARD("GetCMD");
	};

//
// Sees if Stream starts with the named command.  Returns 1 of match,
// 0 if not.  Does not affect the stream.
//
int UNREAL_API PeekCMD (const char *Stream, const char *Match)
	{
	GUARD;
	while ((*Stream==' ')||(*Stream==9)) Stream++;
	if (strnicmp(Stream,Match,strlen(Match))==0)
		{
		Stream += strlen(Match);
		if (!isalnum(*Stream)) return 1; // Success
		else return 0; // Only found partial match
		}
	else return 0; // No match
	UNGUARD("PeekCMD");
	};

//
// Get a line of Stream (everything up to, but not including, CR/LF.
// Returns 0 if ok, nonzero if at end of stream and returned 0-length string.
//
int UNREAL_API GetLINE (const char **Stream, char *Result,int MaxLen)
	{
	GUARD;
	int GotStream=0;
	//
	*Result=0;
	while ((**Stream!=0)&&(**Stream!=10)&&(**Stream!=13)&&(--MaxLen>0))
		{
		*(Result++) = *((*Stream)++); // Get stuff till CR/LF
		GotStream=1;
		};
	while ((**Stream==10)||(**Stream==13)) (*Stream)++; // Eat up all CR/LF's
	//
	*Result=0;
	if ((**Stream!=0)||GotStream) return 0; // Keep going
	return 1; // At end of stream, and returned zero-length string
	UNGUARD("GetLINE");
	};

int UNREAL_API GetONOFF (const char *Stream, const char *Match, int *OnOff)
	{
	GUARD;
	char TempStr[16];
	//
	if (GetSTRING(Stream,Match,TempStr,16))
		{
		if ((!stricmp(TempStr,"ON"))||(!stricmp(TempStr,"TRUE"))||(!stricmp(TempStr,"1"))) *OnOff = 1;
		else *OnOff = 0;
		return 1;
		}
	else return 0;
	UNGUARD("GetONOFF");
	};

int UNREAL_API GetRES (const char *Stream, const char *Match, EResourceType ResType, UResource **DestRes)
	{
	GUARD;
	char 		TempStr[NAME_SIZE];
	UResource	*Res;
	//
	if (!GetSTRING(Stream,Match,TempStr,NAME_SIZE)) return 0; // Match not found
	//
	if (stricmp(TempStr,"NONE"))
		{	
		Res = GRes.PrivateLookup (TempStr,ResType,FIND_Optional);
		if (!Res) return 0;
		//
		*DestRes = Res;
		}
	else *DestRes = NULL; // Resource name "None" was explicitly specified
	//
	return 1;
	UNGUARD("GetRESNAME");
	};

int UNREAL_API GetNAME (const char *Stream, const char *Match, FName *Name)
	{
	GUARD;
	char TempStr[NAME_SIZE];
	//
	if (!GetSTRING(Stream,Match,TempStr,NAME_SIZE)) return 0; // Match not found
	//
	Name->Add(TempStr);
	//
	return 1;
	UNGUARD("GetNAME");
	};

int UNREAL_API GetRESTYPE (const char *Stream, const char *Match, EResourceType *ResType)
	{
	GUARD;
	char 			TempStr[NAME_SIZE];
	EResourceType	TempType;
	//
	if (!GetSTRING(Stream,Match,TempStr,NAME_SIZE)) return 0; // Match not found
	//
	TempType = GRes.LookupType (TempStr);
	//
	if (TempType==RES_None) return 0; // Unrecongized type
	*ResType = TempType;
	return 1;
	UNGUARD("GetRESTYPE");
	};

//
// Gets a "BEGIN" string.  Returns 1 if gotten, 0 if not.
// If not gotten, doesn't affect anything.
//
int UNREAL_API GetBEGIN (const char **Stream, const char *Match)
	{
	GUARD;
	const char *Original = *Stream;
	if (GetCMD (Stream,"BEGIN") && GetCMD (Stream,Match)) return 1; // Gotten
	*Stream = Original;
	return 0;
	UNGUARD("GetBEGIN");
	};

//
// Gets an "END" string.  Returns 1 if gotten, 0 if not.
// If not gotten, doesn't affect anything.
//
int UNREAL_API GetEND (const char **Stream, const char *Match)
	{
	GUARD;
	const char *Original = *Stream;
	if (GetCMD (Stream,"END") && GetCMD (Stream,Match)) return 1; // Gotten
	*Stream = Original;
	return 0;
	UNGUARD("GetEND");
	};

//
// Get next command.  Skips past comments and cr's
//
void UNREAL_API GetNEXT (const char **Stream)
	{
	GUARD;
	//
	// Skip over spaces, tabs, cr's, and linefeeds:
	//
	SkipJunk: while ((**Stream==' ')||(**Stream==9)||(**Stream==13)||(**Stream==10)) (*Stream)++;
	//
	if (**Stream==';') // Skip past comments
		{
		while ((**Stream!=0)&&(**Stream!=10)&&(**Stream!=13)) (*Stream)++;
		goto SkipJunk;
		};
	//
	// Upon exit, *Stream either points to valid Stream or a zero character.
	//
	UNGUARD("GetNEXT");
	};

//
// See if a command-line parameter exists in the stream.
//
int UNREAL_API GetParam (const char *Stream,const char *Param)
	{
	GUARD;
	const char *Start = mystrstr(Stream,Param);
	if (Start && (Start>Stream))
		{
		return (Start[-1]=='-') || (Start[-1]=='/');
		}
	else return 0;
	UNGUARD("GetParam");
	};

//
// Skip to the end of this line
//
void UNREAL_API SkipLINE (const char **Stream)
	{
	GUARD;
	//
	// Skip over spaces, tabs, cr's, and linefeeds:
	//
	while ((**Stream!=10)&&(**Stream!=13)&&(**Stream!=0)) (*Stream)++;
	GetNEXT(Stream);
	UNGUARD("SkipLINE");
	};

//
// Grab the next string from the input stream.
//
int UNREAL_API GrabSTRING(const char *&Str,char *Result, int MaxLen)
	{
	int Len=0;
	while ((*Str==' ')||(*Str==9)) Str++;
	while ((*Str) && (*Str!=' ') && (*Str!=9) && ((Len+1)<MaxLen))
		{
		Result[Len++] = *Str++;
		};
	Result[Len]=0;
	return Len!=0;
	};


/*-----------------------------------------------------------------------------
	Setters
	These don't validate lengths so you need to call them with a big buffer
-----------------------------------------------------------------------------*/

UNREAL_API char *SetFVECTOR (char *Dest,const FVector *FVector)
	{
	GUARD;
	sprintf (Dest,"%+013.6f,%+013.6f,%+013.6f",FVector->X,FVector->Y,FVector->Z);
	return Dest;
	UNGUARD("SetFVECTOR");
	};

UNREAL_API char *SetFIXFVECTOR (char *Dest,const FVector *FVector)
	{
	GUARD;
	sprintf (Dest,"%+013.6f,%+013.6f,%+013.6f",FVector->X/65536.0,FVector->Y/65536.0,FVector->Z/65536.0);
	return Dest;
	UNGUARD("SetFIXFVECTOR");
	};

UNREAL_API char *SetROTATION (char *Dest,const FRotation *Rotation)
	{
	GUARD;
	sprintf (Dest,"%i,%i,%i",Rotation->Pitch,Rotation->Yaw,Rotation->Roll);
	return Dest;
	UNGUARD("SetROTATION");
	};

UNREAL_API char *SetFSCALE (char *Dest,const FScale *FScale)
	{
	GUARD;
	sprintf (Dest,"X=%+013.6f Y=%+013.6f Z=%+013.6f S=%+013.6f AXIS=%i",FScale->Scale.X,FScale->Scale.Y,FScale->Scale.Z,FScale->SheerRate,FScale->SheerAxis);
	return Dest;
	UNGUARD("SetFSCALE");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
