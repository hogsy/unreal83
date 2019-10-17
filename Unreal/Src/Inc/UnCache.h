/*===============================================================================
UnCache.h: Unreal fast memory cache support

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

Description:

    This provides all operations for the FMemoryCache class.  This is used
    for in-memory caching of many tables used throughout the game,
    such as light mesh index tables, remapped textures, etc.

    A cachable object can be identified by a unique DWORD.  This number
    is guaranteed to represent a unique cacheable entry, and it's up 
    to the cache manager to track which entities are in the cache, based
    on their ID's, and discard them on a least-recently-used entries.

Definitions:
    cached:
        Describes an object whose data is known to be in the cache data space.
        Get() for a cached object returns the address of that data.
    data space:
        This is the single area of memory where cached objects are stored. 
        The data space also contains management information used to keep
        track of the used and free parts of the data space.
    discard:
        Remove an object's data from the cache data space. This renders
        uncached an object which was previously cached.
    hint:
        A value associated with a cached object which helps speed up
        Get()'s cache searching. If performance is critical and the
        user of the cache can maintain a hint value, then providing
        the hint later on can significantly speed up the cache search.
    uncached:
        Describes an object whose data is not in the cache data space.
        because it was never cached or because it was cached but
        subsequently discarded.
        Get() for an uncached object returns 0.

Design Notes:
  1. Entities will be looked up in the cache several thousand times per
     second, so the lookup must be blazingly fast.
  2. Cache entry usage will be highly coherent.  You can assume that 99%
     of the calls to the Get() member will pertain to entries that are
     already in the cache.
  3. Entries can vary in size from zero bytes to the maximum size.  You
     can assume that nearly all cache entries will be large, i.e. 90% of
     all entries are >= 8K, and 50% of all entries are >= 32K.
  4. When a call to Get() indicates that an entity is not in the cache,
     Unreal must do a lot of work to rebuild the cache entry, which might
     involve remapping a texture or some other expensive process.  Therefore
     you need an LRU replacement strategy or something similar rather than
     the simplisic strategy of adding to the cache till it fills up, then
     dumping everything.
  5. A typical cache might contain 2 megs of data, with up to 1000 entities
     contained in it.  Therefore you have two limitations that must be
     considered: the Init() size, and the size of your internal index.
     You may want to allocate your index as a function of the Init() size.
  6. For speed considerations, you shouldn't do any allocation outside of 
     Init() or Exit().

Revision history:
    2-20-96, Tim: Prototype.
    2-29-96, Mark: First implementation
    4-19-95, Mark: Added Flush(pattern,...), Create(...,alignment).
                   Generalized alignment.
===============================================================================*/

#ifndef _INC_UNCACHE
#define _INC_UNCACHE

#include "Unreal.h"

//-----------------------------------------------------------------------------
// FMemoryCache: Class for caching various Unreal resources.
//-----------------------------------------------------------------------------
class UNREAL_API FMemoryCache
{
public:
     
    typedef DWORD  TObjectId    ; // To identify objects. 0 means "no object".
    typedef BYTE * TObjectData  ; // The data for a cached object, or 0 for none.
        // TObjectData objects are 0 or point somewhere in the cache data space.
    typedef int    THint        ; // A hint which might help improve cache look-up times. 0 means no hint.

    enum { DefaultAlignment = 8 };

    void Init (int BytesToAllocate, int MaxObjectCount = 0);
        //                  Initialize the cache
        // Initialize and allocate the cache with a specified number of bytes
        // available for caching. This function returns if and only if the cache
        // is successfully initialized. 
        //
        // The number of objects in the cache is limited to 
        // MaxObjectCount, unless MaxObjectCount < 1 in which case
        // a limit is selected based on BytesToAllocate
        // (approximately 1 object for each 1KByte in BytesToAllocate).
        //
        // Assumptions:
        //   1. Init() must be the very first member function called, and
        //      it must be the first member function called after a call
        //      to Exit(). 
        //   2. BytesToAllocate > 0

    int MaxObjectCount() const;
        // This returns the maximum number of objects which are allowed
        // in the cache at any time. This maximum is determined as described
        // above for Init().

