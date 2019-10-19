/*=============================================================================
	UnRes.cpp: Unreal resource manager and name management functions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

//
// Globals:
//
UResource	**GTempResList      = NULL;
int 		GNumResults         = 0;
int 		GCurResult          = 0;
int			*GTempResChildCount = NULL;

enum {MAX_RES_RESULTS	= 4096};
enum {MAX_ALLOC_SIZE	= 16*1024*1024};

//#define ENABLE_RESOURCE_PURGE /* Allow FGlobalResourceManager::Purge */
//#undef  RESOURCE_DEBUG        /* Print debugging info during purges*/

/*-----------------------------------------------------------------------------
	FUnknown implementation
-----------------------------------------------------------------------------*/

DWORD IUNKNOWN FUnknown::AddRef(void)
	{
	appError("FUnknown::AddRef not supported"); return 0;
	};

DWORD IUNKNOWN FUnknown::Release(void)
	{
	appError("FUnknown::Release not supported"); return 0;
	};

DWORD IUNKNOWN FUnknown::QueryInterface(FGUID GUID, void** ppvObj)
	{
	appError("FUnknown::QueryInterface not supported"); return 0;
	};

/*-----------------------------------------------------------------------------
	UResource implementation
-----------------------------------------------------------------------------*/

//
// All base functionality provided for UResource-derived classes
//

//
// Initialize all important parameters in the resource's header.  All
// child classes that have meaningful information in their headers must override
// this.  When this function is called, you cannot assume that the resource's
// data is valid or has been allocated.
//
void UResource::InitHeader(void)
	{
	GUARD;
	// Default implementation does nothing
	UNGUARD("UResource::InitHeader");
	};

//
// Initialize all important parameters related to a resource's data. All
// child classes that track meaningful data must override this. This function
// might set parameters in both the resource's header and the resource's data.
// The resource's data, if of nonzero length (according to QuerySize()), is
// guaranteed to be allocated at this point.
//
void UResource::InitData(void)
	{
	GUARD;
	// Default implementation does nothing
	UNGUARD("UResource::InitData");
	};

//
// Import this resource from a buffer.  This must be overridden by
// all resources which are capable of imporing.  The type of data being
// imported may be determined from *FileType, which represents the import
// file's extension (such as "PCX").
//
// Buffer is a pointer to the data being imported, and BufferEnd is a pointer
// to the end of the buffer.  The import function must return a pointer to
// the end of the data it parsed.  This is meaningless when importing a simple
// file like a PCX, but is important when importing multiple resources from a 
// complex Unreal text file.
//
// If importing from a text file, you are guaranteed that the buffer's last valid
// character will be a zero, so that you may disregard BufferEnd.
//
const char *UResource::Import(const char *Buffer, const char *BufferEnd,const char *FileType)
	{
	GUARD;
	// Default implementation should never be called
	appError("UResource::Import called");
	return NULL;
	UNGUARD("UResource::Import");
	};

//
// Export a resource to a buffer.  This must be overridden by
// all resources which are capable of exporting.  The type
// of data to be exported (text, binary, file format, etc) may
// be determined from the FileType string, which represents the
// file's extension, such as "PCX" for a PCX file.
//
// Buffer is a pointer to the data buffer to begin storing the
// data; the Export function must return a pointer to the end of
// the data it has written; if it exports 10 bytes, for example,
// it must return (Buffer+10).  If the export function fails,
// it should return NULL, indicating that the data should not be
// saved.
//
// Indent is an optional parameter that resources may use in order
// to properly format data exported to Unreal text files.
//
char *UResource::Export(char *Buffer,const char *FileType,int Indent)
	{
	GUARD;
	// Default implementation should never be called
	appError("UResource::Export called");
	return NULL;
	UNGUARD("UResource::Export");
	};

//
// Return the full size of this resource's data.  All resources
// which contain data must override this function.
//
// A resource must be capable of computing its size
// using only the information in its header, as this function
// is called during load/save before the resource's data has
// been loaded.  This is also called during allocation of
// the resource, where the caller (or the resource) must set
// size information in its header prior to allocating data.
//
int UResource::QuerySize(void)
	{
	GUARD;
	return 0; // Default implementation contains no data
	UNGUARD("UResource::QuerySize");
	};

//
// Return the minimal size of this resource's data.  This represents
// the number of contiguous bytes in the resource's data which
// are valid.  This may be smaller than the value returned by
// QuerySize() if the resource contains a variable number of
// records, and not all of the records are full.
//
// A resource must be capable of computing its minimal size
// using only the information in its header, as this function
// is called during load/save before the resource's data has
// been loaded.
//
int UResource::QueryMinSize(void)
	{
	GUARD;
	return QuerySize(); // Default implementation assumes no slack
	UNGUARD("UResource::QueryMinSize");
	};

//
// Called immediately before the resource manager frees this
// resource's data.  Child classes can override this if they
// need to do any special cleanup work.
//
void UResource::FreeData(void)
	{
	GUARD;
	// Default implementation does nothing
	UNGUARD("UResource::FreeData");
	};

//
// Do any resource-specific cleanup required immediately
// after loading a resource.
//
void UResource::PostLoad(void)
	{
	GUARD;
	// Default implementation does nothing
	UNGUARD("UResource::PostLoad");
	};

//
// Do any resource-specific cleanup required immediately
// before saving a resource.
//
void UResource::PreSave(void)
	{
	GUARD;
	// Default implementation does nothing
	UNGUARD("UResource::PreSave");
	};

//
// Do any resource-specific cleanup required immediately
// before a resource is killed.  Child classes may override
// this if they have to do anything here.
//
void UResource::PreKill(void)
	{
	GUARD;
	// Default implementation does nothing
	UNGUARD("UResource::PreKill");
	};

//
// Call the specific callback function with the reference addresses
// every resource and name this resource references in its header
// portion.  Child classes must override this function if they contain 
// references to any resources or data.  This provides a basis
// for dynamic pointer linking at load time.
//
void UResource::QueryHeaderReferences(FResourceCallback &Callback)
	{
	GUARD;
	UNGUARD("UResource::QueryHeaderReferences");
	};

//
// Call the specific callback function with the reference addresses
// every resource and name this resource references in its data
// portion.  Child classes must override this function if they contain 
// references to any resources or data.
//
void UResource::QueryDataReferences(FResourceCallback &Callback)
	{
	GUARD;
	UNGUARD("UResource::QueryDataReferences");
	};

//
// Reallocate this resource's data to a presumably
// new size, keeping the existing contents intact.
// Child classes do not override this.
//
void UResource::Realloc(void)
	{
	char Descr[80];
	sprintf(Descr,"Realloc(%s)",Name);
	if (!Data)	Data = appMalloc(QuerySize(),Descr);
	else		Data = appRealloc(Data,QuerySize(),Descr);
	};

//
// Export this resource to a file.  Child classes do not
// override this, but they do provide an Export() function
// to do the resoource-specific export work.  Returns 1
// if success, 0 if failed.
//
int	UResource::ExportToFile(char *Fname)
	{
	GUARD;
	//
	FILE *File = fopen(Fname,"wb");
	if (!File)
		{
		debugf  (LOG_Res,"Couldn't open %s",Fname);
		return 0;
		};
	GApp->StatusUpdatef ("Exporting %s",0,0,Name);
	//
	// Allocate and export the data:
	//
	char *Buffer    = (char *)appMalloc(MAX_ALLOC_SIZE,"Export(%s)",Name);
	char *BufferTop = Export(Buffer,fext(Fname),0);
	//
	// Write all stuff
	//
	if (BufferTop && (fwrite (Buffer,(int)(BufferTop-Buffer),1,File)==1))
		{
		appFree(Buffer);
		fclose(File);
		return 1;
		}
	else
		{
		debugf (LOG_Res,"Couldn't write %s",Name); 
		appFree (Buffer);
		fclose(File);
		_unlink(Fname);
		return 0;;
		};
	UNGUARD("UResource::ExportToFile");
	};

//
// Import this resource from a file.  Child classes do
// not override this function, but they do provide an
// Import() function to handle resource-specific data
// importing if they are importable.  Returns 1 if success,
// 0 if the import failed.
//
int	UResource::ImportFromFile(char *Fname)
	{
	GUARD;
	//
	// Get file size and allocate import buffer memory
	//
	GApp->StatusUpdate ("Reading file",0,0);
	int BufferLength = fsize(Fname);
	if (BufferLength <= 0)
		{
		debugf(LOG_Res,"Couldn't get size of %s",Fname);
		return 0; // Bad file
		};
	char Descr[80]; sprintf(Descr,"Import(%s)",Name);
	char *Buffer = appMallocArray(BufferLength+1,char,Descr);
	//
	// Open for reading
	//
	FILE *File=fopen(Fname,"rb");
	if (File==NULL)
		{
		appFree (Buffer);
		debugf(LOG_Res,"Couldn't open %s",Fname);
		return 0;
		};
	//
	// Read all file data stuff
	//
	if (fread (Buffer,BufferLength,1,File)!=1)
		{
		fclose  (File);
		appFree (Buffer);
		debugf  (LOG_Res,"Couldn't read %s",Fname);
		return 0;
		};
	fclose (File);
	//
	Buffer[BufferLength]=0; // In case importing text, make sure it's null-terminated
	if (!Import(Buffer,Buffer+BufferLength,fext(Fname)))
		{
		appFree (Buffer);
		return 0; // Failed
		};
	appFree (Buffer);
	return 1; // Success
	UNGUARD("UResource::ImportFromFile");
	};

//
// Unload this resource's data.  Not overridden by
// child classes.
//
void UResource::UnloadData(void)
	{
	GUARD;
	//
	if (Data && !(Flags & RF_NoFree))
	   	{
		FreeData();
	   	if (Data) appFree(Data);
		Data = NULL;
		};
	UNGUARD("UResource::UnloadData");
	};

//
// Kill this resource by freeing its data, freeing
// its header, and removing its entry from the global
// resource table.  Child classes do not override this function,
// but they can have a PreKill() routine to do any resource-specific
// cleanup required before killing.
//
void UResource::Kill (void)
	{
	GUARD;
	RESINDEX ThisIndex = Index;
	PreKill		();
	UnloadData	();
	appFree		(this);
	GRes.ResArray[ThisIndex] = NULL;
	UNGUARD("UResource::Kill");
	};

//
// Allocate or reallocate the data portion of this resource.
// Child classes do not override this function.
//
UNREAL_API void *UResource::AllocData (int Init)
	{
	GUARD;
	FResourceType	*ThisType = &GRes.Types [Type];
	DWORD			Size;
	//
	// Call resource handler to determine size of data:
	//
	Size = QuerySize();
	//
	// If data is already allocated, free it.
	//
	if (Data) UnloadData();
	//
	if (Size>0) // Zero-length data is valid
		{
		//
		// Allocate space for data:
		//
		if (!(Flags&RF_64KAlign))	Data = appMalloc(Size,"AllocData(%s)",Name);
		else						Data = appMallocAligned(Size,65536,"AllocData(%s)",Name);
		//
		if (!Data) appErrorf ("Couldn't allocate %li for %s %s",Size,ThisType->Descr,Name);
		};
	if (Init) InitData();
	//
	// Set resource info:
	//
	Flags |= RF_Modified; // Mark as modified to prevent swapping
	return Data;
	//
	UNGUARD("UResource::AllocData");
	};

