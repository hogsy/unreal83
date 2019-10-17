VERSION 4.00
Begin VB.Form frmMapToolbar 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Map Toolbar"
   ClientHeight    =   3345
   ClientLeft      =   3030
   ClientTop       =   9780
   ClientWidth     =   5025
   ControlBox      =   0   'False
   BeginProperty Font 
      name            =   "MS Sans Serif"
      charset         =   0
      weight          =   700
      size            =   8.25
      underline       =   0   'False
      italic          =   0   'False
      strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H80000008&
   Height          =   3705
   HelpContextID   =   114
   Icon            =   "MapCon.frx":0000
   Left            =   2970
   LinkTopic       =   "Form8"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3345
   ScaleWidth      =   5025
   ShowInTaskbar   =   0   'False
   Top             =   9480
   Width           =   5145
   Begin VB.Frame Frame8 
      Caption         =   "Selected Brushes"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   3135
      Left            =   3240
      TabIndex        =   21
      Top             =   120
      Width           =   1695
      Begin VB.Frame Frame5 
         Caption         =   "Solidity"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   1095
         Left            =   120
         TabIndex        =   33
         Top             =   1200
         Width           =   1455
         Begin VB.CommandButton SetStat 
            Caption         =   "Set"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   0
               weight          =   400
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   840
            TabIndex        =   37
            Tag             =   "Set type of selected brushes"
            Top             =   240
            Width           =   495
         End
         Begin VB.OptionButton NonSolid 
            Caption         =   "Non-solid"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   0
               weight          =   400
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   36
            Top             =   720
            Width           =   1095
         End
         Begin VB.OptionButton SemiSolid 
            Caption         =   "Semi-solid"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   0
               weight          =   400
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   35
            Top             =   480
            Width           =   1095
         End
         Begin VB.OptionButton Solid 
            Caption         =   "Solid"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   0
               weight          =   400
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   34
            Top             =   240
            Value           =   -1  'True
            Width           =   975
         End
      End
      Begin VB.TextBox GrpName 
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   120
         TabIndex        =   23
         Top             =   2760
         Width           =   1455
      End
      Begin VB.CommandButton SetGrpName 
         Caption         =   "Set Group Name"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   300
         Left            =   120
         TabIndex        =   22
         Tag             =   "Set group name of selected brushes"
         Top             =   2400
         Width           =   1455
      End
      Begin Threed.SSCommand SSCommand1 
         Height          =   495
         Left            =   1320
         TabIndex        =   38
         Tag             =   "Reset wireframe color to default"
         Top             =   600
         Width           =   255
         _Version        =   65536
         _ExtentX        =   450
         _ExtentY        =   873
         _StockProps     =   78
         Caption         =   "X"
         BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         BevelWidth      =   1
      End
      Begin Threed.SSCommand C0 
         Height          =   255
         Left            =   240
         TabIndex        =   32
         Top             =   600
         Width           =   255
         _Version        =   65536
         _ExtentX        =   450
         _ExtentY        =   450
         _StockProps     =   78
         BevelWidth      =   1
         Picture         =   "MapCon.frx":030A
      End
      Begin Threed.SSCommand C7 
         Height          =   255
         Left            =   960
         TabIndex        =   31
         Top             =   840
         Width           =   255
         _Version        =   65536
         _ExtentX        =   450
         _ExtentY        =   450
         _StockProps     =   78
         BevelWidth      =   1
         Picture         =   "MapCon.frx":095C
      End
      Begin Threed.SSCommand C6 
         Height          =   255
         Left            =   720
         TabIndex        =   30
         Top             =   840
         Width           =   255
         _Version        =   65536
         _ExtentX        =   450
         _ExtentY        =   450
         _StockProps     =   78
         BevelWidth      =   1
         Picture         =   "MapCon.frx":0FAE
      End
      Begin Threed.SSCommand C5 
         Height          =   255
         Left            =   480
         TabIndex        =   29
         Top             =   840
         Width           =   255
         _Version        =   65536
         _ExtentX        =   450
         _ExtentY        =   450
         _StockProps     =   78
         BevelWidth      =   1
         Picture         =   "MapCon.frx":1600
      End
      Begin Threed.SSCommand C4 
         Height          =   255
         Left            =   240
         TabIndex        =   28
         Top             =   840
         Width           =   255
         _Version        =   65536
         _ExtentX        =   450
         _ExtentY        =   450
         _StockProps     =   78
         BevelWidth      =   1
         Picture         =   "MapCon.frx":1C52
      End
      Begin Threed.SSCommand C3 
         Height          =   255
         Left            =   960
         TabIndex        =   27
         Top             =   600
         Width           =   255
         _Version        =   65536
         _ExtentX        =   450
         _ExtentY        =   450
         _StockProps     =   78
         BevelWidth      =   1
         Picture         =   "MapCon.frx":22A4
      End
      Begin Threed.SSCommand C2 
         Height          =   255
         Left            =   720
         TabIndex        =   26
         Top             =   600
         Width           =   255
         _Version        =   65536
         _ExtentX        =   450
         _ExtentY        =   450
         _StockProps     =   78
         BevelWidth      =   1
         Picture         =   "MapCon.frx":28F6
      End
      Begin Threed.SSCommand C1 
         Height          =   255
         Left            =   480
         TabIndex        =   25
         Top             =   600
         Width           =   255
         _Version        =   65536
         _ExtentX        =   450
         _ExtentY        =   450
         _StockProps     =   78
         BevelWidth      =   1
         Picture         =   "MapCon.frx":2F48
      End
      Begin VB.Label Label3 
         BackStyle       =   0  'Transparent
         Caption         =   "Wireframe Color:"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   24
         Top             =   360
         Width           =   1335
      End
   End
   Begin VB.CommandButton Expand 
      Caption         =   ">"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   2880
      TabIndex        =   20
      Tag             =   "See more options"
      Top             =   3000
      Width           =   255
   End
   Begin VB.CommandButton Help 
      Caption         =   "&Help"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   2280
      TabIndex        =   19
      Tag             =   "Get help"
      Top             =   3000
      Width           =   495
   End
   Begin VB.CommandButton Done 
      Caption         =   "&Done"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   1560
      TabIndex        =   18
      Tag             =   "Finish map-edit mode"
      Top             =   3000
      Width           =   615
   End
   Begin VB.Frame Frame1 
      Caption         =   "Copy"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   9.75
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   975
      Left            =   120
      TabIndex        =   0
      Top             =   1200
      Width           =   1335
      Begin VB.CommandButton MapCopyFromBrush 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "From Brush"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   2
         Tag             =   "Replace selected brushes"
         Top             =   600
         Width           =   1095
      End
      Begin VB.CommandButton MapCopyToBrush 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "To Brush"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   1
         Tag             =   "Copy the selected brush"
         Top             =   360
         Width           =   1095
      End
   End
   Begin VB.Frame Frame4 
      Caption         =   "Commands"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   9.75
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   975
      Left            =   120
      TabIndex        =   13
      Top             =   120
      Width           =   1335
      Begin VB.CommandButton MapDupe 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Duplicate"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   15
         Tag             =   "Duplicates selected brushes"
         Top             =   600
         Width           =   1095
      End
      Begin VB.CommandButton MapDel 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Delete"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   14
         Tag             =   "Deletes selected brushes"
         Top             =   360
         Width           =   1095
      End
   End
   Begin VB.Frame Frame3 
      Caption         =   "Select"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   9.75
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   2775
      Left            =   1560
      TabIndex        =   6
      Top             =   120
      Width           =   1575
      Begin VB.CommandButton MapSelectNons 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Non-solids"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   40
         Tag             =   "Select all non-solid brushes"
         Top             =   2400
         Width           =   1335
      End
      Begin VB.CommandButton MapSelectSemis 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Semi-solids"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   39
         Tag             =   "Select all semi-solid brushes"
         Top             =   2160
         Width           =   1335
      End
      Begin VB.CommandButton MapSelectNext 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Next"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   10
         Tag             =   "Select the next brush"
         Top             =   1200
         Width           =   1335
      End
      Begin VB.CommandButton MapSelectPrevious 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Previous"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   12
         Tag             =   "Select the previous brush"
         Top             =   960
         Width           =   1335
      End
      Begin VB.CommandButton MapSelectLast 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Last"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   840
         TabIndex        =   17
         Tag             =   "Select the last-added brush"
         Top             =   600
         Width           =   615
      End
      Begin VB.CommandButton MapSelectFirst 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "First"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   840
         TabIndex        =   16
         Tag             =   "Select the first-added brush"
         Top             =   360
         Width           =   615
      End
      Begin VB.CommandButton MapSelectSubtracts 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Subtracts"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   11
         Tag             =   "Select all subtractive brushes"
         Top             =   1920
         Width           =   1335
      End
      Begin VB.CommandButton MapSelectAdds 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Adds"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   8
         Tag             =   "Select all additive brushes"
         Top             =   1680
         Width           =   1335
      End
      Begin VB.CommandButton MapSelectNone 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "None"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   9
         Tag             =   "Unselect all brushes"
         Top             =   600
         Width           =   615
      End
      Begin VB.CommandButton MapSelectAll 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "All"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   7
         Tag             =   "Select all brushes"
         Top             =   360
         Width           =   615
      End
   End
   Begin VB.Frame Frame2 
      Caption         =   "Order"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   9.75
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   975
      Left            =   120
      TabIndex        =   3
      Top             =   2280
      Width           =   1335
      Begin VB.CommandButton MapOrderToBack 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "To Last"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   4
         Tag             =   "Move selected brushes to last-added"
         Top             =   600
         Width           =   1095
      End
      Begin VB.CommandButton MapOrderToFront 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "To First"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   5
         Tag             =   "Move selected brushes to first-added"
         Top             =   360
         Width           =   1095
      End
   End
