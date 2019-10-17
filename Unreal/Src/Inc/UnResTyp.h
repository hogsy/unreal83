/*=============================================================================
	UnResTyp.h: Unreal Resource data types

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNRESTYP // Prevent header from being included multiple times
#define _INC_UNRESTYP

/*----------------------------------------------------------------------------
	Definitions
----------------------------------------------------------------------------*/

//
// Forward declarations
//
class UResource;
class FResourceType;
class FResourceCallback;
enum  EResourceType;

/*----------------------------------------------------------------------------
	Names
----------------------------------------------------------------------------*/

//
// Public name, available to the world.  Names are stored as WORD indices
// into the GRes.NameArray table and every name in Unreal is stored once
// and only once in that table.  Names are case-insensitive.
//
class UNREAL_API FName 
	{
	public:
	WORD	Index;
	//
	int		IsNone			(void) {return Index==INDEX_NONE;};
	int		Find			(const char *Name);
	void	Add				(const char *Name);
	void	AddHardcoded	(WORD NewIndex,const char *Name);
	char	*Name			(void);
	//
	int		operator==		(const FName &Other) {return Index==Other.Index;};
	int		operator!=		(const FName &Other) {return Index!=Other.Index;};
	//
	inline	FName(void){};
	inline	FName(int i){Index=i;};
	inline	FName(enum EActorMessage M){Index=M;};
	};

/*-----------------------------------------------------------------------------
	Finding/creating/importing resources via operator new
-----------------------------------------------------------------------------*/

//
// Enumerated types recognized by UResource-derived operator new() functions for 
// creating, finding, and importing resources easily.  See operator new()
//

enum EFindResource 
	{
	FIND_Existing	= 0, // Find an existing, named resource. No-fail; if it doesn't exist, calls GApp->Error.
	FIND_Optional	= 1	 // See if a named resource exists; either returns its pointer, or NULL.
	};

enum ECreateResource
	{
	CREATE_Unique	= 0, // Create a new, unique resource. Calls GApp->Error if the resource already exists.
	CREATE_Replace	= 1	 // Create a new resource.  If exists, kills and replaces but retains the same index and pointer.
	};

enum EImportResource
	{
	IMPORT_Unique	= 0, // Import a resource from a file.  Calls GApp->Error if the resource already exists.
	IMPORT_Replace	= 1  // Import a new resource.  If exists, kills and replaces but retains the same index and pointer.
	};

/*-----------------------------------------------------------------------------
	FLockType
-----------------------------------------------------------------------------*/

//
// Lock types for locking resources
//
enum FLockType
	{
	LOCK_None			= 0, // Not locked
	LOCK_Read			= 1, // Lock for reading only (can't change anything)
	LOCK_Trans			= 2, // Lock for transactional writing (can be transaction tracked)
	LOCK_NoTrans		= 3, // Lock for non-transaction-friendly writing (kills transaction buffer)
	};

/*----------------------------------------------------------------------------
	Resource callback functions
----------------------------------------------------------------------------*/

//
// Callbacks to FResourceCallback classes must use these flags to 
// indicate what to do with the resource in all possible situations.
//
enum EResourceContextFlags
	{
	RC_LoadEdit				= 0x0001,	// Load for editing
	RC_LoadServerDynamic	= 0x0002,	// Load for server, data is dynamic
	RC_LoadServerStatic		= 0x0004,	// Load for server, no linking required
	RC_LoadClientDynamic	= 0x0008,	// Load for client, data is dynamic
	RC_LoadClientStatic		= 0x0010,	// Load for client, no linking required
	RC_SaveNull				= 0x0020,	// Save it NULL'ed out
	};

//
// Resource callback class. Meant to be subclassed for specific
// resource callback instances.
//
class FResourceCallback
	{
	public:
	virtual void Resource (UResource *ParentRes, UResource **RefRes, DWORD ContextFlags) {};
	virtual void Name	  (UResource *ParentRes, FName *ResName,   DWORD ContextFlags) {};
	virtual UResource *GetActualResource(UResource *Owner, UResource *PossiblyDelinkedRes);
	};

/*----------------------------------------------------------------------------
	Items stored in resource files
----------------------------------------------------------------------------*/