//
// Compute the CRC32 of the resource's header.
// Child classes do not override this function.
//
DWORD UResource::HeaderCRC(void)
	{
	return strcrc((const unsigned char *)this,GRes.Types[Type].HeaderSize);
	};

//
// Compute the CRC32 of the resource's data.
// Child classes do not override this function.
//
DWORD UResource::DataCRC(void)
	{
	return strcrc((const unsigned char *)GetData(),QueryMinSize());
	};

//
// Compute the pseudo-CRC32 of the resource's header and
// data combined. Child classes do not override this function.
//
DWORD UResource::FullCRC(void)
	{
	return HeaderCRC() ^ DataCRC();
	};

/*-----------------------------------------------------------------------------
	UDatabase implementation
-----------------------------------------------------------------------------*/

int UDatabase::QuerySize(void)
	{
	GUARD;
	return Max * GRes.Types[Type].RecordSize;
	UNGUARD("UDatabase::QuerySize");
	};
int UDatabase::QueryMinSize(void)
	{
	GUARD;
	return Num * GRes.Types[Type].RecordSize;
	UNGUARD("UDatabase::QueryMinSize");
	};
void UDatabase::InitHeader(void)
	{
	GUARD;
	Num = 0; Max = 0;
	UNGUARD("UDatabase::InitHeader");
	};
void UDatabase::InitData(void)
	{
	GUARD;
	Num = 0;
	UNGUARD("UDatabase::InitData");
	};

/*-----------------------------------------------------------------------------
	FResourceCallback implementation
-----------------------------------------------------------------------------*/

//
// This is called by UResource-derived QueryHeaderReferences() and
// QueryDataReferences() functions to get a guaranteed-correct pointer
// to a particular resource whose pointer may be in a delinked state.
//
UResource *FResourceCallback::GetActualResource(UResource *Owner, UResource *PossiblyDelinkedRes)
	{
	// Default implementation assumes the resource is already linked.
	return PossiblyDelinkedRes;
	};

/*-----------------------------------------------------------------------------
	FGlobalResourceManager privates
-----------------------------------------------------------------------------*/

//
// Globals used by FGlobalResourceManager::Save and its various callbacks:
//
class FResSaveGlobals
	{
	public:
	UResource	**ResMap;     // Maps resources' in-file resources to memory resources
	FName		*NameMap;     // Maps in-file names to memory names
	int			MaxDataSize;  // Maximum data size found (for allocating big dat buffer)
	int			HeaderSize;   // Running total of all header sizes (for allocating header buffer)
	RESINDEX	NumImports;   // Counter: Number of resources to import
	RESINDEX	NumExports;   // Counter: Number of resources to export
	RESINDEX	NumNames;     // Counter: Number of names referenced
	} GSave;

//
// Find a specified resource in the resource table and return it or NULL.
// If not found, sets *AvailableIndex to the first available resource or NULL if table is full.
//
UResource *FGlobalResourceManager::FindResource(const char *Name,EResourceType ResType,RESINDEX *AvailableIndex)
	{
	GUARD;
	//
	RESINDEX i=0;
	while (i<MaxRes)
		{
		UResource *Res = ResArray[i];
		if (!Res)
			{
			*AvailableIndex = i;
			goto FoundAvailable;
			}
		else if ((Res->Type==ResType) && !stricmp(Name,Res->Name)) return Res;
		i++;
		};
	*AvailableIndex = RESINDEX_NONE;
	return NULL; // Found neither name nor available spot for resource.
	//
	// Found an available resource, but didn't find one we're looking for yet.
	//
	FoundAvailable:
	while (++i < MaxRes)
		{
		UResource *Res = ResArray[i];
		if (Res && (Res->Type==ResType) && !stricmp(Name,Res->Name)) return Res;
		};
	return NULL;
	UNGUARD("FindResource");
	};

UResource *FGlobalResourceManager::PrivateLookup (const char *Name,enum EResourceType ResType,EFindResource FindType)
	{
	GUARD;
	RESINDEX BogusIndex;
	UResource *Result = FindResource(Name,ResType,&BogusIndex);
	if ((FindType==FIND_Existing) && !Result) appErrorf ("Can't find %s %s",Types[ResType].Descr,Name);
	return Result;
	UNGUARD("FGlobalResourceManager::Lookup");
	};

//
// Look up a resource type and return its index, or RES_None if not found.
//						
EResourceType FGlobalResourceManager::LookupType (const char *Name)
	{
	GUARD;
	//
	for (DWORD i=0; i<MaxTypes; i++)
		{
		if (stricmp(Name,Types[i].Descr)==0) return (EResourceType)i;
		};
	return RES_None;
	UNGUARD("LookupType");
	};

/*-----------------------------------------------------------------------------
	FGlobalResourceManager Startup & Shutdown
-----------------------------------------------------------------------------*/

//
// Init the resource manager and allocate tables.  Returns 0 if ok,
// nonzero if couldn't allocate memory:
//
void FGlobalResourceManager::Init (void)
	{
	GUARD;
	DWORD i;
	//
	Counter	 = 0;
	MaxRes   = GDefaults.MaxRes;
	MaxNames = GDefaults.MaxNames;
	MaxFiles = GDefaults.MaxFiles;
	MaxTypes = GDefaults.MaxTypes;
	//
	// Allocate all tables:
	//
	ResArray	= appMallocArray (MaxRes,  UResource*,"GResArray");
	Names		= appMallocArray (MaxNames,FNameTableEntry,"GResNames");
	Files		= appMallocArray (MaxFiles,FResourceFileInfo,"GResFiles");
	Types		= appMallocArray (MaxTypes,FResourceType,"GResTypes");
	//
	// Init resources:
	//
	for (i=0; i<MaxRes; i++) ResArray[i] = NULL;
	//
	// Init names:
	//
	for (i=0; i<MaxNames; i++) Names[i].Name[0]='\0';
	//
	// Init files:
	//
	for (i=0; i<MaxFiles; i++)
		{
		Files[i].File         = NULL;
		Files[i].Fname[0]     = '\0';
		Files[i].NumResources = 0;
		Files[i].NumNames     = 0;
		Files[i].NameMap      = NULL;
		Files[i].ResMap       = NULL;
		};
	//
	// Init resource handler table:
	//
	for (i=0; i<MaxTypes; i++)
		{
		Types[i].VTablePtr  = NULL;
		Types[i].HeaderSize = 0;
		Types[i].RecordSize = 0;
		Types[i].Version    = 0;
		sprintf (Types[i].Descr,"TYPE_ERROR_%i",i);
		};
	//
	// Register internal (non-app) resource types:
	//
	for (i=0; i<NumAutoReg; i++) RegisterType
		(
		AutoTypes[i],AutoRes[i],
		AutoA[i],AutoB[i],AutoC[i],AutoD[i]
		);
	//
	// Allocate predefined resources:
	//
	Root 			= new("Root",CREATE_Unique)UArray(256);
	UnclaimedRes	= NULL;
	UnclaimedNames	= NULL;
	UNGUARD("FGlobalResourceManager::Init");
	};

//
// Shut down the resource manager:
//
void FGlobalResourceManager::Exit (void)
	{
	GUARD;
	//
	// Kill all unclaimed resources:
	//
	Purge(0);
	//
	// Kill the root resource:
	//
	Root->Kill();
	//
	debug (LOG_Exit,"FGlobalResourceManager::Exit");
	for (RESINDEX i=0; i<MaxRes; i++)
		{
		UResource *Res = ResArray[i];
		if (Res)
			{
			debugf(LOG_Exit,"Unkilled %s %s",Res->GetTypeName(),Res->Name);
			Res->UnloadData();
			appFree(Res);
			};
		};
	for (RESINDEX i=0; i<MaxFiles; i++)
		{
		if (Files[i].File!=NULL) fclose (Files[i].File);
		};
	appFree (ResArray);
	appFree (Names);
	appFree (Files);
	appFree (Types);
	//
	MaxRes   = 0;
	MaxNames = 0;
	MaxFiles = 0;
	MaxTypes = 0;
	//
	debug (LOG_Exit,"Resource subsystem successfully closed.");
	UNGUARD("FGlobalResourceManager::Exit");
	};

//
// WARNING! This routine is called in the resource type constructors before
// anything has been initialized and before the code has actually begun
// executing.  Anything done here must be 100% safe independent of startup order.
//
void FGlobalResourceManager::PreRegisterType (EResourceType ResType, UResource *TemplateRes,
	DWORD A,DWORD B,DWORD C,DWORD D)
	{
	static int Started=0;
	if (!Started)
		{
		Started    = 1;
		NumAutoReg = 0;
		};
	AutoTypes[NumAutoReg] = ResType;
	AutoRes  [NumAutoReg] = TemplateRes;
	AutoA    [NumAutoReg] = A;
	AutoB    [NumAutoReg] = B;
	AutoC    [NumAutoReg] = C;
	AutoD    [NumAutoReg] = D;
	NumAutoReg++;
	};

//
// Register a resource type and note its handler function, header size,
// and current version.
//
void FGlobalResourceManager::RegisterType (EResourceType ResType, UResource *Template,
	DWORD A,DWORD B,DWORD C,DWORD D)
	{
	GUARD;
	//
	void			*VTablePtr = *(void **)Template;
	FResourceType 	*Type = &Types[ResType];
	//
	Template->Data	 = NULL;
	//
	if (Type->HeaderSize!=0) appError ("Type already registered");
	//
	Type->VTablePtr  = VTablePtr;
	Type->HeaderSize = 0;
	Type->Version    = 0;
	Type->RecordSize = 0;
	Type->TypeFlags  = 0;
	Type->Descr [0]  = '\0';
	//
	Type->GUID [0]	 = A;
	Type->GUID [1]	 = B;
	Type->GUID [2]	 = C;
	Type->GUID [3]	 = D;
	//
	// Call resource's register function to make the resource
	// set its type properties: HeaderSize, Version, Descr, RecordSize, etc.
	//
	Template->Register(Type);
	//
	//if ((Type->RecordSize&3)&&(Type->RecordSize!=1)&&(Type->RecordSize!=2))
	//	debugf(LOG_Info,"Resource %s records are unaligned (Size=%i)",Type->Descr,Type->RecordSize);
	//if ((Type->HeaderSize&3)&&(Type->HeaderSize!=1)&&(Type->HeaderSize!=2))
	//	debugf(LOG_Info,"Resource %s header is unaligned (Size=%i)",Type->Descr,Type->HeaderSize);
	//
	// Make sure ResFunc worked.
	//
	if (Type->HeaderSize==0) appErrorf("Resource %i didn't register properly",ResType);
	UNGUARD("FGlobalResourceManager::RegisterType");
	};

