/*=============================================================================
	UnCache.cpp: Unreal fast memory cache support

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

Description:

    Refer to the associated header file.

Design:

    The cache data space is carved into and managed using
    fragments. A fragment can either be free or used.
    A free fragment is not assigned to a cached object, so it can
    be used to Create() a new object. A used fragment holds 
    a cached object (exactly one cached object).
    
    The data space is partitioned into a list of fragments.
    Each fragment has a header followed by the space available to the fragment. 
    The space might be in use by a cached object.
   
    The first fragment starts at the beginning of the data space.
    The fragments form a doubly-linked list.
   
    Two free fragments will not appear together - they will
    always be coalesced into a single free fragment. This is 
    done to make larger free fragments available and to reduce
    the number of fragments that must be managed.
   
    The dictionary keeps track of the objects in the cache
    by mapping each object id to a used fragment.
   
     DataSpace
        |    F(1)         F(2)    F(3)
        |    |            |       |
        |    +--+---------+--+----+--+-------------         -----+--+----------+
        +--->|ft|  free   |ft|used|ft| used          ...         |ft| free     |
             +--+---------+--+----+--+-------------         -----+--+----------+
             |            |       |          
    FirstFragment         |       |
                          |       |                  |
            Dictionary:   |       |                  |
                  +-----+ |       |                  |
                  | id1 |---------+                  |           
                  +-----+ |                          |
                  | id2 |----------------------------+
                  +-----+ |
                  | id3 |-+ 
                  +-----+
                  |     |
                    ...
   
    Legend:
       ft: fragment header of type TFragment
       F(n): n'th fragment in the data space

Notes:
  1. For simplicity, each fragment and the data for each fragment
     is aligned to an FMemoryCache::DefaultAlignment-byte boundary. Each
     fragment size is rounded to a multiple of FMemoryCache::DefaultAlignment
     bytes.

Revision history:
    2-20-96, Tim: Prototype
    2-29-96, Mark: Implementation
    4-19-95, Mark: Added Flush(pattern,...), Create(...,alignment).
                   Generalized alignment.
=============================================================================*/

#include "Unreal.h"
#include "UnCache.h"
#include "UnChecks.h"
#include <stdio.h>

#define UseAssembly 1  // 1 to use inline assembly code, 0 to use C++ code.

//-----------------------------------------------------------------------------
//                 Convenience declarations
//-----------------------------------------------------------------------------
typedef FMemoryCache::TObjectId     TObjectId   ;
typedef FMemoryCache::TObjectData   TObjectData ;
enum { DefaultAlignment = FMemoryCache::DefaultAlignment };

#if !defined( GUARD )
    #define GUARD
#endif

#if !defined( UNGUARD )
    #define UNGUARD(Text)
#endif

//-----------------------------------------------------------------------------
//                       Alignment functions
//-----------------------------------------------------------------------------
static inline int RoundUp( int Value, int Alignment ) // Round up to multiple of Alignment.
{ 
    return (Value+Alignment-1)/Alignment*Alignment; 
} 

static inline int RoundUp( unsigned Value, int Alignment ) // Round up to multiple of Alignment.
{ 
    return (Value+Alignment-1)/Alignment*Alignment; 
} 

static inline int RoundDown( unsigned Value, int Alignment ) // Round down to multiple of Alignment.
{ 
    return Value/Alignment*Alignment; 
} 

template<class T>
static inline T * RoundUp( T * Address, int Alignment ) // Align an address up to a multiple of Alignment bytes.
{
    // WARNING: This is machine-specific code, relying on the ability
    // to meaningfully convert addresses to and from unsigned integers.
    return  (T *)RoundUp( (unsigned)Address, Alignment );
}

template<class T>
static inline T * RoundDown( T * Address, int Alignment ) // Align an address down to a multiple of Alignment bytes.
{
    // WARNING: This is machine-specific code, relying on the ability
    // to meaningfully convert addresses to and from unsigned integers.
    return  (T *)RoundDown( (unsigned)Address, Alignment );
}

//-----------------------------------------------------------------------------
//           Debug function: Check the cache if automatic checking is on
//-----------------------------------------------------------------------------
inline void FMemoryCache::MaybeCheck() const
{
    if( DebugCode && Checking() )
    {
        Check();
    }
}

//-----------------------------------------------------------------------------
//                Debug function: Check TFragment
//-----------------------------------------------------------------------------
void FMemoryCache::TFragment::Check() const
{
    if( CHECK_STATE )
    {
        checkState( Size >= 0 );
        checkState( RoundUp(Size,DefaultAlignment) == Size );
        checkState( Data() != 0 );
        if( IsUsed() )
        {
            checkState( ObjectId   != 0 );
        }
    }
}

//-----------------------------------------------------------------------------
//                Initialize a dictionary
//-----------------------------------------------------------------------------
void FMemoryCache::TDictionary::Initialize( int MaxCount )
{
    checkInput( MaxCount > 0 );
    this->MaxCount = MaxCount;
    ObjectIds = new TObjectId[MaxCount];
    checkVital( ObjectIds != 0, "FMemoryCache dictionary object allocation failed" );
    typedef TFragment * TFragmentHandle;
    Fragments = new TFragmentHandle[MaxCount];
    checkVital( Fragments != 0, "FMemoryCache dictionary fragment allocation failed" );
    Empty();
}

//-----------------------------------------------------------------------------
//                Empty a dictionary
//-----------------------------------------------------------------------------
void FMemoryCache::TDictionary::Empty()
{
    Count               = 0 ;
    ObjectCount         = 0 ;
    LastIdFound         = 0 ;
    LastIndexFound      = 0 ;
    LastFragmentFound   = 0 ;
}

//-----------------------------------------------------------------------------
//                Terminate a dictionary
//-----------------------------------------------------------------------------
void FMemoryCache::TDictionary::Terminate()
{
    Empty();
    if( ObjectIds != 0 )
    {
        delete[] ObjectIds;
        ObjectIds = 0;
    }
    if( Fragments != 0 )
    {
        delete[] Fragments;
        Fragments = 0;
    }
}

