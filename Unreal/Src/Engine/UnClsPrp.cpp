/*=============================================================================
	UnClsPrp.cpp: FClassProperty implementation

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnScript.h"

/*-----------------------------------------------------------------------------
	Main FClassProperty functions
-----------------------------------------------------------------------------*/

//
// Initialize all FClassProperty members
//
void FClassProperty::Init(void)
	{
	GUARD;
	PropertyName		= NAME_NONE;
	PropertyCategory	= NAME_NONE;
	PropertyType		= CPT_None;
	PropertyOffset		= 0;
	PropertyArrayDim    = 0;
	PropertyElementSize = 0;
	PropertySize		= 0;
	PropertyFlags		= 0;
	UNGUARD("FClassProperty::Init");
	};

//
// See if Override is a valid flag name and, if so, set the associated
// property flags and return 1.  Returns 0 if override is not a valid flag name.
//
int FClassProperty::SetFlags(const char *Override)
	{
	GUARD;
	if (!stricmp(Override,"Private"))
		{
		PropertyFlags |= CPF_Private;
		return 1;
		}
	else if (!stricmp(Override,"NoSave"))
		{
		PropertyFlags |= CPF_NoSaveResource;
		return 1;
		}
	else if (!stricmp(Override,"Const"))
		{
		PropertyFlags |= CPF_Const;
		return 1;
		}
	else if (!stricmp(Override,"Editable"))
		{
		PropertyFlags |= CPF_Edit | CPF_EditInPlay;
		return 1;
		}
	else if (!stricmp(Override,"ExportResource"))
		{
		PropertyFlags |= CPF_ExportResource;
		return 1;
		}
	else return 0;
	UNGUARD("FClassProperty::SetFlags");
	};

//
// Set the class property's type based on the value of Type.  Returns 1 if success,
// or 0 if Type is unrecognized.
//
int FClassProperty::SetType(const char *TypeStr,int ArrayDim,char *Error)
	{
	GUARD;
	//
	// Set up array info:
	//
	if (ArrayDim<0)
		{
		sprintf(Error,"Array size must be positive");
		return 0;
		};
	PropertyArrayDim = ArrayDim;
	//
	// Check all base types
	//
	if (!stricmp(TypeStr,"BYTE")) // Intrinsic BYTE type
		{
		PropertyType	= CPT_Byte;
		Enum			= NULL;
		return 1;
		}
	if (!stricmp(TypeStr,"INTEGER")) // Intrinsic INTEGER type
		{
		PropertyType	= CPT_Integer;
		return 1;
		}
	if (!stricmp(TypeStr,"BOOLEAN")) // Intrinsic FLAG type
		{
		if (ArrayDim!=0)
			{
			sprintf(Error,"Boolean arrays aren't allowed");
			return 0;
			};
		PropertyType	= CPT_Boolean;
		return 1;
		}
	if (!stricmp(TypeStr,"REAL")) // Intrinsic REAL type
		{
		PropertyType	= CPT_Real;
		return 1;
		}
	if (!stricmp(TypeStr,"ACTOR")) // Intrinsic ACTOR type
		{
		PropertyType	= CPT_Actor;
		Class           = NULL; // Allow any kind of class
		return 1;
		}
	if (!stricmp(TypeStr,"NAME")) // Intrinsic NAME type
		{
		PropertyType	= CPT_Name;
		return 1;
		}
	if (!stricmp(TypeStr,"STRING")) // Intrinsic STRING type
		{
		if (ArrayDim==0)
			{
			sprintf(Error,"Must specify string dimension like: Dim S as String(20)");
			return 0;
			};
		PropertyType	= CPT_String;
		return 1;
		}
	if (!stricmp(TypeStr,"VECTOR")) // Intrinsic VECTOR type
		{
		PropertyType	= CPT_Vector;
		return 1;
		}
	if (!stricmp(TypeStr,"ROTATION")) // Intrinsic ROTATION type
		{
		PropertyType	= CPT_Rotation;
		return 1;
		};
	//
	// See if it's a resource type
	//
	EResourceType ResType = GRes.LookupType(TypeStr);
	if ((ResType != RES_None) && (GRes.Types[ResType].TypeFlags & RTF_ScriptReferencable))
		{
		PropertyType	= CPT_Resource;
		PropertyResType = ResType;
		return 1;
		};
	//
	// See if it's an actor class type
	//
	UClass *TempClass = new(TypeStr,FIND_Optional)UClass;
	if (TempClass)
		{
		PropertyType	= CPT_Actor;
		Class			= TempClass; // May only hold a reference to an actor of this kind
		return 1;
		};	
	//
	// See if it's an enumerated type
	//
	UEnum *TempEnum = new(TypeStr,FIND_Optional)UEnum;
	if (TempEnum)
		{
		PropertyType	= CPT_Byte;
		Enum			= TempEnum;
		return 1;
		};	
	//
	// No matching type was found
	//
	sprintf(Error,"Unrecognized variable type");
 	return 0;
	UNGUARD("FClassProperty::SetType");
	};