/*-----------------------------------------------------------------------------
   FGlobalResourceManager command line
-----------------------------------------------------------------------------*/

int FGlobalResourceManager::Exec(const char *Cmd,FOutputDevice *Out)
	{
	GUARD;
	const char *Str = Cmd;
	//
	if (GetCMD(&Str,"STATUS"))
		{
		if (GetCMD(&Str,"RES") || !Str[0])
			{
			Out->Logf("   RES - %i resources, %iK",0,0);
			return Str[0]!=0;
			}
		else return 0;
		}
	else if (GetCMD(&Str,"LS"))
		{
		int Count	[RES_All]; memset(Count,	0,sizeof(Count));	int TotalCount=0;
		int Size	[RES_All]; memset(Size,		0,sizeof(Size));	int TotalSize=0;
		int MinSize	[RES_All]; memset(MinSize,	0,sizeof(MinSize)); int TotalMinSize=0;
		//
		UResource *Res;
		FOR_ALL_RES(Res)
			{
			int ThisSize	= Res->QuerySize();
			int ThisMinSize = Res->QueryMinSize();
			//
			Count[Res->Type]++;					TotalCount++;
			Size	[Res->Type]	+= ThisSize;	TotalSize		+= ThisSize;
			MinSize	[Res->Type]	+= ThisMinSize;	TotalMinSize	+= ThisMinSize;
			}
		END_FOR_ALL_RES;
		//
		Out->Log("Resources:");
		FResourceType *Type = &GRes.Types[0];
		for (int i=0; i<(int)MaxTypes; i++)
			{
			if (Type->VTablePtr)
				{
				Out->Logf(" %s...%i (%iK/%iK)",Type->Descr,Count[i],MinSize[i]/1000,Size[i]/1000);
				};
			Type++;
			};
		Out->Logf("%i Resources (%.3fM/%.3fM)",TotalCount,(FLOAT)TotalMinSize/1000000.0,(FLOAT)TotalSize/1000000.0);
		return 1;
		}
	else if (GetCMD(&Str,"HELP"))
		{
		Out->Log("   LS - Resource directory");
		return 1;
		}
	else return 0; // Not executed
	//
	UNGUARD("FGlobalResourceManager::Exec");
	};

/*-----------------------------------------------------------------------------
   FGlobalResourceManager file loading
-----------------------------------------------------------------------------*/

//
// Callback for dynamically linking resources and names after a resource
// file has been loaded into memory.
//
class FResourceCallbackLoadLink : public FResourceCallback
	{
	public:
	FResourceFileInfo *FileInfo;
	void Resource (UResource *ParentRes, UResource **Res, DWORD ContextFlags)
		{
		GUARD;
		if (*Res) *Res = GRes.Files[ParentRes->FileNum].ResMap[((int)*Res)-1];
		UNGUARD("FResourceCallbackLoadLink::Resource");
		};
	void Name (UResource *ParentRes, FName *Name,  DWORD ContextFlags)
		{
		GUARD;
		if (!Name->IsNone()) *Name = GRes.Files[ParentRes->FileNum].NameMap[Name->Index];
		UNGUARD("FResourceCallbackLoadLink::Name");
		};
	UResource *GetActualResource(UResource *Owner, UResource *PossiblyDelinkedRes)
		{
		GUARD;
		//
		if (!PossiblyDelinkedRes) return NULL;
		if (Owner->Flags & RF_Unlinked) return FileInfo->ResMap[((int)PossiblyDelinkedRes)-1];
		else return PossiblyDelinkedRes;
		//
		UNGUARD_BEGIN;
		UNGUARD_MSGF("FResourceCallbackLoadLink::GetActualResource (%i,%i)",(int)Owner,(int)PossiblyDelinkedRes);
		UNGUARD_END;
		};
	};

//
// Add all resources from a resource file into memory and dynamically link
// their pointers.  Returns resource file number or FILE_NONE if
// error.  All resources you obtain this way have the RF_Modified bit clear.
//
// Once you add a file, its resources become intermixed with the other resources 
// the system tracks.  You can only kill resources indivudually, not on a per-file basis
// thus there is no way to "un-add" a file.
//
INDEX FGlobalResourceManager::AddFile (const char *Fname)
	{
	GUARD;
	FResourceFileTrailer	Trailer;
	FResourceFileInfo		*FileInfo;
	FNameTableEntry			*ResNames;
	FNameFileEntry			*ResExports;
	FNameFileEntry			*ResImports;
	UResource				*Res;
	FResourceType			*Type;
	FILE					*File;
	BYTE					*HeaderBuffer;
	BYTE					*HeaderPtr;
	WORD					FileNum;
	RESINDEX				ThisIndex;
	DWORD					i,Allocated=0;
	char					Status[80],Temp[256],*ErrMsg;
	//
	sprintf        (Status,"Adding file %s...",Fname);
	GApp->StatusUpdate  (Status,0,0);
	debug          (LOG_Info,Status);
	//
	// Find an available index in the file table
	//
	for (FileNum=0; FileNum<MaxFiles; FileNum++)
		if (Files[FileNum].File==NULL)
			break;
	if (FileNum>=MaxFiles) return FILE_NONE; // Resource file table is full
	//
	// Open file
	//
	File=fopen(Fname,"rb");
	if (File==NULL) return FILE_NONE;
	//
	// Read file trailer
	//
	if (fseek (File,-(long)sizeof(FResourceFileTrailer),SEEK_END)!=0) {ErrMsg="Seek error"; goto Error;};
	if (fread (&Trailer,sizeof(FResourceFileTrailer),1,File)!=1) {ErrMsg="Initial read error"; goto Error;};
	//
	if (Trailer.FileFormatVersion != RES_FILE_VERSION)
		{
		sprintf(Temp,
			"The file %s was saved by a previous version of UnrealEd which "
			"may not be compatible with this one.  Reading it may fail, and "
			"may cause Unreal to crash.  Do you want to try anyway?",
			Fname);
		if (!GApp->MessageBox(Temp,"Outdated file version",1))
			{
			fclose(File);
			return FILE_NONE;
			};
		};
	//debug  (LOG_Res,"Read file trailer");
	//debugf (LOG_Res,"Vers=%i, E=%i, I=%i, N=%i",Trailer.FileFormatVersion,Trailer.NumExports,Trailer.NumImports,Trailer.NumNames);
	//
	// Allocate working tables:
	//
	HeaderBuffer = appMallocArray(Trailer.HeadersLength,BYTE,			"AddFileHeader");
	ResNames     = appMallocArray(Trailer.NumNames,FNameTableEntry,		"AddFileResNames");
	ResImports   = appMallocArray(Trailer.NumImports,FNameFileEntry,	"AddFileResImports");
	ResExports   = appMallocArray(Trailer.NumExports,FNameFileEntry,	"AddFileResExports");
	Allocated    = 1;
	//
	// Read all preload stuff from file: Names, imports, exports, headers.
	//
	//debug(LOG_Res,"Reading Names (%i)",Trailer.NumNames);
	if (fseek(File,Trailer.NamesOffset,SEEK_SET)!=0) {ErrMsg="Error seeking names"; goto Error;};
	if (fread(ResNames,sizeof(FNameTableEntry),Trailer.NumNames,File)!=Trailer.NumNames) {ErrMsg="Error reading names"; goto Error;};
	//
	//debug(LOG_Res,"Reading Imports (%i)",Trailer.NumImports);
	if (fseek(File,Trailer.ImportsOffset,SEEK_SET)!=0) {ErrMsg="Error seeking imports"; goto Error;};
	if (fread(ResImports,sizeof(FNameFileEntry),Trailer.NumImports,File)!=Trailer.NumImports) {ErrMsg="Error reading imports"; goto Error;};
	//
	//debug(LOG_Res,"Reading Headers");
	if (fseek(File,Trailer.HeadersOffset,SEEK_SET)!=0) {ErrMsg="Error seeking headers"; goto Error;};
	if (fread(HeaderBuffer,Trailer.HeadersLength,1,File)!=1) {ErrMsg="Error reading headers"; goto Error;};
	//
	//debug(LOG_Res,"Reading Exports");
	if (fseek(File,Trailer.ExportsOffset,SEEK_SET)!=0) {ErrMsg="Error seeking exports"; goto Error;};
	if (fread(ResExports,sizeof(FNameFileEntry),Trailer.NumExports,File)!=Trailer.NumExports) {ErrMsg="Error reading exports"; goto Error;};
	//
	// Set stuff in global file table (required before linking can begin):
	//
	FileInfo = &Files[FileNum];
	//
	Files[FileNum].File = File;
	strcpy(Files[FileNum].Fname,Fname);
	//
	// Allocate resource map and name map.  These are allocated separately because
	// they remain in memory as long as this file is added, because they are needed
	// to link data loaded via the resource cache.
	//
	FileInfo->NameMap = (FName      *)appMalloc ((Trailer.NumNames)*sizeof (FName),"AddFileNameMap");
	FileInfo->ResMap  = (UResource **)appMalloc ((Trailer.NumImports+Trailer.NumExports)*sizeof (UResource *),"AddFileResMap");
	//
	// Validate all imports and add them to resource map.
	//
	if (Trailer.NumImports) debugf(LOG_Res,"%i imports",Trailer.NumImports);
	for (i=0; i<Trailer.NumImports; i++)
		{
		//debugf(LOG_Res,"Importing %s %s",Types[ResImports[i].ResType].Descr,ResImports[i].Name);
		Res = PrivateLookup (ResImports[i].Name, ResImports[i].ResType,FIND_Optional);
		if (!Res) appErrorf ("Import resource %s %s doesn't exist",Types [ResImports[i].ResType].Descr, ResImports[i].Name);
		FileInfo->ResMap[i] = Res;
		};
	//
	// Add names from name map (either creates new name, or notes reference to existing one)
	//
	for (i=0; i<Trailer.NumNames; i++) FileInfo->NameMap[i].Add(ResNames[i].Name);
	//
	// Process all exports (replace if already exists, add if new):
	//
	HeaderPtr = HeaderBuffer; // Goes through all headers in file
	for (i=0; i<Trailer.NumExports; i++)
		{
		//debugf(LOG_Res,"Reading export %s %s",Types[ResImports[i].ResType].Descr,ResImports[i].Name);
		if (!(i&7)) GApp->StatusUpdate (Status,i,Trailer.NumExports);
		Res   = FindResource(ResExports[i].Name,ResExports[i].ResType,&ThisIndex);
		Type  = &Types[ResExports[i].ResType];
		//
		if (!Res) // Create new
			{
			if (ThisIndex==RESINDEX_NONE) appError ("Resource table full");
			Res					= (UResource *)appMalloc(Type->HeaderSize,"Res(%s)",ResExports[i].Name);
			ResArray[ThisIndex]	= Res;
			}
		else if (!(Res->Flags & RF_NoReplace)) // Replace existing
			{
			ThisIndex = Res->Index;
			//
			Res->PreKill();
			Res->UnloadData();
			}
		else // Don't replace anything marked unreplaceable
			{
			FileInfo->ResMap[i+Trailer.NumImports] = Res;
			goto NextRes;
			};
		FileInfo->ResMap[i+Trailer.NumImports] = Res;
		//
		memcpy (Res, HeaderPtr, Type->HeaderSize);
		*(void **)Res	= Type->VTablePtr;
		Res->Index		= ThisIndex;
		Res->FileNum    = FileNum;
		Res->Data       = NULL;
		Res->Flags     &= ~RF_NoReplace;
		//
		if (Res->AllocData(0)) // Allocate but don't init
			{
			if (Res->FileDataSize>0)
				{
				//sprintf(TempStr,"Seeking to %li",Res->FileDataOffset); debug (LOG_Res,TempStr);
				if (fseek(File,Res->FileDataOffset,SEEK_SET)!=0) {ErrMsg="Error seeking data"; goto Error;};
				//sprintf(TempStr,"Reading %li",Res->FileDataSize); debug (LOG_Res,TempStr);
				if (fread(Res->Data,Res->FileDataSize,1,File)!=1) {ErrMsg="Error reading data"; goto Error;};
				};
			};
		NextRes:
		HeaderPtr += Type->HeaderSize;
		};
	//
	// Tag all exported resources as unlinked:
	//
	for (i=0; i<Trailer.NumExports; i++) FileInfo->ResMap[i]->Flags |= RF_Unlinked;
	//
	// Dynamically link the headers and data.  This currently doesn't support caching
	// and doesn't distinguish between data/header fixup.
	//
	GApp->StatusUpdate ("Linking",0,0);
	for (i=0; i<Trailer.NumExports; i++)
		{
		Res = FileInfo->ResMap[i];
		//debugf(LOG_Res,"%i/%i %s %s",i,Trailer.NumExports,Types[Res->Type].Descr,Res->Name);
		if (!(Res->Flags & RF_NoReplace))
			{
			//debugf(LOG_Res,"Dynalinking %i: %s %s",i,Types[Res->Type].Descr,Res->Name);
			FResourceCallbackLoadLink Callback;
			Callback.FileInfo = FileInfo;
			Res->QueryHeaderReferences (Callback);
			Res->QueryDataReferences   (Callback);
			};
		Res->Flags &= ~RF_Unlinked;
		//else debugf(LOG_Res,"Not replacing %i: %s %s",i,Types[Res->Type].Descr,Res->Name);
		};
	//
	// Call postload if desired:
	//
	for (i=0; i<Trailer.NumExports; i++)
		{
		FileInfo->ResMap[i]->PostLoad();
		};
	// Current non-caching exit code:
	appFree(FileInfo->NameMap);
	appFree(FileInfo->ResMap);
	Files [FileNum].File = NULL;
	//
	Out:
	if (File) fclose (File);
	if (Allocated)
		{
		appFree(HeaderBuffer);
		appFree(ResNames);
		appFree(ResImports);
		appFree(ResExports);
		};
	return FileNum;
	//
	// Error exit:
	//
	Error:
	debugf (LOG_Info,"Problem adding file: %s",ErrMsg);
	FileNum = FILE_NONE;
	goto Out;
	//
	UNGUARD("FGlobalResourceManager::AddFile");
	};