    int MaxCreateSize() const;
        // This returns the maximum size allowed in a Create() request
        // with the default alignment.  This is the same as the value 
        // of BytesToAllocate specified when Init() was called.

    void Exit(); 
        //            Terminate the cache
        // Deallocate any allocated data and shut down.
        // 
        // Assumptions:
        //   1. After calling Exit(), the only member function which
        //      can be called is Init().
        //

    void Flush(void); 
        //          Flush all the data from the cache
        // Empty the cache by removing all cached objects, including locked ones.
        //
        // Axioms:
        //   1. X.Flush(); ==> for all Id: X.Get(Id)==0

    void Flush( TObjectId Id );
        //       Flush an object from the cache
        // If the object is in the cache, discard it (even if it is locked).
        // Axioms:
        //   1. X.Flush(Id) ==> X.Get(Id) == 0

    void Flush(TObjectId IdPattern, TObjectId IdMask );
        //          Flush specific cache entries.
        // Remove from the cache all objects with ids Id such that:
        //       (Id & IdMask) == IdPattern
        // Even locked objects are flushed.
        //
        // Axioms:
        //   1. X.Flush(IdPattern,IdMask) ==>
        //      for all Id with (Id & IdMask)==IdPattern: X.Get(Id)==0

    TObjectData Get(TObjectId Id);
        //          Return the data associated with a cached object
        // If the object is cached, return the address (in the cache data space)
        // where the object resides. If the object is uncached, return 0.
        // An object is cached if and only if the following
        // conditions are all satisfied:
        //    1. The object Id was placed in the cache with a previous
        //       call to Create(Id,...).
        //    2. After the object was most recently placed in the cache, the 
        //       cache was not emptied by a call to Exit() or Flush().
        //    3. After the object was most recently placed in the cache, the 
        //       object was not discarded by a call to Create() or Flush().
        // Assumptions:
        //   1. Id != 0
        // Notes:
        //   1. The value returned by Get() is meaningful until the next:
        //      - call to Create()
        //      - call to Exit() or Flush()
        //      After such calls, the results of previous Get()
        //      calls cannot be used to reference data.
        //      This means, for example, that a call to Get() does not render 
        //      useless the results of previous calls to Get().
        //   2. The data returned for an object is aligned as it was when the
        //      object was created (see Create()).

    TObjectData Get(TObjectId Id, THint & Hint)
        //          Return the data associated with a cached object
        // This works just like Get(Id), except that the performance is better
        // when the object is still in the cache and the Hint can be used to
        // speed up locating the cached object. Hints can become "stale", which means
        // they no longer help to speed up the search.
        // On input, Hint provides a previous hint obtained either through
        // GetHint() or Get(...,Hint). If Hint is 0 then the hint is
        // ignored. On output, Hint is set to a fresh hint value for the given
        // object.
    {
        if( Hint > 0 && Hint < State.Dictionary.Count && State.Dictionary.Id(Hint) == Id )
        {
            // The hint was helpful - we found the object.
            return State.Dictionary.Fragment(Hint)->Data();
        }
        else
        {
            // The hint was no good...
            // LastIndexFound might be set by Get() if the object was found, but
            // it might *not* be set if the object was found without a search.
            // We set LastIndexFound to 0, if it changes, then it is a useful 
            // hint. If it doesn't change, then we have no useful hint.
            // This means the hint we return will be 0, which is not useful for 
            // the next search. It is okay to return a useless hint, since otherwise
            // we have to do a search and we should wait until a search is needed.
            State.Dictionary.LastIndexFound = 0;
            TObjectData Data = Get(Id);
            Hint = State.Dictionary.LastIndexFound; // Might be 0 if no useful hint was found.
            return Data;
        }
    }

    THint GetHint( TObjectId Id ) const;
        //             Return the hint for a cached object.
        // If the object Id is in the cache, return a "hint" value which 
        // can be used later to possibly speed up Get() requests. If the object
        // is not cached then 0 is returned.