//
// Add this class property to a property frame.  Verifies that all required members are
// set to valid values, and adjusts any of this property's values that need to be adjusted
// based on the frame, i.e. by merging adjacent bit flags into DWORD's.  Returns pointer
// to the property in the frame if success, NULL if property could not be added.  Any 
// modifications are applied to this class property before adding it to the frame, 
// so the value of this property upon return are sufficient for initializing the data by 
// calling InitPropertyData.
//
// Assumes that there is enough room in the FrameProperties array to add the specified property.
//
// However, doesn't check to see that there are enough data bytes available to hold it.
//
FClassProperty *FClassProperty::AddToFrame(int *NumFrameProperties,FClassProperty *FrameProperties)
	{
	GUARD;
	FClassProperty *Property = &FrameProperties[(*NumFrameProperties)++];
	//
	// Set all values of Property from this:
	//
	*Property = *this;
	//
	// Set length and any special items required:
	//
	switch (PropertyType)
		{
		case CPT_Byte:
			PropertyElementSize = sizeof(BYTE);
			break;
		case CPT_Integer:
			PropertyElementSize = sizeof(INT);
			break;
		case CPT_Boolean:
			if ((*NumFrameProperties > 1) && (Property[-1].PropertyType==CPT_Boolean) &&
				(Property[-1].BitMask<<1))
				{
				// Continue bit flag from the previous DWORD:
				BitMask = Property[-1].BitMask << 1;
				Property[-1].PropertySize        = 0;
				Property[-1].PropertyElementSize = 0;
				}
			else // Create a new DWORD for this bit field:
				{
				BitMask = 1;
				};
			PropertyElementSize = 4;
			//bug ("Bitmask %s %i",PropertyName.Name(),BitMask);
			break;
		case CPT_Real:
			PropertyElementSize = sizeof(FLOAT);
			break;
		case CPT_Actor:
			PropertyElementSize = sizeof(INDEX);
			break;
		case CPT_Resource:
			PropertyElementSize = sizeof(UResource *);
			break;
		case CPT_Name:
			PropertyElementSize = sizeof(FName);
			break;
		case CPT_String:
			PropertyElementSize = 1;
			break;
		case CPT_Vector:
			PropertyElementSize = sizeof(FVector);
			break;
		case CPT_Rotation:
			PropertyElementSize = sizeof(FRotation);
			break;
		default:
			appErrorf("Unknown property type",PropertyName.Name());
			break;
		};
	//
	// Compute total size, in case this is an array:
	//
	PropertySize = ComputePropertySize();
	//
	// Set the offset:
	//
	if (*NumFrameProperties!=1)
		{
		PropertyOffset = Property[-1].PropertyOffset + Property[-1].PropertySize;
		//if (PropertyOffset < Property[-1].PropertyOffset) appError ("Property overflow");
		}
	else PropertyOffset = 0;
	//
	*Property = *this;
	return Property;
	UNGUARD("FClassProperty::AddToFrame");
	};

/*-----------------------------------------------------------------------------
	Data management
-----------------------------------------------------------------------------*/

