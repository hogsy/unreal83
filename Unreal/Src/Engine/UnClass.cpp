/*=============================================================================
	UnClass.cpp: Actor class functions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

//
// Temporary:
//
class AScript : public AActor
	{
	public:
	int Process(ILevel *Level, FName Message, void *Params)
		{
		//appError("AScript::Process");
		return 0;
		};
	};

//
// Globals for linktopic function:
//
extern int 	GNumResults;
extern int 	GCurResult;
UClass  	**GTempClassList       		= NULL;
WORD  		*GTempClassChildCount 		= NULL;

#define MAX_RES_RESULTS 4096

/*-----------------------------------------------------------------------------
	UClass implementation
-----------------------------------------------------------------------------*/

//
// Find the actor function for this class.  The actor function either resides
// in the actor DLL, or it's assumed to be the generic script-executor actor function.
//
void UClass::FindActorFunc(void)
	{
	void **Temp = (void **)GApp->GetProcAddress(GAME_DLL,Name,0);
	//
	if (Temp!=NULL) *(void **)&ActorFunc = *Temp;
	else ActorFunc = (ACTOR_CLASS_FUNC)&AScript::Process;
	//
	//bug ("Found actor function: %s",Class->Name);
	};

//
// Fill this class's properties with its parent properties.  This is performed
// when a new class is created or recompiled, so that inheretance works seamlessly.
//
void UClass::AddParentProperties (void)
	{
	GUARD;
	FClassProperty	*Property,*ParentProperty;
	BYTE			*Default,*ParentDefault;
	int				i;
	//
	if (ParentClass) 
		{
		Num	= ParentClass->Num;
		//
		if (Num >= Max) appErrorf ("AddParentProperties: %s full",Name);
		//
		for (i=0; i<Num; i++)
			{
			Property 		= &Element(i);
			ParentProperty	= &ParentClass->Element(i);
			*Property		= *ParentProperty;
			//
			Default  		= &((BYTE *)&DefaultActor)				[Property->PropertyOffset];
			ParentDefault   = &((BYTE *)&ParentClass->DefaultActor)	[Property->PropertyOffset];
			memcpy (Default,ParentDefault,ParentProperty->PropertySize);
			};
		DefaultActor.bTemplateClass=0;
		};
	UNGUARD("UClass::AddParentProperties");
	};

//
// Delete a class and all its child classes.
//
void UClass::Delete (void)
	{
	GUARD;
	UClass *Class;
	//
	Kill();
	FOR_ALL_TYPED_RES(Class,RES_Class,UClass)
		{
		if (Class->ParentClass==this) Class->ParentClass->Delete();
		}
	END_FOR_ALL_TYPED_RES;
	UNGUARD("UClass::Delete");
	};

//
// See if this class is a descendent of SomeParent.
// Returns 1 if true, 0 if false.
//
int UClass::IsKindOf(UClass *SomeParent)
	{
	UClass *Class = this;
	while (Class)
		{
		if (Class==SomeParent) return 1;
		Class = Class->ParentClass;
		};
	return 0;
	};

//
// Resource functions
//
void UClass::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UClass);
	Type->RecordSize = sizeof (FClassProperty);
	Type->Version    = 1;
	Type->TypeFlags  = RTF_ScriptReferencable;
	//
	// Make sure the resource manager always links UClass before UActorList and UScript:
	//
	strcpy (Type->Descr,"Class");
	UNGUARD("UClass::Register");
	};
void UClass::InitHeader(void)
	{
	GUARD;
	//
	// Init resource header to defaults:
	//
	ParentClass			= NULL;
	ScriptText			= NULL;
	Script				= NULL;
	//
	// Property information:
	//
	Num					= 0;
	Max					= MAX_CLASS_PROPERTIES;
	//
	// Information available at runtime only (not stored on disk):
	//
	mymemset(&DefaultActor,0,sizeof(AActor));
	UNGUARD("UClass::InitHeader");
	};
void UClass::InitData(void)
	{
	GUARD;
	Num = 0;
	UNGUARD("UClass::InitData");
	};
int UClass::QuerySize(void)
	{
	GUARD;
	return Max * sizeof (FClassProperty);
	UNGUARD("UClass::QuerySize");
	};
