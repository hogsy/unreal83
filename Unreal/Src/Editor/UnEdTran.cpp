/*=============================================================================
	UnEdTran.cpp: Unreal transaction-tracking functions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	The functions here are used for tracking modifications to the Bsp, editor
	solids, actors, etc., to support "Undo", "Redo", and automatically aborting
	operations which cause problems (while undoing their changes).

	See end of file for more information about all things tracked.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

/*-----------------------------------------------------------------------------
	All transaction-related data types (only used by unedtran.c)
-----------------------------------------------------------------------------*/

class FTransaction				// Transaction (one entry per transaction)
	{
	public:
	int	TransCount;				// Transaction number, starts at 0 when loaded and counts up
	int	StartDataOffset;		// Data offset where transaction data starts
	int	TotalDataSize;			// Total size of all data for the transaction
	int	NumChanges;				// Number of changes thus far
	char Name[80];				// Description of transaction
	};

class FTransChangeLog  				// Log of additions/deletions/modifications to records
	{
	public:
	UResource		*Res;		// Resource operated on
	EResourceType	ResType;	// Type of resource operated on, for checking
	int				Index;		// Index of element
	int				TransCount;	// Transaction number the change corresponds to
	int				DataOffset;	// Offset of data in data buffer
	int				DataSize;	// Size of this change's data in data buffer
	};

//
// Globals:
//
char *GTransTypeNames[] = {"Actor","BrushLoc","BrushPoly","BspNode","BspSurf"};

/*-----------------------------------------------------------------------------
	Lock, Unlock, and Reset
-----------------------------------------------------------------------------*/

//
// Lock the transaction tracking resource and get a local copy of its info
// into TRANS.
//
void UTransBuffer::Lock(void)
	{
	GUARD;
	DWORD			TransactionSize	= MaxTrans 		* sizeof (FTransaction);
	DWORD			ChangeLogSize	= MaxChanges	* sizeof (FTransChangeLog);
	DWORD			DataSize		= MaxDataOffset;
	//
	if (!GApp->ServerAlive) return;
	if (Locked) appError ("Already locked");
	//
	Locked 			= 1;
	Transactions 	= (FTransaction		*)((BYTE *)Data + 0);
	ChangeLog 		= (FTransChangeLog  *)((BYTE *)Data + TransactionSize);
	Buffer 			= (BYTE				*)((BYTE *)Data + TransactionSize + ChangeLogSize);
	//
	UNGUARD("UTransBuffer::Lock");
	};

//
// Copy the local TRANS stuff into the transaction tracking resource and
// unlock it:
//
void UTransBuffer::Unlock (void)
	{
	GUARD;
	if (!GApp->ServerAlive) return;
	if (!Locked) appError ("Not locked");
	Locked = 0;
	UNGUARD("UTransBuffer::Unlock");
	};

//
// Reset the transaction tracking system, clearing all transactions (so that
// following undo/redo operations can't occur).  This is called before a
// routine that invalidates the main set of level/transaction data, for example
// before rebuilding the Bsp.
//
// Action = description of reason for reset (i.e. "loading level", "rebuilding Bsp")
//
void UTransBuffer::Reset (const char *Action)
	{
	GUARD;
	//
	if (!GApp->ServerAlive) return;
	Lock();
	//
	strcpy (ResetAction,Action);
	//
	Locked			= 0;
	NumTrans		= 0;
	NumChanges		= 0;
	Overflow 		= 0;
	TransCount		= 0;
	UndoTransCount	= 0;
	//
	debugf (LOG_Trans,"Transaction buffer %s reset",Name);
	UNGUARD("UTransBuffer::Reset");
	};

/*-----------------------------------------------------------------------------
	Deletion
-----------------------------------------------------------------------------*/