    TObjectData Create( TObjectId Id, int Size, int Alignment = DefaultAlignment );
        //             Place a new object in the cache
        // Add a new object (with Size bytes) to the cache. 
        // Return the address (in the cache data space) where the object resides.
        // Except for serious errors describe below, this function always succeeds 
        // and returns a non-0 value, unless there are locked cache entries which
        // which prevent adding the object. One, some, or all of the existing cached 
        // objects might be discarded for any of the following reasons:
        //   - there is not enough unused space in the cache data space
        //     for the new object
        //   - there is not enough contiguous unused space in the cache data space
        //     for the new object
        //   - there are already MaxObjectCount() objects in the cache.
        // WARNING:
        //   Alignment must be a multiple of DefaultAlignment.
        // Notes:
        //   1. A call to Create() may change the address of data allocated
        //      to objects in the cache. In other words, the values returned by 
        //      any previous calls to Get cannot be used after calling Create().
        //      However, locked objects will not be moved.
        //   2. The value returned by Create() is meaningful until the next:
        //      - call to Create()
        //      - call to Exit() or Flush()
        //   3. The data allocated for an object is always aligned on
        //      an Alignment-byte boundary.
        //   4. If there are locked cached objects, they might prevent the
        //      creation of the new object.
        // Assumptions:
        //   1. The object Id is not already in the cache. Therefore, you
        //      shouldn't call Create(Id,...) unless Get(Id)==0.
        //   2. 0 < Size 
        //   3. Size <= MaxCreateSize()
        //   4. If Alignment != DefaultAlignment, 
        //         Size+Alignment-1 <= MaxCreateSize()
        //   5. Id != 0
        //   6. Alignment > 0
        // Axioms:
        //   1. The most recently created object is always cached
        //      (unless the cache is flushed or terminated), and its data
        //      is located where it was initially created:
        //         TObjectData Data = X.Create(Id,Size,Alignment); ==> X.Get(Id) == Data

    void Lock(TObjectId Id);
        //         Lock an object in the cache
        // If Get(Id) != 0, then the object is locked into the cache.
        //  This means that the object will not be discarded from the
        //  cache and will be successfully retrieved with future Get()
        //  requests until the object is unlocked.
        // Notes:
        //   1. You should lock objects which need to be manipulated at
        //      the same time and you need to keep them in the cache.
        //   2. Tick() may check to make sure there are no locked entries.
        //   3. A cached item maintains a lock count - each call to Lock
        //      increases the count by 1.
        
    void Unlock(TObjectId Id, BOOL Force = FALSE);
        //       Unlock a locked object.
        // If Get(Id) != 0, and if the object is currently locked,
        // then decrease the count lock for the object (or if Force==TRUE,
        // set the count lock to 0). If this makes the count lock 0, 
        // then the object is unlocked. This means the object can now be discarded. 

    void Tick(void);
        //             Mark the passage of time
        // This is called to mark the passage of a unit of time.
        // The frequency of calls to Tick() is not critical to the 
        // cache manager, although very small frequencies (once per second) or very
        // large frequencies (thousands of times per second) might reduce the
        // general effectiveness of the cache.
        // Notes:
        //   1. Since locked cache entries should be locked for only a short
        //      period of time, Tick() might check to make sure all cache entries
        //      are unlocked, and report an error if they are not.

    void AssertValid(void) const;
        //            Check the validity of the cache
        // Verify the internal validity of the cache information.
        // If successful, the cache is valid and the function returns.
        // If unsuccessful, an error is reported.

    void Status(char *StatusText) const;
        //          Textual description of status of cache
        // Put into StatusText a textual message which summarizes the status
        // of the cache. The message should be useful for performance tuning.
        //
        // Assumptions:
        //   1. StatusText has room for 80 ASCII characters, including
        //      the trailing null character.
        //

        
public:
    //                Debugging stuff
    // The debugging functions do little or nothing when the code is compiled
    // with _DEBUG undefined, so the descriptions apply only when compiled
    // with _DEBUG defined.

    void Check() const; // Check for errors in the state of the cache. 

    void Dump(const char * FileName = "UNCACHE.DMP") const; // Dump cache information to file.

    void DoChecking(BOOL Check = TRUE); // Turn checking on or off.
        // TRUE to turn on automatic checking - this causes the cache state
        // to be checked after every operation. This degrades performance considerably.
        // Use FALSE to turn off checking. By default, checking is initially off.