/*-----------------------------------------------------------------------------
	FGlobalResourceManager file saving
-----------------------------------------------------------------------------*/

//
// Callback for tagging resources and names that must be exported
// to the file.  It tags the resource passed to it, and recursively
// tags all of the resources this resource references.
//
class FResourceCallbackSaveTagExports : public FResourceCallback
	{
	void Resource (UResource *ParentRes, UResource **Res, DWORD ContextFlags)
		{
		GUARD;
		if ((*Res) && !((*Res)->Flags & RF_TagExp))
			{
			(*Res)->Flags |= RF_TagExp; // Tag for export
			//
			(*Res)->QueryHeaderReferences (*this);
			(*Res)->QueryDataReferences   (*this);
			};
		UNGUARD("FResourceCallbackSaveTagExports::Resource");
		};
	void Name (UResource *ParentRes, FName *Name,  DWORD ContextFlags)
		{
		GUARD;
		if (!Name->IsNone()) GRes.Names[Name->Index].Flags |= RF_TagExp;
		UNGUARD("FResourceCallbackSaveTagExports::Name");
		};
	};

//
// Callback for tagging resources and names that must be listed in the
// file's imports table.
//
class FResourceCallbackSaveTagImports : public FResourceCallback
	{
	void Resource (UResource *ParentRes, UResource **Res, DWORD ContextFlags)
		{
		GUARD;
		if ((*Res) && !((*Res)->Flags & RF_TagExp)) (*Res)->Flags |= RF_TagImp;
		UNGUARD("FResourceCallbackSaveTagImports::Resource");
		};
	};

//
// Callback to tag all names referenced by the resource.
//
class FResourceCallbackSaveTagNames : public FResourceCallback
	{
	void Name (UResource *ParentRes, FName *Name,  DWORD ContextFlags)
		{
		if (!Name->IsNone()) GRes.Names[Name->Index].Flags |= RF_TagExp;
		};
	};

//
// Callback to delink all resources and names, remapping each resource pointer
// and name to an index which is placed in the file, and later used for
// dynamic linking when the resource is loaded.
//
class FResourceCallbackSaveDelink : public FResourceCallback
	{
	void Resource (UResource *ParentRes, UResource **Res, DWORD ContextFlags)
		{
		GUARD;
		if (*Res)
			{
			for (RESINDEX i=0; i<(GSave.NumImports + GSave.NumExports); i++)
				{
				if (GSave.ResMap[i]==*Res) {*Res=(UResource *)(i+1); return;};
				};
			appErrorf ("%s %s not found",(*Res)->GetTypeName(),(*Res)->Name);
			};
		UNGUARD("FResourceCallbackSaveDelink::Resource");
		};
	void Name (UResource *ParentRes, FName *Name,  DWORD ContextFlags)
		{
		GUARD;
		if (!Name->IsNone())
			{
			for (RESINDEX i=0; i < GSave.NumNames; i++)
				{
				if (GSave.NameMap[i].Index == Name->Index) {Name->Index=i; return;};
				};
			appErrorf ("Resource %i not found (of %i) in %s %s!",Name->Index,GSave.NumNames,
				GRes.Types[ParentRes->Type].Descr,ParentRes->Name);
			};
		UNGUARD("FResourceCallbackSaveDelink::Name");
		};
	UResource *GetActualResource(UResource *Owner, UResource *PossiblyDelinkedRes)
		{
		GUARD;
		return PossiblyDelinkedRes;
		/*
		if (!PossiblyDelinkedRes) return NULL;
		if (Owner->Flags & RF_Unlinked) return FileInfo->ResMap[((int)PossiblyDelinkedRes)-1];
		else return PossiblyDelinkedRes;
		*/
		UNGUARD("FResourceCallbackSaveDelink::GetActualResource");
		};
	};