//
// Delete the first transaction from the transaction list, remove all of its entries
// from the change log and data tables, and scroll the list down.
//
void UTransBuffer::DeleteFirstTransaction(void)
	{
	GUARD;
	FTransaction 	*FirstTrans,*LastTrans;
	int				MinTransCount;
	int				MinDataOffset,MaxDataOffset;
	int 			i,j;
	//
	if (!GApp->ServerAlive)	return;
	if (!Locked)				appError ("Not locked");
	if (NumTrans==0)			appError ("NumTrans==0");
	//
	// Remove from *Transactions:
	//
	for (i=1; i<NumTrans; i++) Transactions[i-1] = Transactions[i];
	NumTrans--;
	//
	if (NumTrans==0) // All transactions are gone, so just empty everything:
		{
		NumChanges = 0;
		}
	else
		{
		FirstTrans = &Transactions [0];
		LastTrans  = &Transactions [NumTrans-1];
		//
		MinTransCount = FirstTrans->TransCount;
		//
		MinDataOffset = FirstTrans->StartDataOffset;
		MaxDataOffset = LastTrans ->StartDataOffset + LastTrans->TotalDataSize;
		//
		// Move data and update data offsets for all remaining transactions:
		//
		memmove (Buffer, Buffer + MinDataOffset, MaxDataOffset-MinDataOffset);
		for (i=0; i<NumTrans; i++) Transactions[i].StartDataOffset -= MinDataOffset;
		//
		// Delete all old entries from TRANS.ChangeLog:
		//
		j=0;
		for (i=0; i<NumChanges; i++)
			{
			if (ChangeLog[i].TransCount >= MinTransCount)
				{
				if (j != i) ChangeLog[j] = ChangeLog[i];
				j++;
				}
			else {}; // The entry has been passed over (deleted)
			};
		NumChanges = j;
		};
	UNGUARD("UTransBuffer::DeleteFirstTransaction");
	};

/*-----------------------------------------------------------------------------
	Begin, End, Rollback, Continue
-----------------------------------------------------------------------------*/

//
// Begin a new transaction.  If any undone transactions haven't been redone,
// they will be overwritten.
//
void UTransBuffer::Begin (ULevel *Level, const char *SessionName)
	{
	GUARD;
	FTransaction	*Transaction;
	int				Index,Count;
	//
	// Check existing transaction status:
	//
	if (!GApp->ServerAlive) return;
	if (Locked) appErrorf ("%s: Transaction already active",SessionName);
	//
	Lock();
	Overflow = 0;
	//
	//debugf (LOG_Trans,"Begin trans %s",SessionName);
	//
	// If transaction list is full, must scroll it back by deleting first entry:
	//
	if (NumTrans >= MaxTrans) DeleteFirstTransaction();
	//
	// If undo without matching redo's, cut off the rest of the list:
	//
	Count = 0;
	for (int i=0; i<NumTrans; i++)
		{
		if (Transactions[i].TransCount >= UndoTransCount)
			{
			NumTrans    = i;
			TransCount  = UndoTransCount;
			break;
			}
		else
			{
			Count += Transactions[i].NumChanges;
			};
		};
	NumChanges = Count;
	//
	// Fill in *Transaction information:
	//
	Index                	= NumTrans++;
	Transaction				= &Transactions[Index];
	Transaction->TransCount	= TransCount++;
	Transaction->NumChanges = 0;
	//
	strcpy (Transaction->Name,SessionName);
	Transaction->TotalDataSize = 0; // Will be increased as changes are logged
	// 
	// Set undo pointer to very top of transaction list:
	//
	UndoTransCount    = TransCount;
	//
	// Calc starting data offset:
	//
	if (NumTrans==1)
		{
		Transaction->StartDataOffset = 0;
		}
	else
		{
		Transaction->StartDataOffset =
		Transactions [Index-1].StartDataOffset +
		Transactions [Index-1].TotalDataSize;
		};
	//
	// Save all resource headers related to this level for transactional undo/redo:
	//
	NoteResHeader (Level->Model);
	NoteResHeader (Level->ActorList);
	NoteResHeader (Level->Model->BspNodes);
	NoteResHeader (Level->Model->BspSurfs);
	NoteResHeader (Level->Model->Polys);
	//
	UNGUARD("UTransBuffer::Begin");
	};

//
// End a transaction
//
void UTransBuffer::End (void)
	{
	GUARD;
	//
	if (!GApp->ServerAlive) return;
	//
	// Check existing transaction status:
	//
	if (!Locked) appError ("No transaction is in progress");
	//
	Unlock();
	if (Overflow)
		{
		debugf (LOG_Trans,"End overflowed transaction");
		Reset("Undo buffer filled up");
		}
	else
		{
		//debugf (LOG_Trans,"End transaction");
		};
	UNGUARD("UTransBuffer::End");
	};

void UTransBuffer::ForceOverflow (const char *Reason)
	{
	GUARD;
	if (!GApp->ServerAlive) return;
	Overflow = 1;
	UNGUARD("UTransBuffer::ForceOverflow");
	};

