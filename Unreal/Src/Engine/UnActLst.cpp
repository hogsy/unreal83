/*=============================================================================
	UnActor.cpp: UActorList implementation

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

/*-----------------------------------------------------------------------------
	AActorDraw implementation
-----------------------------------------------------------------------------*/

//
// Update a moving brush's position based on its controlling actor's
// position.  If Editor=1 and UnrealEd is active, snaps the brush to the
// grid as needed.
//
void AActorDraw::UpdateBrushPosition(ILevel *Level,INDEX iActor,int Editor)
	{
	if (IsBrush())
		{
		Brush->Location = Location;
		Brush->Rotation = DrawRot;
		//
		if (Editor && GEditor)
			{
			//
			// Snap brush rotation and location to grid:
			//
			if (GEditor->Constraints.RotGridEnabled)
				{
				Brush->Rotation = Brush->Rotation.GridSnap(GEditor->Constraints.RotGrid);
				};
			if (GEditor->Constraints.GridEnabled)
				{
				Brush->Location = Brush->Location.GridSnap(GEditor->Constraints.Grid);
				};
			if (Level) Level->SendMessage(iActor,ACTOR_PostEditMove,NULL);
			};
		Brush->BuildBound(1);
		};
	};

//
// Returns 1 if the actor is a brush, 0 if not.
//
int AActorDraw::IsBrush(void)
	{
	return (DrawType==DT_Brush) && (Brush!=NULL);
	};

//
// Returns 1 if the actor is a moving brush, 0 if not.
//
int AActorDraw::IsMovingBrush(void)
	{
	return (DrawType==DT_Brush) && (Brush!=NULL);
	};

/*-----------------------------------------------------------------------------
	UActorList exportation helpers
-----------------------------------------------------------------------------*/

//
// Set the text type, name, and value of a single actor property.  Used for
// exporting actor properties.
//
void ExportActorProperty(char *Type,char *Name, char *Value, FClassProperty *ClassProperty,
	AActor *Actor, int Flags, int Descriptive, int ArrayElement, AActor *Delta, FName Category)
	{
	GUARD;
	BYTE *RawActor = (BYTE *)Actor;
	//
	Type[0]=0; Name[0]=0; Value[0]=0;
	//
	if (ClassProperty->PropertyName.IsNone()) appErrorf("Unknown name in class %s",Actor->Class->Name,1);
	//
	BYTE *PropertyValue = &RawActor
		[
		ClassProperty->PropertyOffset + ArrayElement * ClassProperty->PropertyElementSize
		];
	if (((ClassProperty->PropertyFlags & Flags) || !Flags) &&
		((!Delta) || !ClassProperty->Compare(Actor,Delta,ArrayElement)) &&
		((Category==NAME_NONE)||(Category==ClassProperty->PropertyCategory)))
		{
		strcpy(Name,ClassProperty->PropertyName.Name());
		switch (ClassProperty->PropertyType)
			{
			case CPT_Byte:
				{
				BYTE Temp = *(BYTE *)PropertyValue;
				if (ClassProperty->Enum) sprintf(Type,"BYTE.%s",ClassProperty->Enum->Name);
				else strcpy(Type,"BYTE");
				//
				if (ClassProperty->Enum && Descriptive)
					{
					sprintf(Value,"%i - %s",Temp,ClassProperty->Enum->Element(Temp).Name());
					}
				else
					{
					sprintf(Value,"%i",Temp);
					};
				};
				break;
			case CPT_Integer:
				{
				strcpy(Type,"INTEGER");
				sprintf(Value,"%i",*(INT *)PropertyValue);
				};
				break;
			case CPT_Real:
				{
				strcpy(Type,"REAL");
				sprintf(Value,"%+013.6f",*(FLOAT *)PropertyValue);
				};
				break;
			case CPT_String:
				{
				strcpy(Type,"STRING");
				if (Descriptive)	sprintf(Value,"%s",(char *)PropertyValue);
				else				sprintf(Value,"\"%s\"",(char *)PropertyValue);
				break;
				};
			case CPT_Boolean:
				{
				strcpy(Type,"BOOLEAN");
				char *Temp = ((*(DWORD *)PropertyValue) & ClassProperty->BitMask) ? "True" : "False";
				sprintf(Value,"%s",Temp);
				break;
				};
			case CPT_Actor:
				{
				strcpy(Type,"ACTOR");
				INDEX Temp = *(INDEX *)PropertyValue;
				if (Temp!=INDEX_NONE) sprintf(Value,"%i",Temp);
				else sprintf(Value,"None");
				};
				break;
			case CPT_Resource:
				{
				strcpy(Type,GRes.Types[ClassProperty->PropertyResType].Descr);
				UResource *Temp = *(UResource **)PropertyValue;
				if (Temp) strcpy(Value,Temp->Name);
				else strcpy(Value,"None");
				};
				break;
			case CPT_Name:
				{
				strcpy(Type,"NAME");
				FName Temp = *(FName *)PropertyValue;
				strcpy(Value,Temp.Name());
				};
				break;
			case CPT_Vector:
				{
				strcpy(Type,"VECTOR");
				FVector *Temp = (FVector *)PropertyValue;
				sprintf(Value,"(%+013.6f,%+013.6f,%+013.6f)",Temp->X,Temp->Y,Temp->Z);
				};
				break;
			case CPT_Rotation:
				{
				strcpy(Type,"ROTATION");
				FRotation *TempRot = (FRotation *)PropertyValue;
				sprintf(Value,"(%i,%i,%i)",TempRot->Pitch,TempRot->Yaw,TempRot->Roll);
				};
				break;
			default:
				appErrorf("ExportActor: Unknown type in class %s",Actor->Class->Name);
				break;
			};
		};
	UNGUARD("ExportActorProperty");
	};

