/*=============================================================================
	UnEdTran.h: Unreal transaction tracking system

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNEDTRAN
#define _INC_UNEDTRAN
#ifdef  EDITOR

/*-----------------------------------------------------------------------------
	UTransBuffer
-----------------------------------------------------------------------------*/

class UNEDITOR_API UTransBuffer : public UResource
	{
	RESOURCE_CLASS(UTransBuffer,BYTE,RES_TransBuffer)
	//
	INT		Locked;			// 1=locked
	INT		Overflow;		// 1 if active transaction overflowed data buffer
	INT		TransCount;		// Ever-increasing counter of maximum transaction
	INT		UndoTransCount; // Transaction count in undo/redo cycle
	INT		NumTrans;		// Number of transactions in *Transactions
	INT		MaxTrans;		// Maximum number of transactions in *Transactions
	INT		NumChanges;		// Number of entries in *ChangeLog
	INT		MaxChanges;		// Maximum entries in *ChangeLog
	INT		MaxDataOffset;	// Maximum bytes allocated for *Data
	char	ResetAction[256]; // Reason transaction system was last reset
	//
	// Pointers, only valid when locked:
	//
	class FTransaction		*Transactions;	// List of all transactions
	class FTransChangeLog	*ChangeLog;		// List of all modifications made in a transaction
	BYTE					*Buffer;		// Big raw data buffer
	//
	// Resource functions:
	//
	virtual void Register				(FResourceType *Type);
	virtual void InitHeader				(void);
	virtual void InitData				(void);
	virtual int  QuerySize				(void);
	virtual int  QueryMinSize			(void);
	virtual void QueryDataReferences	(FResourceCallback &Callback);
	//
	// Custom functions:
	//
	virtual void Lock(void);
	virtual void Unlock(void);
	virtual void Reset(const char *Action);
	virtual void Begin(ULevel *Level, const char *SessionName);
	virtual void End(void);
	virtual void Rollback(void);
	virtual void ForceOverflow(const char *Reason);
	virtual int  CanUndo (char *Str);
	virtual int  CanRedo (char *Str);
	virtual int  Undo (void);
	virtual int  Redo (void);
	virtual int	 RedoAll (void);
	virtual void Continue(void);
	virtual void NoteSingleChange (UResource *Res, int Index);
	virtual void NoteBrushArray (TArray<UModel> *BrushArray);
	virtual void NoteBspNode (IModel *ModelInfo,INDEX iNode);
	virtual void NoteBspSurf (IModel *ModelInfo,INDEX iSurf,int UpdateMaster);
	virtual void NoteSelectedBspSurfs (IModel *ModelInfo,int UpdateMasters);
	virtual void NoteFPoly (IModel *ModelInfo,INDEX iSurf);
	virtual void NoteActor (UActorList *Actors,INDEX iActor);
	virtual void NoteResHeader (UResource *Res);
	//
	private:
	void DeleteFirstTransaction(void);
	void BeginChanges (class FTransaction *Trans);
	void EndChanges (class FTransaction *Trans);
	void RecursiveTransBspNode (IModel *ModelInfo,INDEX iNode);
	void ApplyChange (class FTransChangeLog *Change, BYTE *SourcePtr, DWORD DataSize);
	};

/*----------------------------------------------------------------------------
	The End
----------------------------------------------------------------------------*/
#endif // EDITOR
#endif // _INC_UNEDTRAN