//
// Save all tagged resources into a file.
//
// If you specify an already-added delta file, this will only save the
// changes that occured between the in-memory resources and the original in-file
// resources, based on a CRC comparison.
//
void FGlobalResourceManager::SaveTagged (const char *Fname, BYTE DeltaFile)
	{
	GUARD;
	UResource			*Res;             // Temporary resource pointer
	FResourceType		*Type;            // Temporary resource type pointer
	FNameTableEntry		*ResNames;        // File's name symbol table
	FNameFileEntry		*ResImports;      // File's resource import symbol table
	FNameFileEntry		*ResExports;      // File's export resource table
	FResourceFileTrailer*ResFileTrailer;  // File's trailer
	BYTE				*HeaderBuffer;    // Resource header buffer to be dumped to file
	BYTE				*HeaderPtr;       // Header pointer (in HeaderBuffer)
	BYTE				*SaveBuffer;      // Temp buffer for resource manipulation
	FILE				*File;            // File we're writing to
	DWORD				Offset,MinDataSize; // Current position (offset) in file
	RESINDEX			i,c;
	char				Status[80];
	int					Allocated=0;
	//
	sprintf       (Status,"Saving %s",Fname);
	GApp->StatusUpdate (Status,0,0);
	//
	// Tag all resources for import that are (1) children of
	// exported resources, and (2) not being exported:
	//
	for (i=0; i<MaxRes; i++)
		{
		Res  = ResArray [i];
		if (Res && (Res->Flags & RF_TagExp))
			{
			FResourceCallbackSaveTagImports Callback;
			Res->QueryHeaderReferences (Callback);
			Res->QueryDataReferences   (Callback);
			};
		};
	//
	// Call presave if desired:
	//
	for (i=0; i<MaxRes; i++)
		{
		Res  = ResArray [i];
		if (Res && (Res->Flags & RF_TagExp)) Res->PreSave();
		};
	//
	// Calculate counts and stats:
	//
	GSave.MaxDataSize = 0;	// Maximum size of data for temp buffer
	GSave.HeaderSize  = 0;  // Total size of buffer for all headers
	GSave.NumImports  = 0;  // Total number of symbols to import in file
	GSave.NumExports  = 0;  // Total number of symbols to export in file
	GSave.NumNames    = 0;  // Total number of names to reference in file
	//
	for (i=0; i<MaxRes; i++) // Resources
		{
		Res = ResArray[i];
		if (Res)
			{
			if (Res->Flags & RF_TagExp)
				{
				Type = &Types [Res->Type];
				GSave.NumExports++;
				GSave.HeaderSize += Type->HeaderSize;
				GSave.MaxDataSize = OurMax(GSave.MaxDataSize,Res->QueryMinSize());
				}
			else if (Res->Flags & RF_TagImp) GSave.NumImports++;
			};
		};
	for (i=0; i<MaxNames; i++) // Names
		{
		if (Names[i].Flags & RF_TagExp)
			{
			if (!Names[i].Name[0]) appError ("Invalid name tagged");
			GSave.NumNames++;
			};
		};
	//
	// Allocate all memory we need:
	//
	SaveBuffer		= appMallocArray(GSave.MaxDataSize*2,BYTE,			"SaveSaveBuffer");
	HeaderBuffer	= appMallocArray(GSave.HeaderSize,BYTE,				"SaveHeaderBuffer");
	ResFileTrailer	= appMallocArray(1,FResourceFileTrailer,			"SaveResFileTrailer");
	ResNames		= appMallocArray(GSave.NumNames,FNameTableEntry,	"SaveResNames");
	ResImports		= appMallocArray(GSave.NumImports,FNameFileEntry,	"SaveResImports");
	ResExports		= appMallocArray(GSave.NumExports,FNameFileEntry,	"SaveResExports");
	GSave.NameMap	= appMallocArray(GSave.NumNames,FName,				"SaveNameMap");
	GSave.ResMap	= appMallocArray(GSave.NumImports+GSave.NumExports,UResource *,"SaveResMap");
	Allocated       = 1;
	//
	// Build NameSymbols and NameMap, which link the indices of
	// NAME_LENGTH-character names in the
	// global name table to their indices in the file:
	//
	c=0;
	for (i=0; i<MaxNames; i++)
		{
		if (Names[i].Flags & RF_TagExp)
			{
			mystrncpy(ResNames[c].Name,Names[i].Name,NAME_SIZE);
			ResNames		[c].Flags    = 0; // Unnecessary
			GSave.NameMap	[c++].Index  = i;
			};
		};
	if (c != GSave.NumNames) appError ("Name count mismatch");
	//
	// Build resource map which maps global resources to file resources, while
	// also building import symbol table:
	//
	c=0;
	for (i=0; i<MaxRes; i++) // First process all imports
		{
		Res = ResArray[i];
		if (Res && (Res->Flags & RF_TagImp))
			{
			mystrncpy(ResImports [c].Name,Res->Name,NAME_SIZE);
			ResImports	 [c].ResType = Res->Type;
			GSave.ResMap [c]         = Res;
			c++;
			};
		};
	if (c!=GSave.NumImports) appError ("Import count mismatch");
	//
	// Now process all exports
	//
	for (i=0; i<MaxRes; i++)
		{
		Res = ResArray[i];
		if (Res && (Res->Flags & RF_TagExp))
			{
			GSave.ResMap[c++] = Res;
			//bug ("%s %s",Types[Res->Type].Descr,Res->Name);
			};
		};
	if (c != (GSave.NumImports + GSave.NumExports)) appError ("Mismatch 2");
	//
	// Initialize file trailer:
	//
	ResFileTrailer->FileFormatVersion = RES_FILE_VERSION;
	ResFileTrailer->NumExports        = GSave.NumExports;
	ResFileTrailer->NumImports        = GSave.NumImports;
	ResFileTrailer->NumNames          = GSave.NumNames;
	strncpy (ResFileTrailer->Tag,RES_FILE_TAG,NAME_SIZE);
	//
	// Open dest file, handle any errors:
	//
	Offset=0; // Position within file
	File=fopen(Fname,"wb");
	if (File==NULL)
		{
		appError ("Couldn't open file"); // Handle gracefully !!
		goto Out;
		};
	//
	// Stick text "Unreal resource" at beginning:
	//
	if (fwrite (RES_FILE_TAG,strlen(RES_FILE_TAG)+1,1,File)!=1) goto FileError;
	Offset += strlen(RES_FILE_TAG)+1;
	//
	// Save names:
	//
	if (fwrite (NAMES_MARK,strlen(NAMES_MARK),1,File)!=1) goto FileError;
	Offset += strlen(NAMES_MARK);
	//
	ResFileTrailer->NamesOffset = Offset;
	if (fwrite (ResNames,sizeof(FNameTableEntry),GSave.NumNames,File) != GSave.NumNames) goto FileError;
	Offset += GSave.NumNames * sizeof (FNameTableEntry);
	//
	// Save import resource summary:
	//
	if (fwrite (IMPORT_MARK,strlen(IMPORT_MARK),1,File)!=1) goto FileError;
	Offset += strlen(IMPORT_MARK);
	//
	ResFileTrailer->ImportsOffset = Offset;
	if (fwrite (ResImports,sizeof(FNameFileEntry),GSave.NumImports,File) != GSave.NumImports) goto FileError;
	Offset += GSave.NumImports * sizeof (FNameTableEntry);
	//
	// Save export data buffer in memory:
	//
	if (fwrite (DATA_MARK,strlen(DATA_MARK),1,File)!=1) goto FileError;
	Offset += strlen(DATA_MARK);
	//
	HeaderPtr=HeaderBuffer;
	for (i=0; i < GSave.NumExports; i++)
		{
		if (!(i&7)) GApp->StatusUpdate (Status,i,GSave.NumExports);
		//
		// Copy resource table entry from ResArray to ResTable, append header to
		// header buffer for unlinking, and copy data to data buffer for link.
		//
		//debugf(LOG_Res,"ResT i=%i, Map=%i, SNE=%i",i,SaveResMap[i],SaveNumExports);
		//
		Res						= GSave.ResMap[i + GSave.NumImports];
		Type					= &Types[Res->Type];
		ResExports[i].ResType	= Res->Type;
		ResExports[i].ResSize	= Type->HeaderSize;
		strcpy(ResExports[i].Name,Res->Name);
		//
		memcpy (HeaderPtr, Res, Type->HeaderSize);
		//
		// Should use resGetData to get data even if swapped
		//
		UResource *TempRes		= (UResource *)HeaderPtr;
		MinDataSize				= TempRes->QueryMinSize();
		TempRes->Index			= RESINDEX_NONE;
		TempRes->Data			= SaveBuffer;
		TempRes->Flags          = Res->Flags & RF_SaveMask;
		//
		if (MinDataSize != 0) memcpy (SaveBuffer, Res->Data, MinDataSize);
		//
		FResourceCallbackSaveDelink Callback;
		TempRes->QueryHeaderReferences (Callback);
		TempRes->QueryDataReferences   (Callback);
		//
		TempRes->FileDataSize	= MinDataSize;
		TempRes->FileDataOffset	= Offset;
		TempRes->Index			= i;
		TempRes->FileNum		= FILE_NONE;
		TempRes->Data			= NULL;
		//
		TempRes->FileCRC		= Res->FullCRC();
		//
		// Write resource data to file and advance offset.
		//
		if (TempRes->FileDataSize>0)
			{
			if (fwrite (SaveBuffer,TempRes->FileDataSize,1,File)!=1) goto FileError;
			};
		//
		// Update counters
		//
		Offset    += ((UResource *)HeaderPtr)->FileDataSize;
		HeaderPtr += Type->HeaderSize;
		};
	GApp->StatusUpdate ("Finalizing",0,0);
	//
	// Save export resource summary:
	//
	ResFileTrailer->ExportsOffset = Offset;
	if (fwrite (ResExports,GSave.NumExports*sizeof(FNameFileEntry),1,File)!=1) goto FileError;
	Offset += GSave.NumExports * sizeof(FNameFileEntry);
	//
	// Write all headers
	//
	if (fwrite (HEADER_MARK,strlen(HEADER_MARK),1,File)!=1) goto FileError;
	Offset += strlen(HEADER_MARK);
	//
	ResFileTrailer->HeadersOffset = Offset;
	ResFileTrailer->HeadersLength = GSave.HeaderSize;
	if (GSave.HeaderSize != 0)
		{
		if (fwrite (HeaderBuffer,GSave.HeaderSize,1,File)!=1) goto FileError;
		Offset += GSave.HeaderSize;
		};
	//
	// Write entire export table to file.
	//
	if (fwrite (TABLE_MARK,strlen(TABLE_MARK),1,File)!=1) goto FileError;
	Offset += strlen(TABLE_MARK);
	//
	// Update and write entire ResFileTrailer to file.
	//
	if (fwrite (TRAILER_MARK,strlen(TRAILER_MARK),1,File)!=1) goto FileError;
	Offset += strlen(TRAILER_MARK);
	//
	if (fwrite (ResFileTrailer,sizeof(FResourceFileTrailer),1,File)!=1) goto FileError;
	//
	fclose (File);
	//
	//debugf(TempStr,"%s: I=%i E=%i N=%i, DL=%li HL=%li",
	// Fname,SaveNumImports,SaveNumExports,SaveNumNames,
	// SaveMaxDataSize,SaveHeaderSize);
	//
	goto Out;
	//
	FileError:
	fclose(File);
	unlink(Fname);
	debug(LOG_File,"Error writing to resource file");
	//
	Out:
	if (Allocated)
		{
		appFree(SaveBuffer);
		appFree(HeaderBuffer);
		appFree(ResFileTrailer);
		appFree(ResNames);
		appFree(ResImports);
		appFree(ResExports);
		appFree(GSave.NameMap);
		appFree(GSave.ResMap);
		};
	UNGUARD("FGlobalResourceManager::SaveTagged");
	};

//
// Save one specific resource into a resource file.
//
void FGlobalResourceManager::Save (UResource *Res, const char *Fname, BYTE DeltaFile)
	{
	GUARD;
	FResourceCallbackSaveTagNames Callback;
	UntagAll();
	if (Res)
		{
		Res->Flags |= RF_TagExp;
		Res->QueryHeaderReferences	(Callback);
		Res->QueryDataReferences	(Callback);
		SaveTagged (Fname,DeltaFile);
		};
	UNGUARD("FGlobalResourceManager::Save");
	};

//
// Recursively tag all resources that are referenced by tagged resources.
//
void FGlobalResourceManager::SaveTagAllDependents (void)
	{
	GUARD;
	FResourceCallbackSaveTagExports Callback;
	for (RESINDEX i=0; i<GRes.MaxRes; i++)
		{
		UResource *Res = GRes.ResArray[i];
		if (Res && (Res->Flags & RF_TagExp)) 
			{
			Res->QueryHeaderReferences (Callback);
			Res->QueryDataReferences   (Callback);
			};
		};
	UNGUARD("FGlobalResourceManager::SaveTagAllDependents");
	};

//
// Save the specified resource as well as all resources it references.
//
void FGlobalResourceManager::SaveDependent (UResource *Res, const char *Fname, BYTE DeltaFile)
	{
	GUARD;
	FResourceCallbackSaveTagExports Callback;
	UntagAll					();
	Res->Flags |= RF_TagExp;
	Res->QueryHeaderReferences	(Callback);
	Res->QueryDataReferences	(Callback);
	SaveTagged					(Fname,DeltaFile);
	UNGUARD("FGlobalResourceManager::SaveDependent");
	};

//
// Save all tagged resources, as well as all resources that tagged
// resources reference.
//
void FGlobalResourceManager::SaveDependentTagged (const char *Fname, BYTE DeltaFile)
	{
	GUARD;
	SaveTagAllDependents ();
	SaveTagged (Fname,DeltaFile);
	UNGUARD("FGlobalResourceManager::SaveDependentTagged");
	};

/*-----------------------------------------------------------------------------
	Resource name functions
-----------------------------------------------------------------------------*/

//
// Combine a name, a type character, and a number string and return the result,
// concatenating the name part if necessary to make it fit.
//
char *FGlobalResourceManager::CombineName (char *Result, const char *Name, const char *TypeChar,int Num)
	{
	GUARD;
	char Append [NAME_SIZE];
	//
	strcpy (Append,"_");
	strcat (Append,TypeChar);
	itoa   (Num,Append + strlen(Append),10);
	//
	strcpy (Result,Name);
	//
	if (strlen(Result) + strlen (Append) <= (NAME_SIZE-1))
		{
		strcat (Result,Append);
		}
	else
		{
		strcpy (Result + NAME_SIZE - 1 - strlen (Append), Append);
		};
	return Result;
	UNGUARD("FGlobalResourceManager::CombineName");
	};