//
// Abort a transaction and restore original state
//
void UTransBuffer::Rollback(void)
	{
	GUARD;
	//
	if (!GApp->ServerAlive) return;
	if (!Locked) appError ("transRollback: Not active");
	//
	if (Overflow)
		{
		End(); // Can't rollback overflowed transactions because data is gone
		}
	else
		{
		End();
		Undo();
		//
		// Prevent rolled-back transaction from being redone:
		//
		if (NumTrans>0)		NumTrans--;
		if (TransCount>0)	TransCount--;
		//
		UndoTransCount = TransCount;
		//
		// Wipe out the rolled-back transaction's changes:
		//
		NumChanges = 0;
		for (int i=0; i<NumTrans; i++) NumChanges += Transactions[i].NumChanges;
		};
	UNGUARD("UTransBuffer::Rollback");
	};

//
// Reopen the previous transaction and continue it
//
void UTransBuffer::Continue (void)
	{
	GUARD;
	//
	if (!GApp->ServerAlive) return;
	if (Locked) appError ("Already active");
	//
	debugf (LOG_Trans,"Continue previous transaction");
	//
	Lock();
	//
	if (NumTrans==0)	Overflow    = 1; // Assume previous transaction overflowed
	else				Overflow 	= 0;
	UNGUARD("UTransBuffer::Continue");
	};

/*-----------------------------------------------------------------------------
	Applying Undo/Redo changes
-----------------------------------------------------------------------------*/

//
// Swap memory in one small buffer with another
//
void memswap (void *Ptr1, void *Ptr2, DWORD Size)
	{
	GUARD;
	BYTE *Temp = (BYTE *)GMem.Get(Size);
	memcpy (Temp,Ptr1,Size);
	memcpy (Ptr1,Ptr2,Size);
	memcpy (Ptr2,Temp,Size);
	GMem.Release(Temp);
	UNGUARD("memswap");
	};

//
// Begin applying changes.
//
void UTransBuffer::BeginChanges (FTransaction *Trans)
	{
	GUARD;
	UResource *Res;
	//
	// Tag all resources as "not affected by transaction tracking system":
	//
	FOR_ALL_RES(Res)
		{
		Res->Flags &= ~(RF_TransHeader | RF_TransData);
		}
	END_FOR_ALL_RES;
	//
	UNGUARD("UTransBuffer::BeginChanges");
	};

//
// End applying changes and do any necessary cleanup work.
//
void UTransBuffer::EndChanges (FTransaction *Trans)
	{
	GUARD;
	IModel ModelInfo;
	//
	// Cleanup any resources that need cleaning up:
	//
	UModel *Model;
	FOR_ALL_TYPED_RES(Model,RES_Model,UModel)
		{
		if (Model->Flags & RF_TransHeader)
			{
			//
			// If model's Bsp was changed, rebuild bounding boxes since changes to them
			// aren't always logged in the transaction tracking system:
			//
			Model->Lock (&ModelInfo,LOCK_NoTrans);
			if (ModelInfo.BspNodes && ModelInfo.BspSurfs)
				{
				UBspNodes *BspNodes = ModelInfo.BspNodesResource;
				UBspSurfs *BspSurfs = ModelInfo.BspSurfsResource;
				//
				if ((BspNodes->Flags & RF_TransData) || (BspSurfs->Flags & RF_TransData))
					{
					GUnrealEditor.bspBuildBounds (&ModelInfo);
					};
				};
			Model->Unlock(&ModelInfo);
			};
		}
	END_FOR_ALL_RES;
	//
	UNGUARD("UTransBuffer::EndChanges");
	};