//
// Export one entire actor, or the specified named property, to text.
//
int ExportActor (AActor *Actor,char *Ptr,FName PropertyName,
	int Indent,int Descriptive,int Flags, AActor *Delta,int Resources,int ArrayElement,
	FName Category)
	{
	GUARD;
	char			*NewPtr	= Ptr;
	char			Type[128],Name[128],Value[128];
	FClassProperty	*ClassProperty;
	//
	// Export actor
	//
	ClassProperty = &Actor->Class->Element(0);
	for (int i=0; i<Actor->Class->Num; i++)
		{
		if ((PropertyName.IsNone()) || (PropertyName==ClassProperty->PropertyName))
			{
			if ((ClassProperty->PropertyArrayDim==0)||(ClassProperty->PropertyType==CPT_String))
				{
				// Export single element
				ExportActorProperty(Type,Name,Value,ClassProperty,Actor,Flags,Descriptive,0,Delta,Category);
				if (Type[0] && Name[0])
					{
					if (Resources && (ClassProperty->PropertyType==CPT_Resource) &&
						(ClassProperty->PropertyFlags & CPF_ExportResource))
						{
						UResource *Res = *(UResource **)Actor->GetPropertyPtr(i);
						if (Res && !(Res->Flags & RF_TagImp)) // Don't export more than once
							{
							NewPtr = Res->Export(NewPtr,"",Indent+1);
							Res->Flags |= RF_TagImp;
							};
						};
					if (Descriptive!=2) NewPtr += sprintf(NewPtr,"%s%s %s=%s\r\n",spc(Indent),Descriptive?Type:"",Name,Value);
					else                NewPtr += sprintf(NewPtr,"%s",Value);
					//bug("%s%s %s=%s\r\n",spc(Indent),Descriptive?Type:"",Name,Value);
					};
				}
			else
				{
				// Export array
				for (DWORD j=0; j<ClassProperty->PropertyArrayDim; j++)
					{
					ExportActorProperty(Type,Name,Value,ClassProperty,Actor,Flags,Descriptive,j,Delta,Category);
					if (Type[0] && Name[0] && ((ArrayElement==-1) || (ArrayElement==(int)j)))
						{
						if (Resources && (ClassProperty->PropertyType==CPT_Resource) &&
							(ClassProperty->PropertyFlags & CPF_ExportResource))
							{
							UResource *Res = *(UResource **)Actor->GetPropertyPtr(i,j);
							if (Res && !(Res->Flags & RF_TagImp)) // Don't export more than once
								{
								NewPtr = Res->Export(NewPtr,"",Indent+1);
								Res->Flags |= RF_TagImp;
								};
							};
						if (Descriptive!=2) NewPtr += sprintf(NewPtr,"%s%s %s(%i)=%s\r\n",spc(Indent),Descriptive?Type:"",Name,j,Value);
						else                NewPtr += sprintf(NewPtr,"%s",Value);
						};
					};
				};
			};
		ClassProperty++;
		};
	return (int)(NewPtr-Ptr);
	UNGUARD("ExportActor");
	};