//-----------------------------------------------------------------------------
//                Debug function: Check TDictionary
//-----------------------------------------------------------------------------
void FMemoryCache::TDictionary::Check() const
{
    if( CHECK_STATE )
    {
        checkState( 0 <= Count && Count <= MaxCount );
        checkState( 0 <= ObjectCount && ObjectCount <= Count );

        if( LastIdFound != 0 )
        {
            const int Index = this->Index(LastIdFound);
            checkState( Index != 0 );
            checkState( this->Fragment(Index) == LastFragmentFound );
        }

        // Check each active cached object info:
		int WhichObject;
        for( WhichObject = 1; WhichObject <= Count; ++WhichObject )
        {
            if( Id(WhichObject) != 0 )
            {
                checkState( Fragment(WhichObject) != 0 );
                checkState( Id(WhichObject) == Fragment(WhichObject)->ObjectId );
            }
        }

        // Make sure the object ids are sorted in non-decreasing order.
        for( WhichObject = 1; WhichObject < Count; ++WhichObject )
        {
            const TObjectId         ThisId       = Id(WhichObject)       ;
            const TFragment * const ThisFragment = Fragment(WhichObject) ;
            const TObjectId         NextId       = Id(WhichObject)       ;
            const TFragment * const NextFragment = Fragment(WhichObject) ;
            checkState( ThisId <= NextId );
            if( ThisId == NextId )
            {
                // There might be duplicate runs of the same id - make sure 
                // the information is the same or the object id is 0.
                if( ThisId != 0 )
                {
                    checkState( ThisFragment == NextFragment );
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
//                     Initialize the cache
//-----------------------------------------------------------------------------
void FMemoryCache::Init (int BytesToAllocate, int MaxObjectCount )
{
    GUARD;
    checkInput( BytesToAllocate > 0 );
    MaxObjectCount = MaxObjectCount < 1 ? 1 + BytesToAllocate/1000 : MaxObjectCount ;
    // There can be at most one free fragment following each
    // used fragment (since contiguous free fragments are always
    // coalesced into a single free fragment). 
    // In addition, there might be 1 free fragment before the very first
    // used fragment. So this leads us to conclude:
    const int MaxFragmentCount = 2*MaxObjectCount + 1;

    FMemoryCache::TState & Cache = State;
    Cache.MaxCreateSize = BytesToAllocate;

    Cache.Dictionary.Initialize( MaxObjectCount );

    // Prepare data space:
    {
        // We add enough room to the data space for all the fragment headers.
        // Round up the data space size so we are sure that every fragment
        // and every fragment's data are properly aligned.
        Cache.DataSpaceSize = RoundUp
        ( 
                Cache.MaxCreateSize 
            +   MaxFragmentCount*TFragment::Overhead()
        ,   DefaultAlignment
        );
        Cache.AllocatedSpace = new BYTE
        [
            RoundUp(Cache.DataSpaceSize,int(DefaultAlignment)) // Rounded up in case the allocated space needs to be aligned.
        ];
        checkVital( Cache.AllocatedSpace != 0, "FMemoryCache data space allocation failed" );
        Cache.DataSpace = RoundUp( Cache.AllocatedSpace, int(DefaultAlignment) );
    }
    Flush();
    Check();
    DoChecking(FALSE);
    UNGUARD("FMemoryCache::Init");
}

//-----------------------------------------------------------------------------
//               The maximum number of objects allowed in the cache
//-----------------------------------------------------------------------------
int FMemoryCache::MaxObjectCount() const 
{ 
    return State.Dictionary.MaxCount; 
}

//-----------------------------------------------------------------------------
//               The maximum size of an object in the cache
//-----------------------------------------------------------------------------
int FMemoryCache::MaxCreateSize() const 
{
    return State.MaxCreateSize;
}

//-----------------------------------------------------------------------------
//                     Finalize the cache
//-----------------------------------------------------------------------------
void FMemoryCache::Exit (void)
{
    GUARD;
    {
        MaybeCheck();
        delete[] State.AllocatedSpace;
        State.Dictionary.Terminate();
    }
    UNGUARD("FMemoryCache::Exit");
}

//-----------------------------------------------------------------------------
//                     Flush the cache
//-----------------------------------------------------------------------------
void FMemoryCache::Flush(void)
{
    GUARD;
    FMemoryCache::TState & Cache = State;
    Cache.Time = 1; // Time must start > 0 so that object BirthTime's are all > 0.
    Cache.Dictionary.Empty();
    Cache.GetCount = 0;
    Cache.HitCount = 0;
    // Define the entire data space as a single free fragment:
    {
        TFragment & Fragment = Cache.FirstFragment();
        Fragment.Next       = 0             ;
        Fragment.Previous   = 0             ;    
        Fragment.Size       = Cache.DataSpaceSize - Fragment.Overhead();
        Fragment.Free();
    }
    MaybeCheck();
    UNGUARD("FMemoryCache::Flush");
}

//-----------------------------------------------------------------------------
//                     Flush an object
//-----------------------------------------------------------------------------
void FMemoryCache::Flush(TObjectId Id)
{
    GUARD;
    TState & Cache = State;
    const int Index = Cache.Dictionary.Index(Id);
    if( Id != 0 && Index != 0 )
    {
        TFragment * Fragment = Cache.Dictionary.Fragment(Index);
        Cache.Free( *Fragment );
        Cache.MergeFreeFragments(Fragment);
    }
    MaybeCheck();
    UNGUARD("FMemoryCache::Flush(Id)");
}

//-----------------------------------------------------------------------------
//             Flush cached objects with Id's matching a pattern
//-----------------------------------------------------------------------------
void FMemoryCache::Flush(TObjectId IdPattern, TObjectId IdMask)
{
    GUARD;
    TState & Cache = State;
    for( int WhichDefinition = 1; WhichDefinition <= Cache.Dictionary.Count; ++WhichDefinition )
    {
        const TObjectId Id = Cache.Dictionary.Id(WhichDefinition);
        if( Id != 0 && (Id & IdMask) == IdPattern )
        {
            Flush(Id);
        }
    }
    MaybeCheck();
    UNGUARD("FMemoryCache::Flush(Id)");
}

//-----------------------------------------------------------------------------
//           Find the index of an object in the dictionary 
//-----------------------------------------------------------------------------
// Notes:
//   1. If the object id has duplicates in the dictionary, 
//      it is undefined which dictionary entry is indexed
//      by the value returned by this function.
inline int FMemoryCache::TDictionary::Index(TObjectId Id) const
{
    const BOOL UseUnrolledSearch = TRUE;

    if( UseUnrolledSearch )
    {
        // Do a binary search, unrolled one level for slight improvement.
        int Low     = 1     ; // Index of lower extreme of binary search.
        int High    = Count ; // Index of upper extreme of binary search.
        while( Low <= High )
        {
            // Divide the range Low..High into 4 quadrants separated by 
            // Quarter1, Quarter2, and Quarter3.
            const int Quarter2 = (High+Low)>>1;
            const TObjectId Id2 = this->Id(Quarter2);
            if( Id > Id2 ) 
            {
                const int Quarter3 = (Quarter2+High)>>1;
                const TObjectId Id3 = this->Id(Quarter3);
                if( Id > Id3 ) 
                {
                    Low = Quarter3 + 1;
                }
                else if( Id < Id3 ) 
                {
                    Low = Quarter2 + 1;
                    High = Quarter3 - 1;
                }
                else
                {
                    // We found the id.
                    return Quarter3; // <==== Gross unstructured return! Done for performance.
                }
            }
            else if( Id < Id2 ) 
            {
                const int Quarter1 = (Low+Quarter2)>>1;
                const TObjectId Id1 = this->Id(Quarter1);
                if( Id > Id1 ) 
                {
                    Low = Quarter1 + 1;
                    High = Quarter2 - 1;
                }
                else if( Id < Id1 ) 
                {
                    High = Quarter1 - 1;
                }
                else
                {
                    // We found the id.
                    return Quarter1; // <==== Gross unstructured return! Done for performance.
                }
            }
            else
            {
                // We found the id.
                return Quarter2; // <==== Gross unstructured return! Done for performance.
            }
        }
    }
    else
    {
        // Do a binary search for the object's id:
        int Low     = 1     ; // Index of lower extreme of binary search.
        int High    = Count ; // Index of upper extreme of binary search.
        while( Low <= High )
        {
            const int Middle = (High+Low)>>1;
            const TObjectId MiddleId = this->Id(Middle);
            if( Id > MiddleId ) 
            {
                // Keep searching in Middle+1..High
                Low = Middle + 1;
            }
            else if( Id < MiddleId ) 
            {
                // Keep searching in Low..Middle-1
                High = Middle - 1;
            }
            else
            {
                // We found the id.
                return Middle; // <==== Gross unstructured return! Done for performance.
            }
        }
    }
    return 0; // We get here only if the object was not found.
}

//-----------------------------------------------------------------------------
//                  Find an entry in the cache
//-----------------------------------------------------------------------------
FMemoryCache::TObjectData FMemoryCache::Get(TObjectId Id) 
{
    TState & Cache = State;
//?    checkInput( Id != 0 );
    Cache.GetCount++;

    if( Id==Cache.Dictionary.LastIdFound )
    {
        TFragment & Fragment = *Cache.Dictionary.LastFragmentFound;
        //todo: Clean this up (duplicated later on)
        Fragment.ObjectHistory.GetTime = Cache.Time;
        Cache.HitCount++;
        return Fragment.Data();
    }

    TObjectData Data;
    
    const int Index = Cache.Dictionary.Index(Id);
    if( Index == 0 )
    {
        Data = 0;
    }
    else
    {
        TFragment & Fragment = *Cache.Dictionary.Fragment(Index);
        Fragment.ObjectHistory.GetTime = Cache.Time;
        Cache.HitCount++;
//?        MaybeCheck();
        Data = Fragment.Data();
        Cache.Dictionary.LastIdFound        = Id        ;
        Cache.Dictionary.LastIndexFound     = Index     ;
        Cache.Dictionary.LastFragmentFound  = &Fragment ;
    }
    return Data;
}

//-----------------------------------------------------------------------------
//                  What is the hint for a cached object?
//-----------------------------------------------------------------------------
FMemoryCache::THint FMemoryCache::GetHint( TObjectId Id ) const
{
    THint Hint;
    if
    (
        // See if we can avoid a search: 
        State.Dictionary.LastIdFound == Id 
     && State.Dictionary.LastIndexFound > 0
     && State.Dictionary.LastIndexFound <= State.Dictionary.Count
     && State.Dictionary.Id(State.Dictionary.LastIndexFound) == Id
    )
    {
        Hint = State.Dictionary.LastIndexFound;
    }
    else
    {
        Hint = State.Dictionary.Index(Id);
    }
    return Hint;
}

//-----------------------------------------------------------------------------
//                       Lock a cache entry
//-----------------------------------------------------------------------------
void FMemoryCache::Lock(TObjectId Id) 
{
    TState & Cache = State;
    TFragment * Fragment = 0;
    //todo: Too much duplicated checking (here and in other functions)
    // of LastIdFound. Create a helper function to find the fragment
    // associated with an Id.
    if( Id==Cache.Dictionary.LastIdFound )
    {
        Fragment = Cache.Dictionary.LastFragmentFound;
    }
    else
    {
        const int Index = Cache.Dictionary.Index(Id);
        if( Index != 0 )
        {
            Fragment = Cache.Dictionary.Fragment(Index);
        }
    }
    if( Fragment != 0 )
    {
        Fragment->LockCount++;
    }
}

//-----------------------------------------------------------------------------
//                       Unlock a cache entry
//-----------------------------------------------------------------------------
void FMemoryCache::Unlock(TObjectId Id, BOOL Force) 
{
    TState & Cache = State;
    TFragment * Fragment = 0;
    if( Id==Cache.Dictionary.LastIdFound )
    {
        Fragment = Cache.Dictionary.LastFragmentFound;
    }
    else
    {
        const int Index = Cache.Dictionary.Index(Id);
        if( Index != 0 )
        {
            Fragment = Cache.Dictionary.Fragment(Index);
        }
    }
    if( Fragment != 0 )
    {
        if( Force )
        {
            Fragment->LockCount = 0;
        } 
        else if( Fragment->IsLocked() )
        {
            Fragment->LockCount--;
        }
    }
}

//-----------------------------------------------------------------------------
//           Estimated cost of using a fragment
//-----------------------------------------------------------------------------
// The idea here is that using a fragment has an associated cost - the higher
// the cost, the less willing we should be to use the fragment.
// The cost isn't the immediate cost of using a fragment, although that
// might be part of the cost. Instead, the cost is mostly the implied cost of
// discarding an object currently using the fragment, and then later 
// recomputing that object (since is was discarded).
//
// For example, if we discard a fragment which holds a large, frequently-used
// object, then it is likely that the object will be needed again and therefore
// recomputed. It would be better to discard a smaller or less frequently-used
// object since it is less likely the object will be needed again.
//
// The following assumptions *might* be used in estimating the cost:
//   1. Objects used frequently have a higher cost.
//   2. Larger objects have a higher cost.
//   3. Fresh objects (recently used or created) have a higher cost.
// Most of these apply only to used fragments (fragments with an object in them).
// However, we extend the idea to free fragments. Since using a free fragment
// does not require an object to be discarded, the cost is low.
inline int FMemoryCache::TFragment::Cost(TimeType Time) const
{
    const int StalenessThreshold = 20;
        // Objects which haven't been accessed in this many
        // ticks are considered very stale and likely candidates
        // for replacement.
    int Cost;
    if( IsFree() )
    {
        // Free fragments have very low cost.
        Cost = 0;
    }
    else
    {
        // Various cost functions could be chosen.
        // We want one that can be computed quickly.
        const int Staleness = this->Staleness(Time);
        Cost = 
            10      // A small cost for any discarded object.
        +   (
                Staleness >= StalenessThreshold
            ?   0
            :   (1 << StalenessThreshold) - (1 << Staleness) 
            )
        ;
    }
    return Cost;
}


//-----------------------------------------------------------------------------
//             Find the best sequence to hold a new cache entry
//-----------------------------------------------------------------------------
// This is a helper function for ::Create().
// The returned value is the first fragment in the sequence, and 
// the value returned in Count is the length of the sequence (number of fragments).
static const FMemoryCache::TFragment * FindBestSequence
(
    const FMemoryCache::TState &  Cache    // The cache in which we search.
,   int                           Size     // The size needed.
,   int                        &  Count    // Output: The number of fragments starting at returned value.
)
{
    // We search through the entire fragment list looking for a contiguous
    // sequences of fragments with enough space to hold the new object.
    // We compute the costs of using such sequences in order to find the one
    // with the lowest cost.
    
    // The prefix "C" below identifies information about the current sequence (under consideration).
    const FMemoryCache::TFragment * CFirst  ; // First fragment in the current sequence.
    const FMemoryCache::TFragment * CLast   ; // Last fragment in the current sequence.
    int               CCount  ; // Count of fragments in the current sequence.
    int               CSize   ; // Space available (in bytes) if the current sequence was merged into a single fragment.
    int               CCost   ; // Cost of using the current sequence.
    // The prefix "B" below identifies information about the best sequence found so far.
    const FMemoryCache::TFragment * BFirst  ; // First fragment in the best sequence.
    int               BCount  ; // Count of fragments in the best sequence.
    int               BCost   ; // Cost of using the best sequence.

    // Initialize the search (Initial sequence has 0 fragments starting at the first fragment).
    CFirst      = &Cache.FirstFragment();
    CCount      = 0                     ;
    BFirst      = 0                     ; // Indicates no best sequence has been found yet.
    BCost       = 0x7fffffff            ; // There must be something cheaper than this!

    // CFirst==0 marks the end of searching (no more sequences to check)
    while( CFirst != 0 )
    {
        // If CCount == 0, we have an empty sequence starting at CFirst.
        // Add one fragment, the first unlocked one we can find.
        if( CCount == 0 )
        {
            while( CFirst != 0 && CFirst->IsLocked() ) { CFirst = CFirst->Next; }
            if( CFirst != 0 )
            {
                CLast   = CFirst                    ;
                CCost   = CFirst->Cost(Cache.Time)  ;
                CSize   = CFirst->Size              ;
                CCount++;
            }
        }
        // Here we have a sequence of fragments CFirst .. CLast. We extend the
        // sequence (add fragments at the end) until the sequence has enough
        // space to hold the new object.
        while( CFirst != 0 && CSize < Size && CLast->Next != 0 )
        {
            // Add next fragment to the sequence. When we add a fragment to the
            // sequence we can use the space holding the TFragment header
            // to hold the new object.
            CLast = CLast->Next ; 
            CCount++;
            CCost += CLast->Cost(Cache.Time);
            CSize += CLast->Size + CLast->Overhead();
            if( CLast->IsLocked() )
            {
                // Ooops, we just added a locked fragment to the list. This is
                // not good, since locked fragments must be left alone and cannot
                // form part of the new object. We restart the search at the fragment
                // beyond the locked (last) fragment:
                CFirst = CLast->Next;
                while( CFirst != 0 && CFirst->IsLocked() ) { CFirst = CFirst->Next; }
                CCount = 0;
                if( CFirst != 0 )
                {
                    CLast = CFirst                   ;
                    CSize = CFirst->Size             ;
                    CCost = CFirst->Cost(Cache.Time) ;
                    CCount++;
                }
            }
        }
        if( CSize < Size || CFirst == 0)
        {
            // There wasn't enough room in the sequence (even extended to the
            // end of the data space) to hold the object. We are done.
            CFirst = 0;
        }
        else if( CCount == 1 && CFirst->IsFree() )
        {
            // Special case: We have a free fragment big enough to hold
            // the new object. This gives us a chance to create the new
            // object without discarding existing objects. However, there
            // might be smaller free fragments which are also large enough.
            // Let's scan the rest of the fragments and find the smallest
            // free fragment large enough to hold the new object. Such a 
            // fragment will form our best sequence. 
            BFirst = CFirst         ;
            BCost  = BFirst->Size   ; // Use the size as cost (the smaller the better).
            BCount = 1              ;
            CFirst = CFirst->Next   ;
            while( CFirst != 0 )
            {
                if( CFirst->IsFree() && CFirst->Size >= Size )
                {
                    // We have a free fragment large enough to hold the new object.
                    CCost = CFirst->Size; // Use the size as the cost.
                    if( CCost < BCost )
                    {
                        // We have a new best sequence.
                        BCost   = CCost     ;
                        BFirst  = CFirst    ;
                    }
                }
                CFirst = CFirst->Next   ;
            }
            // Note: At this point, we have found a best sequence and exhausted the fragment list.
        }
        else
        {
            // We have a sequence which holds the new object. Is it better
            // (less costly) than the best sequence found so far?
            if( CCost < BCost )
            {
                // We have a new best sequence.
                BFirst  = CFirst    ;
                BCount  = CCount    ;
                BCost   = CCost     ;
            }

            // Move on to the next sequence. Rather than restarting the search
            // at CFirst->Next, and recomputing costs and sizes, we use the following
            // fact: since BFirst..BLast is the smallest sequence starting at BFirst
            // which can contain the new object, then either:
            //       BFirst->Next ... BLast cannot hold the new object, 
            //   or  BFirst->Next ... BLast is the smallest sequence starting at 
            //       BFirst->Next which can contain the new object
            // In other words, we can keep searching using the same sequence
            // adjusted to remove the first fragment (CFirst).

            // Remove CFirst from the sequence:
            {
                CCost -= CFirst->Cost(Cache.Time) ;
                CSize -= CFirst->Size + CFirst->Overhead();
                CFirst = CFirst->Next       ; // This might be 0.
                CCount--;
            }
        }
    }
    Count = BCount;
    return BFirst;
}

//-----------------------------------------------------------------------------
//                Find the least recently used entry
//-----------------------------------------------------------------------------
const FMemoryCache::TFragment * FMemoryCache::TState::LRU() const
{
    // Scan all the fragments and find the least recently used (LRU) one.
    const FMemoryCache::TFragment * Fragment    = &this->FirstFragment()    ;
    TimeType          LRUGetTime  = 0x7fffffff                ;
    const FMemoryCache::TFragment * LRU         = 0                         ;
    while( Fragment != 0 )
    {
        if( Fragment->IsUsed() )
        {
            const TimeType GetTime = Fragment->ObjectHistory.GetTime;
            if( GetTime < LRUGetTime )
            {
                LRU         = Fragment  ;
                LRUGetTime  = GetTime   ;
            }
        }
        Fragment = Fragment->Next;
    }
    return LRU;
}

//-----------------------------------------------------------------------------
//                Add a new entry to the cache
//-----------------------------------------------------------------------------
FMemoryCache::TObjectData FMemoryCache::Create(TObjectId Id, int Size, int Alignment)
{
    GUARD;
    TState & Cache = State;
    const BOOL MustBeAligned = Alignment != DefaultAlignment;
    // Make sure we make a fragment big enough to hold
    // the object and any extra space needed for alignment.
    // There is some potential waste here, since we might find a fragment aligned
    // on the required boundary *without* requiring any additional space. However,
    // this simplifies the searching mechanism.
    const int SizeWithAlignmentPadding = Size + (MustBeAligned ? Alignment-1 : 0 );
    checkInput( SizeWithAlignmentPadding >= 0 );
    checkInput( SizeWithAlignmentPadding <= MaxCreateSize() );
    checkInput( Id != 0 );
    checkInput( Get(Id) == 0 );
    
    int         Count;
    FMemoryCache::TFragment * UseFragment  = const_cast<TFragment *>(FindBestSequence(Cache,SizeWithAlignmentPadding,Count));

    checkVital( UseFragment != 0, "Unable to create cache object" );
        // This check will usually pass since, if necessary, the entire
        // cache data space is used to create a new object. However, if
        // there are locked cache objects, the entire data space is not
        // available.
    
    // We have a sequence of Count fragments starting at UseFragment. 
    // We want to merge these fragments into a single fragment. 
    {
        FMemoryCache::TFragment * FreeThis = UseFragment;
        for( int MergeCount = 1; MergeCount <= Count; ++MergeCount )
        {
            Cache.Free( *FreeThis );
            FreeThis = FreeThis->Next;
        }
        Cache.MergeFreeFragments( UseFragment ); // This might change UseFragment.
        checkLogic( UseFragment->Size >= SizeWithAlignmentPadding ); // The resulting fragment must be large enough.
    }

    // (At this point Count is just a reminder of how many fragments we 
    // needed. Since we might have merged some fragments, Count no longer 
    // reflects the number of consecutive fragments we need.)

    if( Cache.Dictionary.IsFull() )
    {
        // We have enough free data space for the object, but we don't
        // have enough room in the Dictionary. We must discard an object.
        FMemoryCache::TFragment * LRU = (TFragment *)Cache.LRU();
        checkLogic( LRU != 0 ); // The dictionary is full, there must be an LRU object!

        // We want to free the LRU fragment, but there is one danger:
        // the LRU fragment might be immediately before the UseFragment in
        // which we want to put the object. This means that we must be
        // careful when we merge consecutive free fragments around LRU.
        Cache.Free( *LRU );
        const BOOL UseLRU = LRU->Next == UseFragment;
        Cache.MergeFreeFragments(LRU);
        if( UseLRU )
        {
            UseFragment = LRU;
        }
        checkLogic( !Cache.Dictionary.IsFull() );
        checkLogic( UseFragment->Size >= SizeWithAlignmentPadding );
    }

    // Sometimes there are very large alignment requirements. We don't want to 
    // waste a lot of space in the cache for padding, so for large alignment
    // padding let's split up the fragment.
    if( MustBeAligned )
    {
        BYTE * const AvailableData = UseFragment->Data();
        BYTE * const AlignedData   = RoundUp(AvailableData,Alignment);
        const int PaddingSpace = AlignedData - AvailableData;
        if( PaddingSpace >= FMemoryCache::TFragment::Overhead() + 200 ) // (Heuristic comparison to decide when to split.)
        {
            // Let's divide UseFragment into a free fragment (FragmentA)
            // and a second fragment (FragmentB). We will choose FragmentB so
            // that the data is precisely aligned as required.
            const BYTE * const DataLimit = UseFragment->DataLimit(); // 1 byte beyond last byte in fragment's data.
            const BYTE * const RightData = RoundDown(  DataLimit - Size, Alignment ); // Rightmost data chunk in UseFragment which can hold the required (aligned) data.
            const int TotalSpace = UseFragment->TotalSpace();
            
            FMemoryCache::TFragment * FragmentA = UseFragment;
            FMemoryCache::TFragment * FragmentB = (TFragment *) RoundDown(RightData-TFragment::Overhead(),int(DefaultAlignment));
            // Set up the new right-hand FragmentB:
            FragmentB->Free();
            FragmentB->LinkAfter( FragmentA );
            FragmentB->Size = DataLimit - FragmentB->Data();
            // Correct FragmentA:
            FragmentA->Size = (BYTE*)FragmentB - FragmentA->Data();

            UseFragment = FragmentB; // We want to proceed using this fragment.

            // Some checks to make sure we didn't mess up:
            checkLogic( UseFragment->Size >= Size );
            checkLogic( FragmentA->TotalSpace() + FragmentB->TotalSpace() == TotalSpace ); // Make sure we didn't miscalculate the space.
        }
    }

    // At last we are ready to add the new object to the cache.
    UseFragment->ObjectHistory.GetTime    = Cache.Time  ; // We treat Create() as an implicit Get().
    UseFragment->ObjectId                 = Id          ;
    Cache.Dictionary.Add( Id, UseFragment );

    checkLogic( UseFragment->DataOffset == UseFragment->Overhead() );
    if( MustBeAligned) 
    {
        // Set the DataOffset so that UseFragment->Data() is aligned as requested.
        UseFragment->DataOffset = RoundUp(UseFragment->Data(),Alignment) - (BYTE *)UseFragment;
    }

    // We might split the fragment into a used and free fragment, if
    // there is enough unused space to make it worthwhile.
    const BYTE * DataLimit = UseFragment->DataLimit(); // 1 byte beyond last byte in data space of fragment.
    const BYTE * UserDataLimit = UseFragment->Data() + Size; // 1 byte beyond last byte in data space available to caller
    const int UnusedSize = DataLimit - UserDataLimit;
    if( UnusedSize >= FMemoryCache::TFragment::Overhead() + 200 ) // Heuristic amount to decide when to split.
    {
        // Create the new free fragment:
        FMemoryCache::TFragment * NewFreeFragment = (TFragment*) RoundUp( UserDataLimit, int(DefaultAlignment) );
        NewFreeFragment->Free();
        NewFreeFragment->LinkAfter( UseFragment );
        NewFreeFragment->Size = DataLimit - NewFreeFragment->Data();
        UseFragment->Size -= NewFreeFragment->TotalSpace();
        // Let's check things to make sure we didn't mess up the split:
        checkLogic( UseFragment->Size >= Size );
        checkLogic( DataLimit - (BYTE *)UseFragment == UseFragment->TotalSpace() + NewFreeFragment->TotalSpace() );
    }
    checkLogic( RoundDown( UseFragment->Data(), Alignment ) == UseFragment->Data() ); // Check alignment
    MaybeCheck();

    return UseFragment->Data();
    UNGUARD("FMemoryCache::Create");
}

//-----------------------------------------------------------------------------
//           Textual representation of a byte count
//-----------------------------------------------------------------------------
// Adds K (Kilo) or M (Mega) as appropriate.
// Notes:
//   1. This returns a pointer to a static string, so beware
//      of multiple uses of this function in the same expression.
static const char * ByteCount(int Count)
{
    static char Text[20];
    const int KiloBytes = 1024;
    const int MegaBytes = KiloBytes*KiloBytes;
    if( Count >= MegaBytes*3/4 )    
    {
        sprintf( Text, "%.2fM", float(Count)/float(MegaBytes) );
    }
    else if( Count >= KiloBytes )
    {
        sprintf( Text, "%.2fK", float(Count)/float(KiloBytes) );
    }
    else
    {
        sprintf( Text, "%i", Count );
    }
    return Text;
}

//-----------------------------------------------------------------------------
//                    Summarize the status of the cache
//-----------------------------------------------------------------------------
void FMemoryCache::Status(char *StatusText) const
{
    GUARD;
    const TState & Cache = State;
    const int MaximumStatusLength = 80; // Maximum length of returned text.
    const int MaximumFullStatusLength = 200; // Maximum length of constructed text.
    char FullText[MaximumFullStatusLength];
    char * Text = FullText;
    if( Cache.GetCount > 0 )
    {
        Text += sprintf
        ( 
            Text
        ,   " Hit=%i%%"
        ,   Cache.HitCount * 100 / Cache.GetCount
        );
    }
    Text += sprintf // Show the size and usage of the Dictionary
    ( 
        Text
    ,   " Defs=%i%% (%i)"
    ,   Cache.Dictionary.ObjectCount * 100 / Cache.Dictionary.MaxCount
    ,   Cache.Dictionary.MaxCount
    );
    // Show the size, usage, and freshness of the data space
    {
        // Compute the size in use:
        int UsedSize = 0;
        int MinSize  = 0x7fffffff;
        int MaxSize  = 0;
        // Compute how many objects are fresh
        int FreshCount = 0;
        {
            const FMemoryCache::TFragment * Fragment = &Cache.FirstFragment();
            while( Fragment != 0 )
            {
                if( Fragment->IsUsed() )
                {
                    UsedSize += Fragment->Size;
                    if( Fragment->Staleness(Cache.Time) <= 1 )
                    {
                        FreshCount++;
                    }
                    if( Fragment->Size > MaxSize )
                    {
                        MaxSize = Fragment->Size;
                    }
                    if( Fragment->Size < MinSize )
                    {
                        MinSize = Fragment->Size;
                    }
                }
                Fragment = Fragment->Next;
            }
        }
        Text += sprintf 
        (
            Text
        ,   " Data=%i%%"
        ,   UsedSize * 100 / MaxCreateSize()
        );
        if( Cache.Dictionary.ObjectCount > 0 )
        {
            Text += sprintf
            (
                Text
            ,   " %i%%=Fresh"
            ,   FreshCount * 100 / Cache.Dictionary.ObjectCount
            );
            // Note: The size measures are misleading since a cached object
            // might not fill the fragment it is in, yet we counted the
            // size of the fragment, not the size of the object.
			/* tim
            Text += sprintf 
            (
                Text
            ,   " Size:%s-"
            ,   ByteCount(MinSize)
            );
            Text += sprintf 
            (
                Text
            ,   "%s"
            ,   ByteCount(MaxSize)
            );
			*/
            Text += sprintf
            (
                Text
            ,   " Avg=%s"
            ,   ByteCount(UsedSize/Cache.Dictionary.ObjectCount)
            );
        }
    }
    // Limit the length of the text:
    FullText[MaximumStatusLength-1] =0;
    strcpy(StatusText,FullText);
    UNGUARD("FMemoryCache::Status");
}

//-----------------------------------------------------------------------------
//                    Record the passage of time
//-----------------------------------------------------------------------------
void FMemoryCache::Tick(void)
{
    // In practice, we don't have to worry about Time overflowing because
    // it would take almost 2 years (at 35 Tick()'s per second).
    // However, as the time gets large, the age of cached objects
    // might get large. This in turn might cause the 
    // function TFragment::Cost to behave badly. So 
    // we will periodically flush the cache to reset the time.
    TState & Cache = State;
    Cache.Time++;
    if( Cache.Time > 1*24*60*60*35 ) // At 35 ticks/sec, this is 1 day.
    {
        Flush();
    }
    MaybeCheck();
}

//-----------------------------------------------------------------------------
//                     Verify the state of the cache
//-----------------------------------------------------------------------------
void FMemoryCache::AssertValid(void) const
{
    GUARD;
    const TState & Cache = State;
    Cache.Check();
    UNGUARD("FMemoryCache::AssertValid");
}

//-----------------------------------------------------------------------------
//                  Merge contiguous free fragments.
//-----------------------------------------------------------------------------
void FMemoryCache::TState::MergeFreeFragments(TFragment * & Fragment)
{
    // Merge any following free fragments:
    while( Fragment->Next != 0 && Fragment->Next->IsFree() )
    {
        MergeWithNext(*Fragment);
    }
    // Merge any preceding free fragments:
    while( Fragment->Previous != 0 && Fragment->Previous->IsFree() )
    {
        Fragment = Fragment->Previous;
        MergeWithNext(*Fragment);
    }
}

//-----------------------------------------------------------------------------
//          Merge a fragment with the immediately following fragment.
//-----------------------------------------------------------------------------
void FMemoryCache::TState::MergeWithNext(TFragment & Fragment)
{
    // We are converting this fragment sequence:
    //     A, B, [C]      (There might not be fragment C)
    // Into this sequence:
    //     A+B, [C]
    FMemoryCache::TFragment * A    = &Fragment ; // First in the sequence.
    FMemoryCache::TFragment * B    = A->Next   ; // Second in the sequence.
    FMemoryCache::TFragment * C    = B->Next   ; // Third in the sequence, might be 0.
    checkInput( B != 0 );
    // The merged fragment will have the combined size of
    // the two fragments plus the size of the TFragment header
    // used in the Next fragment.
    A->Size += B->Size + B->Overhead();
    A->Next = B->Next  ;
    if( C != 0 )
    {
        C->Previous = A;
    }
}

//-----------------------------------------------------------------------------
//           Free a used fragment and discard the associated object
//-----------------------------------------------------------------------------
void FMemoryCache::TState::Free( FMemoryCache::TFragment & Fragment )
{
    if( Fragment.IsUsed() )
    {
        Dictionary.Discard(Fragment.ObjectId);
        Fragment.Free();
    }
}

//-----------------------------------------------------------------------------
//           Discard an object from the Dictionary
//-----------------------------------------------------------------------------
void FMemoryCache::TDictionary::Discard(TObjectId Id)
{
    int Index = FirstIndex(Id);
    if( Index > 0 )
    {
        if( Id == LastIdFound ) // Flush last found info
        {
            LastIdFound = 0;
        }
        TObjectId ReplacementId;
        TFragment * ReplacementFragment;
        // Rather than shifting the elements of *Info to
        // vacate the space(s) left by removing the object, we
        // will simply fill in the vacated spaces with 
        // copies of the next lower id, or with 0's if there is
        // no such id. This creates duplicate object info in the 
        // dictionary, but these duplicates don't interfere with the
        // dictionary searches (the dictionary is still sorted by object id).
        if( Index > 1 )
        {
            ReplacementId       = this->Id(Index-1)       ;
            ReplacementFragment = Fragment(Index-1) ;
        }
        else
        {
            ReplacementId = 0;
        }
        while( Index <= Count && this->Id(Index) == Id )
        {
            this->Id(Index) = ReplacementId;
            Fragment(Index) = ReplacementFragment;
            Index++;
        }
        ObjectCount--;
    }
}

//-----------------------------------------------------------------------------
//     Find where an object should be inserted in the dictionary
//-----------------------------------------------------------------------------
// For example:
//   - 1 is returned if Id is smaller than all other Id's in the dictionary.
//   - .Count+1 is returned if Id is larger than all other Id's in the dictionary.
// The Id cannot already be in the dictionary.
int FMemoryCache::TDictionary::InsertionIndex(TObjectId Id)
{
    int Low     = 1     ; // Index of lower extreme of binary search.
    int High    = Count ; // Index of upper extreme of binary search.
    while( Low < High )
    {
        const int Middle = (High+Low)>>1;
        const TObjectId MiddleId = this->Id(Middle);
        if( Id > MiddleId ) 
        {
            // Keep searching in Middle+1..High
            Low = Middle + 1;
        }
        else if( Id < MiddleId ) 
        {
            // Keep searching in Low..Middle
            High = Middle;
        }
        else
        {
            checkLogic(FALSE);
        }
    }
    // Here, Low == High. Both indicate roughly
    // where we would insert the object. We would insert the
    // object either at Low or Low adjusted
    // to skip over the (possibly repeated) object at [Low].
    int Index = Low;
    while( Index <= Count && Id > this->Id(Index) )
    {
        Index++;
    }
    return Index;
}

//-----------------------------------------------------------------------------
//           Find the first index of an object in the dictionary 
//-----------------------------------------------------------------------------
// Notes:
//   1. If the object appears at most once in the dictionary, this
//      function behaves precisely as ::Index() does. Otherwise,
//      this function returns the lowest index (into the dictionary)
//      which defines the given object id.
int FMemoryCache::TDictionary::FirstIndex(TObjectId Id)
{
    int Index = this->Index(Id);
    while( Index > 1 && this->Id(Index-1) == Id )
    {
        Index--;
    }
    return Index;
}

//-----------------------------------------------------------------------------
//           Add an object to the dictionary.
//-----------------------------------------------------------------------------
// Cannot be called if IsFull().
// Notes:
//   1. If Create() performance needs improving, this is a good place
//      to look since this function reorganizes the dictionary
//      which can be expensive.
void FMemoryCache::TDictionary::Add(TObjectId ObjectId, TFragment * Fragment)
{
    checkInput( !IsFull() );
    int Index = this->InsertionIndex(ObjectId);
    if( Count < Index && Index <= MaxCount )
    {
        // There is space at the end to put this object.
        Count++;
        Index = Count;
    }
    // Can we reuse a duplicated entry immediately before?
    else if( Index > 2 && this->Id(Index-1) == this->Id(Index-2) )
    {
        Index--;
    }
    // Can we reuse the a duplicated entry immediately after?
    else if( Index < Count && this->Id(Index) == this->Id(Index+1) )
    {
        // We can reuse the entry at Index.
    }
    else if( Count < MaxCount )
    {
        // Shift entries to make room for the new entry.
        const int ShiftCount = Count - Index + 1;
        memmove( &(this->Id(Index+1)), &(this->Id(Index)), ShiftCount*sizeof( this->Id(1) ) );
        memmove( &(this->Fragment(Index+1)), &(this->Fragment(Index)), ShiftCount*sizeof( this->Fragment(1) ) );
        Count++;
    }
    else 
    {
        // We have no choice - we must compress the 
        // dictionary to make room for the new object.
        const int OldCount = Count;
        Count = 0;
        TObjectId PreviousId = 0; // Set to 0 to clear out first entries which might be 0.
        int CopyFrom = 1        ;
        int CopyTo   = CopyFrom ;
        BOOL InsertionFound = FALSE; // True when we found (at Index) where to insert the new id.
        for( int WhichCopy = 1; WhichCopy <= OldCount; ++WhichCopy )
        {
            const TObjectId CopyId = this->Id(CopyFrom);
            if( CopyId == PreviousId )
            {
                // The id duplicates the previous id, so
                // we don't need to add this one.
                // This also catches 0 id's added to the beginning
                // of the dictionary.
            }
            else 
            {
                Count++;
                this->Id(CopyTo) = this->Id(CopyFrom);
                this->Fragment(CopyTo) = this->Fragment(CopyFrom);
                PreviousId = CopyId;
                CopyTo++;
                if( !InsertionFound && CopyId > ObjectId )
                {
                    // We have found the place to insert the new id.
                    Index = Count;
                    InsertionFound = TRUE;
                }
            }
            CopyFrom++;
        }
        checkLogic( Count == ObjectCount );
        if( InsertionFound )
        {
            // Shift entries to make room for the new entry.
            const int ShiftCount = Count - Index + 1;
            memmove( &(this->Id(Index+1)), &(this->Id(Index)), ShiftCount*sizeof( this->Id(1) ) );
            memmove( &(this->Fragment(Index+1)), &(this->Fragment(Index)), ShiftCount*sizeof( this->Fragment(1) ) );
            Count++;
        }
        else
        {
            // The new object must belong at the end.
            Count++;
            Index = Count;
        }
    }
    checkLogic( Count <= MaxCount );
    this->Id(Index)         = ObjectId    ;
    this->Fragment(Index)   = Fragment    ;
    ObjectCount++;

    LastIdFound        = ObjectId  ;
    LastIndexFound     = Index     ;
    LastFragmentFound  = Fragment  ;

}

//-----------------------------------------------------------------------------
//                   Debug function: Check the cache
//-----------------------------------------------------------------------------
void FMemoryCache::TState::Check() const 
{
    if( CHECK_STATE )
    {
        const FMemoryCache::TState & Cache = *this;
        Cache.Dictionary.Check();

        // Make sure the first fragment is at the beginning of the data space:
        checkState( (BYTE *)&Cache.FirstFragment() == Cache.DataSpace );
        // Check all the fragments.
        const TFragment * Fragment = &Cache.FirstFragment();
        const TFragment * LastFragment;
        int FragmentCount = 0;
        int UsedCount = 0;
        while( Fragment != 0 )
        {
            Fragment->Check();
            TFragment * Next = Fragment->Next;
            if( Next != 0 )
            {
                // Make sure each fragment is contiguous with the next fragment.
                checkState( (BYTE*)Fragment + Fragment->Overhead() + Fragment->Size == (BYTE *)Next );
                // Make sure there are no contiguous free fragments:
                checkState( Fragment->IsUsed() || Next->IsUsed() );
            }
            if( Fragment->IsUsed() )
            {
                UsedCount++;
                checkState( Fragment->ObjectHistory.GetTime <= Cache.Time );
                checkState( Fragment->ObjectId != 0 );
            }
            FragmentCount++;
            LastFragment = Fragment;
            Fragment = Fragment->Next;
        }
        checkState( UsedCount == Cache.Dictionary.ObjectCount );
        // Make sure the last fragment covers the end of the data space:
        checkState( (BYTE*)LastFragment + LastFragment->Overhead()+LastFragment->Size == Cache.DataSpace+Cache.DataSpaceSize );
    }
}

//-----------------------------------------------------------------------------
//                   Debug function: Check the cache
//-----------------------------------------------------------------------------
void FMemoryCache::Check() const 
{
    if( CHECK_STATE )
    {
        State.Check();
    }
}

//-----------------------------------------------------------------------------
//           Debug function: Turn automatic checking on or off
//-----------------------------------------------------------------------------
void FMemoryCache::DoChecking(BOOL Check)
{
    State.Checking = DebugCode && Check;
}

//-----------------------------------------------------------------------------
//           Debug function: Is automatic checking on?
//-----------------------------------------------------------------------------
BOOL FMemoryCache::Checking() const // Are automatic checks on?
{
    return DebugCode && State.Checking;
}

//-----------------------------------------------------------------------------
//           Debug function: Dump all information to a file.
//-----------------------------------------------------------------------------
void FMemoryCache::Dump(const char * FileName) const
{
    if( TRUE || DebugCode )
    {
        const FMemoryCache::TState & Cache = State;
        FILE * File = fopen( FileName, "w" );
        checkVital( File != 0, "File error" );
        fprintf( File, "Cache Info:\n" );
        fprintf( File, "    DataSpace       : %p\n", Cache.DataSpace      );
        fprintf( File, "    DataSpaceSize   : %i\n", Cache.DataSpaceSize  );
        fprintf( File, "    MaxCreateSize   : %i\n", Cache.MaxCreateSize  );
        fprintf( File, "    Time            : %i\n", Cache.Time           );
        fprintf( File, "Dictionary:\n" );
        fprintf( File, "    MaxCount        : %i\n", Cache.Dictionary.MaxCount      );
        fprintf( File, "    Count           : %i\n", Cache.Dictionary.Count         );
        fprintf( File, "    ObjectCount     : %i\n", Cache.Dictionary.ObjectCount   );
        fprintf( File, "              Id    Fragment   Data\n" );
        fprintf( File, "           -------- -------- --------\n" );
        for( int WhichObject = 1; WhichObject <= Cache.Dictionary.Count; WhichObject++ )
        {
            fprintf
            ( 
                File
            ,   "    [%4i] %08x %p %p\n"
            ,   WhichObject
            ,   Cache.Dictionary.Id(WhichObject)
            ,   Cache.Dictionary.Fragment(WhichObject)
            ,   Cache.Dictionary.Fragment(WhichObject)->Data()
            );
        }
        fprintf( File, "Fragments:\n" );
        const TFragment * Fragment = &Cache.FirstFragment();
        fprintf(File,"    Fragment   Next   Previous  Size   ObjectId GetTime\n");
        fprintf(File,"    -------- -------- -------- ------- -------- -------\n");
        while( Fragment != 0 )
        {
            fprintf
            (
                File
            ,   "    %p%c %p %p %7i %08x %7i\n"
            ,   Fragment
            ,   Fragment->IsLocked() ? '*' : ' '
            ,   Fragment->Next
            ,   Fragment->Previous
            ,   Fragment->Size
            ,   Fragment->ObjectId
            ,   Fragment->ObjectHistory.GetTime
            );
            Fragment = Fragment->Next;
        }
        fclose(File);
    }
}