//
// Apply one logged change, Undo = 1 (Undo), 0 (Redo):
//
void UTransBuffer::ApplyChange (FTransChangeLog *Change, BYTE *SourcePtr, DWORD DataSize)
	{
	GUARD;
	UResource		*Res   	= Change->Res;
	FResourceType	*Type  	= &GRes.Types [Res->Type];
	BYTE			*DestPtr;
	//
	// Validate everything and set up dest data pointer:
	//
	if (Res->Type != Change->ResType)  appError ("ResType mismatch");
	//
	if (Change->Index >= 0) // Change a data record
		{
		if (DataSize  != Type->RecordSize) appError ("DataSize mismatch");
		//
		DestPtr = ((BYTE *)Change->Res->GetData()) + Change->Index * Type->RecordSize;
		//
		Res->Flags |= RF_TransData;
		}
	else // Change resource header
		{
		if (DataSize  != Type->HeaderSize) appError ("HeaderSize mismatch");
		//
		DestPtr = (BYTE *)Change->Res;
		//
		Res->Flags |= RF_TransHeader;
		//
		// Special case cleanup required for particular kinds of resources:
		//
		switch (Res->Type)
			{
			case RES_ActorList:
				((UActorList *)SourcePtr)->LockType = 0;
				break;
			case RES_Model:
				((UModel *)SourcePtr)->LockType = 0;
				break;
			};
		};
	memswap (DestPtr,SourcePtr,DataSize);
	UNGUARD("UTransBuffer::ApplyChange");
	};

/*-----------------------------------------------------------------------------
	Undo
-----------------------------------------------------------------------------*/

//
// Returns 1 if "undo" is possible, sets Str to name
//
int UTransBuffer::CanUndo (char *Str)
	{
	GUARD;
	//
	if (!GApp->ServerAlive) return 0;
	Lock();
	//
	if ((NumTrans==0) ||
		(UndoTransCount <= Transactions[0].TransCount))
		{
		strcpy (Str,ResetAction);
		Unlock();
		return 0;
		};
	//
	// Find transaction name:
	//
	for (int i=0; i < NumTrans; i++)
		{
		if (Transactions[i].TransCount == (UndoTransCount-1))
			{
			strcpy (Str,Transactions[i].Name);
			Unlock();
			return 1;
			};
		};
	strcpy(Str,"aborted transaction");
	Unlock();
	return 1;
	UNGUARD("UTransBuffer::CanUndo");
	};

//
// Undo a transaction, 1 if undone, 0 if not possible
//
int UTransBuffer::Undo (void)
	{
	GUARD;
	FTransaction	*Transaction;
	FTransChangeLog	*Change;
	char			Descr[256];
	int 			i;
	//
	if (!GApp->ServerAlive) return 0;
	//
	// See if undo is possible:
	//
	if (!CanUndo(Descr))
		{
		if (UndoTransCount == 0) // At beginning of transaction list
			{
			debugf (LOG_Trans,"Can't undo after %s",Descr); // Print reason that undo isn't possible
			}
		else
			{
			debugf (LOG_Trans,"At end of undo buffer");
			};
		return 0; // Can't undo
		};
	debugf (LOG_Trans,"Undo %s",Descr); // Print name of what we're undoing
	//
	Lock();
	UndoTransCount--;
	//
	// Find transaction corresponding to UndoTransCount:
	//
	Transaction = &Transactions [0];
	for (i=0; i<NumTrans; i++)
		{
		if (Transaction->TransCount == UndoTransCount) goto Found;
		Transaction++;
		};
	debugf(LOG_Info,"Not found");
	Unlock();
	return 0;
	//
	// Apply all "undo" changes corresponding to TRANS.UndoTransCount, in reverse order:
	//
	Found:
	BeginChanges (Transaction);
	for (i=NumChanges-1; i >= 0; i--)
		{
		Change = &ChangeLog[i];
		if (Change->TransCount == UndoTransCount)
			{
			ApplyChange (Change,Buffer + Transaction->StartDataOffset + Change->DataOffset,Change->DataSize);
			};
		};
	EndChanges (Transaction);
	//
	Unlock();
	return 1;
	UNGUARD("UTransBuffer::Undo");
	};

/*-----------------------------------------------------------------------------
	Redo
-----------------------------------------------------------------------------*/

//
// Returns 1 if "redo" is possible, sets Str to name
//
int UTransBuffer::CanRedo (char *Str)
	{
	GUARD;
	FTransaction	*Transaction;
	int 			i;
	//
	if (!GApp->ServerAlive) return 0;
	Lock();
	//
	if ((NumTrans==0) || (UndoTransCount >= TransCount))
		{
		strcpy (Str,"");
		Unlock();
		return 0;
		};
	//
	// Find transaction name:
	//
	Transaction = &Transactions[0];
	for (i=0; i<NumTrans; i++)
		{
		if (Transaction->TransCount ==  UndoTransCount)
			{
			strcpy (Str,Transaction->Name);
			Unlock();
			return 1;
			};
		Transaction++;
		};
	strcpy(Str,"aborted transaction");
	Unlock();
	return 1;
	UNGUARD("UTransBuffer::CanRedo");
	};