//
// Export multiple selected actors for editing.  This sends only the properties that are
// shared between all of the selected actors, and the text values is sent as a blank if
// the property values are not all identical.
//
// Only exports CPF_Edit (editable) properties.
// Does not export arrays properly, except for strings (only exports first element of array).
// Assumes that the script compiler prevents non-string arrays from being declared as editable.
//
int ExportMultipleActors (UActorList *Actors,char *Ptr,FName PropertyName,
	int Indent,int Descriptive,FName Category)
	{
	GUARD;
	AActor			*FirstActor,*Actor;
	char			*NewPtr	= Ptr;
	int				iActor,i,j,iStart=-1;
	char			FirstType[128],FirstName[128],FirstValue[128];
	char			Type[128],Name[128],Value[128];
	//
	// Find first actor:
	//
	for (iActor=0; iActor<Actors->Max; iActor++)
		{
		FirstActor = &Actors->Element(iActor);
		if (FirstActor->Class && FirstActor->bSelected)
			{
			iStart = iActor+1;
			break;
			};
		};
	if (iStart>=0) // At least one actor is selected
		{
		//
		// Find property in first actor:
		//
		FClassProperty *FirstClassProperty = &FirstActor->Class->Element(0);
		for (i=0; i<FirstActor->Class->Num; i++)
			{
			if (((PropertyName.IsNone()) || (PropertyName==FirstClassProperty->PropertyName)) &&
				(FirstClassProperty->PropertyArrayDim==0))
				{
				ExportActorProperty(FirstType,FirstName,FirstValue,FirstClassProperty,FirstActor,CPF_Edit,Descriptive,0,NULL,Category);
				if (FirstType[0] && FirstName[0])
					{
					//
					// Now go through all other actors and see if this property is shared:
					//
					for (iActor=iStart; iActor<Actors->Max; iActor++)
						{
						Actor = &Actors->Element(iActor);
						if (Actor->Class && Actor->bSelected)
							{
							FClassProperty *ClassProperty = &Actor->Class->Element(0);
							for (j=0; j<Actor->Class->Num; j++)
								{
								if (ClassProperty->PropertyName==FirstClassProperty->PropertyName)
									{
									if (ClassProperty->PropertyArrayDim != 0) goto NextProperty;
									ExportActorProperty(Type,Name,Value,ClassProperty,Actor,CPF_Edit,Descriptive,0,NULL,Category);
									if (stricmp(Type,FirstType)||stricmp(Name,FirstName))
										{										
										goto NextProperty; // Mismatch
										};
									if (stricmp(Value,FirstValue))
										{
										FirstValue[0]=0; // Blank out value if not identical
										};
									};
								ClassProperty++;
								};
							};
						};
					if (strcmp(Value,FirstValue)==0)
						{
						NewPtr += sprintf(NewPtr,"%s%s %s=%s\r\n",spc(Indent),Descriptive?Type:"",Name,Value);
						}
					else
						{
						NewPtr += sprintf(NewPtr,"%s%s %s=\r\n",spc(Indent),Descriptive?Type:"",Name);
						};
					};
				};
			NextProperty:;
			FirstClassProperty++;
			};
		};
	return (int)(NewPtr-Ptr);
	UNGUARD("ExportMultipleActors");
	};

/*-----------------------------------------------------------------------------
	UActorList importation helpers
-----------------------------------------------------------------------------*/