int UClass::QueryMinSize(void)
	{
	GUARD;
	return Num * sizeof (FClassProperty);
	UNGUARD("UClass::QueryMinSize");
	};
const char *UClass::Import(const char *Buffer, const char *BufferEnd,const char *FileType)
	{
	GUARD;
	char StrLine[256],Temp[256],TempName[NAME_SIZE];
	//bug ("Start import %s",Name);
	//
	DefaultActor.Class = this; // Required by ImportActorProperties
	//
	while (GetLINE(&Buffer,StrLine,256)==0)
		{
		const char *Str=&StrLine[0];
		if (GetCMD(&Str,"DECLARECLASS") && GetSTRING(Str,"NAME=",Temp,NAME_SIZE))
			{
			// Forward-declare a class, necessary because actor properties may refer to
			// classes which haven't been declared yet (such as InventoryClass):
			if (!new(Temp,FIND_Optional)UClass)
				{
				UClass *TempClass = new(Temp,CREATE_Replace)UClass;
				};
			}
		else if (GetBEGIN(&Str,"CLASS") && GetSTRING(Str,"NAME=",TempName,NAME_SIZE))
			{
			//bug ("Importing class %s",TempName);
			UClass *TempClass = new(TempName,CREATE_Replace)UClass((UClass*)-1);
			TempClass->AllocData();
			//
			Buffer = TempClass->Import(Buffer,BufferEnd,FileType);
			//
			if (!TempClass->DefaultActor.Class)
				{
				//bug ("Failed import, killing %s",Name);
				TempClass->Kill(); // Import failed
				};
			}
		else if (GetBEGIN(&Str,"TEXT"))
			{
			ScriptText = new(Name,CREATE_Replace)UTextBuffer;
			Buffer     = ScriptText->Import(Buffer,BufferEnd,FileType);
			CompileScript(this,0);
			//bug ("Script <%s> parent <%s>",Name,ParentClass?ParentClass->Name:"NONE");
			}
		else if (GetBEGIN(&Str,"DEFAULTPROPERTIES") && DefaultActor.Class)
			{
			//bug ("Getting defaults...");
			Buffer = ImportActorProperties(&DefaultActor,Buffer);
			}
		else if (GetEND(&Str,"CLASS")) break;
		};
	if (Num==0) // Import failed
		{
		//bug ("Empty import, killing script %s",Name);
		if (ScriptText) ScriptText->Kill();
		ScriptText = NULL;
		};
	//bug ("Finish importing %s",Name);
	return Buffer;
	UNGUARD("UClass::Import");
	};