//
// A global name, as stored on-disk in a resource file.
//
class FNameTableEntry // Private name table entry, tracked by Unres.
	{
	public:
	char	Name[NAME_SIZE];	// Name
	DWORD	Flags;				// RF_TAGIMPORT or RF_TAGEXPORT
	};

//
// A resource name, as stored on disk.
//
class FNameFileEntry 
	{
	public:
	char				Name[NAME_SIZE];	// Name of resource
	DWORD				ResVersionNumber;	// Version number
	enum EResourceType	ResType;	  		// Type of resource
	DWORD				ResSize;			// Size of resource header
	};

//
// Markers for informational purposes only:
//
#define NAMES_MARK       "[REFERENCED NAMES]"
#define IMPORT_MARK      "[IMPORTED RESOURCES]"
#define EXPORT_MARK      "[EXPORTED RESOURCES]"
#define DATA_MARK        "[EXPORTED RESOURCE DATA]"
#define HEADER_MARK      "[EXPORTED RESOURCE HEADERS]"
#define TABLE_MARK       "[EXPORTED RESOURCE TABLE]"
#define TRAILER_MARK     "[SUMMARY]"

//
// Resource file trailer, stored at end of resource files
//
class FResourceFileTrailer 
	{
	public:
	//
	DWORD	FileFormatVersion;	// Resource file format version
	DWORD	NumExports;			// Number of resources exported
	DWORD	NumImports;			// Number of resources imported
	DWORD	NumNames;			// Number of names referenced
	//
	DWORD	NamesOffset;		// Offset to name symbol table              (UNNAME array)
	DWORD	ImportsOffset;		// Offset to resource imports               (UNNAME array)
	DWORD	ExportsOffset;		// Offset to resource exports               (UNNAME array)
	DWORD	HeadersOffset;		// Offset to resource headers               (byte stream)
	DWORD	HeadersLength;		// Total length of header buffer
	//
	BYTE	Pad  		[16];	// For future expansion
	//
	CHAR	DeltaFname	[80];	// File this is a delta from, or blank if this is a complete file
	CHAR	Tag			[32];	// Should be RES_FILE_TAG, "Unreal Resource\x1a"
	};

//
// Information about a resource file that's loaded into memory:
//
class FResourceFileInfo // Information about a resource file (in memory only)
	{
	public:
	FILE		*File;			// File pointer or NULL if not open
	CHAR		Fname[80];		// Full filename of resource file, blank=available
	DWORD		NumResources;	// Number of resources stored in file
	DWORD		NumNames;		// Number of names stored in file
	FName		*NameMap;		// Maps file's FNames to memory name-table FNames
	UResource	**ResMap;		// Maps file's resources to memory resource-table resources
	};

/*----------------------------------------------------------------------------
	FResourceType
----------------------------------------------------------------------------*/

//
// Flags describing a resource type:
//
enum EResourceTypeFlags
	{
	RTF_ScriptReferencable	= 0x0001,	// Scripts can reference this resource type
	};

//
// Information about a type of resource.  One entry is stored in
// FGlobalResourceManager for each type of resource that registered
// itself via the AUTOREGISTER_RESOURCE macro.  The index of the
// resource type in the Types array is the resource's enumeration
// value as declared in EResourceType.
//
class FResourceType 
	{
	public:
	//
	void		*VTablePtr;			// Pointer to virtual function table
	DWORD		HeaderSize; 		// Size of resource header
	DWORD		RecordSize;			// Size of each individual record, or -1 if not applicable
	DWORD		Version;			// Current version of resource handler
	DWORD		TypeFlags;			// Resource type flags (RTF_)
	DWORD		GUID[4];			// Globally unique identifier
	char		Descr[NAME_SIZE];	// Description of type -- just a string, not a global name
	};

/*----------------------------------------------------------------------------
	FGlobalResourceManager
----------------------------------------------------------------------------*/