//
// Import all of the properties in Data into the specified actor.
// Data may contain multiple properties, but it might not contain all of 
// the properties for the actor, so the actor should be initialized
// first.
//
// This requires that the actor's Class be set in advance.
//
const char *ImportActorProperties(AActor *Actor, const char *Data)
	{
	GUARD;
	UClass	*Class		= Actor->Class;
	BYTE	*RawActor	= (BYTE *)Actor;
	char	*PropText	= (char *)GMem.Get(65536);
	char	*Top		= &PropText[0];
	char	*MemTop		= &PropText[0];
	//
	// Parse all resources stored in the actor.
	// Build list of all text properties.
	//
	*PropText = 0;
	char StrLine[256];
	while (GetLINE (&Data,StrLine,256)==0)
		{
		const char *Str = &StrLine[0];
		if (GetBEGIN(&Str,"BRUSH")) // Parse brush on this line
			{
			char BrushName[NAME_SIZE];
			if (GetSTRING(Str,"NAME=",BrushName,NAME_SIZE))
				{
				//
				// If a brush with this name already exists in the
				// level, rename the existing one.  This is necessary
				// because we can't rename the brush we're importing without
				// losing our ability to associate it with the actor properties
				// that reference it.
				//
				UModel *ExistingBrush = new(BrushName,FIND_Optional)UModel;
				if (ExistingBrush)
					{
					char TempName[NAME_SIZE];
					GRes.MakeUniqueName(TempName,BrushName,"",RES_Model);
					//
					debugf(LOG_Info,"Renaming %s to %s",ExistingBrush->Name,TempName);
					strcpy(ExistingBrush->Name,TempName);
					if (ExistingBrush->Polys) strcpy(ExistingBrush->Polys->Name,TempName);
					};
				UModel *Brush = new(BrushName,CREATE_Unique)UModel;
				Data = Brush->Import(Data,NULL,"");
				};
			}
		else if (GetEND(&Str,"ACTOR") || GetEND(&Str,"DEFAULTPROPERTIES")) // End of actor properties
			{
			break;
			}
		else // More actor properties
			{
			strcpy(Top,Str);
			Top += strlen(Top);
			};
		};
	//
	// Parse all text properties
	//
	FClassProperty	*Property = Class->GetData();
	for (int i=0; i<Class->Num; i++)
		{
		char LookFor[80];
		//
		int IsSingleElement = (Property->PropertyArrayDim==0) || (Property->PropertyType==CPT_String);
		int n               = IsSingleElement ? 1 : Property->PropertyArrayDim;
		//
		for (int j=0; j<n; j++)
			{
			BYTE *Value	= &RawActor
				[
				Property->PropertyOffset + j*Property->PropertyElementSize
				];
			if (IsSingleElement)	sprintf(LookFor,"%s=",Property->PropertyName.Name());
			else					sprintf(LookFor,"%s(%i)=",Property->PropertyName.Name(),j);
			//
			switch(Property->PropertyType)
				{
				case CPT_Byte:
					GetBYTE(PropText,LookFor,(BYTE *)Value);
					break;
				case CPT_Integer:
					GetINT(PropText,LookFor,(INT *)Value);
					break;
				case CPT_Boolean:
					int Result;
					if (GetONOFF(PropText,LookFor,&Result))
						{
						if (Result) *(DWORD *)Value |=  Property->BitMask;
						else		*(DWORD *)Value &= ~Property->BitMask;
						};
					break;
				case CPT_Real:
					GetFLOAT(PropText,LookFor,(FLOAT *)Value);
					break;
				case CPT_Actor:
					GetINDEX(PropText,LookFor,(INDEX *)Value);
					break;
				case CPT_Resource:
					GetRES(PropText,LookFor,Property->PropertyResType,(UResource **)Value);
					break;
				case CPT_Name:
					GetNAME(PropText,LookFor,(FName *)Value);
					break;
				case CPT_String:
					GetSTRING(PropText,LookFor,(char *)Value,Property->PropertySize);
					break;
				case CPT_Vector:
					GetFVECTOR(PropText,LookFor,(FVector *)Value);
					break;
				case CPT_Rotation:
					GetFROTATION(PropText,LookFor,(FRotation *)Value,1);
					break;
				default:
					appErrorf("Bad class property type %i in %s",Property->PropertyType,Class->Name);
				};
			};
		Property++;
		};
	GMem.Release(MemTop);
	return Data;
	//
	UNGUARD("ImportActorProperties");
	};

/*-----------------------------------------------------------------------------
	UActorList resource implementation
-----------------------------------------------------------------------------*/

//
// Standard resource functions
//
void UActorList::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UActorList);
	Type->RecordSize = sizeof (AActor);
	Type->Version    = 1;
	strcpy (Type->Descr,"ActorList");
	UNGUARD("UActorList::Register");
	};
void UActorList::InitHeader(void)
	{
	GUARD;
	Max = 0;
	Num = 0;
	LockType  = LOCK_None;
    StaticActors        = 0;
    DynamicActors       = 0;
    CollidingActors     = 0;
    ActiveActors        = 0;
    UnusedActors        = 0;
    JustDeletedActors   = 0;
	UNGUARD("UActorList::InitHeader");
	};
void UActorList::InitData(void)
	{
	GUARD;
	Num = 0;
	UNGUARD("UActorList::InitData");
	};
const char *UActorList::Import(const char *Buffer, const char *BufferEnd,const char *FileType)
	{
	GUARD;
	char StrLine[256];
	//
	// Empty all current actors:
	//
	for (INDEX i=0; i<Max; i++) Element(i).Class=NULL;
    StaticActors        ->Empty(TRUE);
    DynamicActors       ->Empty(TRUE);
    CollidingActors     ->Empty(TRUE);
    ActiveActors        ->Empty(TRUE);
    UnusedActors        ->Empty(TRUE);
    JustDeletedActors   ->Empty(TRUE);
	//
	// Import sparse actor list:
	//
	AActor *Actor = NULL;
	while (GetLINE (&Buffer,StrLine,256)==0)
		{
		const char *Str = &StrLine[0];
		if (GetBEGIN(&Str,"ACTOR"))
			{
			INDEX  iActor;
			UClass *TempClass;
			if (GetINDEX(Str,"INDEX=",&iActor) && GetUClass(Str,"CLASS=",&TempClass))
				{
				if (iActor<Max)
					{
					Actor			= &Element(iActor);
					Actor->Class	= TempClass;
					memcpy (Actor,&Actor->Class->DefaultActor,sizeof(AActor));
					};
				Buffer = ImportActorProperties(Actor,Buffer);
				Actor->iMe = iActor;
				};
			}
		else if (GetEND(&Str,"ACTORLIST")) break;
		};
	Num=Max;
	Realloc();
    RelistActors();
	return Buffer;
	UNGUARD("UActorList::Import");
	};