char *UClass::Export(char *Buffer,const char *FileType,int Indent)
	{
	GUARD;
	static int RecursionDepth=0,i;
	//
	int FirstProp=0; if (ParentClass) FirstProp=ParentClass->Num;
	if (!stricmp(FileType,"H")) // Export as C++ header
		{
		if (RecursionDepth==0)
			{
			Buffer += sprintf(Buffer,
				"/*===========================================================================\r\n"
				"	C++ \"%s\" actor class definitions exported from UnrealEd\r\n"
				"===========================================================================*/\r\n"
				"#pragma pack (push,1) /* Actor class data must be unaligned */\r\n"
				"\r\n",
				Name);
			};
		// Text description
		Buffer += sprintf(Buffer,"///////////////////////////////////////////////////////\r\n// Actor class ");
		UClass *TempClass = this;
		while (TempClass)
			{
			Buffer += sprintf(Buffer,"A%s",TempClass->Name);
			TempClass = TempClass->ParentClass;
			if (TempClass) Buffer += sprintf(Buffer,":");
			};
		Buffer += sprintf(Buffer,"\r\n///////////////////////////////////////////////////////\r\n\r\n");
		// Enum definitions
		for (i=FirstProp; i<Num; i++)
			{
			if ((Element(i).PropertyType==CPT_Byte) && Element(i).Enum && !(Element(i).Enum->Flags & RF_TagImp))
				{
				Buffer = Element(i).Enum->Export(Buffer,FileType,Indent);
				Element(i).Enum->Flags |= RF_TagImp; // Don't export more than once
				};
			};
		// class CLASSNAME [: public PARENTCLASS]
		Buffer += sprintf(Buffer,"class A%s",Name);
		//
		if (ParentClass)	Buffer += sprintf(Buffer," : public A%s",ParentClass->Name);
		else				Buffer += sprintf(Buffer," : public AActorBase");
		//
		Buffer += sprintf(Buffer," {\r\npublic:\r\n");
		// All properties:
		for (i=FirstProp; i<Num; i++)
			{
			Buffer += sprintf(Buffer,spc(Indent+4));
			Buffer  = Element(i).ExportH(Buffer);
			Buffer += sprintf(Buffer,"\r\n");
			};
		if (Intrinsic)
			{
			// C++ AI class function declaration:
			Buffer += sprintf(Buffer,"    int Process(ILevel *Level, FName Message, void *Params);\r\n");
			};
		// EndClass:
		Buffer += sprintf(Buffer,"};\r\n");
		Buffer += sprintf(Buffer,"AUTOREGISTER_CLASS(A%s);\r\n\r\n",Name);
		//
		}
	else if (!stricmp(FileType,"TCX")) // Export as actor class text
		{
		if (!ScriptText) appError ("Null ScriptText");
		if (RecursionDepth==0)
			{
			Buffer += sprintf(Buffer,
				"'\r\n"
				"' Actor classes exported from UnrealEd\r\n"
				"'\r\n"
				);
			// Class declarations:
			UClass *TempClass;
			FOR_ALL_TYPED_RES(TempClass,RES_Class,UClass)
				{
				if ((TempClass->Flags & RF_TagExp) && TempClass->IsKindOf(this))
					Buffer += sprintf(Buffer,"DeclareClass Name=%s\r\n",TempClass->Name);
				}
			END_FOR_ALL_TYPED_RES;
			Buffer += sprintf(Buffer,"\r\n");
			};
		const char TEXT_SEPARATOR[] =
			"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
			"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n";
		// Class CLASSNAME Expands PARENTNAME [Intrinsic]:
		Buffer += sprintf(Buffer,"Begin Class Name=%s\r\n",Name);
		Buffer += sprintf(Buffer,"   Begin Text\r\n");
		Buffer += sprintf(Buffer,TEXT_SEPARATOR);
		Buffer  = ScriptText->Export(Buffer,FileType,Indent);
		Buffer += sprintf(Buffer,"\r\n");
		Buffer += sprintf(Buffer,TEXT_SEPARATOR);
		Buffer += sprintf(Buffer,"   End Text\r\n");
		// Default properties:
		Buffer += sprintf(Buffer,"   Begin DefaultProperties\r\n");
		if ((RecursionDepth==0) || (ParentClass==NULL))
			{
			// Export all default properties
			Buffer += ExportActor (&DefaultActor,Buffer,NAME_NONE,Indent+5,0,0,NULL,1,-1,NAME_NONE);
			}
		else
			{
			// Export only default properties that differ from parent's
			Buffer += ExportActor (&DefaultActor,Buffer,NAME_NONE,Indent+5,0,0,&ParentClass->DefaultActor,1,-1,NAME_NONE);
			};
		Buffer += sprintf(Buffer,"   End DefaultProperties\r\n");
		// EndClass:
		Buffer += sprintf(Buffer,"End Class\r\n\r\n");
		};
	//
	// Export all child classes that are tagged for export:
	//
	UClass *ChildClass;
	FOR_ALL_TYPED_RES(ChildClass,RES_Class,UClass)
		{
		if ((ChildClass->ParentClass==this) && (ChildClass->Flags & RF_TagExp))
			{
			RecursionDepth++;
			Buffer = ChildClass->Export(Buffer,FileType,Indent);
			RecursionDepth--;
			};
		};
	END_FOR_ALL_TYPED_RES;
	//
	if ((!stricmp(FileType,"H")) && (RecursionDepth==0)) // Finish C++ header
		{
		Buffer += sprintf(Buffer,"#pragma pack (pop) /* Restore alignment to previous setting */\r\n");
		};
	return Buffer;
	//
	UNGUARD("UClass::Export");
	};