//
// The global resource manager.  This tracks all information about all
// active resources, names, types, and files.
//
class UNREAL_API FGlobalResourceManager // Global resource table (in memory only)
	{
	public:
	//
	// Member variables:
	//
	DWORD				NumAutoReg;		// Must be first member of class, to be initialized to zero
	DWORD				Counter;	 	// Counter for caching
	DWORD				MaxRes;	 		// Maximum resources in table
	DWORD				MaxNames; 		// Maximum names in global name table
	DWORD				MaxFiles; 		// Maximum resource files
	DWORD				MaxTypes; 		// Maximum resource types
	//
	class UArray		*Root;	 		// Root array, for tracking active resources
	class UArray		*UnclaimedRes;	// Unclaimed resource
	class UEnum			*UnclaimedNames;// Unclaimed name list
	//
	UResource 			**ResArray;	 	// Global table of all resources
	FNameTableEntry		*Names;   		// Global table of names
	FResourceFileInfo	*Files;	 		// Global table of all loaded resource files
	FResourceType		*Types;   		// Global table of all resource types
	//
	enum EResourceType	AutoTypes[256];
	UResource			*AutoRes [256];
	DWORD				AutoA[256],AutoB[256],AutoC[256],AutoD[256];
	//
	// Public functions:
	//
	void Init(void);
	void Exit(void);
	int	 Exec(const char *Cmd,FOutputDevice *Out=GApp);
	//
	/////////////////////////////////////
	//	Adding & Saving resource files //
	/////////////////////////////////////
	//
	INDEX AddFile					(const char *Fname);
	//
	void Save						(UResource *Res, const char *Fname, BYTE DeltaFile);
	void SaveDependent				(UResource *Res, const char *Fname, BYTE DeltaFile);
	void SaveTagged					(const char *Fname, BYTE DeltaFile);
	void SaveDependentTagged		(const char *Fname, BYTE DeltaFile);
	void SaveTagAllDependents		(void);
	//
	/////////////////////
	//	Resource types //
	/////////////////////
	//
	enum EResourceType LookupType	(const char *Name);
	void PreRegisterType			(enum EResourceType ResType, UResource *Template,DWORD A,DWORD B,DWORD C,DWORD D);
	//
	/////////////
	// Tagging //
	/////////////
	//
	void TagUnused					(UResource *Res);
	void UntagAll					(void);
	int  TagAllReferencingResource	(UResource *Res,  enum EResourceType ResType);
	int  TagAllReferencingName		(FName Name, enum EResourceType ResType);
	void TagReferencingTagged		(enum EResourceType ResType);
	//
	///////////////////////
	// Cleanup functions //
	///////////////////////
	//
	void FindUnclaimed				(void);	// Add unclaimed resources to "Unclaimed" array
	void KillUnclaimed				(void); // Kill unused resources and names
	void DebugDumpUnclaimed			(void); // Display unused resources and names
	void Purge						(int DebugDump); // Purge all unreferenced resources
	//
	/////////////////////////////
	// Resource name functions //
	/////////////////////////////
	//
	char *CombineName				(char *Result, const char *Name, const char *TypeChar,int Num);
	char *MakeUniqueName			(char *Result, char *BaseName, char *Append, EResourceType ResType);
	//
	///////////////////////////////////////
	// Internals: Only used in UnRes.cpp //
	///////////////////////////////////////
	//
	UResource *PrivateAlloc 		(const char *Name, enum EResourceType ResType, int Replace, DWORD SetFlags=0);
	UResource *PrivateLookup		(const char *Name,enum EResourceType ResType,enum EFindResource FindType);
	//
	private:
	UResource *FindResource			(const char *Name,EResourceType ResType,RESINDEX *AvailableIndex);
	void RegisterType				(enum EResourceType ResType, UResource *Template,DWORD A,DWORD B,DWORD C,DWORD D);
	void InitResource				(UResource *Res,EResourceType ResType,RESINDEX Index,const char *Name);
	};

/*-----------------------------------------------------------------------------
	Autoregistration macro
-----------------------------------------------------------------------------*/

//
// Register a resource with Unreal's global resource manager (GRes).  This
// macro should be used exactly once at global scope for every unique resource
// type that the resource manager must manage.
//
// The macro creates a bogus global variable and assigns it a value returned from
// the resource class's EResourceType constructor, which has the effect of
// registering the resource type primordially, before main() has been called.
//
#define AUTOREGISTER_RESOURCE(RESTYPE,CLASSNAME,A,B,C,D) \
	static CLASSNAME autoregister##CLASSNAME(RESTYPE,A,B,C,D);