char *UActorList::Export(char *Buffer,const char *FileType,int Indent)
	{
	GUARD;
	//
	// Export all active actors:
	//
	Buffer += sprintf (Buffer,"%sBegin ActorList Max=%i\r\n",spc(Indent),Max);
	for (INDEX iActor=0; iActor<Num; iActor++)
		{
		AActor *Actor = &Element(iActor);
		if (Actor->Class && (Actor->Class!=GClasses.Camera))
			{
			Buffer += sprintf(Buffer,"%s   Begin Actor Index=%i Class=%s\r\n",spc(Indent),iActor,Actor->Class->Name);
			Buffer += ExportActor (Actor,Buffer,NAME_NONE,Indent+6,0,0,&Actor->Class->DefaultActor,1,-1,NAME_NONE);
			Buffer += sprintf(Buffer,"%s   End Actor\r\n",spc(Indent));
			};
		};
	Buffer += sprintf (Buffer,"%sEnd ActorList\r\n",spc(Indent));
	return Buffer;
	UNGUARD("UActorList::Export");
	};
void UActorList::QueryHeaderReferences(FResourceCallback &Callback)
	{
	GUARD;
	UNGUARD("UActorList::QueryHeaderReferences");
	};
void UActorList::QueryDataReferences(FResourceCallback &Callback)
	{
	GUARD;
	//
	AActor *Actor = &Element(0);
	for (INDEX iActor=0; iActor<Num; iActor++)
		{
		if (Actor->Class) Actor->QueryReferences(this,Callback,0);
		Actor++;
		};
	UNGUARD("UActorList::QueryDataReferences");
	};
void UActorList::PostLoad(void)
	{
	GUARD;
	AActor *Actor = &Element(0);
	for (int i=0; i<Num; i++)
		{
		if (Actor->Class)
			{
			FClassProperty	*Property = &Actor->Class->Element(0);
			for (int j=0; j<Actor->Class->Num; j++)
				{
				if ((Property->PropertyType==CPT_Resource)&&(Property->PropertyFlags & CPF_NoSaveResource))
					{
					BYTE *RawActor = (BYTE *)Actor;
					*(UResource **)(&RawActor[Property->PropertyOffset]) = NULL;
					};
				Property++;
				};
			};
		Actor++;
		};
    StaticActors        = 0;
    DynamicActors       = 0;
    CollidingActors     = 0;
    ActiveActors        = 0;
    UnusedActors        = 0;
    JustDeletedActors   = 0;
    RelistActors();
	UNGUARD("UActorList::PostLoad");
	};
AUTOREGISTER_RESOURCE(RES_ActorList,UActorList,0xB2D90850,0xCCD211cf,0x91360000,0xC028B992);

/*---------------------------------------------------------------------------------------
   Actor link topic handler
---------------------------------------------------------------------------------------*/

enum {MAX_PROP_CATS=64};
FName GPropCats[MAX_PROP_CATS];
int GNumPropCats;

void CheckPropCats(UClass *Class)
	{
	for (int i=0; i<Class->Num; i++)
		{
		FClassProperty *Prop = &Class->Element(i);
		FName CatName = Prop->PropertyCategory;
		if ((CatName!=NAME_NONE) && (GNumPropCats < MAX_PROP_CATS))
			{
			int j;
			for (j=0; j<GNumPropCats; j++) if (GPropCats[j]==CatName) break;
			if (j>=GNumPropCats) GPropCats[GNumPropCats++] = CatName;
			};
		};
	};