//
// Redo a transaction, 1 if undone, 0 if not possible
//
int UTransBuffer::Redo (void)
	{
	GUARD;
	FTransaction	*Transaction;
	FTransChangeLog	*Change;
	char			Descr[256];
	int 			i;
	//
	if (!GApp->ServerAlive) return 0;
	//
	// See if redo is possible:
	//
	if (!CanRedo(Descr))
		{
		debugf (LOG_Trans,"Nothing to redo",Descr);
		return 0; // Can't redo
		};
	debugf (LOG_Trans,"Redo %s",Descr); // Print name of what we're undoing
	//
	// Find transaction corresponding to UndoTransCount:
	//
	Lock();
	//
	Transaction = &Transactions [0];
	for (i=0; i<NumTrans; i++)
		{
		if (Transaction->TransCount == UndoTransCount) goto Found;
		Transaction++;
		};
	debugf(LOG_Info,"Not found");
	UndoTransCount++;
	Unlock();
	return 0;
	//
	// Apply all "redo" changes corresponding to TRANS.UndoTransCount:
	//
	Found:
	//
	BeginChanges (Transaction);
	for (i=0; i<NumChanges; i++)
		{
		Change = &ChangeLog[i];
		if (Change->TransCount == UndoTransCount)
			{
			ApplyChange (Change,Buffer + Transaction->StartDataOffset + Change->DataOffset,Change->DataSize);
			};
		};
	EndChanges (Transaction);
	UndoTransCount++;
	//
	Unlock();
	return 1;
	UNGUARD("UTransBuffer::Redo");
	};

//
// Redo all transactions remaining, return number redone
//
int UTransBuffer::RedoAll (void)
	{
	GUARD;
	if (!GApp->ServerAlive) return 0;
	return 0;
	UNGUARD("UTransBuffer::RedoAll");
	};

/*-----------------------------------------------------------------------------
	Logging individual transactions
-----------------------------------------------------------------------------*/

//
// Notce a single record added, deleted, or changed during a transaction.  Index
// is the index of the record in the resource Res, or -1 to note changes to stuff
// in the resource's header.
//
void UTransBuffer::NoteSingleChange (UResource *Res, int Index)
	{
	GUARD;
	FResourceType   *Type 		 = &GRes.Types	 [Res->Type ];
	FTransaction	*Transaction = &Transactions [NumTrans-1];
	FTransChangeLog	*Changes;
	BYTE			*SourcePtr,*DestPtr;
	int				DataSize;
	//
	if (!GApp->ServerAlive)	return;
	if ((!Res) || (!Locked)) 		return;
	if (Overflow)					return;
	//
	Transaction->NumChanges++;
	while (NumChanges >= MaxChanges)
		{
		//
		// There are more changes on file from past transactions than the system
		// can hold, so delete the first one:
		//
		if (NumTrans==0)
			{
			Overflow:
			//
			// This single transaction had more changes than the system can hold; end
			// transaction and mark it as "overflowed":
			//
			if (NumChanges != 0) appError ("NumChanges inconsistent");
			NumChanges = 0;
			Overflow   = 1;
			return;
			};
		DeleteFirstTransaction();
		Transaction = &Transactions [NumTrans-1];
		};
	//
	// Add to index:
	//
	Changes = &ChangeLog [NumChanges++];
	//
	Changes->TransCount	= TransCount - 1;
	Changes->Res		= Res;
	Changes->ResType	= Res->Type;
	Changes->Index	 	= Index;
	Changes->DataOffset	= Transaction->TotalDataSize;
	//
	if (Index >= 0)
		{
		//
		// Save a record of the resource's data:
		//
		DataSize  = Type->RecordSize;
		SourcePtr = ((BYTE *)Res->GetData()) + Index * Type->RecordSize;
		}
	else
		{
		//
		// Save resource header:
		//
		DataSize  = Type->HeaderSize;
		SourcePtr = (BYTE *)Res;
		};
	Changes->DataSize = DataSize;
	//
	// If there's not enough room in the data buffer, make room:
	//
	while ((Transaction->StartDataOffset + Transaction->TotalDataSize + DataSize) > MaxDataOffset)
		{
		if (NumTrans==0) goto Overflow;
		//
		DeleteFirstTransaction();
		Transaction = &Transactions [NumTrans-1];
		};
	//
	// Copy data
	//
	DestPtr   = Buffer + Transaction->StartDataOffset + Transaction->TotalDataSize;
	memcpy (DestPtr,SourcePtr,DataSize);
	//
	// Update transaction's running data length:
	//
	Transaction->TotalDataSize += DataSize;
	//
	// Handle special case resource types:
	//
	if (Index >= 0) // Resource data records:
		{
		if (Res->Type == RES_BspNodes)
			{
			((FBspNode *)DestPtr)->NodeFlags &= ~(NF_IsNew | NF_TagForEmpty); // Reset Bsp flags
			};
		};
	UNGUARD("UTransBuffer::NoteSingleChange");
	};