/*-----------------------------------------------------------------------------
	UResource
-----------------------------------------------------------------------------*/

//
// Flags associated with each resource in the global resource table.
//
enum EResourceFlags
	{
	RF_Modified			= 0x0001, // Was modified since last load
	RF_Unused			= 0x0002, // Unused, can be freed in next call to resCleanup
	RF_TagChild			= 0x0004, // Is child of unused resource (used in resCleanup only)
	RF_TagImp			= 0x0008, // Temporary import tag in load/save
	RF_TagExp			= 0x0010, // Temporary export tag in load/save
	RF_NoFree			= 0x0020, // Don't free this resource (custom memory allocation)
	RF_TransData		= 0x0080, // Used by transaction tracking system to track changed data
	RF_TransHeader		= 0x0100, // Used by transaction tracking system to track changed headers
	RF_NoReplace		= 0x0200, // Don't replace when loading resource files
	RF_64KAlign			= 0x1000, // Resource should be aligned on 64K boundary
	RF_Temp1			= 0x2000, // Temporary flag for user routines
	RF_Unlinked			= 0x4000, // During load/save, indicates that resources/names are unlinked
	RF_SaveMask			= RF_64KAlign, // Flags that should be saved in resource files on disk
	};

//
// The parent class of all resources.
//
// The parent class of all Unreal resources.  See the comments by
// each member function definition in UnRes.cpp for an explanation of what
// the member function does.
//
class UNREAL_API UResource : public FUnknown
	{
	//
	// Member variables:
	//
	public:
	EResourceType	Type;			// Resource type (from EResourceType)
	char			Name[NAME_SIZE];// Name of resource, blank=unused
	void			*Data;			// Pointer to data in memory, not meaningful when stored on disk
	DWORD			Index;			// Index of resource into FGlobalResourceManager's ResArray table
	DWORD			VersionNumber;	// Version number, specific to type of resource
	DWORD			FileNum;		// File it came from or FILE_NONE if new
	DWORD			Flags;			// Private EResourceFlags (RF_*) used by resource manager
	DWORD 			FileDataOffset;	// Data offset in file, valid only on disk, not meaningful in memory
	DWORD 			FileDataSize;	// Size of data stored in file, not meaningful in memory
	DWORD			FileCRC;		// CRC32 value of header+data as stored in file, not meaningful in file
	DWORD			Unused1;		// Pad
	//
	// Standard resource functions that are and not overriden by child classes:
	//
	void Kill(void);
	inline char *GetTypeName(void) {return GRes.Types[Type].Descr;};
	inline void Modify(void) {Flags|=RF_Modified;};
	inline void SetFlags(DWORD NewFlags) {Flags|=NewFlags;};
	inline void ClearFlags(DWORD NewFlags) {Flags&=~NewFlags;};
	inline void *GetData(void) {return Data;};
	DWORD HeaderCRC(void);
	DWORD DataCRC(void);
	DWORD FullCRC(void);
	//
	// Standard functions which must be overriden by all child classes:
	//
	virtual void Register				(FResourceType *Type)=0;
	//
	// Standard functions which may be overridden by all child classes to
	// provide whatever functionality is needed by each resource type:
	//
	virtual void InitHeader				(void);
	virtual void InitData				(void);
	virtual int  QuerySize				(void);
	virtual int  QueryMinSize			(void);
	virtual const char *Import			(const char *Buffer, const char *BufferEnd,const char *FileType);
	virtual char *Export				(char *Buffer,const char *FileType,int Indent);
	virtual void FreeData				(void);
	virtual void PostLoad				(void);
	virtual void PreSave				(void);
	virtual void PreKill				(void);
	virtual void QueryHeaderReferences	(FResourceCallback &Callback);
	virtual void QueryDataReferences	(FResourceCallback &Callback);
	//
	void		Realloc					(void);
	int			ExportToFile			(char *Fname);
	int			ImportFromFile			(char *Fname);
	void		UnloadData				(void);
	void		*AllocData				(int Init);
	};