AUTOREGISTER_TOPIC("Actor",ActorTopicHandler);
void ActorTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	GUARD;
	UActorList	*Actors = Level->ActorList;
	UClass		*AllClass   = NULL;
	INDEX		i,n,AnyClass;
	//
	Actors->Lock(LOCK_Read);
	n        = 0;
	AnyClass = 0;
	for (i=0; i<Actors->Max; i++)
		{
		if (Actors->Element(i).Class && Actors->Element(i).bSelected)
			{
			n++;
			if (AnyClass && (Actors->Element(i).Class != AllClass)) AllClass=NULL;
			else AllClass = Actors->Element(i).Class;
			AnyClass=1;
			};
		};
	Actors->Unlock();
	//
	if ((!strnicmp(Item,"Properties",10)) || (!strnicmp(Item,"DefaultProperties",17)))
		{
		//
		// Parse name and optional array element:
		//
		FName PropertyName = NAME_NONE;
		int Element = -1;
		char Temp[80];
		if (GetSTRING(Item,"NAME=",Temp,80))
			{
			char *c = strstr(Temp,"(");
			if (c)
				{
				Element = atoi(&c[1]);
				*c = 0;
				};
			if (!PropertyName.Find(Temp)) return;
			};
		int PropertyMask = CPF_Edit; // Only return editable properties
		int Descriptive  = 1;
		//
		int Raw=0; GetONOFF(Item,"RAW=",&Raw);
		if (Raw)
			{
			PropertyMask = 0; // Return all requested properties
			Descriptive  = 2; // Data only, not names or formatting
			};
		FName CategoryName=NAME_NONE;
		GetNAME(Item,"CATEGORY=",&CategoryName);
		//
		if (!strnicmp(Item,"Properties",10))
			{
			if (n==1) // 1 actor is selected - just send it
				{
				for (i=0; i<Actors->Max; i++)
					{
					AActor *Actor = &Actors->Element(i);
					if (Actor->Class && Actor->bSelected)
						{
						ExportActor (Actor,(char *)Data,PropertyName,0,Descriptive,PropertyMask,NULL,0,Element,CategoryName);
						break;
						};
					};
				}
			else if (n>1) ExportMultipleActors(Actors,(char *)Data,PropertyName,0,1,CategoryName);
			}
		else if (!strnicmp(Item,"DefaultProperties",17))
			{
			UClass *Class;
			if (GetUClass(Item,"CLASS=",&Class))
				{
				if(Class!=Class->DefaultActor.Class) appError ("Actor class mismatch");
				ExportActor (&Class->DefaultActor,(char *)Data,PropertyName,0,1,PropertyMask,NULL,0,Element,CategoryName);
				};
			};
		}
	if ((!strnicmp(Item,"PropCats",8)) || (!strnicmp(Item,"DefaultPropCats",15)))
		{
		GNumPropCats=0;
		if (!strnicmp(Item,"PropCats",8))
			{
			for (int i=0; i<Actors->Max; i++)
				{
				AActor *Actor = &Actors->Element(i);
				if (Actor->Class && Actor->bSelected) CheckPropCats(Actor->Class);
				};
			}
		else
			{
			UClass *Class;
			if (GetUClass(Item,"CLASS=",&Class)) CheckPropCats(Class);
			};
		*Data=0;
		for (int i=0; i<GNumPropCats; i++) Data += sprintf(Data,"%s ",GPropCats[i].Name());
		}
	else if (!stricmp(Item,"NumSelected"))
		{
		sprintf(Data,"%i",n);
		}
	else if (!stricmp(Item,"ClassSelected"))
		{
		if (AnyClass && AllClass) sprintf(Data,"%s",AllClass->Name);
		else sprintf(Data,"");
		}
	else if (!strnicmp(Item,"IsKindOf",8)) // Sees if the 1 selected actor belongs to a class
		{
		UClass *Class;
		if (GetUClass(Item,"CLASS=",&Class) && AllClass && AllClass->IsKindOf(Class)) sprintf(Data,"1");
		else sprintf(Data,"0");
		};
	UNGUARD("ActorTopicHandler::Get");
	};
void ActorTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
	{
	GUARD;
	UActorList	*Actors = Level->ActorList;
	//
	if (!stricmp(Item,"Properties"))
	    {
		GTrans->Begin(Level,"Changing actor property");
		//
		ILevel LevelInfo;
		Level->Lock(&LevelInfo,LOCK_Trans);
		//
		for (int i=0; i<Actors->Max; i++)
			{
			AActor *Actor = &Actors->Element(i);
			if (Actor->Class && Actor->bSelected)
				{
				LevelInfo.SendMessageReverse(i,ACTOR_PreEditChange,NULL);
				//
				GTrans->NoteActor (Actors,i);
				ImportActorProperties(&Actors->Element(i),Data);
				Actors->Element(i).iMe = i;
				//
				LevelInfo.SendMessageReverse(i,ACTOR_PostEditChange,NULL);
				//
				Actor->bTempLightChanged = 1; // Force light meshes to be rebuilt
				};
			};
		Level->Unlock(&LevelInfo);
		//
		GTrans->End();
		}
	else if (!strnicmp(Item,"DefaultProperties",17))
	    {
		UClass *Class;
		if (GetUClass(Item,"CLASS=",&Class))
			{
			GTrans->Begin			(Level,"Changing default actor property");
			GTrans->NoteResHeader	(Class);
			ImportActorProperties	(&Class->DefaultActor,Data);
			GTrans->End				();
			};
		};
	UNGUARD("ActorTopicHandler::Set");
	};