//
// Initialize a class property value.  FrameDataStart is a pointer to the beginning
// of the property frame where this property resides.
//
int FClassProperty::InitPropertyData(BYTE *FrameDataStart,FToken *InitToken)
	{
	GUARD;
	//
	BYTE *Data = &FrameDataStart[PropertyOffset];
	int  n     = PropertyArrayDim ? PropertyArrayDim : 1;
	int  i;
	//
	// Init each element of array (or the single element, if not an array):
	//
	switch (PropertyType)
		{
		case CPT_Byte:
			{
			int v=0;
			if (InitToken && !InitToken->GetInteger(&v)) return 0;
			for (i=0; i<n; i++)
				{
				*(BYTE *)Data = v;
				Data += sizeof(BYTE);
				};
			};
			break;
		case CPT_Integer:
			{
			int v=0;
			if (InitToken && !InitToken->GetInteger(&v)) return 0;
			for (i=0; i<n; i++)
				{
				*(INT *)Data = v;
				Data += sizeof(INT);
				};
			};
			break;
		case CPT_Boolean:	
			{
			// Note that boolean arrays aren't allowed
			int v=0;
			if (InitToken && !InitToken->GetInteger(&v)) return 0;
			if (v) *(DWORD *)Data |=  BitMask;
			else   *(DWORD *)Data &= ~BitMask;
			};
			break;
		case CPT_Real:
			{
			FLOAT v=0;
			if (InitToken && !InitToken->GetFloat(&v)) return 0;
			for (i=0; i<n; i++)
				{
				*(FLOAT *)Data = v;
				Data += sizeof(FLOAT);
				};
			};
			break;
		case CPT_Actor:
			{
			if (InitToken) return 0; // Actor initializers aren't allowed
			for (i=0; i<n; i++)
				{
				*(INDEX *)Data = INDEX_NONE;
				Data += sizeof(INDEX);
				};
			};
			break;
		case CPT_Resource:
			{
			UResource *R = NULL;
			if (InitToken && !InitToken->GetResource(&R,PropertyResType)) return 0;
			for (i=0; i<n; i++)
				{
				*(UResource **)Data = R;
				Data += sizeof(UResource *);
				};
			};
			break;
		case CPT_Name:
			{
			FName Name=NAME_NONE;
			if (InitToken && !InitToken->GetName(&Name)) return 0;
			for (i=0; i<n; i++)
				{
				*(FName *)Data = Name;
				Data += sizeof(FName);
				};
			};
			break;
		case CPT_String:
			{
			mymemset(Data,0,n*sizeof(char));
			if (!PropertyArrayDim) return 0; // Non-arrays of characters are not allowed
			if (InitToken)
				{
				if (InitToken->Type!=TOKEN_StringConst) return 0;
				mystrncpy((char *)Data,InitToken->String,PropertyArrayDim);
				};
			};
			break;
		case CPT_Vector:
			{
			FVector V = GMath.ZeroVector;
			if (InitToken && !InitToken->GetVector(&V)) return 0;
			for (i=0; i<n; i++)
				{
				*(FVector *)Data = V;
				Data += sizeof(FVector);
				};
			};
			break;
		case CPT_Rotation:
			{
			FRotation R = GMath.ZeroRotation;
			if (InitToken && !InitToken->GetRotation(&R)) return 0;
			for (i=0; i<n; i++)
				{
				*(FRotation *)Data = R;
				Data += sizeof(FRotation);
				};
			};
			break;
		default:
			{
			appErrorf("Unknown property type",PropertyName.Name());
			};
			break;
		};
	return 1;
	UNGUARD("FClassProperty::InitPropertyData");
	};