    BOOL Checking() const; // Are automatic checks on?

    void MaybeCheck() const; // If automatic checks are on, call Check().


    // The following private details are included here to allow inlining.

    typedef int             TimeType    ; // A measure of time. 0 has special meaning.
    typedef int             AgeType     ; // A measure of age (difference between two times).

    //-----------------------------------------------------------------------------
    //                       TObjectHistory
    //-----------------------------------------------------------------------------
    struct TObjectHistory // Information about a cached object.
    {
        TimeType        GetTime     ; // When the object was last retrieved with Get().
    };


    //-----------------------------------------------------------------------------
    //                         TFragment
    //-----------------------------------------------------------------------------
    struct TFragment // The cache data space is divided up into fragments.
    {
        static inline int RoundUp( int Value, int Alignment ) // Round up to multiple of Alignment.
        { 
            return (Value+Alignment-1)/Alignment*Alignment; 
        } 
        static inline int RoundUp( unsigned Value, int Alignment ) // Round up to multiple of Alignment.
        { 
            return (Value+Alignment-1)/Alignment*Alignment; 
        } 
        TFragment     * Next            ; // Next fragment in data space (0 for none).
        TFragment     * Previous        ; // Previous fragment in data space (0 for none).
        int             DataOffset      ; // Offset from TFragment to start of fragment data.
        int             Size            ; // The size of the fragment, in bytes. Does not include any Overhead().
        TObjectId       ObjectId        ; // The object held in this fragment, or 0 for none.
        TObjectHistory  ObjectHistory   ; // Meaningful only when this->IsUsed().
        int             LockCount       ; // A count of the number of locks for this entru. Meaningful only when this->IsUsed().
            // Notes: 
            //   1. The Next member is (almost) redundant since it can (usually) be
            //      computed using other members. The only exception is the very last
            //      fragment, where Next==0 (this fact cannot be deduced from other
            //      members).
            //   2. DataOffset is normally Overhead(), but for specially aligned
            //      cached objects it may be different.
    
        BOOL IsUsed() const { return ObjectId != 0; }
        BOOL IsFree() const { return ObjectId == 0; }
        BOOL IsLocked() const { return LockCount > 0; }
        void Free()         { ObjectId = 0; DataOffset = Overhead(); LockCount = 0; }

        int Staleness(TimeType Time) const // How stale is the object (assumes IsUsed())?
            { return Time-ObjectHistory.GetTime; }

        int Cost(TimeType Time) const; // Estimated cost of using a fragment.

        static int Overhead()  // The extra memory needed to manage a fragment.
            { return RoundUp(sizeof(TFragment),DefaultAlignment); }

        TObjectData Data() const // The data associated with a fragment (follows it in memory).
            { return (BYTE *)this + DataOffset; }

        void LinkAfter(TFragment * AfterThis ) // Link *this into the chain after AfterThis.
        {
            this->Next = AfterThis->Next;
            this->Previous = AfterThis;
            if( this->Next != 0 )
            {
                this->Next->Previous = this;
            }
            AfterThis->Next = this;
        }

        int TotalSpace() const { return Overhead() + Size; } // Total space used by a fragment.
        const BYTE * DataLimit() const { return (BYTE *)this + TotalSpace(); } // 1 byte beyond last byte in fragment.

        void Check() const; // Check validity of this.
    };