/*-----------------------------------------------------------------------------
	Actor list locking and unlocking
-----------------------------------------------------------------------------*/

//
// Lock an actor list
//
void UActorList::Lock (int NewLockType)
	{
	GUARD;
	//
	if (NewLockType == LOCK_None) 	appError ("LOCK_NONE");
	if (LockType != LOCK_None) 		appErrorf("%s is already locked",Name);
	//
	if (GTrans && (NewLockType==LOCK_Trans))
		{
		GTrans->NoteResHeader(this);
		Trans=1;
		}
	else Trans=0;
	//
	LockType = NewLockType;
	Modify(); // Mark as modified to prevent swapping
	//
	UNGUARD("UActorList::Lock");
	};

//
// Unlock an actor list
//
void UActorList::Unlock (void)
	{
	GUARD;
	LockType = LOCK_None;
	UNGUARD("UActorList::Unlock");
	};

/*-----------------------------------------------------------------------------
	Actor list manipulations
-----------------------------------------------------------------------------*/
//
// Rebuild the above redundant lists from scratch by scanning the actor list.
//
void UActorList::RelistActors()
{
    GUARD;

    if( UnusedActors == 0 )
    {
        StaticActors        = new CompactList;
        DynamicActors       = new CompactList;
        CollidingActors     = new CompactList;
        ActiveActors        = new CompactList;
        UnusedActors        = new CompactList;
        JustDeletedActors   = new CompactList;
    }
    else
    {
        ActiveActors->Empty();
        UnusedActors->Empty();
        JustDeletedActors->Empty();
        StaticActors->Empty();
        DynamicActors->Empty();
        CollidingActors->Empty();
    }
    for( int Which = 0; Which < Max; Which++)
    {
        AActor * Actor = &Element(Which);
        if( Which >= Num )
        {
            // Actors in the latter part of the array (beyond Num) are unused.
            Actor->Class        = 0     ;
            Actor->bJustDeleted = FALSE ;
        }
        if( Actor->Class != 0 )
        {
            ActiveActors->Add(Actor);
            ListActor( Actor );
        }
        else if( Actor->bJustDeleted )
        {
            JustDeletedActors->Add(Actor);
        }
        else
        {
            UnusedActors->Add(Actor);
        }
    }
    UNGUARD("UActorList::RelistActors");
}
 
//
// Remove an actor from the appropriate main lists (based on its properties).
//
void UActorList::UnlistActor(AActor * Actor)
    {
        if( Actor->bStaticActor )
        {
            StaticActors->RemoveActor(Actor);
        }
        else
        {
            DynamicActors->RemoveActor(Actor);
        }
        if( Actor->bCollideActors )
        {
            CollidingActors->RemoveActor(Actor);
        }
    }

//
// Add an actor to appropriate main lists (based on its properties).
//
void UActorList::ListActor(AActor * Actor)
{
    if( Actor->Class != 0 )
    {
        if( Actor->bStaticActor )
        {
            StaticActors->Add(Actor);
        }
        else
        {
            DynamicActors->Add(Actor);
        }
        if( Actor->bCollideActors )
        {
            CollidingActors->Add(Actor);
        }
    }
}