/*-----------------------------------------------------------------------------
	All transaction functions for specific resources
-----------------------------------------------------------------------------*/

//
// Actors (iActor = index)
//
void UTransBuffer::NoteActor (UActorList *Actors,INDEX iActor)
	{
	GUARD;
	INDEX i;
	//
	if (!GApp->ServerAlive) return;
	if (!Actors->Trans) return; // Not transaction-tracked
	//
	if (iActor==INDEX_NONE) // Wildcard
		{
		for (i=0; i<Actors->Max; i++)
			{
			if (Actors->Element(i).Class) NoteSingleChange (Actors,i);
			};
		}
	else NoteSingleChange (Actors,iActor);
	//
	UNGUARD("UTransBuffer::NoteActor");
	};

void UTransBuffer::RecursiveTransBspNode (IModel *ModelInfo,INDEX iNode)
	{
	FBspNode		*Node  = &ModelInfo->BspNodes[iNode];
	//
	GUARD;
	if (!GApp->ServerAlive) return;
	NoteSingleChange (ModelInfo->BspNodesResource,iNode);
	//
	if (Node->iFront != INDEX_NONE) RecursiveTransBspNode (ModelInfo,Node->iFront);
	if (Node->iBack  != INDEX_NONE) RecursiveTransBspNode (ModelInfo,Node->iBack );
	if (Node->iPlane != INDEX_NONE) RecursiveTransBspNode (ModelInfo,Node->iPlane);
	UNGUARD("UTransBuffer::RecursiveTransBspNode");
	};

//
// Bsp Nodes (iNode = index)
//
void UTransBuffer::NoteBspNode (IModel *ModelInfo,INDEX iNode)
	{
	GUARD;
	//
	if (!GApp->ServerAlive) return;
	if (!ModelInfo->Trans) return; // Not transaction-tracked
	//
	if (iNode==INDEX_NONE) // Wildcard
		{
		if (ModelInfo->NumBspNodes>0) RecursiveTransBspNode (ModelInfo,0);
		}
	else
		{
		NoteSingleChange (ModelInfo->BspNodesResource,iNode);
		};
	UNGUARD("UTransBuffer::NoteBspNode");
	};

//
// Bsp Polys (iSurf = index).  Also saves information corresponding to Bsp polygon's
// generating brush. (OLD:Call with MasterBrush=1 to note master brush change also)
//
void UTransBuffer::NoteBspSurf (IModel *ModelInfo,INDEX iSurf,int UpdateMaster)
	{
	GUARD;
	//
	if (!GApp->ServerAlive) return;
	if (!ModelInfo->Trans) return; // Not transaction-tracked
	//
	if (iSurf==INDEX_NONE) // Wildcard (does all polys, even unused ones)
		{
		for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++) NoteBspSurf(ModelInfo,i,UpdateMaster);
		}
	else
		{
		FBspSurf *Poly = &ModelInfo->BspSurfs[iSurf];
		//
		NoteSingleChange (ModelInfo->BspSurfsResource,iSurf);
		//
		if (UpdateMaster && Poly->Brush) NoteSingleChange (Poly->Brush->Polys,Poly->iBrushPoly);
		};
	UNGUARD("UTransBuffer::NoteBspSurf");
	};