//
// Compare a common property that exists in two compatible actors.
// Returns 1 if equal, 0 if not equal.
//
int FClassProperty::Compare(const AActor *RawActor1, const AActor *RawActor2, int Element)
	{
	GUARD;
	int			Offset	= PropertyOffset + Element * PropertyElementSize;
	const BYTE	*P1		= &((BYTE *)RawActor1)[Offset];
	const BYTE	*P2		= &((BYTE *)RawActor2)[Offset];
	//
	switch(PropertyType)
		{
		case CPT_None:
			return 0;
		case CPT_Byte:
			return *(BYTE *)P1 == *(BYTE *)P2;
		case CPT_Integer:
			return *(INT *)P1 == *(INT *)P2;
		case CPT_Boolean:
			return ((*(INT *)P1 ^ *(INT *)P2) & BitMask) == 0;
		case CPT_Real:
			return *(FLOAT *)P1 == *(FLOAT *)P2;
		case CPT_Actor:
			return *(INDEX *)P1 == *(INDEX *)P2;
		case CPT_Resource:
			return *(UResource **)P1 == *(UResource **)P2;
		case CPT_Name:
			return *(FName *)P1 == *(FName *)P2;
		case CPT_String:
			return strcmp((char *)P1,(char *)P2)==0;
		case CPT_Vector:
			return *(FVector *)P1 == *(FVector *)P2;
		case CPT_Rotation:
			return *(FRotation *)P1 == *(FRotation *)P2;
		default:
			appErrorf("Bad property type %i",PropertyType);
			return 0;
		};
	UNGUARD("FClassProperty::Compare");
	};

//
// Check and see if a class property value isn't in it's empty/null/zero
// state.  Returns 1 if true, 0 if empty/null/zero.
//
int FClassProperty::IsTrue(const BYTE *FrameDataStart)
	{
	GUARD;
	const BYTE *P = &FrameDataStart[PropertyOffset + 0 * PropertyElementSize];
	//
	switch(PropertyType)
		{
		case CPT_None:		return 1;
		case CPT_Byte:		return *(BYTE		*)P != 0;
		case CPT_Integer:	return *(INT		*)P != 0;
		case CPT_Boolean:	return (*(INT *)P & BitMask) != 0;
		case CPT_Real:		return *(FLOAT		*)P != 0.0;
		case CPT_Actor:		return *(INDEX		*)P != INDEX_NONE;
		case CPT_Resource:	return *(UResource **)P != NULL;
		case CPT_Name:		return *(FName      *)P != NAME_NONE;
		case CPT_String:	return *(char       *)P != 0;
		case CPT_Vector:	return *(FVector    *)P != GMath.ZeroVector;
		case CPT_Rotation:	return *(FRotation  *)P != GMath.ZeroRotation;
		default:			appErrorf("Bad property type %i",PropertyType); return 0;
		};
	UNGUARD("FClassProperty::IsTrue");
	};

/*-----------------------------------------------------------------------------
	Exporting
-----------------------------------------------------------------------------*/

//
// Export this class property to a buffer, using the native text .TCX format.
// Returns the new end-of-buffer pointer.
//
char *FClassProperty::ExportTCX(char *Buffer,BYTE *Data)
	{
	GUARD;
	char Extra[80]=" ";
	int Len=0;
	//
	if (!(PropertyFlags & (CPF_Param | CPF_ReturnValue)))
		{
		Buffer += sprintf(Buffer,"Dim ");
		if (PropertyFlags & CPF_Private)		strcat(Extra,"Private ");
		if (PropertyFlags & CPF_Const)			strcat(Extra,"Const ");
		if (PropertyFlags & CPF_Edit)			strcat(Extra,"Editable ");
		if (PropertyFlags & CPF_NoSaveResource)	strcat(Extra,"NoSave ");
		if (PropertyFlags & CPF_ExportResource)	strcat(Extra,"ExportResource ");
		};
	if (!(PropertyFlags & CPF_ReturnValue)) Buffer += sprintf(Buffer,"%s ",PropertyName.Name());
	switch(PropertyType)
		{
		case CPT_Byte:		
			if (Enum)	Buffer+=sprintf(Buffer,"as%s%s",          Extra,Enum->Name);
			else		Buffer+=sprintf(Buffer,"as%sByte",        Extra); 
			break;
		case CPT_Integer:	Buffer+=sprintf(Buffer,"as%sInteger", Extra); break;
		case CPT_Boolean:	Buffer+=sprintf(Buffer,"as%sBoolean", Extra); break;
		case CPT_Real:		Buffer+=sprintf(Buffer,"as%sReal",    Extra); break;
		case CPT_Actor:		Buffer+=sprintf(Buffer,"as%sActor",   Extra); break;
		case CPT_Resource:	Buffer+=sprintf(Buffer,"as%s%s",      Extra,GRes.Types[PropertyResType].Descr); break;
		case CPT_Name:		Buffer+=sprintf(Buffer,"as%sName",    Extra); break;
		case CPT_String:	Buffer+=sprintf(Buffer,"as%sString",  Extra); break;
		case CPT_Vector:	Buffer+=sprintf(Buffer,"as%sVector",  Extra); break;
		case CPT_Rotation:	Buffer+=sprintf(Buffer,"as%sRotation",Extra); break;
		default:			Buffer+=sprintf(Buffer,"#Error"); break;
		};
	if (PropertyArrayDim)
		{
		Buffer += sprintf(Buffer,"(%i)",PropertyArrayDim);
		};
	//
	// If the property is not editable, and its initializer value is non-default, should
	// append "=Value" to it:
	//
	if (Data && (!(PropertyFlags & CPF_Edit)) && IsTrue(Data))
		{
		char Type[256],Name[256],Value[256];
		ExportActorProperty(Type,Name,Value,this,(AActor *)Data,0,0,0,NULL,NAME_NONE);
		Buffer += sprintf(Buffer,"=%s",Value);
		};
	return Buffer;
	UNGUARD("FClassProperty::ExportTCX");
	};