//
// Debug: Check the redundant lists for correctness and completeness.
void UActorList::CheckLists()
{
    GUARD;
    // As a bonus, let's give some info about the lists:
    debugf( LOG_Info, "ActiveActors: %i", ActiveActors->Count() );
    debugf( LOG_Info, "UnusedActors: %i", UnusedActors->Count() );
    debugf( LOG_Info, "JustDeletedActors: %i", JustDeletedActors->Count() );
    debugf( LOG_Info, "StaticActors: %i", StaticActors->Count() );
    debugf( LOG_Info, "DynamicActors: %i", DynamicActors->Count() );
    debugf( LOG_Info, "CollidingActors: %i", CollidingActors->Count() );
    for( int Which = 0; Which < Num; Which++)
    {
        AActor * Actor = &Element(Which);
        const BOOL IsOnActiveList       = ActiveActors->Find(Actor)      >= 0;
        const BOOL IsOnUnusedList       = UnusedActors->Find(Actor)      >= 0;
        const BOOL IsOnJustDeletedList  = JustDeletedActors->Find(Actor) >= 0;
        const int Count = // Number of administrative lists this actor is on.
            ( IsOnActiveList       ? 1 : 0 )
        +   ( IsOnUnusedList       ? 1 : 0 )
        +   ( IsOnJustDeletedList  ? 1 : 0 )
        ;
        if( Count != 1 ) // Actor is not on exactly one administrative list?
        {
            debugf( LOG_Info, "Actor [%i] is on %i administrative lists!", Which, Count );
        }
        if( IsOnActiveList && Actor->Class == 0 )
        {
            debugf( LOG_Info, "Actor [%i] on active list with class==0", Which );
        }
        if( IsOnUnusedList && Actor->Class != 0 )
        {
            debugf( LOG_Info, "Actor [%i] on unused list with class!=0", Which );
        }
        if( IsOnUnusedList && Actor->bJustDeleted )
        {
            debugf( LOG_Info, "Actor [%i] on unused list with bJustDeleted", Which );
        }
        if( IsOnJustDeletedList && Actor->Class != 0 )
        {
            debugf( LOG_Info, "Actor [%i] on just deleted list with class!=0", Which );
        }
        if( IsOnJustDeletedList && !Actor->bJustDeleted )
        {
            debugf( LOG_Info, "Actor [%i] on just deleted list with !bJustDeleted", Which );
        }
        const BOOL IsOnStaticList    = StaticActors->Find(Actor)    >= 0;
        const BOOL IsOnDynamicList   = DynamicActors->Find(Actor)   >= 0;
        const BOOL IsOnCollidingList = CollidingActors->Find(Actor) >= 0;
        if( IsOnStaticList && Actor->Class == 0 )
        {
            debugf( LOG_Info, "Actor [%i] on static list with Class==0", Which );
        }
        if( IsOnDynamicList && Actor->Class == 0 )
        {
            debugf( LOG_Info, "Actor [%i] on dynamic list with Class==0", Which );
        }
        if( IsOnCollidingList && Actor->Class == 0 )
        {
            debugf( LOG_Info, "Actor [%i] on colliding list with Class==0", Which );
        }
        if( Actor->Class != 0 )
        {
            if( IsOnStaticList && !Actor->bStaticActor )
            {
                debugf( LOG_Info, "Actor [%i] on static list with !bStaticActor", Which );
            }
            else if( !IsOnStaticList && Actor->bStaticActor )
            {
                debugf( LOG_Info, "Actor [%i] not on static list but has bStaticActor", Which );
            }
            if( IsOnDynamicList && Actor->bStaticActor )
            {
                debugf( LOG_Info, "Actor [%i] on dynamic list with bStaticActor", Which );
            }
            else if( !IsOnDynamicList && !Actor->bStaticActor )
            {
                debugf( LOG_Info, "Actor [%i] not on dynamic list but has !bStaticActor", Which );
            }
            if( IsOnCollidingList && !Actor->bCollideActors )
            {
                debugf( LOG_Info, "Actor [%i] on colliding list with !bCollideActors", Which );
            }
            else if( !IsOnCollidingList && Actor->bCollideActors )
            {
                debugf( LOG_Info, "Actor [%i] not on colliding list but has bCollideActors", Which );
            }
        }
    }
    UNGUARD("UActorList::CheckLists");
}

/*-----------------------------------------------------------------------------
	AActor implementation
-----------------------------------------------------------------------------*/

//
// Initialize all of the private, in-memory information used by UnrealServer.
// 
void AActor::InitServerInfo(void)
	{
	for (int i=0; i<FActorServerInfo::MAX_TOUCHING_ACTORS; i++)
		{
		ServerInfo.iTouchingActors[i] = INDEX_NONE;
		};
	};

//
// Compute the actor's view coordinate system.  Upon return:
//
// Coords->XAxis points in the direction that the actor perceives as 'right'.
// Coords->YAxis points in the direction that the actor perceives as 'down'.
// Coords->ZAxis points in the direction that the actor perceives as 'forward'.
//
void AActor::GetViewCoords(FCoords *Coords) const
	{
	*Coords = GMath.CameraViewCoords;
	Coords->DeTransformByRotation (ViewRot);
	};

//
// Compute the actor's rendering coordinate system.
//
void AActor::GetDrawCoords(FCoords *Coords) const
	{
	*Coords = GMath.CameraViewCoords;
	Coords->DeTransformByRotation (DrawRot);
	};

//
// Transform a world point into a point relative to the actor.  
// On return:
//
// LocalPoint.X indicates how far right the point is of the actor.
// LocalPoint.Y indicates how far the point is below the actor.
// LocalPoint.Z indicates how far ahead the point is of the actor.
//
void AActor::TransformPoint(FVector *LocalPoint, const FVector *WorldPoint)
	{
	FCoords Coords;
	GetViewCoords(&Coords);
	//
	*LocalPoint = *WorldPoint;
	LocalPoint->TransformVector(Coords);
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