End
Attribute VB_Name = "frmMapToolbar"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub Done_Click()
    Call Ed.Tools.Click("MAPEDIT")
End Sub

Private Sub Help_Click()
    ToolHelp (114)
End Sub


Private Sub Expand_Click()
    If Width = 5130 Then
        Width = 3300
    Else
        Width = 5130
    End If
End Sub

Private Sub SetStat_Click()
    Dim Flags As Long
    '
    If SemiSolid.Value Then
        Flags = Flags + PF_SEMISOLID
    ElseIf NonSolid.Value Then
        Flags = Flags + PF_NOTSOLID
    Else
        Flags = 0
    End If
    '
    Ed.Server.Exec "MAP SETBRUSH CLEARFLAGS=" & _
        Trim(Str(PF_SEMISOLID + PF_NOTSOLID)) & _
        " SETFLAGS=" & Trim(Str(Flags))
End Sub

Private Sub Form_Load()
    Width = 3300
    Call Ed.SetOnTop(Me, "MapToolbar", TOP_PANEL)
End Sub

Private Sub MapCopyFromBrush_Click()
    Ed.Server.Exec "MAP BRUSH PUT"
End Sub

Private Sub MapCopyToBrush_Click()
    Ed.Server.Exec "MAP BRUSH GET"