//
// Create a unique name by combining a base name, string to append, and an
// arbitrary number string.  The resource name returned is guaranteed not to
// exist.
//
char *FGlobalResourceManager::MakeUniqueName(char *Result, char *BaseName, const char *Append, EResourceType ResType)
	{
	static int TempInt = 0;
	int StartInt = TempInt;
	//
	GUARD;
	do
		{
		mystrncpy(Result,BaseName,NAME_SIZE-strlen(Append)-5);
		strcat(Result,Append);
		//
		itoa(TempInt,Result+strlen(Result),10);
		if (++TempInt>=10000) TempInt=0;
		if (TempInt==StartInt) appError ("Out of unique names");
		//
		} while (PrivateLookup(Result,ResType,FIND_Optional));
	return Result;
	UNGUARD("FGlobalResourceManager::MakeUniqueName");
	};

/*-----------------------------------------------------------------------------
	Resource tagging-for-export:
-----------------------------------------------------------------------------*/

//
// Tag a resource as 'unused' so that it will be deleted from the resource
// table on the next call to resCleanup.  If this resource is truly unused,
// it and all of the resources that depend solely on unused resources will
// be deleted.  If you tag a resource as 'unused' when it is, in fact,
// referenced by active resources, resCleanup will generate a debugging
// warning.  This will normally be used only in the editor, not in the
// game itsself.  You normally do NOT want to unload resource headers.
//
void FGlobalResourceManager::TagUnused (UResource *Res)
	{
	GUARD;
	Res->Flags |= RF_Unused;
	UNGUARD("FGlobalResourceManager::TagUnused");
	};

//
// Untag all resources.  Call before tagging resources for saving.
//
void FGlobalResourceManager::UntagAll (void)
	{
	GUARD;
	for (RESINDEX i=0; i<MaxRes;   i++)
		{
		if (ResArray[i]) ResArray[i]->Flags &= ~(RF_TagImp | RF_TagExp);
		};
	for (RESINDEX i=0; i<MaxNames; i++) Names [i]. Flags &= ~(RF_TagImp | RF_TagExp);
	UNGUARD("FGlobalResourceManager::UntagAll");
	};

//
// Tag all resources of type ResType that reference a resource.
// Returns number of resources tagged (may be zero).
//
class FResourceCallbackTagRefRes : public FResourceCallback
	{
	public:
	UResource	*TagRes;
	int			TagCount;
	void Resource (UResource *ParentRes, UResource **Res, DWORD ContextFlags)
		{
		GUARD;
		if ((*Res==TagRes) && !(ParentRes->Flags & RF_TagExp))
			{
			ParentRes->Flags |= RF_TagExp;
			TagCount++;
			};
		UNGUARD("FResourceCallbackTagRef::Resource");
		};
	};
int FGlobalResourceManager::TagAllReferencingResource (UResource *TagRes, EResourceType ResType)
	{
	GUARD;
	//
	FResourceCallbackTagRefRes Callback;
	Callback.TagRes		= TagRes;
	Callback.TagCount	= 0;
	//
	for (DWORD i=0; i<MaxRes; i++)
		{
		UResource *Res = ResArray[i];
		if (Res && (Res->Type == ResType))
			{
			Res->QueryHeaderReferences	(Callback);
			Res->QueryDataReferences	(Callback);
			};
		};
	return Callback.TagCount;
	UNGUARD("FGlobalResourceManager::TagAllReferencingResource");
	};

//
// Tag all resources of type ResType that reference a particular name.
// Returns number of resources tagged (may be zero).
//
class FResourceCallbackTagRefName : public FResourceCallback
	{
	public:
	FName TagName;
	int   TagCount;
	void Name (UResource *ParentRes, FName *Name,  DWORD ContextFlags)
		{
		GUARD;
		if (*Name==TagName)
			{
			ParentRes->Flags |= RF_TagExp;
			TagCount++;
			};
		UNGUARD("FResourceCallbackTagRef::Name");
		};
	};
int FGlobalResourceManager::TagAllReferencingName (FName Name, EResourceType ResType)
	{
	GUARD;
	UResource		*Res;
	DWORD 			i;
	//
	FResourceCallbackTagRefName Callback;
	Callback.TagName  = Name;
	Callback.TagCount = 0;
	//
	for (i=0; i<MaxRes; i++)
		{
		Res = ResArray[i];
		if (Res && (Res->Type == ResType))
			{
			Res->QueryHeaderReferences	(Callback);
			Res->QueryDataReferences	(Callback);
			};
		};
	return Callback.TagCount;
	UNGUARD("FGlobalResourceManager::TagAllReferencingName");
	};


//
// Tag all resources of type ResType that reference tagged resources.
// Recurses until all resources referencing tagged resources are tagged.
//
class FResourceCallbackTagRefTagged : public FResourceCallback
	{
	public:
	int NumNewlyTagged;
	//
	void Resource (UResource *ParentRes, UResource **Res, DWORD ContextFlags)
		{
		if (*Res && !(ParentRes->Flags & RF_TagExp))
			{
			NumNewlyTagged++;
			ParentRes->Flags |= RF_TagExp;
			};
		};
	};
void FGlobalResourceManager::TagReferencingTagged (EResourceType ResType)
	{
	GUARD;
	FResourceCallbackTagRefTagged Callback;
	do
		{
		Callback.NumNewlyTagged=0;
		for (RESINDEX i=0; i<MaxRes; i++)
			{
			UResource *Res = ResArray[i];
			if (Res && (Res->Type == ResType))
				{
				Res->QueryHeaderReferences	(Callback);
				Res->QueryDataReferences	(Callback);
				};
			};
		} while (Callback.NumNewlyTagged>0);
	UNGUARD("FGlobalResourceManager::TagReferencingTagged");
	};

/*-----------------------------------------------------------------------------
	Creating and allocating data for new resources:
-----------------------------------------------------------------------------*/

//
// Initialize the generic information in a resource.  Internal only.
//
void FGlobalResourceManager::InitResource(UResource *Res,EResourceType ResType,RESINDEX Index,const char *Name)
	{
	GUARD;
	FResourceType *Type = &Types[ResType];
	if (Index==RESINDEX_NONE)
		{
		for (Index=0; Index<MaxRes; Index++) if (!ResArray[Index]) break;
		if (Index>=MaxRes) appError ("Resource table is full");
		};
	ResArray[Index]	= Res;
	//
	*(void **)Res		= Type->VTablePtr;
	Res->Index          = Index;
	Res->Type			= ResType;
	Res->FileNum		= FILE_NONE;
	Res->Data			= NULL;
	Res->Flags			= 0;
	Res->FileDataSize	= 0;
	strcpy (Res->Name,Name);
	//
	Res->InitHeader();
	//
	UNGUARD("FGlobalResourceManager::InitResource");
	};

//
// Create a new resource of a certain type, and allocate its header but not
// its data.  Sets the RF_Modified flag. Retuns resource if ok, NULL if error.
//
// If Name is NULL, tries to create a resource with an arbitrary unique name.
//
UResource *FGlobalResourceManager::PrivateAlloc(const char *Name, EResourceType ResType, int Replace, DWORD SetFlags)
	{
	GUARD;
	FResourceType	*Type = &Types[ResType];
	UResource 		*Res;
	RESINDEX		ThisIndex;
	char			TempName[NAME_SIZE];
	//
	if (!Name) // Must create a unique name
		{
		GRes.MakeUniqueName(TempName,GRes.Types[ResType].Descr,"",RES_Camera);
		Name = TempName;
		};
	//
	// Validation check:
	//
	if (strlen(Name)>=NAME_SIZE) appErrorf("Name %s is too big",Name);
	else if (ResType>=(EResourceType)MaxTypes) appError ("Bad type");
	//
	// See if resource already exists:
	//
	Res = FindResource(Name,ResType,&ThisIndex);
	if (!Res) // Resource doesn't already exist
		{
		if (ThisIndex==RESINDEX_NONE) appErrorf("%s, resource table full",Name);
		Res = (UResource *)appMalloc(Type->HeaderSize,"Res(%s)",Name);
		}
	else // Resource already exists
		{
		if (!Replace) appErrorf("%s %s already exists",Types[ResType].Descr,Name);
		debugf(LOG_Res,"Replacing %s",Name);
		ThisIndex = Res->Index;
		Res->UnloadData();
		};
	InitResource(Res,ResType,ThisIndex,Name);
	Res->Flags = SetFlags | RF_Modified; // Prevent swapping as with loaded resources
	return Res;
	//
	UNGUARD("FGlobalResourceManager::PrivateAlloc");
	};

/*-----------------------------------------------------------------------------
   Cleanup functions
-----------------------------------------------------------------------------*/

//
// Callback for finding unused resources.
//
class FResourceCallbackTagUsed : public FResourceCallback
	{
	public:
	void Resource (UResource *ParentRes, UResource **Res, DWORD ContextFlags)
		{
		GUARD;
		if (*Res)
			{
			FResourceType *Type = &GRes.Types [(*Res)->Type];
			if ((*Res)->Flags & RF_Unused) // Only recurse the first time resource is claimed
				{
				(*Res)->Flags &= ~(RF_Unused);
				//
				(*Res)->QueryHeaderReferences	(*this);
				(*Res)->QueryDataReferences		(*this);
				};
			};
		UNGUARD("FResourceCallbackTagUsed::Resource");
		};
	void Name (UResource *ParentRes, FName *Name,  DWORD ContextFlags)
		{
		GUARD;
		if (*Name!=NAME_NONE)
			{
			GRes.Names[Name->Index].Flags &= ~(RF_Unused);
			};
		UNGUARD("FResourceCallbackTagUsed::Name");
		};
	};

//
// Find all unused resources and add them to the "Unused" array:
//
void FGlobalResourceManager::FindUnclaimed (void)
	{
	GUARD;
	//
	UnclaimedRes   = new("Unclaimed",CREATE_Replace)UArray (8192);
	UnclaimedNames = new("Unclaimed",CREATE_Replace)UEnum  (8192);
	//
	// Tag all resources as unused:
	//
	for (RESINDEX i=0; i<MaxRes; i++)
		{
		UResource *Res = ResArray[i];
		if (Res) Res->Flags |= RF_Unused;
		};
	for (RESINDEX i=0; i<MaxRes; i++)
		{
		if (Names[i].Name[0]) Names[i].Flags |= RF_Unused;
		};
	Root->Add(UnclaimedRes);
	Root->Add(UnclaimedNames);
	//
	// Recursively tag all used resources, starting at root:
	//
	FResourceCallbackTagUsed TagUsedCallback;
	TagUsedCallback.Resource(NULL,(UResource **)&Root,0);
	//
	// Add all unused, unempty resources to unclaimed array:
	//
	for (RESINDEX i=0; i<MaxRes; i++)
		{
		UResource *Res = ResArray[i];
		if (Res && (Res->Flags & RF_Unused)) UnclaimedRes->Add(Res);
		};
	//
	// Add all unused, unempty names to unclaimed array:
	//
	for (RESINDEX i=0; i<MaxNames; i++)
		{
		if (Names[i].Name[0] && (Names[i].Flags & RF_Unused)) UnclaimedNames->Add((FName)i);
		};
	UNGUARD("FGlobalResourceManager::DebugUnclaimed");
	};

