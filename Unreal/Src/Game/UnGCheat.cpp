/*
==============================================================================
UnCheat.cpp: Unreal cheats

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0, Calling method=__fastcall

Description:
    This provides an implementation for the virtual functions
    defined in class FCheat.

Revision history:
    * 08/11/96: Created by Mark
==============================================================================
*/

#include "UnGame.h"
#include "UnCheat.h"
#include "UnFActor.h"
#include "UnInput.h"
#include "UnParse.h"

class FGameCheat : public FCheat
{
public:
    FGameCheat();
    virtual void DoAdjustments(INDEX iPlayer, FLOAT * Values); // Overrides the base class function.
    virtual BOOL DoCheatCommands  // Overrides the base class function.
    (
        const char    * Text
    ,   AActor        * Player
    ,   ULevel        * Level    
    ,   FOutputDevice * Out
    );
    virtual void ApplyPendingCheats
    (
        INDEX   iPlayer    // Index of player        
    );
};

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
FGameCheat::FGameCheat()
{
    GCheat = this;
}

//----------------------------------------------------------------------------
//      
//----------------------------------------------------------------------------
void FGameCheat::ApplyPendingCheats(INDEX iPlayer)
{
    FActor & Player = FActor::Actor(iPlayer);
    APawn  & Pawn   = Player.Pawn();
    if( GetAllWeapons )
    {
        UClass * Class;
        FOR_ALL_TYPED_RES(Class,RES_Class,UClass)
            if
            (  
                    Class->IsKindOf( GClasses.Weapon ) 
                &&  Class != GClasses.Weapon 
                &&  FActor::Actor(Class->DefaultActor).Weapon().OwningSet > 0
            )
            {
                if( Player.InventoryItem(Class) == 0 ) // If the pawn doesn't already have the weapon...
                {
                    FActor & NewActor = FActor::Actor( GLevel->SpawnActor(Class,NAME_NONE,&Player.Location) );
                    AWeapon & Weapon = NewActor.Weapon();
                    AInventory & NewInventory = NewActor.Inventory();
                    NewInventory.bInPickupState = 0     ;
                    NewInventory.bHidden        = 1     ;
                    NewActor.SetActorCollision(FALSE);
                    NewInventory.bCollideWorld  = 0     ;
                    NewInventory.bAnimate       = TRUE  ;
                    NewInventory.AnimSeq        = 0     ;
                    NewInventory.MeshMap        = NewInventory.PlayerViewMesh;
                    NewInventory.DrawScale      = NewInventory.PlayerViewScale;
                    Pawn.AmmoCount[Weapon.AmmoType] = Pawn.AmmoCapacity[Weapon.AmmoType];
                    if( Player.Send_AddInventory(NewActor.iMe) == 1 )
                    {
                        NewActor.Send_PickupNotify( NewActor.iMe );
                    }
                }
            }
        END_FOR_ALL_TYPED_RES;
        GetAllWeapons = FALSE;
    }
    if( Spawn != 0 )
    {
    	FCoords Coords = GMath.UnitCoords;
    	Coords.DeTransformByRotation(Pawn.ViewRot);
        FVector Location = Pawn.Location + SpawnDistance * Coords.XAxis;
        GLevel->SpawnActor(Spawn,NAME_NONE,&Location);
        Spawn = 0;
    }
}