void UTransBuffer::NoteSelectedBspSurfs (IModel *ModelInfo,int UpdateMasters)
	{
	GUARD;
	FBspSurf 		*Poly;
	int 			i,n;
	//
	if (!GApp->ServerAlive) return;
	n    = ModelInfo->NumBspSurfs;
	//
	Poly = &ModelInfo->BspSurfs[0];
	for (i=0; i<n; i++)
		{
		if (Poly->PolyFlags & PF_Selected)
			{
			NoteBspSurf (ModelInfo,i,UpdateMasters);
			};
		Poly++;
		};
	UNGUARD("UTransBuffer::NoteSelectedBspSurfs");
	};


//
// FPolys (iSurf = index)
//
void UTransBuffer::NoteFPoly (IModel *ModelInfo,INDEX iSurf)
	{
	GUARD;
	//
	if (!GApp->ServerAlive) return;
	if (!ModelInfo->Trans) return; // Not transaction-tracked
	//
	if (iSurf==INDEX_NONE)
		{
		for (INDEX i=0; i<ModelInfo->NumFPolys; i++) NoteSingleChange (ModelInfo->PolysResource,i);
		}
	else
		{
		NoteSingleChange (ModelInfo->PolysResource,iSurf);
		};
	UNGUARD("UTransBuffer::NoteFPoly");
	};

//
// Brush array
//
void UTransBuffer::NoteBrushArray (TArray<UModel> *BrushArray)
	{
	GUARD;
	if (!GApp->ServerAlive) return;
	if (Locked)
		{
		NoteSingleChange (BrushArray,-1); // Save header
		//
		// Save all level brushes except #0 (the special-case current brush!)
		//
		for (int i=1; i<BrushArray->Num; i++)
			{
			NoteSingleChange (BrushArray,i); // Save data
			};
		};
	UNGUARD("UTransBuffer::NoteBrushArray");
	};

//
// Resource header
//
void UTransBuffer::NoteResHeader (UResource *Res)
	{
	GUARD;
	if (!GApp->ServerAlive) return;
	if (Res) NoteSingleChange (Res,-1);
	UNGUARD("UTransBuffer::NoteResHeader");
	};

/*-----------------------------------------------------------------------------
	UTransBuffer resource implementation
-----------------------------------------------------------------------------*/

void UTransBuffer::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UTransBuffer);
	Type->RecordSize = sizeof (BYTE);
	Type->Version    = 1;
	strcpy (Type->Descr,"TransBuffer");
	UNGUARD("UTransBuffer::Register");
	};
void UTransBuffer::InitHeader(void)
	{
	GUARD;
	//
	// Init resource header to defaults:
	//
	Locked			= 0;
	Overflow		= 0;
	TransCount		= 0;
	UndoTransCount	= 0;
	NumTrans		= 0;
	MaxTrans		= 0;
	NumChanges		= 0;
	MaxChanges		= 0;
	MaxDataOffset	= 0;
	//
	strcpy (ResetAction,"create");
	UNGUARD("UTransBuffer::InitHeader");
	};
void UTransBuffer::InitData(void)
	{
	GUARD;
	Overflow		= 0;
	TransCount		= 0;
	UndoTransCount	= 0;
	NumTrans		= 0;
	NumChanges		= 0;
	UNGUARD("UTransBuffer::InitData");
	};
int UTransBuffer::QuerySize(void)
	{
	GUARD;
	DWORD TransactionSize  = MaxTrans 	* sizeof (FTransaction);
	DWORD ChangeLogSize    = MaxChanges * sizeof (FTransChangeLog);
	DWORD DataSize		   = MaxDataOffset;
	DWORD TotalSize		   = TransactionSize + ChangeLogSize + DataSize;
	return TotalSize;
	UNGUARD("UTransBuffer::QuerySize");
	};
int UTransBuffer::QueryMinSize(void)
	{
	GUARD;
	return QuerySize();
	UNGUARD("UTransBuffer::QueryMinSize");
	};
void UTransBuffer::QueryDataReferences(FResourceCallback &Callback)
	{
	GUARD;
	FTransChangeLog *Changes = (FTransChangeLog  *)((BYTE *)GetData() + MaxTrans * sizeof (FTransaction));
	for (int i=0; i<NumChanges; i++)
		{
		if (Changes[i].Index == -1)
			{
			Callback.Resource(this,&Changes[i].Res,0);
			};
		};
	UNGUARD("UTransBuffer::QueryDataReferences");
	};
AUTOREGISTER_RESOURCE(RES_TransBuffer,UTransBuffer,0xB2D90854,0xCCD211cf,0x91360000,0xC028B992);