//
// Export this class property to a buffer as a C++ header file.
// Returns the new end-of-buffer pointer.
//
char *FClassProperty::ExportH(char *Buffer)
	{
	GUARD;
	int Len=0;
	char ArrayStr[80]="";
	//
	if (PropertyArrayDim) sprintf(ArrayStr,"[%i]",PropertyArrayDim);
	//
	switch(PropertyType)
		{
		case CPT_Byte:
			if (Enum)	Buffer+=sprintf(Buffer,"BYTE       %-24s%s /* %-20s */;",PropertyName.Name(),ArrayStr,Enum->Name);
			else		Buffer+=sprintf(Buffer,"BYTE       %s%s;",      PropertyName.Name(),ArrayStr);
			break;
		case CPT_Integer:	Buffer+=sprintf(Buffer,"int        %s%s;",  PropertyName.Name(),ArrayStr); break;
		case CPT_Boolean:	Buffer+=sprintf(Buffer,"DWORD      %s%s:1;",PropertyName.Name(),ArrayStr); break;
		case CPT_Real:		Buffer+=sprintf(Buffer,"FLOAT      %s%s;",  PropertyName.Name(),ArrayStr); break;
		case CPT_Actor:		Buffer+=sprintf(Buffer,"INDEX      i%s%s;", PropertyName.Name(),ArrayStr); break;
		case CPT_Resource:	Buffer+=sprintf(Buffer,"U%-9s *%s%s;",GRes.Types[PropertyResType].Descr,PropertyName.Name(),ArrayStr); break;
		case CPT_Name:		Buffer+=sprintf(Buffer,"FName      %s%s;",  PropertyName.Name(),ArrayStr); break;
		case CPT_String:	Buffer+=sprintf(Buffer,"char       %s%s;",  PropertyName.Name(),ArrayStr); break;
		case CPT_Vector:	Buffer+=sprintf(Buffer,"FVector    %s%s;",  PropertyName.Name(),ArrayStr); break;
		case CPT_Rotation:	Buffer+=sprintf(Buffer,"FRotation  %s%s;",  PropertyName.Name(),ArrayStr); break;
		};
	return Buffer;
	UNGUARD("FClassProperty::ExportH");
	};

/*-----------------------------------------------------------------------------
	Utility functions
-----------------------------------------------------------------------------*/

char *GetPropertyTypeName(EClassPropertyType Type)
	{
	GUARD;
	//
	static char *PropertyTypeNames[CPT_MAX] =
		{
		"None","Byte","Integer","Boolean",
		"Real","Actor","Resource","Name","String","Vector","Rotation"
		};
	if (Type>=CPT_MAX) appError("Invalid property type");
	return PropertyTypeNames[Type];
	//
	UNGUARD("GetPropertyTypeName");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