/*-----------------------------------------------------------------------------
	UDatabase
-----------------------------------------------------------------------------*/

//
// The parent class of all database-oriented resources.
//
// This provides all of the basic functionality required for resource types
// which are arrays of simple, constant-length records.  These resource types
// have the capability of being transaction-tracked for Undo/Redo features.
// The Register() member function of each UDatabase-derived resource class must 
// set the resource type's RecordSize member to the size of one fixed-length record.
//
class UNREAL_API UDatabase : public UResource
	{
	public:
	//
	INT		Num; // Number of active records in the database
	INT		Max; // Maximum number of records the database can hold
	//
	// Standard functions which may be overridden by all child classes to
	// provide whatever functionality is needed by each resource type:
	//
	virtual int  QuerySize				(void);
	virtual int  QueryMinSize			(void);
	virtual void InitHeader				(void);
	virtual void InitData				(void);
	};

/*-----------------------------------------------------------------------------
	RESOURCE_CLASS & RESOURCE_DB_CLASS macros
-----------------------------------------------------------------------------*/

//
// Macros for creating resource classes.  These would be implemented as C++ templates
// were it not for various VC++ 4.0 problems in dealing with templates imported from DLL's.
//

//
// Macro to create a UResource-derived resource class.  Includes automatic support for
// many commonly-used member functions.
//
#define RESOURCE_CLASS(TClass,TDataType,TResEnum)\
	public:\
	/* Get the resource's type-safe data pointer */\
	inline TDataType *GetData(void)\
		{\
		return (TDataType *)Data;\
		};\
	/* Safely copy all header properties from one resource to another, without screwing up indices */\
	inline void CopyHeaderFrom(TClass *Source)\
		{\
		memcpy((UResource *)this+1,(UResource *)Source+1,sizeof(TClass)-sizeof(UResource));\
		};\
	/* Allocate the resource's data.  Assumes that the header is valid and QuerySize() will return the proper size*/\
	inline TDataType *AllocData(int Init=1)\
		{\
		return (TDataType *)UResource::AllocData(Init);\
		};\
	/* Destroy the resource on exception */\
	inline void operator delete(void *Mem,const char *Name,ECreateResource Create,DWORD SetFlags) {}\
	inline void operator delete(void *Mem,const char *Name,char *Fname,EImportResource ImportType) {}\
	inline void operator delete(void *Mem,const char *Name,EFindResource FindType) {}\
	/* Create a new resource of this type; see ECreateResource */\
	void *operator new(size_t Size,const char *Name,ECreateResource Create,DWORD SetFlags=0)\
		{\
		return GRes.PrivateAlloc(Name,TResEnum,Create==CREATE_Replace,SetFlags);\
		};\
	/* Try to import a resource from a file; returns pointer if success, NULL if failure */\
	void *operator new(size_t Size,const char *Name,char *Fname,EImportResource ImportType)\
		{\
		TClass *Temp = new(Name,(ECreateResource)ImportType)TClass;\
		if (Temp->ImportFromFile(Fname)) return Temp;\
		Temp->Kill(); return NULL;\
		};\
	/* Try to find an existing resource */\
	void *operator new(size_t Size,const char *Name,EFindResource FindType)\
		{\
		return GRes.PrivateLookup(Name,TResEnum,FindType);\
		};\
	/* Try to parse the resource's name */\
	friend int Get##TClass(const char *Stream, const char *Match, TClass **Res)\
		{\
		return GetRES(Stream,Match,TResEnum,(UResource **)Res);\
		};\
	/* Default constructor; must do nothing */\
	TClass(void) {};\
	/* Constructor used solely by AUTOREGISTER_RESOURCE macro */\
	TClass(EResourceType Type,DWORD A,DWORD B,DWORD C,DWORD D){GRes.PreRegisterType(Type,this,A,B,C,D);};\

