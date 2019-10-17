/*==============================================================================
UnCheat.h: Manages Unreal cheats
Used by: Various

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0, Calling method=__fastcall

Revision history:
    * 08/11/96, Created by Mark
==============================================================================*/

#ifndef _INC_UnCheat
#define _INC_UnCheat

#include "UnReal.h"

class FCheat
{
public:
    typedef enum // When playing with game parameters...
    {
        NoAdjustment    = 0
    ,   AdjustWeaponPosition    // For adjusting current weapon's pitch and position relative to player
    ,   AdjustWeaponMotion      // For adjusting all weapons' motion while player moves
    ,   AdjustPlayerViewRoll    // For adjusting amount of view rolling done while player moves
    }
    AdjustmentType;

    AdjustmentType  Adjustment; // Current adjustment in progress, or 0.
    virtual void DoAdjustments(INDEX iPlayer, FLOAT * Values = 0) {}; 
        // Do adjustments, using iPlayer as the player (if needed).
        // If adjustments need extra values, the initial values should be 
        // in Values[] and the resulting values will be in Values[].
        // DoAdjustments can be called regularly (such as each tick). If Adjustment != 0, this
        // functions gathers input, makes any adjustments, and gives a message.

    virtual BOOL DoCheatCommands
    (
        const char    * Text
    ,   AActor        * Player // Identifies the player, since cheats affect player attributes.
    ,   ULevel        * Level
    ,   FOutputDevice * Out
    ) 
    { return TRUE; }
        // Interprets the cheat commands in Text, returning TRUE if
        // there were no errors and FALSE otherwise. Uses Out to give
        // messages.

    virtual void ApplyPendingCheats
    (
        INDEX   iPlayer    // Index of player        
    )
    {}
        // This is really a link between the cheat being parsed
        // and recognized in DoCheatCommands and the cheat actually
        // taking effect. ApplyPendingCheats is called when there is
        // a locked level and the FActor functions are all available.
        // DoCheatCommands may be called when these conditions are not true.
    // Persistent flags that are toggled on/off:
    BOOL     LethalHits              ; // Pawn hits are all lethal.
    BOOL     SlowMotion              ; // Game play is in slow motion.
    BOOL     SlowProjectiles         ; // Slow down all projectiles.
    BOOL     ShowDamage              ; // Show damage to actors.
    BOOL     NoDamage                ; // No damage to all actors.
    BOOL     NoBrains                ; // Turn off monsters AI (makes them just stand there)
    BOOL     IsWizard                ; // Turn wizard powers on/off.
    BOOL     MeasurePerformance      ; // Turn performance measurements on/off

    // Values:
    int      MonsterSlowMotionValue  ; // 0 for normal motion, or the number of real ticks for each monster tick.

    // One-shot flags that can be set and later nabbed at the appropriate time:
    BOOL     GetAllWeapons           ; // Player gets all weapons
    BOOL     LethalHit               ; // Next pawn hit is lethal.
    UClass * Spawn                   ; // Class of object to spawn in front of player.
    FLOAT    SpawnDistance           ; // Distance away from player to spawn object.
};

extern UNREAL_API FCheat * GCheat;

#endif