    //-----------------------------------------------------------------------------
    //                         TDictionary
    //-----------------------------------------------------------------------------
    // The dictionary lists the object definitions for all cached objects.
    // The definitions tells us where to find objects in the cache data space.
    //
    // An uncached object is not in the dictionary - the object 
    // and its id are both considered "undefined" (since they have no
    // dictionary definition).
    // A cached object is in the dictionary - the object and its id
    // are both considered "defined" (since they have a dictionary
    // definition).
    //
    // The dictionary is sorted by object id. There may be more than one 
    // definition for an object id, but all such definitions will be the same. 
    // There may be some definitions with an object id of 0 - such entries are
    // ignored (their definition is meaningless).
    struct TDictionary 
    {
        int             MaxCount    ; // Maximum number of definitions (including duplicates) in the dictionary.
        int             Count       ; // Number of definitions (including duplicates) currently in the dictionary.
        int             ObjectCount ; // Number of unique objects in the dictionary.
        TObjectId     * ObjectIds   ; // A list of MaxCount object ids.
        TFragment    ** Fragments   ; // A list of MaxCount fragments corresponding to the object ids.
        TObjectId       LastIdFound ; // Last id looked up, or 0.
        int             LastIndexFound; // Index of LastIdFound, or 0.
        TFragment     * LastFragmentFound; // Last fragment looked up for LastIdFound.
            // Notes:
            //   1. ObjectIds and Fragments each hold MaxCount elements, the first Count of which are used.
            //   2. The used entries are sorted in nondecreasing order of object id.
            //   3. Definitions may contain duplicate entries - entries with the same ObjectId
            //      will be contiguous and will have the same definitions. The first entries
            //      in *ObjectIds may have ObjectId==0.

        void Initialize( int MaxCount ); // Initialize with room for MaxCount definitions.
        void Terminate(); // Terminate and free any data.
        void Empty(); // Empty the dictionary of all definitions.
        BOOL  IsFull() const // Is the dictionary full?
            { return ObjectCount == MaxCount; }
        void Discard(TObjectId Id); // Discard an object from the dictionary.
        int Index(TObjectId Id) const; // Index of object in dictionary, or 0 if not found.
        int FirstIndex(TObjectId Id); // Index of first object id in dictionary, or 0 if not found.
        int InsertionIndex(TObjectId Id); // Where Id should be inserted. 1..Count+1.
        void Add(TObjectId ObjectId, TFragment * Fragment);
            // Add to the dictionary a definition for the specified object (ObjectId!=0).

        TObjectId Id(int N) const { return ObjectIds[N-1]; }
        TObjectId & Id(int N) { return ObjectIds[N-1]; }
        TFragment * Fragment(int N) const { return Fragments[N-1]; }
        TFragment * & Fragment(int N) { return Fragments[N-1]; }
        void Check() const;  // Check validity of this.
    };

    //-----------------------------------------------------------------------------
    //                  TState - the state of the cache
    //-----------------------------------------------------------------------------
    struct TState
    {
        BYTE          * DataSpace       ; // The cache data space (holds object data and management info).
        BYTE          * AllocatedSpace  ; // The allocated space for the data space. (Note 1).
        int             DataSpaceSize   ; // Bytes available for objects and management info.
        int             MaxCreateSize   ; // Maximum size allowed during Create().
        TDictionary     Dictionary      ; // Dictionary of cached objects.
        TimeType        Time            ; // Increased each call to Tick(). Always > 0.
        int             GetCount        ; // Number of times Get() was called.
        int             HitCount        ; // Number of times Get() succeeded.
        BOOL            Checking        ; // TRUE when automatic checking is on.
            // Notes:
            //   1. The AllocatedSpace is the memory allocated from the free store.
            //      DataSpace is aligned in AllocatedSpace. We must keep
            //      AllocatedSpace so it can be freed.

        // The first fragment is always at the beginning of the data space:
        TFragment & FirstFragment()             { return *(TFragment *)DataSpace; }
        const TFragment & FirstFragment() const { return *(TFragment *)DataSpace; }

        void Free( TFragment & Fragment );
            // Free the fragment by removing any associated object
            // from the dictionary and marking the fragment as free.
            // You should subsequently call MergeFreeFragments() to merge 
            // any nearby free fragments.

        void MergeFreeFragments(TFragment * & Fragment);
            // Coalesce any immediately preceding or following free fragments
            // with the given fragment to produce one free fragment.
            // On output, Fragment points to the merged fragment.
            // Assumptions:
            //   1. Fragment->IsFree()

        void MergeWithNext(TFragment & Fragment);
            // Merge Fragment with its immediately following 
            // fragment to product a single larger fragment.

        const TFragment * LRU() const; // What is the least recently used fragment?

        void Check() const; // Check validity of this.
    };

private:
    TState State;

};

#endif