//
// Macro to create a UDatabase-derived resource class.  Includes automatic support for
// many commonly-used member functions.
//
#define RESOURCE_DB_CLASS(TClass,TDataType,TResEnum)\
	public:\
	/* Get the resource's type-safe data pointer */\
	inline TDataType *GetData(void)\
		{\
		return (TDataType *)Data;\
		};\
	/* Safely copy all header properties from one resource to another, without screwing up indices */\
	inline void CopyHeaderFrom(TClass *Source)\
		{\
		memcpy((UResource *)this+1,(UResource *)Source+1,sizeof(TClass)-sizeof(UResource));\
		};\
	/* Allocate the resource's data.  Assumes that the header is valid and QuerySize() will return the proper size*/\
	inline TDataType *AllocData(int Init=1)\
		{\
		return (TDataType *)UResource::AllocData(Init);\
		};\
	/* Destroy the resource on exception */\
	inline void operator delete(void *Mem,const char *Name,ECreateResource Create,DWORD SetFlags) {}\
	inline void operator delete(void *Mem,const char *Name,char *Fname,EImportResource ImportType) {}\
	inline void operator delete(void *Mem,const char *Name,EFindResource FindType) {}\
	/* Create a new resource of this type; see ECreateResource */\
	void *operator new(size_t Size,const char *Name,ECreateResource Create,DWORD SetFlags=0)\
		{\
		return GRes.PrivateAlloc(Name,TResEnum,Create==CREATE_Replace,SetFlags);\
		};\
	/* Try to import a resource from a file; returns pointer if success, NULL if failure */\
	void *operator new(size_t Size,const char *Name,char *Fname,EImportResource ImportType)\
		{\
		TClass *Temp = new(Name,(ECreateResource)ImportType)TClass;\
		if (Temp->ImportFromFile(Fname)) return Temp;\
		Temp->Kill(); return NULL;\
		};\
	/* Try to import a resource from a file; returns pointer if success, NULL if failure */\
	void *operator new(size_t Size,const char *Name,EFindResource FindType)\
		{\
		return GRes.PrivateLookup(Name,TResEnum,FindType);\
		};\
	/* Try to parse the resource's name */\
	friend int Get##TClass(const char *Stream, const char *Match, TClass **Res)\
		{\
		return GetRES(Stream,Match,TResEnum,(UResource **)Res);\
		};\
	/* Default constructor; must do nothing */\
	TClass(void) {};\
	/* Constructor used solely by AUTOREGISTER_RESOURCE macro */\
	TClass(EResourceType Type,DWORD A,DWORD B,DWORD C,DWORD D){GRes.PreRegisterType(Type,this,A,B,C,D);};\
	/* Create a database resource with a certain number of elements, optionally allocating it */\
	inline TClass(int MaxElements,int Occupy=0)\
		{\
		GUARD;\
		Num=0; Max=MaxElements; AllocData(0);\
		if (Occupy) Num=Max;\
		UNGUARD(#TClass "::Constructor");\
		};\
	/* Return the type-safe ith element of a database resource */\
	inline TDataType &Element(int i)\
		{\
		return ((TDataType *)Data)[i];\
		};

/*----------------------------------------------------------------------------
	Resource iteration macros
----------------------------------------------------------------------------*/

//
// Iterate for all active resources:
//
#define FOR_ALL_RES(RESVAR)\
	{\
	for (DWORD RESVAR##index=0; RESVAR##index<GRes.MaxRes; RESVAR##index++)\
		{\
		RESVAR = GRes.ResArray[RESVAR##index];\
		if (RESVAR)\
			{

//
// Iterate for all active resources of a particular type:
//
#define FOR_ALL_TYPED_RES(RESVAR,RESTYPE,RESCLASS)\
	{\
	for (DWORD RESVAR##index=0; RESVAR##index<GRes.MaxRes; RESVAR##index++)\
		{\
		RESVAR = (RESCLASS *)GRes.ResArray[RESVAR##index];\
		if (RESVAR && (RESVAR->Type==RESTYPE))\
			{
#define END_FOR_ALL_TYPED_RES END_FOR_ALL_RES\

//
// Close the iteration, for FOR_ALL_RES and FOR_ALL_TYPED_RES.
//
#define END_FOR_ALL_RES\
			};\
		};\
	};

/*----------------------------------------------------------------------------
	The End
----------------------------------------------------------------------------*/
#endif // _INC_UNRESTYP

