VERSION 4.00
Begin VB.Form frmPopups2 
   Caption         =   "frmMorePopups"
   ClientHeight    =   3975
   ClientLeft      =   4785
   ClientTop       =   2835
   ClientWidth     =   6690
   Height          =   4620
   Icon            =   "Popups2.frx":0000
   Left            =   4725
   LinkTopic       =   "Form1"
   ScaleHeight     =   3975
   ScaleWidth      =   6690
   ShowInTaskbar   =   0   'False
   Top             =   2250
   Width           =   6810
   Begin VB.Menu ClassBrowser 
      Caption         =   "ClassBrowser"
      Begin VB.Menu ClassEditScript 
         Caption         =   "&Edit ... Script..."
      End
      Begin VB.Menu ClassEditActor 
         Caption         =   "Default &Properties..."
      End
      Begin VB.Menu ClassCreateNew 
         Caption         =   "&Create New Class Below..."
      End
      Begin VB.Menu cbDelete 
         Caption         =   "&Delete This Class"
      End
      Begin VB.Menu ZZOATS 
         Caption         =   "-"
      End
      Begin VB.Menu ClassLoad 
         Caption         =   "&Load Classes..."
      End
      Begin VB.Menu ClassSave 
         Caption         =   "&Save Classes..."
      End
   End
   Begin VB.Menu ActorRtClick 
      Caption         =   "ActorRtClick"
      Begin VB.Menu arProps 
         Caption         =   "Actor &Properties (3 selected)..."
      End
      Begin VB.Menu arMoverKeyframe 
         Caption         =   "Mover &Keyframe"
         Begin VB.Menu arKey0 
            Caption         =   "Key &0 (Base)"
         End
         Begin VB.Menu arKey1 
            Caption         =   "Key &1"
         End
         Begin VB.Menu arKey2 
            Caption         =   "Key &2"
         End
         Begin VB.Menu arKey3 
            Caption         =   "Key &3"
         End
      End
      Begin VB.Menu ZACTA 
         Caption         =   "-"
      End
      Begin VB.Menu arSelNone 
         Caption         =   "Select &None"
      End
      Begin VB.Menu arSelectAllOfType 
         Caption         =   "&Select all <type> actors"
      End
      Begin VB.Menu arRememberClass 
         Caption         =   "&Make this the current class"
      End
      Begin VB.Menu ZARA 
         Caption         =   "-"
      End
      Begin VB.Menu arFloor 
         Caption         =   "Place on &Floor"
      End
      Begin VB.Menu arCeiling 
         Caption         =   "&Place on &Ceiling"
      End
      Begin VB.Menu arFloorAbove 
         Caption         =   "Floor &Above"
      End
      Begin VB.Menu arBelow 
         Caption         =   "Floor &Below"
      End
      Begin VB.Menu ZPIFFY 
         Caption         =   "-"
      End
      Begin VB.Menu arDuplicate 
         Caption         =   "D&uplicate"
      End
      Begin VB.Menu arDelete 
         Caption         =   "&Delete"
      End
   End
   Begin VB.Menu PolyRtClick 
      Caption         =   "PolyRtClick"
      Begin VB.Menu prProperties 
         Caption         =   "&Surface Properties (1 Selected)..."
      End
      Begin VB.Menu ZORROW 
         Caption         =   "-"
      End
      Begin VB.Menu prAlignSelected 
         Caption         =   "&Align Selected"
         Begin VB.Menu paFloorCeiling 
            Caption         =   "Align As &Floor/Ceiling"
         End
         Begin VB.Menu paOneTile 
            Caption         =   "Align &One Tile"
         End
         Begin VB.Menu ZALMUD 
            Caption         =   "-"
         End
         Begin VB.Menu paWallDirection 
            Caption         =   "Align Wall &Direction"
         End
         Begin VB.Menu paWallPanning 
            Caption         =   "Align Wall &Panning"
         End
         Begin VB.Menu ZOWKA 
            Caption         =   "-"
         End
         Begin VB.Menu paUnalign 
            Caption         =   "&Unalign back to default"
         End
      End
      Begin VB.Menu ZFG 
         Caption         =   "-"
      End
      Begin VB.Menu prSelect 
         Caption         =   "&Select"
         Begin VB.Menu psMatchingGroups 
            Caption         =   "Matching &Groups (Shift-G)"
         End
         Begin VB.Menu psMatchingItems 
            Caption         =   "Matching &Items (Shift-I)"
         End
         Begin VB.Menu psMatchingBrush 
            Caption         =   "Matching &Brush (Shift-B)"
         End
         Begin VB.Menu psMatchingTexture 
            Caption         =   "Matching &Texture (Shift-T)"
         End
         Begin VB.Menu ZPSAQA 
            Caption         =   "-"
         End
         Begin VB.Menu psAllAdjacents 
            Caption         =   "All Ad&jacents (Shift-J)"
         End
         Begin VB.Menu psAdjacentCoplanars 
            Caption         =   "Adjacent &Coplanars (Shift-C)"
         End
         Begin VB.Menu psAdjacentWalls 
            Caption         =   "Adjacent &Walls (Shift-W)"
         End
         Begin VB.Menu psAdjacentFloorCeils 
            Caption         =   "Adjacent &Floors/Ceils (Shift-F)"
         End
         Begin VB.Menu psAdjacentSlants 
            Caption         =   "Adjacent &Slants (Shift-S)"
         End
         Begin VB.Menu ZPSOUGH 
            Caption         =   "-"
         End
         Begin VB.Menu psReverse 
            Caption         =   "Reverse (Shift-Q)"
         End
         Begin VB.Menu PSQWER 
            Caption         =   "-"
         End
         Begin VB.Menu psMemorizeSet 
            Caption         =   "Memorize Set (Shift-M)"
         End
         Begin VB.Menu psRecallMemory 
            Caption         =   "&Recall Memory (Shift-R)"
         End
         Begin VB.Menu psOrMemory 
            Caption         =   "&Or with memory (Shift-O)"
         End
         Begin VB.Menu psAndMemory 
            Caption         =   "&And with memory (Shift-U)"
         End
         Begin VB.Menu psXorMemory 
            Caption         =   "&Xor with memory (Shift-X)"
         End
      End
      Begin VB.Menu prSelectNone 
         Caption         =   "Select &None"
      End
      Begin VB.Menu ZOUST 
         Caption         =   "-"
      End
      Begin VB.Menu prApplyTex 
         Caption         =   "Apply &Texture"
      End
   End
End
Attribute VB_Name = "frmPopups2"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub arKey0_Click()
    Call Ed.Server.SetProp("Actor", "Properties", "KeyNum=0")
    Call Ed.Server.Exec("LEVEL REDRAW")
    frmActorProperties.NoteClassChange
End Sub

Private Sub arKey1_Click()
    Call Ed.Server.SetProp("Actor", "Properties", "KeyNum=1")
    Call Ed.Server.Exec("LEVEL REDRAW")
    frmActorProperties.NoteClassChange
End Sub

Private Sub arKey2_Click()
    Call Ed.Server.SetProp("Actor", "Properties", "KeyNum=2")
    Call Ed.Server.Exec("LEVEL REDRAW")
    frmActorProperties.NoteClassChange
End Sub

Private Sub arKey3_Click()
    Call Ed.Server.SetProp("Actor", "Properties", "KeyNum=3")
    Call Ed.Server.Exec("LEVEL REDRAW")
    frmActorProperties.NoteClassChange
End Sub

Private Sub cbDelete_Click()
    frmClassBrowser.Delete_Click
End Sub

'
' ClassBrowser
'

Private Sub ClassEditScript_Click()
    frmClassBrowser.EditScript_Click
End Sub

Private Sub ClassCreateNew_Click()
    frmClassBrowser.NewClass_Click
End Sub

Private Sub ClassEditActor_Click()
    frmClassBrowser.EditDefActor_Click
End Sub

Private Sub ClassLoad_Click()
    frmClassBrowser.LoadClass_Click
End Sub

Private Sub ClassSave_Click()
    frmClassBrowser.SaveClass_Click
End Sub

'
' ActorRtClick
'

Private Sub arProps_Click()
    frmActorProperties.GetSelectedActors
End Sub

Private Sub arCeiling_Click()
    Ed.Server.Exec "ACTOR MOVETO CEILING"
End Sub

Private Sub arDelete_Click()
    Ed.Server.Exec "ACTOR DELETE"
End Sub

Private Sub arDuplicate_Click()
    Ed.Server.Exec "ACTOR DUPLICATE"
End Sub

Private Sub arFloor_Click()
    Ed.Server.Exec "ACTOR MOVETO FLOOR"
End Sub

Private Sub arSelectAllOfType_Click()
    Ed.Server.Exec "ACTOR SELECT OFCLASS CLASS=" & GPopupActorClass
End Sub

Private Sub arSelNone_Click()
    Ed.Server.Exec "ACTOR SELECT NONE"
End Sub

Private Sub arBelow_Click()
    Ed.Server.Exec "ACTOR MOVETO FLOORBELOW"
End Sub

Private Sub arFloorAbove_Click()
    Ed.Server.Exec "ACTOR MOVETO FLOORABOVE"
End Sub

Private Sub arRememberClass_Click()
    If GPopupActorClass <> "" Then
        Ed.Server.Exec "ACTOR SET ADDCLASS=" & GPopupActorClass
        frmMain.ActorCombo.List(0) = GPopupActorClass
        frmMain.ActorCombo.ListIndex = 0
    End If
End Sub

'
' PolyRtClick
'

Private Sub prProperties_Click()
    frmSurfaceProps.Show
End Sub

Private Sub prSelect_Click()
    '
End Sub

Private Sub prSelectNone_Click()
    Ed.Server.Exec "POLY SELECT NONE"
End Sub

Private Sub prAlignSelected_Click()
    '
End Sub

Private Sub prApplyTex_Click()
    Ed.Server.Exec "POLY SET TEXTURE=" & frmMain.TextureCombo.List(0)
End Sub

Private Sub prGrabTex_Click()
    '
End Sub

'
' PolyRtClick: Alignment
'

Private Sub paFloorCeiling_Click()
    Ed.Server.Exec "POLY TEXALIGN FLOOR"
End Sub

Private Sub paOneTile_Click()
    Ed.Server.Exec "POLY TEXALIGN ONETILE"
End Sub

Private Sub paUnalign_Click()
    Ed.Server.Exec "POLY TEXALIGN DEFAULT"
End Sub

Private Sub paWallDirection_Click()
    Ed.Server.Exec "POLY TEXALIGN WALLDIR"
End Sub

Private Sub paWallPanning_Click()
    Ed.Server.Exec "POLY TEXALIGN WALLPAN"
End Sub


'
' PolyRtClick: Selection
'

Private Sub psAdjacentCoplanars_Click()
    Ed.Server.Exec "POLY SELECT ADJACENT COPLANARS"
End Sub

Private Sub psAdjacentFloorCeils_Click()
    Ed.Server.Exec "POLY SELECT ADJACENT FLOORS"
End Sub

Private Sub psAdjacentSlants_Click()
    Ed.Server.Exec "POLY SELECT ADJACENT SLANTS"
End Sub

Private Sub psAdjacentWalls_Click()
    Ed.Server.Exec "POLY SELECT ADJACENT WALLS"
End Sub

Private Sub psAllAdjacents_Click()
    Ed.Server.Exec "POLY SELECT ADJACENT ALL"
End Sub

Private Sub psAndMemory_Click()
    Ed.Server.Exec "POLY SELECT MEMORY UNION"
End Sub

Private Sub psMatchingBrush_Click()
    Ed.Server.Exec "POLY SELECT MATCHING BRUSH"
End Sub

Private Sub psMatchingGroups_Click()
    Ed.Server.Exec "POLY SELECT MATCHING GROUPS"
End Sub

Private Sub psMatchingItems_Click()
    Ed.Server.Exec "POLY SELECT MATCHING ITEMS"
End Sub

Private Sub psMatchingTexture_Click()
    Ed.Server.Exec "POLY SELECT MATCHING TEXTURE"
End Sub

Private Sub psMemorizeSet_Click()
    Ed.Server.Exec "POLY SELECT MEMORY SET"
End Sub

Private Sub psOrMemory_Click()
    Ed.Server.Exec "POLY SELECT MEMORY INTERSECTION"
End Sub

Private Sub psRecallMemory_Click()
    Ed.Server.Exec "POLY SELECT MEMORY RECALL"
End Sub

Private Sub psReverse_Click()
    Ed.Server.Exec "POLY SELECT REVERSE"
End Sub

Private Sub psXorMemory_Click()
    Ed.Server.Exec "POLY SELECT MEMORY XOR"
End Sub