void UClass::QueryHeaderReferences(FResourceCallback &Callback)
	{
	GUARD;
	//
	// Main references:
	//
	Callback.Resource (this,(UResource **)&ParentClass,0);
	Callback.Resource (this,(UResource **)&ScriptText ,0);
	Callback.Resource (this,(UResource **)&Script     ,0);
	//
	// References in the default actor:
	//
	DefaultActor.QueryReferences(this,Callback,0);
	//
	UNGUARD_BEGIN;
	UNGUARD_MSGF("UClass::QueryHeaderReferences (%s,%i)",Name,(int)this);
	UNGUARD_END;
	};
void UClass::QueryDataReferences(FResourceCallback &Callback)
	{
	GUARD;
	//
	// References in the class property list:
	//
	for (int i=0; i<Num; i++)
		{
		FClassProperty *Property = &Element(i);
		//
		Callback.Name (this,&Property->PropertyName,0);
		Callback.Name (this,&Property->PropertyCategory,0);
		//
		if (Property->PropertyType==CPT_Byte) // Byte properties may reference an enum resource
			{
			Callback.Resource(this,(UResource **)&Property->Enum,0);
			};
		};
	UNGUARD("UClass::QueryDataReferences");
	};
void UClass::PostLoad(void)
	{
	GUARD;
	FindActorFunc();
	UNGUARD("UActorList::PostLoad");
	};
AUTOREGISTER_RESOURCE(RES_Class,UClass,0xB2D90853,0xCCD211cf,0x91360000,0xC028B992);

/*-----------------------------------------------------------------------------
	AActor functions
-----------------------------------------------------------------------------*/

//
// Call the callback with all names and resources referenced by the actor.  This
// is used both by UClass's query functions and UActorList's query functions, since
// they both contain actors.
//
void AActor::QueryReferences(UResource *Owner,FResourceCallback &Callback,DWORD ContextFlags)
	{
	BYTE    *RawActor = (BYTE *)this;
	UClass  *TrueClass;
	//
	GUARD;
	if (Owner->Type==RES_ActorList) TrueClass = (UClass *)Callback.GetActualResource(Owner,Class);
	else							TrueClass = (UClass *)Owner;
	UNGUARD("AActor::QueryReferences setup");
	//
	// Return all references according to class property list.
	// Relies on the class's data being in place, but can not expect that the
	// class's resources and names are linked.
	//
	GUARD;
	for (int i=0; i<TrueClass->Num; i++)
		{
		FClassProperty	*Property	= &TrueClass->Element(i);
		int				n			= Property->PropertyArrayDim ? Property->PropertyArrayDim : 1;
		BYTE			*Value		= &RawActor[Property->PropertyOffset];
		//
		if ((Property->PropertyType == CPT_Resource) && !(Property->PropertyFlags & CPF_NoSaveResource))
			{
			for (int j=0; j<n; j++)
				{
				Callback.Resource(Owner,(UResource **)Value,ContextFlags);
				Value += sizeof(UResource *);
				};
			}
		else if (Property->PropertyType == CPT_Name)
			{
			for (int j=0; j<n; j++)
				{
				Callback.Name(Owner,(FName *)Value,ContextFlags);
				Value += sizeof(FName);
				};
			};
		};
	UNGUARD_BEGIN;
	UNGUARD_MSGF("AActor::QueryReferences: Ptr=%i, Name=%s ",(int)TrueClass,TrueClass->Name);
	UNGUARD_END;
	};

/*-----------------------------------------------------------------------------
	Global functions
-----------------------------------------------------------------------------*/

//
// Initialize all root properties of an actor.
//
UNREAL_API void InitActor(AActor *Actor,UClass *Class)
	{
	mymemset(Actor,0,sizeof(AActor));
	//
	Actor->Class		= Class;
	Actor->Camera		= NULL;
	Actor->Texture		= NULL;
	Actor->MeshMap		= NULL;
	Actor->Brush		= NULL;
	Actor->Name			= NAME_NONE;
	Actor->EventName	= NAME_NONE;
	Actor->iParent		= INDEX_NONE;
	Actor->iTarget		= INDEX_NONE;
	Actor->iWeapon		= INDEX_NONE;
	Actor->iInventory	= INDEX_NONE;
	Actor->BlitType		= BT_Normal;
	Actor->DrawType		= DT_Sprite;
	Actor->DrawScale	= 1.0;
	Actor->Mass			= 200.0;
	Actor->DefaultEdCategory = NAME_NONE;
	};