//----------------------------------------------------------------------------
//      Do any adjustments based on cheats and user input
//----------------------------------------------------------------------------
void FGameCheat::DoAdjustments(INDEX iPlayer, FLOAT * Values)
{
    if( Adjustment != 0 && (GServer.Ticks&0x7)==0 )
    {
        char Message[100];
        FActor * Actor = FActor::Handle(iPlayer);
        AWeapon * Weapon = Actor==0 ? 0 : Actor->iWeapon==INDEX_NONE ? 0 : &FActor::Weapon(Actor->iWeapon);
        // Let's use the numeric keypad to define groups of directional
        // keys (up and down, left and right).
        // Below, we use the following names to define a standard key layout:
        //   Up    : 8  for up (or forward, or increase)
        //   Down  : 2 for down (or backward, or decrease)
        //   Left  : 4 for left 
        //   Right : 6 for right
        //   Up2/Down2: 7/1
        //   Up3/Down3: 9/3
        //   Up4/Down4: -/+
        //   Left2/Right2: 1/3
        //   Left3/Right3: 7/9
        //   Left4/Right4: 0/.
        // Note that there  is some overlap, since sometimes you
        // may need more left/right pairs and other times you may
        // need more up/down pairs.
        // The shift key will increase the speed. Double-press shift for even more speed.
        const FInput::TSwitchState * Switches = GInput.Switches;
        const int  Up      = Switches[FInput::S_N8].IsOn? 1 : 0;
        const int  Down    = Switches[FInput::S_N2].IsOn? 1 : 0;
        const int  Up2     = Switches[FInput::S_N7].IsOn? 1 : 0;
        const int  Down2   = Switches[FInput::S_N1].IsOn? 1 : 0;
        const int  Up3     = Switches[FInput::S_N9].IsOn? 1 : 0;
        const int  Down3   = Switches[FInput::S_N3].IsOn? 1 : 0;
        const int  Up4     = Switches[FInput::S_NMinus].IsOn? 1 : 0;
        const int  Down4   = Switches[FInput::S_NPlus].IsOn? 1 : 0;
        const int  Left    = Switches[FInput::S_N4].IsOn? 1 : 0;
        const int  Right   = Switches[FInput::S_N6].IsOn? 1 : 0;
        const int  Left2   = Switches[FInput::S_N1].IsOn? 1 : 0;
        const int  Right2  = Switches[FInput::S_N3].IsOn? 1 : 0;
        const int  Left3   = Switches[FInput::S_N7].IsOn? 1 : 0;
        const int  Right3  = Switches[FInput::S_N9].IsOn? 1 : 0;
        const int  Left4   = Switches[FInput::S_N0].IsOn? 1 : 0;
        const int  Right4  = Switches[FInput::S_NPeriod].IsOn? 1 : 0;

        const int Speed = 
            GInput.Switches[FInput::S_Shift].IsDouble   ?   8
        :   GInput.Switches[FInput::S_Shift].IsOn       ?   4
        :   1
        ;

        //---------------------------------------
        //         Weapon positioning?
        //---------------------------------------
        // Up/Down   = Forward/Backwards
        // Up2/Down2 = Upwards/Downwards
        // Up3/Down3 = Pitch up/down
        // Up4/Down4 = Scale up/down
        if( Adjustment == AdjustWeaponPosition && Weapon != 0 )
        {
            Weapon->DrawForward += 0.125 * Up      * Speed;
            Weapon->DrawForward -= 0.125 * Down    * Speed;
            Weapon->DrawDown    -= 0.125 * Up2     * Speed;
            Weapon->DrawDown    += 0.125 * Down2   * Speed;
            Weapon->DrawPitch   +=  10   * Up3     * Speed;
            Weapon->DrawPitch   -=  10   * Down3   * Speed;
            Weapon->DrawScale   += 0.01  * Up4     * Speed;
            Weapon->DrawScale   -= 0.01  * Down4   * Speed;
            sprintf( Message, "Scale(-+): %2.2f Pitch(9/3): %i Forw(8/2): %2.2f Down(7/1): %2.2f", Weapon->DrawScale, int(Weapon->DrawPitch), Weapon->DrawForward, Weapon->DrawDown );
            if( Actor != 0 ) Actor->Send_TextMsg( Message );
        }            
        else if( Adjustment == AdjustPlayerViewRoll && Values != 0 )
        {
            Values[0] += .05 * Up   * Speed;
            Values[0] -= .05 * Down * Speed;
            sprintf( Message, "View roll factor(8/2): %3.2f", Values[0] );
            if( Actor != 0 ) Actor->Send_TextMsg( Message );
        }
        else if( Adjustment == AdjustWeaponMotion && Values != 0 )
        {
            Values[0] += 5 * Left   * Speed;
            Values[0] -= 5 * Right  * Speed;
            Values[1] += 5 * Left2  * Speed;
            Values[1] -= 5 * Right2 * Speed;
            Values[2] += Up     * Speed;
            Values[2] -= Down   * Speed;
            sprintf( Message, "MaxYaw:(4/6): %4i MaxRoll(1/3): %4i AccelRatio(8/1): %4i", int(Values[0]), int(Values[1]), int(Values[2]) );
            if( Actor != 0 ) Actor->Send_TextMsg( Message );
        }
    }
}

// Parsing helper function: If *Text starts with Match and is followed by
// an integer value, interpret the value.
static BOOL ParseInt( const char ** Text_, const char * Match, int & Value )
{
    BOOL Matched = FALSE;
    const char * Text = *Text_;
    while( isspace(*Text) ) { Text++; }
    const int MatchLength = strlen(Match);
    Value = 0;
    if( strnicmp(Text,Match,MatchLength) == 0 && isdigit(Text[MatchLength]) )
    {
        Matched = TRUE;
        Text += MatchLength;
        while( isdigit(*Text) ) // Parse the integer value.
        {
            Value = Value * 10 + (*Text - '0');
            Text++;
        }
        *Text_ = Text;
    }
    return Matched;
}

// Parse a (possibly abbreviated) word out of the given text.
// Returns TRUE if the text starts with the given Match or
// abbreviation thereof. Returns FALSE otherwise.
static BOOL ParseAbbreviation
(
    const char * * Text_
,   const char   * Match
,   int            MinAbbreviationLength
)
{
    BOOL Matched = FALSE;
    const char * Text = *Text_;
    while( isspace(*Text) ) { Text++; }
    const int MaxAbbreviationLength = strlen(Match);
    int MatchLength = MaxAbbreviationLength;
    while( MatchLength >= MinAbbreviationLength && strnicmp( Text, Match, MatchLength ) != 0 ) { MatchLength--; }
    if( MatchLength >= MinAbbreviationLength )
    {
        Text += MatchLength;
        if( *Text == 0 || isspace(*Text) ) // End of word or end of string?
        {
            Matched = TRUE;
        }
    }
    if( Matched )
    {
        *Text_ = Text;
    }
    return Matched;    
}

// Parse the given text to look for strings of the following form:
//      Match=IntegerValue         
//      Match+IntegerValue
//      Match-IntegerValue
//      Match
// The last one is allowed only if DefaultValue != 0, and its effect is
// to set NewValue to DefaultValue.
// In addition, if AbbreviationLength != 0, Match can be abbreviated
// up to AbbreviationLength characters. If successful, NewValue is set
// to the original value appropriately modified and TRUE is returned.
// Otherwise, NewValue is undefined and FALSE is returned.
static BOOL ParseIntChange
(
    const char * * Text_
,   const char   * Match
,   int            OriginalValue
,   int          & NewValue
,   int            DefaultValue      
,   int            AbbreviationLength = 0
)
{
    BOOL Matched = FALSE;
    const char * Text = *Text_;
    NewValue = OriginalValue;
    while( isspace(*Text) ) { Text++; }
    const int MaxAbbreviationLength = strlen(Match);
    const int MinAbbreviationLength = AbbreviationLength == 0  ? MaxAbbreviationLength : AbbreviationLength;
    int MatchLength = MaxAbbreviationLength; 
    while( MatchLength >= MinAbbreviationLength && strnicmp( Text, Match, MatchLength ) != 0 ) { MatchLength--; }
    if( MatchLength >= MinAbbreviationLength )
    {
        int SignOfChange = 0; // 0 if no value is specified (maybe use default value)
        Text += MatchLength;
        if( DefaultValue != 0 &&  (*Text==0 || isspace(*Text)) ) // Allow simple match if DefaultValue != 0
        {
            NewValue = DefaultValue;
            Matched = TRUE;
        }
        else 
        {
            if( *Text == '=' )
            {
                NewValue = 0;
                SignOfChange = 1;
                Text++;
                Matched = TRUE;
            }
            else if( *Text == '+' )
            {
                SignOfChange = 1;
                Text++;
                Matched = TRUE;
            }
            else if( *Text == '-' )
            {
                SignOfChange = -1;
                Text++;
                Matched = TRUE;
            }
            Matched = Matched && isdigit(*Text);
            if( Matched )
            {
                int Change = 0;
                while( isdigit(*Text) )
                {
                    Change = Change * 10 + ( *Text - '0' );
                    Text++;
                }
                NewValue += SignOfChange * Change;
            }
        }
    }
    if( Matched )
    {
        *Text_ = Text;
    }
    return Matched;
}