End Sub

Private Sub MapDel_Click()
    Ed.Server.Exec "MAP DELETE"
End Sub

Private Sub MapDupe_Click()
    Ed.Server.Exec "MAP DUPLICATE"
End Sub

Private Sub MapOrderToBack_Click()
    Ed.Server.Exec "MAP SENDTO LAST"
End Sub

Private Sub MapOrderToFront_Click()
    Ed.Server.Exec "MAP SENDTO FIRST"
End Sub

Private Sub MapSelectAdds_Click()
    Ed.Server.Exec "MAP SELECT ADDS"
End Sub

Private Sub MapSelectAll_Click()
    Ed.Server.Exec "MAP SELECT ALL"
End Sub

Private Sub MapSelectFirst_Click()
    Ed.Server.Exec "MAP SELECT FIRST"
End Sub

Private Sub MapSelectLast_Click()
    Ed.Server.Exec "MAP SELECT LAST"
End Sub

Private Sub MapSelectNext_Click()
    Ed.Server.Exec "MAP SELECT NEXT"
End Sub

Private Sub MapSelectNone_Click()
    Ed.Server.Exec "MAP SELECT NONE"
End Sub

Private Sub MapSelectNons_Click()
    Ed.Server.Exec "MAP SELECT NONSOLIDS"
End Sub

Private Sub MapSelectPrevious_Click()
    Ed.Server.Exec "MAP SELECT PREVIOUS"
End Sub

Private Sub MapSelectSemis_Click()
    Ed.Server.Exec "MAP SELECT SEMISOLIDS"
End Sub

Private Sub MapSelectSubtracts_Click()
    Ed.Server.Exec "MAP SELECT SUBTRACTS"
End Sub

Private Sub SetGrpName_Click()
    Ed.Server.Exec "MAP SETBRUSH GROUP=" & GrpName.Text
End Sub

Private Sub C0_Click()
    Ed.Server.Exec "MAP SETBRUSH COLOR=0"
End Sub

Private Sub C1_Click()
    Ed.Server.Exec "MAP SETBRUSH COLOR=1"
End Sub

Private Sub C2_Click()
    Ed.Server.Exec "MAP SETBRUSH COLOR=2"
End Sub

Private Sub C3_Click()
    Ed.Server.Exec "MAP SETBRUSH COLOR=3"
End Sub

Private Sub C4_Click()
    Ed.Server.Exec "MAP SETBRUSH COLOR=4"
End Sub

Private Sub C5_Click()
    Ed.Server.Exec "MAP SETBRUSH COLOR=5"
End Sub

Private Sub C6_Click()
    Ed.Server.Exec "MAP SETBRUSH COLOR=6"
End Sub

Private Sub C7_Click()
    Ed.Server.Exec "MAP SETBRUSH COLOR=7"
End Sub

Private Sub SSCommand1_Click()
    Ed.Server.Exec "MAP SETBRUSH COLOR=65535"
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub


