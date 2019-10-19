/*=============================================================================
	UnTexEd.cpp: Unreal editor texture code

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

//
// Globals for link topic system:
//
DWORD	GNumTexResults = 0;
DWORD	GCurTexResult  = 0;
DWORD	GNumFamResults = 0;
DWORD	GCurFamResult  = 0;
FName	*GTempFamList  = NULL;
UTexture **GTempTexList  = NULL;

#define MAX_TEX_RESULTS 1024
#define MAX_FAM_RESULTS 256

/*-----------------------------------------------------------------------------
	Tex link topic functions
-----------------------------------------------------------------------------*/

//
// Query a list of textures.  Call with texture family's name, or "All" for all families.
//
void texQueryTextureForLink (char *FamilyName)
	{
	GUARD;
	FName			Name;
	int		  		All = !stricmp(FamilyName,"All");
	UTexture		*Texture;
	//
	Name.Add(FamilyName);
	if (GTempTexList==NULL) GTempTexList = (UTexture **)appMalloc (MAX_TEX_RESULTS * sizeof (UTexture *),"TexQuery");
	//
	GCurTexResult  = 0;
	GNumTexResults = 0;
	//
	FOR_ALL_TYPED_RES(Texture,RES_Texture,UTexture)
		{
		if ((GNumTexResults < MAX_TEX_RESULTS) &&
			((Texture->FamilyName==Name) ||
			(All&&(!Texture->FamilyName.IsNone()))))
			{
			GTempTexList [GNumTexResults++] = Texture;
			};
		}
	END_FOR_ALL_TYPED_RES;
	//
	UNGUARD("texQueryTextureForLink");
	};

//
// Query a list of texture families.
//
void texQueryFamilyForLink (int All)
	{
	GUARD;
	UTexture		*Texture;
	FName			ListName;
	//
	if (GTempFamList==NULL) GTempFamList = (FName *)appMalloc(MAX_FAM_RESULTS * sizeof (FName),"TexQuery");
	//
	GCurFamResult  = 0;
	GNumFamResults = 0;
	//
	GTempFamList [GNumFamResults++].Add("All");
	FOR_ALL_TYPED_RES(Texture,RES_Texture,UTexture)
		{
		ListName = Texture->FamilyName;
		if ((!ListName.IsNone()) && (All || (ListName.Name()[0]!='!')))
			{
			DWORD j;
			for (j=0; j<GNumFamResults; j++) if (GTempFamList[j] == ListName) break;
			if ((j >= GNumFamResults) && (GNumFamResults < MAX_FAM_RESULTS))
				GTempFamList [GNumFamResults++] = ListName;
			};
		};
	END_FOR_ALL_TYPED_RES;
	//
	UNGUARD("texQueryFamilyForLink");
	};

//
// Tex link topic function
//
AUTOREGISTER_TOPIC("Tex",TexTopicHandler);
void TexTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	GUARD;
	UTexture	*Texture;
	FName		FamilyName;
	//
	if ((stricmp(Item,"QUERYTEX")==0) && (GCurTexResult < GNumTexResults))
		{
		Texture = GTempTexList [GCurTexResult];
		//
		sprintf(Data,"%s (%ix%i)",Texture->Name,Texture->USize, Texture->VSize);
		GCurTexResult++;
		}
	else if ((stricmp(Item,"QUERYFAM")==0) && (GCurFamResult < GNumFamResults))
		{
		FamilyName = GTempFamList [GCurFamResult];
		sprintf(Data,"%s",FamilyName.Name());
		GCurFamResult++;
		};
	UNGUARD("TexTopicHandler::Get");
	};
void TexTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
	{
	GUARD;
	UNGUARD("TexTopicHandler::Set");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