//
// Create a new UClass given its parent.
//
UNREAL_API UClass::UClass (UClass *NewParentClass)
	{
	GUARD;
	//
	if (NewParentClass == (UClass*)-1) NewParentClass=NULL;
	//
	// Allocate the resource and its data:
	//
	Max = MAX_CLASS_PROPERTIES;
	AllocData(1);
	//
	// Resources:
	//
	ParentClass		= NewParentClass;
	ScriptText		= NULL;
	Script			= NULL;
	//
	// Copy information structure from parent:
	//
	if (ParentClass)	AddParentProperties();
	else				InitActor(&DefaultActor,this);
	//
	DefaultActor.Class = this;
	DefaultActor.bTemplateClass = 0;
	//
	// Find the actor function associated with this new class:
	//
	FindActorFunc();
	//
	//	Done creating class
	//
	UNGUARD("UClass::UClass(UClass *)");
	};

//
// Init actor classes and set up intrinsic classes (camera, light, etc)
//
void FGlobalClasses::Init (void)
	{
	GUARD;
	//
	// Make hardcoded names for all property types:
	//
	FName Name;
	Name.AddHardcoded(CPT_None,		"None");
	Name.AddHardcoded(CPT_Byte,		"Byte");
	Name.AddHardcoded(CPT_Integer,	"Integer");
	Name.AddHardcoded(CPT_Boolean,	"Boolean");
	Name.AddHardcoded(CPT_Real,		"Real");
	Name.AddHardcoded(CPT_Actor,	"Actor");
	Name.AddHardcoded(CPT_Resource,	"Resource");
	Name.AddHardcoded(CPT_Name,		"Name");
	Name.AddHardcoded(CPT_String,	"String");
	Name.AddHardcoded(CPT_Vector,	"Vector");
	Name.AddHardcoded(CPT_Rotation,	"Rotation");
	//
	// Create the class list:
	//
	mymemset (this,0,sizeof(FGlobalClasses));
	AddClass    = NULL;
	//
	IntrinsicClasses = new("IntrinsicClasses",CREATE_Unique)TArray<UClass>(1024);
	GRes.Root->Add(IntrinsicClasses);
	//
	// Set all base classes to NULL:
	//
	Root		= NULL;
	TextureRes	= NULL;
	SoundRes	= NULL;
	AmbientRes	= NULL;
	LevelRes	= NULL;
	//
	debugf(LOG_Info,"Actor classes initialized, size=%i",sizeof(AActor));
	//
	UNGUARD("FGlobalClasses::Init");
	};

//
// Associate classes with related resources.
//
void FGlobalClasses::Associate(void)
	{
	GUARD;
	//
	// Import classes for UnrealEd
	//
	if (GRes.AddFile(DEFAULT_CLASS_FNAME)==FILE_NONE)
		{
		if (!GEditor) appError("Can't find actor classes");
		//
		GUARD;
		GApp->BeginSlowTask	("Building Unreal.ucx",1,0);
		GEditor->Exec		("MACRO PLAY NAME=Classes FILE=" CLASS_BOOTSTRAP_FNAME);
		GApp->EndSlowTask	();
		UNGUARD("Parsing " CLASS_BOOTSTRAP_FNAME);
		};
	Root			= new ("Root",			FIND_Existing)UClass; IntrinsicClasses->Add(Root);
	Camera			= new ("Camera",		FIND_Existing)UClass; IntrinsicClasses->Add(Camera);
	Light			= new ("Light",			FIND_Existing)UClass; IntrinsicClasses->Add(Light);
	Pawn			= new ("Pawn",			FIND_Existing)UClass; IntrinsicClasses->Add(Pawn);
	Player			= new ("Woman",			FIND_Existing)UClass; IntrinsicClasses->Add(Player);
	PlayerStart		= new ("PlayerStart",	FIND_Existing)UClass; IntrinsicClasses->Add(PlayerStart);
	Inventory		= new ("Inventory",		FIND_Existing)UClass; IntrinsicClasses->Add(Inventory);
	Weapon			= new ("Weapon",		FIND_Existing)UClass; IntrinsicClasses->Add(Weapon);
	Pickup			= new ("Pickup",		FIND_Existing)UClass; IntrinsicClasses->Add(Pickup);
	Mover			= new ("Mover",			FIND_Existing)UClass; IntrinsicClasses->Add(Mover);
	ZoneDescriptor	= new ("ZoneDescriptor",FIND_Existing)UClass; IntrinsicClasses->Add(ZoneDescriptor);
	LevelDescriptor	= new ("LevelDescriptor",FIND_Existing)UClass;IntrinsicClasses->Add(LevelDescriptor);
	//
	Ammo        = new ("Ammo",				FIND_Existing)UClass; IntrinsicClasses->Add(Ammo);
	PowerUp     = new ("PowerUp",			FIND_Existing)UClass; IntrinsicClasses->Add(PowerUp);
	Projectile  = new ("Projectile",		FIND_Existing)UClass; IntrinsicClasses->Add(Projectile);
	//
	TextureRes	= NULL;
	SoundRes	= NULL;
	AmbientRes	= NULL;
	LevelRes	= NULL;
	//
	UNGUARD("FGlobalClasses::Associate");
	};