//----------------------------------------------------------------------------
//                Interpret a list of cheat commands
//----------------------------------------------------------------------------
BOOL FGameCheat::DoCheatCommands
(
    const char    * Text
,   AActor        * Player // Identifies the player, since cheats affect player attributes.
,   ULevel        * Level
,   FOutputDevice * Out
) 
{
    BOOL IsOkay = TRUE;
    APawn & Pawn = (APawn &)*Player;
    Pawn.bCheated = TRUE;
    Pawn.bStatusChanged = TRUE;
    AWeapon * Weapon = Pawn.iWeapon == INDEX_NONE ? 0 : (AWeapon*)&Level->ActorList->Element(Pawn.iWeapon);
    int Value;
    while( IsOkay && *Text != 0 )
        {
        while( *Text != 0 && isspace(*Text) ) { Text++; }
        if( ParseAbbreviation(&Text,"fly",1) ) // Flying?
            {
                Pawn.bGravity = !Pawn.bGravity; 
                Pawn.bLimitRotation = Pawn.bGravity;
                Out->Log( Pawn.bGravity ? "Gravity prevails" : "You defy gravity" );
            }
        else if( ParseAbbreviation(&Text,"playerturns",11) )
            {
                Pawn.bCannotTurn = !Pawn.bCannotTurn; 
                Out->Log( Pawn.bCannotTurn ? "Cannot turn" : "Can turn" );
            }
        else if( ParseAbbreviation(&Text,"playermoves",11) )
            {
                Pawn.bCannotMove = !Pawn.bCannotMove; 
                Out->Log( Pawn.bCannotMove ? "Cannot move" : "Can move" );
            }
        else if( ParseAbbreviation(&Text,"invisible",5) ) // Invisibility?
            {
                Pawn.bHasInvisibility = !Pawn.bHasInvisibility;
                Pawn.InvisibilityTimeLeft = 100000; // Ignored if !bHasInvisibility
                Out->Log( Pawn.bHasInvisibility ? "Invisible" : "Visible" );
            }
        else if( ParseAbbreviation(&Text,"invincible",5) ) // Invincibility?
            {
                Pawn.bHasInvincibility = !Pawn.bHasInvincibility;
                Pawn.InvincibilityTimeLeft = 100000; // Ignored if !bHasInvincibility
                Out->Log( Pawn.bHasInvincibility ? "Invincible" : "Vincible" );
            }
        else if( ParseAbbreviation( &Text,"quiet",1) ) // Quiet?
            {
                Pawn.bHasSilence = !Pawn.bHasSilence;
                Pawn.SilenceTimeLeft = 100000; // Ignored if !bHasSilence
                Out->Log( Pawn.bHasSilence ? "Silence" : "Not Silence" );
            }
        else if( ParseIntChange(&Text,"armor", int(Pawn.Armor[DMT_Basic]), Value, 100, 2) ) // Armor?
            {
                Pawn.Armor[DMT_Basic] = Value;
                Out->Logf( "Armor: %i", Value );
            }
        else if( ParseIntChange(&Text,"health", int(Pawn.Health), Value, 100, 1) ) // Health?
            {
                Pawn.Health = Value;
                Out->Log( "Health" );
            }
        else if( ParseAbbreviation(&Text,"bigtime",7) ) // Cheat big-time?
            {
                DoCheatCommands
                (
                    "weapons allammo invincible"
                ,   Player
                ,   Level
                ,   Out
                );
            }
        else if( ParseAbbreviation(&Text,"allammo", 4 ) )
            {
                for( int AmmoType = 0; AmmoType < AmmoType_Count; AmmoType++ )
                {
                    Pawn.AmmoCount[AmmoType] = Pawn.AmmoCapacity[AmmoType];
                }
                Out->Log( "Ammo for all weapons" );
            }
        else if // Ammo?
            ( 
                Weapon != 0 
            &&  ParseIntChange
                (
                    &Text
                ,   "ammo"
                ,   Pawn.AmmoCount[Weapon->AmmoType]
                ,   Value
                ,   Pawn.AmmoCapacity[Weapon->AmmoType]
                ,   2
                ) 
            )
            {
                Pawn.AmmoCount[Weapon->AmmoType] = Value;
                Out->Logf( "Ammo for %s: %i", Weapon->Class->Name, Value );
            }
        else if( ParseAbbreviation(&Text,"ghost",1) ) // Float freely through world?
            {
                Pawn.bCollideWorld  = !Pawn.bCollideWorld  ;
                FActor::Actor(Player->iMe).SetActorCollision(!Player->bCollideActors);
                Out->Log( Pawn.bCollideWorld ? "Corporeal" : "Ghost" );
            }
        else if( ParseAbbreviation(&Text,"weapons",1) ) // Get all weapons?
            {
                
                GetAllWeapons = TRUE;
                Out->Log( "All weapons" );
            }
        else if( ParseAbbreviation(&Text,"lethalhit",9) ) // Next hit to monster is lethal?
            {
                LethalHit = TRUE;
                Out->Log( "Next damage to monster is lethal" );
            }
        else if( ParseAbbreviation(&Text,"slowmotion",5) ) // Slow motion?
            {
                SlowMotion = !SlowMotion;
                Out->Logf( "%s Motion", SlowMotion ? "Slow" : "Normal" );
            }
        else if( ParseAbbreviation(&Text,"slowprojectiles",5) ) // Slow projectiles?
            {
                SlowProjectiles = !SlowProjectiles;
                Out->Logf( "%s projectiles", SlowProjectiles ? "Slow" : "Normal" );
            }
        else if( ParseAbbreviation(&Text,"slowmonsters",7) )
            {
                MonsterSlowMotionValue = MonsterSlowMotionValue==0 ? 0x7 : 0;
                Out->Logf( "%s monsters", MonsterSlowMotionValue != 0 ? "Slow" : "Normal" );
            }
        else if( ParseAbbreviation(&Text,"buzzard",7) ) // Slow projectiles?
            {
                IsWizard = !IsWizard;
                Out->Logf( "Wizard: %s", IsWizard ? "True" : "False" );
            }
        else if( ParseAbbreviation(&Text,"performance",11) ) 
            {
                MeasurePerformance = !MeasurePerformance;
                Out->Logf( "Performance: %s", MeasurePerformance ? "On" : "Off" );
            }
        else if( ParseAbbreviation(&Text,"showdamage",5) )
            {
                ShowDamage = !ShowDamage;
                Out->Logf( "%s damage", ShowDamage ? "Show" : "Don't show" );
            }
        else if( ParseAbbreviation(&Text,"damage",3) )
            {
                NoDamage = !NoDamage;
                Out->Logf( "All actors: damage %s", NoDamage ? "off" : "on" );
            }
        else if( ParseAbbreviation(&Text,"brains",3) )
            {
                NoBrains = !NoBrains;
                Out->Logf( "Actors are %s", NoBrains ? "mindless" : "mindful" );
            }
        else if( ParseAbbreviation(&Text,"lethalhits",10) ) // Hits are lethal?
            {
                LethalHits = !LethalHits;
                Out->Logf( "Hits to monsters are %slethal", LethalHits ? "" : "not " );
            }
        else if( ParseAbbreviation(&Text,"adjustweapon",7) )
            {
                Adjustment = AdjustWeaponPosition;
                Out->Logf( "Adjusting weapon position" );
            }
        else if( ParseAbbreviation(&Text,"adjustweaponmotion",14) )
            {
                Adjustment = AdjustWeaponMotion;
                Out->Logf( "Adjusting weapon motion" );
            }
        else if( ParseAbbreviation(&Text,"adjustplayerviewroll",20) )
            {
                Adjustment = AdjustPlayerViewRoll;
                Out->Logf( "Adjusting player view roll" );
            }
        else if( ParseAbbreviation(&Text,"adjust",3) )
            {
                Adjustment = NoAdjustment;
                Out->Logf( "Adjustments off" );
            }
        else if( ParseAbbreviation(&Text,"spawn",5) )
            {
                BOOL DoSpawn = FALSE;
                char ClassName[101];
                if( FParse::GetWord(Text,ClassName,100) > 0 )
                {
                    Spawn = new(ClassName,FIND_Optional)UClass;
                    if( Spawn )
                    {
                        DoSpawn = TRUE;
                        if( !FParse::GetFloat( Text, SpawnDistance ) )
                        {
                            SpawnDistance = 200.0;
                        }
                    }
                }
                if( DoSpawn )
                {
                    Out->Logf( "Spawning: %s %3.1f units away", Spawn->Name, SpawnDistance );
                }
                else
                {
                    Out->Logf( "Bad spawn" );
                }
            }
        else 
            {
                Text += strlen(Text); // Skip to end
                Out->Log( "Bad cheat" );
            }
        }
    return IsOkay;
}

//----------------------------------------------------------------------------
//         The actor-dll instance of the cheat object
//----------------------------------------------------------------------------
static FGameCheat Cheat;