//
// Free unused resources and names. Disposes of everything in "Unclaimed":
//
void FGlobalResourceManager::KillUnclaimed(void)
	{
	GUARD;
	if (UnclaimedRes)
		{
		for (int i=0; i<UnclaimedRes->Num; i++)
			{
			// Kill this unused resource:
			UnclaimedRes->Element(i)->Kill();
			};
		Root->Delete(UnclaimedRes);
		UnclaimedRes->Kill();
		UnclaimedRes = NULL;
		};
	if (UnclaimedNames)
		{
		for (int i=0; i<UnclaimedNames->Num; i++)
			{
			// Kill this unused name:
			Names[UnclaimedNames->Element(i).Index].Name[0] = 0;
			};
		Root->Delete(UnclaimedNames);
		UnclaimedNames->Kill();
		UnclaimedNames = NULL;
		};
	UNGUARD("FGlobalResourceManager::Cleanup");
	};

//
// Display all unclaimed names and resources:
//
void FGlobalResourceManager::DebugDumpUnclaimed(void)
	{
	GUARD;
	#ifdef RESOURCE_DEBUG
	if (UnclaimedRes)
		{
		for (int i=0; i<UnclaimedRes->Num; i++)
			{
			debugf(LOG_Res,"Unclaimed res: %s %s",
				UnclaimedRes->Element(i)->GetTypeName(),
				UnclaimedRes->Element(i)->Name);
			};
		};
	if (UnclaimedNames)
		{
		for (int i=0; i<UnclaimedNames->Num; i++)
			{
			debugf(LOG_Res,"Unclaimed name: %s",UnclaimedNames->Element(i).Name());
			};
		};
	#endif
	UNGUARD("FGlobalResourceManager::Cleanup");
	};

//
// Delete all unreferenced resources
//
void FGlobalResourceManager::Purge (int DebugDump)
	{
	GUARD;
	//
	#ifdef ENABLE_RESOURCE_PURGE
		if (!GDefaults.LaunchEditor) // Don't allow purge in UnrealEd
			{
			FindUnclaimed();
			if (DebugDump) DebugDumpUnclaimed();
			KillUnclaimed();
			};
	#endif
	//
	UNGUARD("FGlobalResourceManager::Purge");
	};

/*-----------------------------------------------------------------------------
	UTextBuffer implementation
-----------------------------------------------------------------------------*/

//
// Standard resource functions
//
void UTextBuffer::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UTextBuffer);
	Type->RecordSize = sizeof (char);
	Type->Version    = 1;
	Type->TypeFlags  = RTF_ScriptReferencable;
	strcpy (Type->Descr,"TextBuffer");
	UNGUARD("UTextBuffer::Register");
	};
void UTextBuffer::InitHeader(void)
	{
	UDatabase::InitHeader();
	Pos = 0;
	};
const char *UTextBuffer::Import(const char *Buffer, const char *BufferEnd,const char *FileType)
	{
	GUARD;
	//
	const char *RealBufferEnd = mystrstr(Buffer,"End Text");
	if (!RealBufferEnd) RealBufferEnd = BufferEnd;
	//
	// Skip junk at end:
	//
	while ((Buffer<RealBufferEnd) && ((*Buffer=='%')||(*Buffer=='\r')||(*Buffer=='\n'))) Buffer++;
	while ((RealBufferEnd>Buffer) && ((RealBufferEnd[-1]==' ')||(RealBufferEnd[-1]=='%')||(RealBufferEnd[-1]=='\r')||(RealBufferEnd[-1]=='\n'))) RealBufferEnd--;
	//
	// Keep last cr/lf:
	//
	if ((RealBufferEnd<BufferEnd) && ((RealBufferEnd[0]=='\r')||(RealBufferEnd[0]=='\n'))) RealBufferEnd++;
	if ((RealBufferEnd<BufferEnd) && ((RealBufferEnd[0]=='\r')||(RealBufferEnd[0]=='\n'))) RealBufferEnd++;
	//
	Max = (int)(RealBufferEnd-Buffer+1);
	Num = Max;
	Realloc();
	//
	char *Text = GetData();
	memcpy (Text,Buffer,Num-1);
	//
	Text[Num-1] = 0;
	return RealBufferEnd;
	//
	UNGUARD("UTextBuffer::Import");
	};
char *UTextBuffer::Export(char *Buffer,const char *FileType,int Indent)
	{
	GUARD;
	char *Start = GetData();
	char *End   = Start + Num-1;
	//
	while ((Start<End) && ((Start[0]=='\r')||(Start[0]=='\n')||(Start[0]==' '))) Start++;
	while ((End>Start) && ((End [-1]=='\r')||(End [-1]=='\n')||(End [-1]==' '))) End--;
	//
	int Len = (int)(End-Start);
	memcpy(Buffer,Start,Len);
	return Buffer+Len;
	UNGUARD("UTextBuffer::Export");
	};
AUTOREGISTER_RESOURCE(RES_TextBuffer,UTextBuffer,0xB2D90869,0xCCD211cf,0x91360000,0xC028B992);

/*-----------------------------------------------------------------------------
	UArray implementation
-----------------------------------------------------------------------------*/

//
// Standard resource functions
//
void UArray::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UArray);
	Type->RecordSize = sizeof (UResource *);
	Type->Version    = 1;
	Type->TypeFlags  = RTF_ScriptReferencable;
	strcpy (Type->Descr,"Array");
	UNGUARD("UArray::Register");
	};
void UArray::QueryDataReferences(FResourceCallback &Callback)
	{
	GUARD;
	for (int i=0; i<Num; i++) Callback.Resource (this,(UResource **)&Element(i),0);
	UNGUARD("UArray::QueryDataReferences");
	};
AUTOREGISTER_RESOURCE(RES_Array,UArray,0xB2D90870,0xCCD211cf,0x91360000,0xC028B992);

//
// Add an element to an array and return its index, or -1 if array is full:
//
int UArray::Add(UResource *NewElement)
	{
	GUARD;
	if (Num >= Max) appErrorf ("Array %s overflow (%i/%i)",Name,Num,Max);
	Element(Num++) = NewElement;
	return Num-1;
	UNGUARD("UArray::Add");
	};

//
// Delete an element from an array.  If it doesn't exist, ignores it.
//
void UArray::Delete(UResource *DelElement)
	{
	GUARD;
	int j=0;
	for (int i=0; i<Num; i++)
		{
		if (j != i) Element(j) = Element(i);
		if (Element(i) != DelElement) j++;
		};
	Num = j;
	UNGUARD("UArray::Delete");
	};

//
// Empty an array:
//
void UArray::Empty(void)
	{
	GUARD;
	Num = 0;
	UNGUARD("UArray::Empty");
	};

/*-----------------------------------------------------------------------------
	FName implementation
-----------------------------------------------------------------------------*/

//
// Add a name to table.  Names are never duplicated.
// Names are case-insensitive.  Should maintain a B-Tree index for
// faster access; the linear search is excessively slow.  If you call with
// a blank name or the string "NONE", the name is set to none, which is valid.
//
void FName::Add(const char *Name)
	{
	GUARD;
	FNameTableEntry	*TempName = &GRes.Names[0];
	FNameTableEntry	*Available;
	RESINDEX		i,AvailableIndex;
	//
	if ((stricmp(Name,"NONE")==0) || (Name[0]==0))
		{
		Index = INDEX_NONE;
		return;
		};
	if (strlen(Name)>=NAME_SIZE) appErrorf ("Name %s is too big",Name);
	//
	// Find name in table, while also finding first available blank name:
	//
	i=0;
	while (i<GRes.MaxNames)
		{
		if (*TempName->Name==0)
			{
			Available      = TempName;
			AvailableIndex = i;
			goto FoundAvailable;
			}
		else if (stricmp(Name,TempName->Name)==0)
			{
			Index = i;
			return;
			};
		i++; TempName++;
		};
	Index = INDEX_NONE;
	return;
	//
	// Found an available name, but didn't find name we're looking for.
	//
	FoundAvailable:
	///
	i++; TempName++;
	while (i<GRes.MaxNames)
		{
		if ((*TempName->Name!=0) && (stricmp(Name,TempName->Name)==0))
			{
			Index = i;
			return;
			};
		i++; TempName++;
		};
	//
	// Add name to table:
	//
	Available->Flags=0;
	strcpy (Available->Name,Name);
	//
	Index = AvailableIndex;
	//
	UNGUARD("FName::Add");
	};

//
// Add a hardcoded name to the name table.  Handle errors if not possible.
//
void FName::AddHardcoded(WORD NewIndex,const char *Name)
	{
	GUARD;
	FNameTableEntry *TempName = &GRes.Names[NewIndex];
	//
	if (NewIndex >= GRes.MaxNames)	appErrorf ("Index %i exceeds %i maximum",NewIndex,GRes.MaxNames);
	if (TempName->Name[0]!=0)		appErrorf ("Name %s is taken",Name);
	if (strlen(Name)>=NAME_SIZE)	appErrorf ("Name %s is too big",Name);
	//
	strcpy (TempName->Name,Name);
	Index = NewIndex;
	//
	UNGUARD("FName::AddHardcoded");
	};

//
// Find and set a name, return 1 if found, 0 if not found.
//
int FName::Find(const char *Name)
	{
	GUARD;
	FNameTableEntry *TempName=&GRes.Names[0];
	//
	// Find name in table, while also finding first available blank name:
	//
	if ((!Name[0]) || (!stricmp(Name,"NONE")))
		{
		Index = INDEX_NONE;
		return 1;
		};
	for (RESINDEX i=0; i<GRes.MaxNames; i++)
		{
		if (!stricmp(Name,TempName->Name))
			{
			Index = i;
			return 1;
			};
		TempName++;
		};
	Index = INDEX_NONE;
	return 0; // Didn't find name.
	UNGUARD("FName::Find");
	};

//
// Lookup a name and return it or a blank (not a NULL).
//
char *FName::Name(void)
	{
	GUARD;
	if (Index==INDEX_NONE)	return "None";
	else 					return GRes.Names[Index].Name;
	UNGUARD("FName::Name");
	};

/*-----------------------------------------------------------------------------
	UEnum implementation
-----------------------------------------------------------------------------*/

//
// Add an element to an array and return its index, or -1 if array is full:
//
int UEnum::Add (FName NewElement)
	{
	GUARD;
	if (Num >= Max) appErrorf ("Enum %s overflow (%i/%i)",Name,Num,Max);
	Element(Num++) = NewElement;
	return Num-1;
	UNGUARD("UEnum::Add");
	};

//
// Add a name element to an array and return its index, or -1 if array is full:
//
int UEnum::AddTag (const char *Name)
	{
	GUARD;
	if (Num >= Max) appErrorf ("Enum %s overflow (%i/%i)",Name,Num,Max);
	Element(Num++).Add(Name);
	return Num-1;
	UNGUARD("UEnum::Add");
	};