//
// Shut down all actor classes
//
void FGlobalClasses::Exit (void)
	{
	GUARD;
	//
	GRes.Root->Delete(IntrinsicClasses);
	IntrinsicClasses->Kill();
	//
	UNGUARD("FGlobalClasses::Exit");
	};

/*-----------------------------------------------------------------------------
	Link topic function
-----------------------------------------------------------------------------*/

void AddToClassList (UClass *ThisClass, int CountChildren)
	{
	GUARD;
	UClass *Class;
	if (GNumResults < MAX_RES_RESULTS)
		{
		GTempClassList       [GNumResults] = ThisClass;
		GTempClassChildCount [GNumResults] = 0;
		//
		if (CountChildren)
			{
			FOR_ALL_TYPED_RES(Class,RES_Class,UClass)
				{
				if (Class->ParentClass==ThisClass) GTempClassChildCount [GNumResults]++;
				};
			END_FOR_ALL_TYPED_RES;
			};
		GNumResults++;
		};
	UNGUARD("AddToClassList");
	};

//
// Query a list of classes.  Call with resource or NULL=All.
//
void UNREAL_API classQueryForLink (UResource *Res)
	{
	UClass *Class;
	//
	GUARD;
	if (GTempClassList==NULL)
		{
		GTempClassList       = (UClass **)appMalloc(MAX_RES_RESULTS * sizeof (UResource *),"ClassQuery1"); // Must free !!
		GTempClassChildCount = (WORD    *)appMalloc(MAX_RES_RESULTS * sizeof (WORD),"ClassQuery2"); // Must free !!
		};
	GCurResult  = 0;
	GNumResults = 0;
	//
	FOR_ALL_TYPED_RES(Class,RES_Class,UClass)
		{
		if ((Class->ParentClass==Res)||(!Res)) AddToClassList(Class,Res!=NULL);
		}
	END_FOR_ALL_TYPED_RES;
	//
	UNGUARD("classQueryForLink");
	};

AUTOREGISTER_TOPIC("Class",ClassTopicHandler);
void ClassTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	GUARD;
	UClass	*Class;
	int		Children;
	//
	if (!stricmp(Item,"QueryRes")) // Query results
		{
		if (GCurResult < GNumResults)
			{
			Class    = GTempClassList       [GCurResult];
			Children = GTempClassChildCount [GCurResult];
			sprintf(Data,"%s%s|%s",Class->Intrinsic?"":"*", Class->Name, (Children==0)?"X":"C");
			GCurResult++;
			};
		}
	else if (!_strnicmp(Item,"EXISTS",6))
		{
		if (GetUClass(Item,"NAME=",&Class)) strcpy (Data,"1");
		else strcpy (Data,"0");
		};
	UNGUARD("ClassTopicHandler::Get");
	};
void ClassTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
	{
	GUARD;
	UNGUARD("ClassTopicHandler::Set");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