//
// Delete an element from an enum.  If it doesn't exist, ignores it.
//
void UEnum::Delete (FName DeleteElement)
	{
	GUARD;
	int j=0;
	for (int i=0; i<Num; i++)
		{
		if (j != i) Element(j) = Element(i);
		if (Element(i) != DeleteElement) j++;
		};
	Num = j;
	UNGUARD("UArray::Delete");
	};

//
// Empty an enum:
//
void UEnum::Empty (void)
	{
	GUARD;
	Num = 0;
	UNGUARD("UArray::Empty");
	};

//
// Resource functions
//
void UEnum::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UEnum);
	Type->RecordSize = sizeof (FName);
	Type->Version    = 1;
	Type->TypeFlags  = RTF_ScriptReferencable;
	strcpy (Type->Descr,"Enum");
	UNGUARD("UEnum::Register");
	};
void UEnum::QueryDataReferences(FResourceCallback &Callback)
	{
	GUARD;
	for (int i=0; i<Num; i++) Callback.Name (this,&Element(i),0);
	UNGUARD("UEnum::QueryDataReferences");
	};
const char *UEnum::Import(const char *Buffer, const char *BufferEnd,const char *FileType)
	{
	return Buffer;
	};
char *UEnum::Export(char *Buffer,const char *FileType,int Indent)
	{
	if (!stricmp(FileType,"H")) // C++ header
		{
		Buffer += sprintf(Buffer,"%senum %s {\r\n",spc(Indent),Name);
		for (int i=0; i<Num; i++)
			{
			Buffer += sprintf(Buffer,"%s    %-24s=%i,\r\n",spc(Indent),Element(i).Name(),i);
			};
		Buffer += sprintf(Buffer,"};\r\n\r\n");
		}
	else if (!stricmp(FileType,"TCX")) // Actor class text file
		{
		int LineLen = sprintf(Buffer,"%sEnumDef %s = ",spc(Indent),Name); Buffer += LineLen;
		for (int i=0; i<Num; i++)
			{
			int Len = sprintf(Buffer,"%s",Element(i).Name()); Buffer += Len; LineLen += Len;
			if (i<(Num-1))
				{
				Buffer += sprintf(Buffer,", "); LineLen += 2;
				if (LineLen>60)
					{
					Buffer += sprintf(Buffer,"_\r\n");
					LineLen = sprintf(Buffer,"   %s",spc(Indent)); Buffer += LineLen;
					};
				};
			};
		Buffer += sprintf(Buffer,"\r\n");
		};
	return Buffer;
	};
AUTOREGISTER_RESOURCE(RES_Enum,UEnum,0xB2D90871,0xCCD211cf,0x91360000,0xC028B992);

//
// Enumeration link topic function
//
AUTOREGISTER_TOPIC("Enum",EnumTopicHandler);
void EnumTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	GUARD;
	UEnum *Enum = new(Item,FIND_Optional)UEnum;
	if (Enum)
		{
		for (int i=0; i<Enum->Num; i++)
			{
			if (i>0) *Data++ = ',';
			Data += sprintf(Data,"%i - %s",i,Enum->Element(i).Name());
			};
		};
	UNGUARD("EnumTopicHandler::Get");
	};
void EnumTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
	{
	GUARD;
	UNGUARD("EnumTopicHandler::Set");
	};

/*-----------------------------------------------------------------------------
	UBuffer implementation
-----------------------------------------------------------------------------*/

//
// Resource functions
//
void UBuffer::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UBuffer);
	Type->RecordSize = sizeof (char);
	Type->Version    = 1;
	Type->TypeFlags  = RTF_ScriptReferencable;
	strcpy (Type->Descr,"Buffer");
	UNGUARD("UBuffer::Register");
	};
const char *UBuffer::Import(const char *Buffer, const char *BufferEnd, const char *FileType)
	{
	GUARD;
	Num = (int)(BufferEnd-Buffer);
	memcpy(GetData(),Buffer,Num);
	return Buffer + Num;
	UNGUARD("UBuffer::Import");
	};
char *UBuffer::Export(char *Buffer,const char *FileType,int Indent)
	{
	GUARD;
	memcpy(Buffer,GetData(),Num);
	return Buffer+Num;
	UNGUARD("UBuffer::Export");
	};
AUTOREGISTER_RESOURCE(RES_Buffer,UBuffer,0xB2D90872,0xCCD211cf,0x91360000,0xC028B992);

/*-----------------------------------------------------------------------------
	Resource list operations
-----------------------------------------------------------------------------*/

class FResList
	{
	public:
	WORD		NumEntries;
	WORD		CurEntry;
	UResource	**List;
	};

//
// Allocate a new list of resources
//
UNREAL_API FResList *resNewList (int MaxEntries)
	{
	GUARD;
	FResList *TempList;
	//
	TempList = (FResList *)appMalloc (sizeof (FResList) + MaxEntries * sizeof (UResource *),"ResNewList");
	TempList->NumEntries = 0;
	TempList->CurEntry   = 0;
	TempList->List       = (UResource **) &(TempList[1]);
	//
	return TempList;
	UNGUARD("resNewList");
	};

//
// Free a list of resources
//
UNREAL_API void resFreeList (FResList *List)
	{
	GUARD;
	appFree(List);
	UNGUARD("resFreeList");
	};

//
// Query a list of all resources with a certain name and type (may be
// wildcards).  Returns NULL if none were found.
//
UNREAL_API FResList *resQueryList (char *Name, EResourceType ResType)
	{
	GUARD;
	FResList  	*TempList;
	UResource	*Res;
	RESINDEX	i,n;
	//
	//	Count 'em:
	//
	n=0;
	for (i=GRes.MaxRes; i>0; i--)
		{
		Res = GRes.ResArray[i];
		if (Res)
			{
			if ((ResType==Res->Type) || (ResType==RES_All))
				if ((*Name==0) || (stricmp(Name,Res->Name)==0))
					n++;
			};
		};
	//
	// Allocate list if non-empty:
	//
	if (n==0) return NULL;
	TempList = resNewList(n);
	//
	// Build list:
	//
	for (i=0; i<GRes.MaxRes; i--)
		{
		Res = GRes.ResArray[i];
		if (Res)
			{
			if ((ResType==(EResourceType)Res->Type) || (ResType==RES_All))
				if ((*Name==0) || (stricmp(Name,Res->Name)==0))
					TempList->List[TempList->NumEntries++] = Res;
			};
		};
	//
	// Return list:
	//				
	return TempList;
	UNGUARD("resQueryList");
	};

/*-----------------------------------------------------------------------------
	Res Link topic handler function
-----------------------------------------------------------------------------*/

//
// Callback for counting children:
//
class FResourceCallbackCountChildren : public FResourceCallback
	{
	void Resource (UResource *ParentRes, UResource **Res, DWORD ContextFlags)
		{
		GUARD;
		if (*Res) GTempResChildCount [GNumResults]++;
		UNGUARD("FResourceCallbackCountChildren::Resource");
		};
	} GResCountChildren;

void AddToTempList (UResource *Res, int CountChildren)
	{
	GUARD;
	if (Res && (GNumResults < MAX_RES_RESULTS))
		{
		for (int i=0; i<GNumResults; i++) if (GTempResList[i]==Res) return; // Don't add duplicates
		//
		GTempResList       [GNumResults] = Res;
		GTempResChildCount [GNumResults] = 0;
		//
		if (CountChildren)
			{
			Res->QueryHeaderReferences	(GResCountChildren);
			Res->QueryDataReferences	(GResCountChildren);
			};
		GNumResults++;
		};
	UNGUARD("AddToTempList");
	};

//
// Link query callback
//
class FResourceCallbackLinkQuery : public FResourceCallback
	{
	void Resource (UResource *ParentRes, UResource **Res, DWORD ContextFlags)
		{
		GUARD;
		if (*Res)
			{
			if ((ParentRes==NULL) || (ParentRes==GRes.UnclaimedRes)) AddToTempList (*Res,0); // Don't count children
			else AddToTempList (*Res,1); // Count children
			};
		UNGUARD("FResourceCallbackLinkQuery::Resource");
		};
	} GResLinkQuery;

//
// Query a list of resources.  Call with resource or NULL=All.
//
void UNREAL_API resQueryForLink (UResource *Res,EResourceType OfType)
	{
	GUARD;
	if (GTempResList==NULL)
		{
		GTempResList       = (UResource **)appMalloc(MAX_RES_RESULTS * sizeof(UResource *),"ResQFL"); // Must free !!
		GTempResChildCount = (int        *)appMalloc(MAX_RES_RESULTS * sizeof(int        ),"ResTCC"); // Must free !!
		};
	GCurResult  = 0;
	GNumResults = 0;
	//
	if (!Res) // Query all resources
		{
		UResource *Res;
		FOR_ALL_TYPED_RES(Res,OfType,UResource)
			{
			AddToTempList (Res,0);
			}
		END_FOR_ALL_RES;
		}
	else // Query children of a particular resource
		{
		Res->QueryHeaderReferences	(GResLinkQuery);
		Res->QueryDataReferences	(GResLinkQuery);
		};
	UNGUARD("resQueryForLink");
	};

AUTOREGISTER_TOPIC("Res",ResTopicHandler);
void ResTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	GUARD;
	UResource		*Res;
	FResourceType	*Type;
	int				Children;
	//
	if (stricmp(Item,"QueryRes")==0)
		{
		if (GCurResult < GNumResults)
			{
			Res      = GTempResList       [GCurResult];
			Children = GTempResChildCount [GCurResult];
			Type     = &GRes.Types       [Res->Type];
			sprintf(Data,"%s %s|%s",
				Type->Descr,Res->Name,(Children==0)?"X":"C");
			GCurResult++;
			};
		};
	UNGUARD("ResTopicHandler::Get");
	};
void ResTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
	{
	GUARD;
	UNGUARD("ResTopicHandler::Set");
	};

/*-----------------------------------------------------------------------------
	Text Link topic handler function
-----------------------------------------------------------------------------*/

AUTOREGISTER_TOPIC("Text",TextTopicHandler);
void TextTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	GUARD;
	UTextBuffer	*Text = new(Item,FIND_Optional)UTextBuffer;
	if (Text && (Text->Max>0)) strcpy(Data,Text->GetData());
	UNGUARD("TextTopicHandler::Get");
	};
void TextTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
	{
	GUARD;
	int Num = strlen(Data)+1;
	UTextBuffer	*Text = new(Item,CREATE_Replace)UTextBuffer(Num,1);
	strcpy(Text->GetData(),Data);
	UNGUARD("TextTopicHandler::Set");
	};

AUTOREGISTER_TOPIC("TextPos",TextPosTopicHandler);
void TextPosTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	GUARD;
	UTextBuffer *Text = new(Item,FIND_Optional)UTextBuffer;
	if (Text) itoa(Text->Pos,Data,10);
	UNGUARD("TextPosTopicHandler::Get");
	};
void TextPosTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
	{
	GUARD;
	UTextBuffer *Text = new(Item,FIND_Optional)UTextBuffer;
	if (Text) Text->Pos = atoi(Data);
	UNGUARD("TextPosTopicHandler::Set");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
