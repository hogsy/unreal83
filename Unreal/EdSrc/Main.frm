VERSION 4.00
Begin VB.Form frmMain 
   BackColor       =   &H00400000&
   Caption         =   "UnrealEd"
   ClientHeight    =   8115
   ClientLeft      =   2595
   ClientTop       =   3480
   ClientWidth     =   11865
   Height          =   8805
   Icon            =   "Main.frx":0000
   Left            =   2535
   LinkTopic       =   "Form1"
   ScaleHeight     =   8115
   ScaleWidth      =   11865
   Top             =   2850
   Visible         =   0   'False
   Width           =   11985
   WindowState     =   2  'Maximized
   Begin Threed.SSPanel MainBar 
      Align           =   1  'Align Top
      Height          =   435
      Left            =   0
      TabIndex        =   1
      Top             =   0
      Visible         =   0   'False
      Width           =   11865
      _Version        =   65536
      _ExtentX        =   20929
      _ExtentY        =   767
      _StockProps     =   15
      ForeColor       =   16777215
      BackColor       =   -2147483633
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   700
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      BorderWidth     =   0
      FloodColor      =   0
      ShadowColor     =   1
      Begin VB.TextBox Callback 
         Height          =   345
         Left            =   870
         MultiLine       =   -1  'True
         TabIndex        =   23
         Text            =   "Main.frx":030A
         Top             =   45
         Visible         =   0   'False
         Width           =   345
      End
      Begin VB.ComboBox CalcText 
         BackColor       =   &H00C0C0C0&
         Height          =   315
         HelpContextID   =   328
         Left            =   3555
         TabIndex        =   17
         Tag             =   "Enter calculation here, ex. 1+2*3"
         Text            =   "0"
         Top             =   60
         Width           =   2265
      End
      Begin VB.ComboBox ModeCombo 
         BackColor       =   &H00C0C0C0&
         Height          =   315
         HelpContextID   =   111
         Left            =   6480
         Style           =   2  'Dropdown List
         TabIndex        =   12
         TabStop         =   0   'False
         Tag             =   "Editing Mode"
         Top             =   60
         Width           =   2055
      End
      Begin VB.Timer Timer 
         Interval        =   60000
         Left            =   390
         Top             =   0
      End
      Begin VB.ComboBox TextureCombo 
         BackColor       =   &H00C0C0C0&
         Height          =   315
         HelpContextID   =   123
         Left            =   13440
         Style           =   2  'Dropdown List
         TabIndex        =   7
         TabStop         =   0   'False
         Tag             =   "Current texture"
         Top             =   60
         Width           =   1815
      End
      Begin VB.ComboBox GridCombo 
         BackColor       =   &H00C0C0C0&
         Height          =   315
         HelpContextID   =   111
         Left            =   9000
         Style           =   2  'Dropdown List
         TabIndex        =   5
         TabStop         =   0   'False
         Tag             =   "Grid Size"
         Top             =   60
         Width           =   855
      End
      Begin VB.ComboBox ActorCombo 
         BackColor       =   &H00C0C0C0&
         Height          =   315
         Left            =   10920
         Style           =   2  'Dropdown List
         TabIndex        =   2
         TabStop         =   0   'False
         Tag             =   "Actor class for adding new actors"
         Top             =   60
         Width           =   1695
      End
      Begin Threed.SSCommand CalcZero 
         Height          =   285
         HelpContextID   =   328
         Left            =   2715
         TabIndex        =   16
         Tag             =   "Zero the calculator value"
         Top             =   75
         Width           =   210
         _Version        =   65536
         _ExtentX        =   370
         _ExtentY        =   503
         _StockProps     =   78
         Caption         =   "0"
         BevelWidth      =   1
         RoundedCorners  =   0   'False
      End
      Begin Threed.SSCommand CalcButton 
         Default         =   -1  'True
         Height          =   285
         HelpContextID   =   328
         Left            =   3000
         TabIndex        =   15
         Tag             =   "Perform calculation"
         Top             =   75
         Width           =   495
         _Version        =   65536
         _ExtentX        =   873
         _ExtentY        =   503
         _StockProps     =   78
         Caption         =   "Calc"
         BevelWidth      =   1
         RoundedCorners  =   0   'False
      End
      Begin VB.Label ModeLabel 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Mode: "
         Height          =   210
         Left            =   5880
         TabIndex        =   13
         Top             =   105
         Width           =   615
      End
      Begin Threed.SSCommand HelpButton 
         Height          =   285
         Left            =   1080
         TabIndex        =   11
         Tag             =   "Help"
         Top             =   75
         Width           =   615
         _Version        =   65536
         _ExtentX        =   1085
         _ExtentY        =   503
         _StockProps     =   78
         Caption         =   "Help"
         BevelWidth      =   1
         RoundedCorners  =   0   'False
         AutoSize        =   1
      End
      Begin Threed.SSCommand EpicButton 
         Height          =   285
         Left            =   1800
         TabIndex        =   10
         Tag             =   "Visit Epic's Web page"
         Top             =   75
         Width           =   615
         _Version        =   65536
         _ExtentX        =   1085
         _ExtentY        =   503
         _StockProps     =   78
         Caption         =   "Epic"
         BevelWidth      =   1
         RoundedCorners  =   0   'False
      End
      Begin Threed.SSCommand RedoButton 
         Height          =   285
         Left            =   600
         TabIndex        =   9
         Tag             =   "Redo"
         Top             =   75
         Width           =   375
         _Version        =   65536
         _ExtentX        =   661
         _ExtentY        =   503
         _StockProps     =   78
         Caption         =   ">>"
         BevelWidth      =   1
         RoundedCorners  =   0   'False
      End
      Begin Threed.SSCommand UndoButton 
         Height          =   285
         Left            =   120
         TabIndex        =   8
         Tag             =   "Undo"
         Top             =   75
         Width           =   375
         _Version        =   65536
         _ExtentX        =   661
         _ExtentY        =   503
         _StockProps     =   78
         Caption         =   "<<"
         BevelWidth      =   1
         RoundedCorners  =   0   'False
      End
      Begin VB.Label TextureLabel 
         Alignment       =   1  'Right Justify
         Caption         =   "Texture: "
         Height          =   255
         Left            =   12600
         TabIndex        =   6
         Top             =   120
         Width           =   855
      End
      Begin VB.Label GridLabel 
         Alignment       =   1  'Right Justify
         Caption         =   "Grid: "
         Height          =   210
         Left            =   8520
         TabIndex        =   4
         Top             =   105
         Width           =   495
      End
      Begin VB.Label ActorLabel 
         Alignment       =   1  'Right Justify
         Caption         =   "Actor Class: "
         Height          =   195
         Left            =   9960
         TabIndex        =   3
         Top             =   105
         Width           =   975
      End
   End
   Begin Threed.SSPanel Toolbar 
      Align           =   3  'Align Left
      Height          =   7290
      Left            =   0
      TabIndex        =   18
      Top             =   435
      Visible         =   0   'False
      Width           =   2070
      _Version        =   65536
      _ExtentX        =   3651
      _ExtentY        =   12859
      _StockProps     =   15
      BackColor       =   -2147483633
      Begin VB.PictureBox Holder 
         BackColor       =   &H00808080&
         Height          =   4695
         Left            =   0
         ScaleHeight     =   4635
         ScaleWidth      =   1755
         TabIndex        =   20
         Top             =   1080
         Width           =   1815
         Begin VB.Label StatusText 
            Alignment       =   2  'Center
            BackStyle       =   0  'Transparent
            Caption         =   "Status"
            Height          =   495
            Left            =   0
            TabIndex        =   22
            Top             =   600
            Width           =   1695
         End
         Begin Threed.SSRibbon ToolIcons 
            Height          =   615
            Index           =   0
            Left            =   0
            TabIndex        =   21
            Top             =   0
            Visible         =   0   'False
            Width           =   615
            _Version        =   65536
            _ExtentX        =   1085
            _ExtentY        =   1085
            _StockProps     =   65
            BackColor       =   8421504
            PictureDnChange =   0
            RoundedCorners  =   0   'False
            BevelWidth      =   0
            Outline         =   0   'False
         End
      End
      Begin VB.VScrollBar Scroller 
         Height          =   5775
         Left            =   1815
         TabIndex        =   19
         Top             =   0
         Visible         =   0   'False
         Width           =   230
      End
   End
   Begin Threed.SSPanel BrowserPanel 
      Align           =   4  'Align Right
      Height          =   7290
      Left            =   9435
      TabIndex        =   0
      Top             =   435
      Visible         =   0   'False
      Width           =   2430
      _Version        =   65536
      _ExtentX        =   4286
      _ExtentY        =   12859
      _StockProps     =   15
      BackColor       =   -2147483633
      Begin VB.ComboBox BrowserTopicCombo 
         BackColor       =   &H00C0C0C0&
         Height          =   315
         Left            =   810
         Style           =   2  'Dropdown List
         TabIndex        =   24
         Tag             =   "Various resources you can browse"
         Top             =   45
         Width           =   1605
      End
      Begin VB.Label Label1 
         Caption         =   "Browse"
         BeginProperty Font 
            name            =   "Arial"
            charset         =   0
            weight          =   700
            size            =   9
            underline       =   0   'False
            italic          =   -1  'True
            strikethrough   =   0   'False
         EndProperty
         Height          =   300
         Left            =   75
         TabIndex        =   25
         Top             =   75
         Width           =   735
      End
   End
   Begin Threed.SSPanel CameraHolder 
      Height          =   2415
      Left            =   2040
      TabIndex        =   26
      Top             =   420
      Visible         =   0   'False
      Width           =   2655
      _Version        =   65536
      _ExtentX        =   4683
      _ExtentY        =   4260
      _StockProps     =   15
      ForeColor       =   12582912
      BevelOuter      =   0
      FloodColor      =   12582912
      Begin VB.OLE RBox 
         BackColor       =   &H00400000&
         Height          =   90
         Left            =   420
         MousePointer    =   15  'Size All
         TabIndex        =   28
         Top             =   1215
         Visible         =   0   'False
         Width           =   90
      End
      Begin VB.OLE LBox 
         BackColor       =   &H00400000&
         Height          =   90
         Left            =   465
         MousePointer    =   15  'Size All
         TabIndex        =   29
         Top             =   1005
         Visible         =   0   'False
         Width           =   90
      End
      Begin VB.OLE LBar 
         BackColor       =   &H00FFFFFF&
         Height          =   90
         Left            =   570
         MousePointer    =   7  'Size N S
         TabIndex        =   31
         Top             =   1005
         Visible         =   0   'False
         Width           =   1440
      End
      Begin VB.OLE VBar 
         BackColor       =   &H00FFFFFF&
         Height          =   1020
         Left            =   1425
         MousePointer    =   9  'Size W E
         TabIndex        =   30
         Top             =   600
         Visible         =   0   'False
         Width           =   90
      End
      Begin VB.OLE RBar 
         BackColor       =   &H00FFFFFF&
         Height          =   90
         Left            =   525
         MousePointer    =   7  'Size N S
         TabIndex        =   27
         Top             =   1215
         Visible         =   0   'False
         Width           =   1440
      End
   End
   Begin PicClip.PictureClip Pics 
      Left            =   2160
      Top             =   600
      _Version        =   65536
      _ExtentX        =   7408
      _ExtentY        =   7408
      _StockProps     =   0
      Rows            =   8
      Cols            =   8
      Picture         =   "Main.frx":0313
   End
   Begin PicClip.PictureClip HiPics 
      Left            =   2640
      Top             =   960
      _Version        =   65536
      _ExtentX        =   7408
      _ExtentY        =   7408
      _StockProps     =   0
      Rows            =   8
      Cols            =   8
      Picture         =   "Main.frx":139A5
   End
   Begin ComctlLib.StatusBar StatusBar1 
      Align           =   2  'Align Bottom
      Height          =   390
      Left            =   0
      TabIndex        =   14
      Top             =   7725
      Visible         =   0   'False
      Width           =   11865
      _Version        =   65536
      _ExtentX        =   20929
      _ExtentY        =   688
      _StockProps     =   68
      AlignSet        =   -1  'True
      SimpleText      =   ""
      i1              =   "Main.frx":27037
   End
   Begin VB.Menu File 
      Caption         =   "&File"
      Begin VB.Menu New 
         Caption         =   "&New Map"
      End
      Begin VB.Menu Open 
         Caption         =   "&Open Map"
         Shortcut        =   ^O
      End
      Begin VB.Menu Save 
         Caption         =   "&Save Map"
         Shortcut        =   ^V
      End
      Begin VB.Menu SaveAs 
         Caption         =   "Save &As..."
         Shortcut        =   ^E
      End
      Begin VB.Menu X 
         Caption         =   "-"
      End
      Begin VB.Menu ImportLevel 
         Caption         =   "&Import map"
      End
      Begin VB.Menu ExportLevel 
         Caption         =   "&Export map"
      End
      Begin VB.Menu ZSTOS 
         Caption         =   "-"
      End
      Begin VB.Menu Exit 
         Caption         =   "E&xit"
      End
   End
   Begin VB.Menu Edit 
      Caption         =   "&Edit"
      Begin VB.Menu Undo 
         Caption         =   "&Undo"
         Shortcut        =   ^Z
      End
      Begin VB.Menu Redo 
         Caption         =   "&Redo"
         Shortcut        =   ^R
      End
      Begin VB.Menu XX 
         Caption         =   "-"
      End
      Begin VB.Menu Duplicate 
         Caption         =   "&Duplicate"
         Shortcut        =   ^W
      End
      Begin VB.Menu Delete 
         Caption         =   "Dele&te"
         Shortcut        =   {DEL}
      End
      Begin VB.Menu XXX 
         Caption         =   "-"
      End
      Begin VB.Menu SelectNone 
         Caption         =   "Select &None"
      End
      Begin VB.Menu SelectAll 
         Caption         =   "Select &All"
      End
      Begin VB.Menu SelectDialog 
         Caption         =   "Select Polys..."
         Begin VB.Menu SelMatchGroups 
            Caption         =   "Matching Groups (Shift-G)"
         End
         Begin VB.Menu SelMatchItems 
            Caption         =   "Matching Items (Shift-I)"
         End
         Begin VB.Menu SelMatchBrush 
            Caption         =   "Matching Brush (Shift-B)"
         End
         Begin VB.Menu SelMatchTex 
            Caption         =   "Matching Texture (Shift-T)"
         End
         Begin VB.Menu WZYRA 
            Caption         =   "-"
         End
         Begin VB.Menu SelAllAdj 
            Caption         =   "All Adjacents (Shift-J)"
         End
         Begin VB.Menu SelCoplAdj 
            Caption         =   "Adjacent Coplanars (Shift-C)"
         End
         Begin VB.Menu SelAdjWalls 
            Caption         =   "Adjacent Walls (Shift-W)"
         End
         Begin VB.Menu SelAdjFloors 
            Caption         =   "Adjacent Floors/Ceils (Shift-F)"
         End
         Begin VB.Menu SelAdjSlants 
            Caption         =   "Adjacent Slants (Shift-S)"
         End
         Begin VB.Menu ZIJWZ 
            Caption         =   "-"
         End
         Begin VB.Menu SelReverse 
            Caption         =   "Reverse (Shift-Q)"
         End
         Begin VB.Menu WIJQZA 
            Caption         =   "-"
         End
         Begin VB.Menu SelMemorize 
            Caption         =   "Memorize Set (Shift-M)"
         End
         Begin VB.Menu SelRecall 
            Caption         =   "Recall Memory (Shift-R)"
         End
         Begin VB.Menu SelIntersection 
            Caption         =   "Or with Memory (Shift-O)"
         End
         Begin VB.Menu SelUnion 
            Caption         =   "And with Memory (Shift-U)"
         End
         Begin VB.Menu SelXor 
            Caption         =   "Xor with Memory (Shift-X)"
         End
      End
      Begin VB.Menu ZEDITZ 
         Caption         =   "-"
      End
      Begin VB.Menu MapEditMode 
         Caption         =   "&Map Edit Mode"
      End
   End
   Begin VB.Menu Brush 
      Caption         =   "&Brush"
      Begin VB.Menu BrushAdd 
         Caption         =   "&Add"
         Shortcut        =   ^A
      End
      Begin VB.Menu BrushSubtract 
         Caption         =   "&Subtract"
         Shortcut        =   ^S
      End
      Begin VB.Menu BrushIntersect 
         Caption         =   "&Intersect"
         Shortcut        =   ^N
      End
      Begin VB.Menu BrushDeintersect 
         Caption         =   "&Deintersect"
         Shortcut        =   ^D
      End
      Begin VB.Menu AddMovableBrush 
         Caption         =   "&Add Movable Brush"
      End
      Begin VB.Menu AddSpecial 
         Caption         =   "&Add Special..."
      End
      Begin VB.Menu ZRK 
         Caption         =   "-"
      End
      Begin VB.Menu ParametricSolids 
         Caption         =   "&Parametric solids"
         Begin VB.Menu ParSolRect 
            Caption         =   "&Rectangle"
         End
         Begin VB.Menu ParSolTube 
            Caption         =   "Cyllinder/Tube"
         End
         Begin VB.Menu ParSolCone 
            Caption         =   "Cone/Spire"
         End
         Begin VB.Menu ParSolLinearStair 
            Caption         =   "Linear Staircase"
         End
         Begin VB.Menu ParSolSpiralStair 
            Caption         =   "Spiral Staircase"
         End
         Begin VB.Menu CurvedStair 
            Caption         =   "Curved Staircase"
         End
         Begin VB.Menu ParSolSphereDome 
            Caption         =   "Sphere/Dome"
         End
         Begin VB.Menu ParSolHeightMap 
            Caption         =   "Height Map"
         End
      End
      Begin VB.Menu ZGYM 
         Caption         =   "-"
      End
      Begin VB.Menu Resize 
         Caption         =   "&Resize/Move..."
      End
      Begin VB.Menu BrushReset 
         Caption         =   "R&eset"
         Begin VB.Menu ResetRotation 
            Caption         =   "&Rotation"
         End
         Begin VB.Menu ResetScale 
            Caption         =   "&Scale"
         End
         Begin VB.Menu ResetPosition 
            Caption         =   "&Position"
         End
         Begin VB.Menu ZYCLUNT 
            Caption         =   "-"
         End
         Begin VB.Menu ResetAll 
            Caption         =   "&All"
         End
      End
      Begin VB.Menu ZZZ 
         Caption         =   "-"
      End
      Begin VB.Menu LoadBrush 
         Caption         =   "&Load"
         Shortcut        =   ^B
      End
      Begin VB.Menu SaveBrush 
         Caption         =   "&Save"
      End
      Begin VB.Menu SaveBrushAs 
         Caption         =   "Sa&ve As..."
      End
      Begin VB.Menu XCYZ 
         Caption         =   "-"
      End
      Begin VB.Menu BrushHull 
         Caption         =   "&Advanced.."
         Begin VB.Menu BrushImport 
            Caption         =   "&Import..."
         End
         Begin VB.Menu BrushExport 
            Caption         =   "&Export..."
         End
         Begin VB.Menu WYRZC 
            Caption         =   "-"
         End
         Begin VB.Menu AddCutaway 
            Caption         =   "&Cutaway Zone"
         End
         Begin VB.Menu BrushSliceTex 
            Caption         =   "No-&Terrain Zone"
         End
         Begin VB.Menu AddNoCut 
            Caption         =   "&No-Cut Zone"
         End
      End
   End
   Begin VB.Menu Camera 
      Caption         =   "&Camera"
      Begin VB.Menu CamAllViews 
         Caption         =   "&All Views"
      End
      Begin VB.Menu CamTwoViews 
         Caption         =   "&Persp + Overhead"
      End
      Begin VB.Menu CamPersp 
         Caption         =   "P&ersp Only"
      End
      Begin VB.Menu CamOvh 
         Caption         =   "&Overhead Only"
      End
      Begin VB.Menu ZILBERT 
         Caption         =   "-"
      End
      Begin VB.Menu CamOpenFree 
         Caption         =   "&Open Free Camera"
      End
      Begin VB.Menu CamCloseAllFree 
         Caption         =   "&Close All Free Cameras"
      End
      Begin VB.Menu CameraResetAll 
         Caption         =   "&Reset All"
      End
   End
   Begin VB.Menu Options 
      Caption         =   "&Options"
      Begin VB.Menu Project 
         Caption         =   "&Level"
      End
      Begin VB.Menu ViewLevelLinks 
         Caption         =   "&View Links..."
      End
      Begin VB.Menu Preferences 
         Caption         =   "&Preferences"
      End
      Begin VB.Menu Directories 
         Caption         =   "&Directories"
         Visible         =   0   'False
      End
      Begin VB.Menu Rebuild 
         Caption         =   "&Rebuild..."
      End
   End
   Begin VB.Menu WIndow 
      Caption         =   "&Window"
      Begin VB.Menu MeshViewer 
         Caption         =   "&Mesh Viewer"
      End
      Begin VB.Menu TwoDee 
         Caption         =   "&2D Shape Editor"
      End
      Begin VB.Menu WorldBrowser 
         Caption         =   "Resource &Browser"
      End
      Begin VB.Menu WindowLog 
         Caption         =   "&Log"
      End
      Begin VB.Menu ZFUS 
         Caption         =   "-"
      End
      Begin VB.Menu WinToolbar 
         Caption         =   "&Toolbar"
         Begin VB.Menu WinToolbarLeft 
            Caption         =   "&Left"
         End
         Begin VB.Menu WinToolbarRight 
            Caption         =   "&Right"
         End
      End
      Begin VB.Menu WinPanel 
         Caption         =   "&Panel"
         Visible         =   0   'False
         Begin VB.Menu WinPanelBottom 
            Caption         =   "&Bottom"
         End
         Begin VB.Menu WinPanelTop 
            Caption         =   "&Top"
         End
         Begin VB.Menu WinPanelHide 
            Caption         =   "&Hide"
         End
      End
      Begin VB.Menu WinBrowser 
         Caption         =   "&Browser"
         Begin VB.Menu WinBrowserRight 
            Caption         =   "&Right"
         End
         Begin VB.Menu WinBrowserLeft 
            Caption         =   "&Left"
         End
         Begin VB.Menu WinBrowserHide 
            Caption         =   "&Hide"
         End
      End
   End
   Begin VB.Menu MacroS 
      Caption         =   "&Macros"
      Begin VB.Menu MacroNew 
         Caption         =   "&New"
      End
      Begin VB.Menu MacroLoad 
         Caption         =   "&Load..."
      End
      Begin VB.Menu MacroPlay 
         Caption         =   "&Play"
         Enabled         =   0   'False
         Shortcut        =   ^P
      End
      Begin VB.Menu ZOELAZ 
         Caption         =   "-"
      End
      Begin VB.Menu MacroRecord 
         Caption         =   "&Record"
      End
      Begin VB.Menu MacroEnd 
         Caption         =   "&End recording"
         Enabled         =   0   'False
      End
      Begin VB.Menu ZOOPOO 
         Caption         =   "-"
      End
      Begin VB.Menu MacroEdit 
         Caption         =   "&Edit"
         Enabled         =   0   'False
      End
      Begin VB.Menu MacroSave 
         Caption         =   "&Save"
         Enabled         =   0   'False
      End
      Begin VB.Menu MacroSaveAs 
         Caption         =   "Save &As..."
         Enabled         =   0   'False
      End
   End
   Begin VB.Menu Help 
      Caption         =   "&Help"
      Begin VB.Menu About 
         Caption         =   "&About UnrealEd"
      End
      Begin VB.Menu EpicWeb 
         Caption         =   "&Epic's Web Site"
      End
      Begin VB.Menu QIDJWE 
         Caption         =   "-"
      End
      Begin VB.Menu HelpIndex 
         Caption         =   "&Help Topics"
         Shortcut        =   {F1}
      End
      Begin VB.Menu HelpCam 
         Caption         =   "Help on &Cameras"
         Shortcut        =   {F2}
      End
      Begin VB.Menu RelNotes 
         Caption         =   "&Release Notes"
      End
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Dim GInitialResized As Integer
Dim VBarHold As Integer, LBarHold As Integer, RBarHold
Dim LBoxHold As Integer, RBoxHold As Integer
Dim VDelta As Integer, HDelta As Integer
Public CX, CY, CXL, CYL As Integer ' Client window dimensions

Const BrowserWidth1 = 140
Const BrowserWidth2 = 152

Private Sub About_Click()
    frmDialogs.About.ShowHelp ' WinHelp
End Sub

Private Sub ActorCombo_Click()
    If Ed.Startup Then
        ' Disregard
    ElseIf ActorCombo.ListIndex <> 0 Then
        ActorCombo.ListIndex = 0
        Ed.SetBrowserTopic ("Classes")
    End If
End Sub

Private Sub AddCutFirst_Click()
    Ed.BeginSlowTask "Adding brush to world"
    Ed.Server.SlowExec "BRUSH ADD CUTFIRST"
    Ed.EndSlowTask
End Sub

Private Sub AddCutaway_Click()
    Ed.BeginSlowTask "Adding Cutaway Zone"
    Ed.Server.SlowExec "BRUSH ADD CUTAWAY"
    Ed.EndSlowTask
End Sub

Private Sub AddMovableBrush_Click()
    Ed.BeginSlowTask "Adding movable brush to world"
    Ed.Server.SlowExec "BRUSH ADDMOVABLE"
    Ed.EndSlowTask
End Sub

Private Sub AddNoCut_Click()
    Ed.BeginSlowTask "Adding No-Cut Zone"
    Ed.Server.SlowExec "BRUSH ADD NOCUT"
    Ed.EndSlowTask
    '
    Call MsgBox("A no-cut zone will be added.  This will take effect the next time you rebuild geometry.", 64, "Adding No-Cut Zone")
End Sub

Private Sub AddSpecial_Click()
    frmAddSpecial.Show
End Sub

Private Sub Browser_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 2 Then
        PopupMenu frmPopups.Browser
    End If
End Sub

Private Sub BrowserHolder_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 2 Then
        PopupMenu frmPopups.Browser
    End If
End Sub

Private Sub BrowserTopicCombo_Click()
    Ed.SetBrowserTopic (BrowserTopicCombo.Text)
End Sub

Private Sub BrushAdd_Click()
    Ed.BeginSlowTask "Adding brush to world"
    Ed.Server.SlowExec "BRUSH ADD"
    Ed.EndSlowTask
End Sub

Private Sub BrushDeintersect_Click()
    Ed.BeginSlowTask "Deintersecting brush"
    Ed.Server.SlowExec "BRUSH FROM DEINTERSECTION"
    Ed.EndSlowTask
End Sub

Private Sub BrushExport_Click()
    '
    Dim ExportFname As String
    '
    ' Prompt for filename
    '
    On Error GoTo Skip
    Ed.Server.Disable
    frmDialogs.ExportBrush.ShowSave 'Modal Save-As Box
    ExportFname = frmDialogs.ExportBrush.filename
    '
    Call UpdateDialog(frmDialogs.ExportBrush)
    If (ExportFname <> "") Then
        Ed.BeginSlowTask "Exporting brush"
        Ed.Server.SlowExec "BRUSH EXPORT FILE=" & Quotes(ExportFname)
        Ed.EndSlowTask
    End If
Skip: Ed.Server.Enable
End Sub

Private Sub BrushImport_Click()
    '
    ' Dialog for "Brush Import":
    '
    On Error GoTo Skip
    Ed.Server.Disable
    frmDialogs.ImportBrush.filename = ""
    frmDialogs.ImportBrush.DefaultExt = "t3d"
    frmDialogs.ImportBrush.ShowOpen 'Modal File-Open Box
    '
    Call UpdateDialog(frmDialogs.ImportBrush)
    If (frmDialogs.ImportBrush.filename <> "") Then
        frmBrushImp.Show 1
    End If
Skip: Ed.Server.Enable
End Sub

Private Sub BrushIntersect_Click()
    Ed.BeginSlowTask "Intersecting brush"
    Ed.Server.SlowExec "BRUSH FROM INTERSECTION"
    Ed.EndSlowTask
End Sub

Private Sub BrushReset_Click()
    Ed.Server.Exec "BRUSH RESET"
End Sub

Private Sub BrushSliceTex_Click()
    Ed.BeginSlowTask "Adding No-Terrain Zone"
    Ed.Server.SlowExec "BRUSH ADD NOTERRAIN"
    Ed.EndSlowTask
    '
    Call MsgBox("A No-Terrain zone will be added.  This will take effect the next time you rebuild geometry.  See the terrain help for complete information on using No-Terrain zones in UnrealEd.", 64, "Adding No-Terrain Zone")
End Sub

Private Sub BrushSubtract_Click()
    Ed.BeginSlowTask "Subtracting brush from world"
    Ed.Server.SlowExec "BRUSH SUBTRACT"
    Ed.EndSlowTask
End Sub

Private Sub CameraAhead_Click()
    Ed.Server.Exec "CAMERA LOOK AHEAD"
End Sub

Private Sub CameraDown_Click()
    Ed.Server.Exec "CAMERA LOOK DOWN"
End Sub

Private Sub CameraEast_Click()
    Ed.Server.Exec "CAMERA LOOK EAST"
End Sub

Private Sub CameraEntire_Click()
    Ed.Server.Exec "CAMERA LOOK ENTIREMAP"
End Sub

Private Sub CameraNorth_Click()
    Ed.Server.Exec "CAMERA LOOK NORTH"
End Sub

Private Sub CameraSouth_Click()
    Ed.Server.Exec "CAMERA LOOK SOUTH"
End Sub

Private Sub CameraUp_Click()
    Ed.Server.Exec "CAMERA LOOK UP"
End Sub

Private Sub CameraWest_Click()
    Ed.Server.Exec "CAMERA LOOK WEST"
End Sub

Private Sub CalcButton_Click()
    Dim Result As Double
    Dim OrigStr As String
    '
    OrigStr = CalcText.Text
    If Eval(CalcText.Text, Result) Then
        CalcText.ForeColor = &H80000008
        If Trim(OrigStr) <> "0" Then
            If Trim(OrigStr) <> CalcText.List(0) Then
                CalcText.AddItem OrigStr, 0
            End If
        End If
        '
        If Result <> 0 Then
            If Trim(Str(Result)) <> CalcText.List(0) Then
                CalcText.AddItem Trim(Str(Result)), 0
            End If
        End If
        CalcText.Text = Trim(Str(Result))
        CalcText.SetFocus
        SendKeys "{HOME}+{END}" ' Select all
    Else
        CalcText.ForeColor = &HC0&
        CalcText.SetFocus
        SendKeys "{End}"
    End If
End Sub

Private Sub CalcText_Click()
    If CalcText.Text = "Reset" Then
        CalcText.Clear
        CalcText.AddItem "Reset"
        CalcText.Text = "0"
        CalcText.SetFocus
        SendKeys "{HOME}+{END}" ' Select all
    End If
    CalcText.ForeColor = &H80000008
End Sub

Private Sub CalcZero_Click()
    CalcText.Text = "0"
    CalcText.ForeColor = &H80000008
    CalcText.SetFocus
    SendKeys "{HOME}+{END}" ' Select all
End Sub

Private Sub ClassBrows_Click()
    frmClassBrowser.Show
End Sub


Private Sub CamAllViews_Click()
    Ed.CameraVertRatio = 0.66
    Ed.CameraLeftRatio = 0.5
    Ed.CameraRightRatio = 0.5
    ResizeAll (True)
End Sub

Private Sub CamCloseAllFree_Click()
    Ed.Server.Exec "CAMERA CLOSE FREE"
End Sub


Private Sub CameraResetAll_Click()
    Ed.Server.Exec "CAMERA CLOSE ALL"
    Ed.CameraVertRatio = 0.66
    Ed.CameraLeftRatio = 0.5
    Ed.CameraRightRatio = 0.5
    ResizeAll (False)
End Sub

Private Sub CamOpenFree_Click()
    Ed.OpenFreeCamera
End Sub

Private Sub CamOvh_Click()
    Ed.CameraVertRatio = 1#
    Ed.CameraLeftRatio = 1#
    ResizeAll (True)
End Sub

Private Sub CamPersp_Click()
    Ed.CameraVertRatio = 1#
    Ed.CameraLeftRatio = 0#
    ResizeAll (True)
End Sub

Private Sub CamTwoViews_Click()
    Ed.CameraVertRatio = 1#
    Ed.CameraLeftRatio = 0.4
    ResizeAll (True)
End Sub

Private Sub Command1_Click()
    ToolHelp (123)
End Sub

Private Sub CurvedStair_Click()
    frmParSolCurvedStair.Show
End Sub

Private Sub Delete_Click()
    Ed.Server.Exec "DELETE"
End Sub

Private Sub Directories_Click()
   'Ed.Server.Disable
   'frmDirectories.Show 1
   'Ed.Server.Enable
End Sub

Private Sub Duplicate_Click()
    Ed.Server.Exec "DUPLICATE"
End Sub

Private Sub EpicButton_Click()
    Ed.Server.Exec "LAUNCH WEB"
End Sub

Private Sub EpicWeb_Click()
    Ed.Server.Exec "LAUNCH WEB"
End Sub

Private Sub Exit_Click()
   Unload Me
End Sub

Private Sub ExportLevel_Click()
    '
    Dim ExportFname As String
    '
    ' Prompt for filename
    '
    On Error GoTo Skip
    Ed.Server.Disable
    frmDialogs.ExportMap.Flags = 2 'Prompt if overwrite
    frmDialogs.ExportMap.ShowSave
    ExportFname = frmDialogs.ExportMap.filename
    '
    Call UpdateDialog(frmDialogs.ExportMap)
    If (ExportFname <> "") Then
        PreSave
        Ed.BeginSlowTask "Exporting map"
        Ed.Server.SlowExec "MAP EXPORT FILE=" & Quotes(ExportFname)
        Ed.EndSlowTask
    End If
    '
Skip: Ed.Server.Enable
End Sub

Private Sub GridCombo_Click()
    Dim S As Integer
    '
    If Ed.Startup Then
        ' Disregard
    ElseIf GridCombo.Text = "Off" Then
        Call Ed.SetGridMode(0)
    ElseIf GridCombo.Text = "Custom" Then
        frmGrid.Show
        Call Ed.SetGridMode(1)
    Else
        S = Val(GridCombo.Text)
        Call Ed.SetGridSize(S, S, S)
        Call Ed.SetGridMode(1)
    End If
End Sub

Private Sub HelpButton_Click()
    frmMain.PopupMenu frmPopups.Help
End Sub

Private Sub HelpCam_Click()
    '
    ' Bring up camera help specific to the
    ' current editor mode.
    '
    Call Ed.Tools.Handlers(Ed.ToolMode).DoHelp(Ed.ToolMode, Ed)
    '
End Sub

Private Sub HelpIndex_Click()
    frmDialogs.HelpContents.HelpFile = App.Path + "\\help\\unrealed.hlp"
    frmDialogs.HelpContents.ShowHelp ' Run WinHelp
End Sub

Private Sub Holder_KeyDown(KeyCode As Integer, Shift As Integer)
    If KeyCode = &H70& Then ' Intercept F1
       KeyCode = 0
       Call Ed.Tools.Handlers(Ed.MRUTool).DoHelp(Ed.MRUTool, Ed)
    End If
End Sub

Private Sub ImportLevel_Click()
    '
    ' Dialog for "Map Import"
    '
    On Error GoTo Skip
    Ed.Server.Disable
    frmDialogs.ImportMap.filename = ""
    frmDialogs.ImportMap.ShowOpen 'Modal File-Open Box
    '
    Call UpdateDialog(frmDialogs.ImportMap)
    If (frmDialogs.ImportMap.filename <> "") Then
        '
        frmImportMap.Show 1
        '
        If GResult Then
            Ed.BeginSlowTask "Importing map"
            If GImportExisting Then ' Import new map
                Ed.Server.SlowExec "MAP IMPORTADD FILE=" & Quotes(frmDialogs.ImportMap.filename)
            Else ' Add to existing map
                Ed.Server.SlowExec "MAP IMPORT FILE=" & Quotes(frmDialogs.ImportMap.filename)
            End If
            Ed.EndSlowTask
            If Ed.MapEdit Then Call Ed.Tools.Click("MAPEDIT")
            PostLoad
        End If
    End If
Skip: Ed.Server.Enable
End Sub

Private Sub LoadBrush_Click()
    '
    ' Dialog for "Load Brush":
    '
    On Error GoTo Skip
    Ed.Server.Disable
    frmDialogs.BrushOpen.filename = ""
    frmDialogs.BrushOpen.ShowOpen 'Modal Brush-Open Box
    '
    Call UpdateDialog(frmDialogs.BrushOpen)
    If (frmDialogs.BrushOpen.filename <> "") Then
        '
        ' Load the brush
        '
        Call UpdateDialog(frmDialogs.BrushOpen)
        Ed.BrushFname = frmDialogs.BrushOpen.filename
        Ed.Server.Exec "BRUSH LOAD FILE=" & Quotes(Ed.BrushFname)
    End If
Skip: Ed.Server.Enable
End Sub

Private Sub MacroEdit_Click()
    Dim EditForm As New frmEdit
    '
    EditForm.Caption = "Unreal Macro Editor"
    EditForm.Topic = "Text"
    EditForm.Item = "Macro"
    EditForm.HelpContextID = 500
    EditForm.Show 1
    '
End Sub

Private Sub MacroEnd_Click()
    '
    Ed.Server.Exec "MACRO ENDRECORD"
    '
    MacroRecord.Enabled = 1
    MacroLoad.Enabled = 1
    MacroPlay.Enabled = 1
    MacroSave.Enabled = 1
    MacroSaveAs.Enabled = 1
    MacroEdit.Enabled = 1
    MacroEnd.Enabled = 0
    '
End Sub

Private Sub MacroLoad_Click()
    '
    ' Dialog for "Load Macro":
    '
    On Error GoTo Skip
    Ed.Server.Disable
    frmDialogs.MacroLoad.filename = Ed.MacroFname
    frmDialogs.MacroLoad.ShowOpen 'Modal File-Open Box
    '
    Call UpdateDialog(frmDialogs.MacroLoad)
    If (frmDialogs.MacroLoad.filename <> "") Then
        Ed.MacroFname = frmDialogs.MacroLoad.filename
        Ed.Server.Exec "MACRO LOAD FILE=" & Quotes(Ed.MacroFname) & " NAME=MACRO"
        '
        MacroPlay.Enabled = 1
        MacroEdit.Enabled = 1
        MacroSave.Enabled = 1
        MacroSaveAs.Enabled = 1
    End If
Skip: Ed.Server.Enable
End Sub

Private Sub MacroNew_Click()
    Dim EditForm As New frmEdit
    '
    EditForm.Caption = "Unreal Macro Editor"
    EditForm.HelpContextID = 500
    EditForm.Topic = "Text"
    EditForm.Item = "*Macro" ' The "*" means Create new
    EditForm.Show 1
    '
    MacroPlay.Enabled = 1
    MacroEdit.Enabled = 1
    MacroSave.Enabled = 1
    MacroSaveAs.Enabled = 1
    '
    Ed.MacroFname = ""
End Sub

Private Sub MacroPlay_Click()
    Ed.BeginSlowTask "Playing macro"
    Ed.Server.SlowExec "MACRO PLAY NAME=MACRO"
    Ed.EndSlowTask
    '
    ' Should resend all setable settings so the toolbar
    ' isn't indicating the wrong mode and parameters.
    '
End Sub

Private Sub MacroRecord_Click()
    '
    Ed.Server.Exec "MACRO RECORD NAME=MACRO"
    '
    ' Enable/disable appropriate menu items
    '
    Ed.MacroRecording = 1
    '
    MacroRecord.Enabled = 0
    MacroLoad.Enabled = 0
    MacroPlay.Enabled = 0
    MacroSave.Enabled = 0
    MacroSaveAs.Enabled = 0
    MacroEdit.Enabled = 0
    MacroEnd.Enabled = 1
    '
End Sub

Private Sub MacroSave_Click()
    If Ed.MacroFname = "" Then
        '
        ' Prompt for filename
        '
        On Error GoTo Skip
        Ed.Server.Disable
        frmDialogs.MacroSaveAs.Flags = 2 'Prompt if overwrite
        frmDialogs.MacroSaveAs.ShowSave
        '
        Call UpdateDialog(frmDialogs.MacroSaveAs)
        If frmDialogs.MacroSaveAs.filename = "" Then Exit Sub
        Ed.MacroFname = frmDialogs.MacroSaveAs.filename
    End If
    '
    ' Save the macro
    '
    Ed.Server.Exec "MACRO SAVE FILE=" & Quotes(Ed.MacroFname)
Skip: Ed.Server.Enable
End Sub

Private Sub MacroSaveAs_Click()
    frmDialogs.MacroSaveAs.filename = Ed.MacroFname
    Ed.MacroFname = ""
    MacroSave_Click
End Sub

Private Sub Map320x200_Click()
    Ed.Server.Exec "CAMERA SIZE XR=320 YR=200"
End Sub

Private Sub Map400x300_Click()
    Ed.Server.Exec "CAMERA SIZE XR=400 YR=300"
End Sub

Private Sub Map480x360_Click()
    Ed.Server.Exec "CAMERA SIZE XR=480 YR=360"
End Sub

Private Sub Map560x420_Click()
    Ed.Server.Exec "CAMERA SIZE XR=560 YR=420"
End Sub

Private Sub Map640x480_Click()
    Ed.Server.Exec "CAMERA SIZE XR=640 YR=480"
End Sub

Private Sub MapFlat_Click()
    Ed.Server.Exec "CAMERA SET MODE=FLAT"
End Sub

Private Sub MapFlatNorms_Click()
    Ed.Server.Exec "CAMERA SET MODE=FLATNORMS"
End Sub

Private Sub MapIllum_Click()
    Ed.Server.Exec "CAMERA SET MODE=ILLUM"
End Sub

Private Sub MapLight_Click()
    Ed.Server.Exec "CAMERA SET MODE=SHADE"
End Sub

Private Sub MapPersp_Click()
    Ed.Server.Exec "CAMERA SET MODE=MAP3D"
End Sub

Private Sub MapTextures_Click()
    Ed.Server.Exec "CAMERA SET MODE=TEXTURES"
End Sub

Private Sub MapXY_Click()
    Ed.Server.Exec "CAMERA SET MODE=MAPXY"
End Sub

Private Sub MapXZ_Click()
    Ed.Server.Exec "CAMERA SET MODE=MAPXZ"
End Sub

Private Sub MapYZ_Click()
    Ed.Server.Exec "CAMERA SET MODE=MAPYZ"
End Sub

Private Sub Form_Load()
    '
    Dim i, X As Integer
    Dim S As String, T As String
    Dim Temp As String
    Dim Highlight As Boolean
    '
    Call InitApp ' Init App object properties
    '
    ' Create global UnrealEdApp object.
    '
    Set Ed = New UnrealEdApp
    '
    ' Show startup screen
    '
    Ed.Startup = 1
    App.Title = Ed.EditorAppName
    frmMain.Caption = Ed.EditorAppName
    frmMain.Show
    '
    ' Stick values in combo boxes
    '
    If Screen.Width <= 640 * Screen.TwipsPerPixelX Then
        EpicButton.Visible = False
        ModeLabel.Visible = False
        ModeCombo.Visible = False
    End If
    '
    If Screen.Width < 1024 * Screen.TwipsPerPixelX Then
        CalcButton.Visible = False
        CalcText.Visible = False
        CalcZero.Visible = False
    End If
    '
    ModeCombo.AddItem "Move Camera/Brush"
    ModeCombo.AddItem "Zoom Camera/Brush"
    ModeCombo.AddItem "Brush Rotate"
    ModeCombo.AddItem "Brush Scale"
    ModeCombo.AddItem "Brush Sheer"
    ModeCombo.AddItem "Brush Stretch"
    ModeCombo.AddItem "Brush SnapScale"
    ModeCombo.AddItem "Add Actor"
    ModeCombo.AddItem "Add Light"
    ModeCombo.AddItem "Move Actor/Light"
    ModeCombo.AddItem "Set/Pan/Scale Textures"
    ModeCombo.AddItem "Rotate Textures"
    ModeCombo.AddItem "Terraform"
    ModeCombo.ListIndex = 0
    '
    GridCombo.AddItem "Off"
    GridCombo.AddItem "1"
    GridCombo.AddItem "2"
    GridCombo.AddItem "4"
    GridCombo.AddItem "8"
    GridCombo.AddItem "16"
    GridCombo.AddItem "32"
    GridCombo.AddItem "64"
    GridCombo.AddItem "128"
    GridCombo.AddItem "256"
    GridCombo.AddItem "Custom"
    GridCombo.ListIndex = 5
    '
    ActorCombo.AddItem "Light"
    ActorCombo.AddItem "Show Browser..."
    ActorCombo.ListIndex = 0
    '
    TextureCombo.AddItem "Default"
    TextureCombo.AddItem "Show Browser..."
    TextureCombo.ListIndex = 0
    '
    ' Position combo boxes
    '
    X = MainBar.Width - (TextureCombo.Left + _
        TextureCombo.Width + _
        8 * Screen.TwipsPerPixelX)
    CalcText.Left = CalcText.Left + X
    CalcButton.Left = CalcButton.Left + X
    CalcZero.Left = CalcZero.Left + X
    ModeLabel.Left = ModeLabel.Left + X
    ModeCombo.Left = ModeCombo.Left + X
    TextureLabel.Left = TextureLabel.Left + X
    TextureCombo.Left = TextureCombo.Left + X
    ActorLabel.Left = ActorLabel.Left + X
    ActorCombo.Left = ActorCombo.Left + X
    GridLabel.Left = GridLabel.Left + X
    GridCombo.Left = GridCombo.Left + X
    '
    ' Launch UnrealServer.  There is only
    ' one of these per instance.
    '
    Ed.GetProfile
    Ed.LaunchServer (frmMain.hwnd)
    If Ed.Licensed = 0 Then frmLicense.Show 1 ' Show license info
    Call Ed.Server.Init(frmMain.hwnd, frmMain.Callback.hwnd) ' Initialize the server
    '
    ' Set help file dirs:
    '
    frmDialogs.ToolHelp.HelpFile = App.HelpFile
    frmDialogs.RelNotes.HelpFile = App.HelpFile
    frmDialogs.HelpContents.HelpFile = App.HelpFile
    frmDialogs.About.HelpFile = App.HelpFile
    '
    ' Initialize tools
    '
    Call Ed.Tools.InitTools(Ed)
    '
    Ed.Startup = 0
    GInitialResized = 1
    '
    Ed.Server.Exec "APP HIDE"
    '
    ' Set server parameters to the defaults the
    ' editor expects:
    '
    Ed.Server.Exec "MAP GRID X=16 Y=16 Z=16 BASE=ABSOLUTE SHOW2D=ON SHOW3D=OFF"
    Ed.Server.Exec "MAP ROTGRID PITCH=4 YAW=4 ROLL=4"
    Ed.Server.Exec "MODE CAMERAMOVE GRID=ON ROTGRID=ON SHOWVERTICES=ON SNAPTOPIVOT=ON SNAPDIST=10"
    '
    ResizeAll (False)
    '
    ' Init and show toolbar (must be drawn after camera in order
    ' for palette to come out right):
    '
    InitToolbar
    '
    ' Load initial resources, if any
    '
    If Ed.InitialFiles <> "" Then
        Open "UnrealEd.tmp" For Append As #1
        If LOF(1) <> 0 Then
            Close #1
            Call MsgBox("It appears that UnrealEd did not start up successfully the last time it was run.  This may be due to invalid startup files that were specified.  You may want to remove them from the Preferences dialog.", _
                0, "Possible UnrealEd startup problem")
            Kill "UnrealEd.tmp"
            frmPreferences.Show 1
        Else
            Print #1, "UnrealEd"
            Close #1
            S = Ed.InitialFiles
            Do
                If InStr(S, " ") Then
                    T = Trim(Left(S, InStr(S, " ") - 1))
                    S = Trim(Mid(S, InStr(S, " ") + 1))
                Else
                    T = S
                    S = ""
                End If
                If T <> "" Then
                    Ed.BeginSlowTask "Loading file"
                    Ed.Server.SlowExec "RES LOAD FILE=" & Quotes(T)
                    Ed.EndSlowTask
                End If
            Loop Until T = ""
            Kill "UnrealEd.tmp"
        End If
    End If
    '
    ' Play startup macro, if any
    '
    If Ed.InitialMacro <> "" Then
        Ed.BeginSlowTask "Playing initial macro"
        Ed.Server.Exec "MACRO LOAD FILE=" & Quotes(Ed.InitialMacro) & " NAME=MACRO"
        Ed.Server.SlowExec "MACRO PLAY NAME=MACRO"
        Ed.EndSlowTask
    End If
    '
    ' Load command-line level, if any
    '
    If GetString(Command$, "FILE=", Temp) Then
        If InStr(Temp, ":") = 0 And (Left(Temp, 1) <> "\") Then
            Temp = App.Path + "\" + Temp
        End If
        Ed.BeginSlowTask "Loading " & Temp
        Ed.Server.Exec "RES LOAD FILE=" & Quotes(Temp)
        Ed.EndSlowTask
        '
        Ed.MapFname = Temp
        Caption = Ed.EditorAppName + " - " + Ed.MapFname
        Ed.LoadParamsFromLevel
        ResizeAll (True)
    End If
    '
    Ed.StatusText "UnrealEd is ready to go"
    '
    Call Ed.RegisterBrowserTopic(frmTexBrowser, "Textures")
    Call Ed.RegisterBrowserTopic(frmClassBrowser, "Classes")
    'Call Ed.RegisterBrowserTopic(frmBrushBrowser, "Brushes")
    Call Ed.RegisterBrowserTopic(frmSoundFXBrowser, "SoundFX")
    'Call Ed.RegisterBrowserTopic(frmAmbientBrowser, "Ambient")
    Call Ed.SetBrowserTopic(Ed.InitialBrowserTopic)
    '
    If CalcText.Visible Then
        CalcText.AddItem "Reset"
        CalcText.Text = "0"
        CalcButton.SetFocus
        SendKeys "{HOME}+{END}" ' Select all
    End If
    '
    PreferencesChange
    '
End Sub

Private Sub MDIForm_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If (Button And 2) <> 0 Then ' Right click
        PopupMenu frmPopups.Window
    End If
End Sub

Private Sub Form_Resize()
    ResizeAll (True)
End Sub

Private Sub Form_Unload(Cancel As Integer)
   Dim N As Integer
   '
   ' Unload all forms
   '
   Unload frmActorProperties ' Actor properties lister
   Unload frmClassBrowser   ' Actor Class browser
   Unload frmSoundFXBrowser ' Sound browser
   Unload frmScriptEd       ' Actor script editor
   Unload frmBrush          ' Brush tools
   Unload frmSurfaceProps   ' Surface properties
   Unload frmResBrowse      ' World resource browser
   Unload frmTexProp        ' Texture properties
   Unload frmLevel          ' Level window
   Unload frmPopups         ' Rt. click menus
   Unload frmPopups2        ' More rt. clicks
   Unload frmGrid           ' Grid settings
   Unload frmRotGrid        ' Rotational grid settings
   Unload frmMapToolbar     ' Map-mode controls
   Unload frmDialogs        ' Common dialogs
   Unload frmRebuilder      ' Rebuilder
   Unload frmPreferences    ' Preferences
   Unload frmTexImport      ' Texture import dialog
   Unload frmAddSpecial     ' Add special brush
   Unload frmBrushImp       ' Import brush
   Unload frmMeshViewer     ' Mesh viewer
   Unload frmNewClass       ' New Actor Class
   Unload frmResults        ' Text results
   Unload frmEditVector     ' Actor properties vector editing
   Unload frmEditRotation   ' Actor properties rotation editing
   Unload frmScriptFind     ' Script text finder
   Unload frmSoundImportDlg ' Sound import thing
   Unload frmTwoDee         ' 2D tools
   Unload frmExtrude        ' 2D Extrude
   Unload frmRevolve        ' 2D Revolve
   Unload frmBevel          ' 2D Bevel
   Unload frmExPoint        ' 2D Extrude to point
   '
   Unload frmParSolRect
   Unload frmParSolCone
   Unload frmParSolHeightMap
   Unload frmParSolLinearStair
   Unload frmParSolSphere
   Unload frmParSolSpiralStair
   Unload frmParSolCurvedStair
   Unload frmParSolTube
   Unload frmPsSheet
   '
   UnloadMiscForms
   Ed.UnloadBrowser
   '
   ' Save profile now that all forms have
   ' called their EndOnTop's.
   '
   Ed.SaveProfile
   '
End Sub

Private Sub MapEditMode_Click()
    Ed.Tools.Click "MAPEDIT"
End Sub

Private Sub MeshViewer_Click()
    frmMeshViewer.Show
End Sub

Private Sub ModeCombo_Click()
    If Ed.Startup <> 0 Or GSettingMode <> 0 Then
        ' Disregard
    Else
        GSettingMode = 1
        Select Case ModeCombo.List(ModeCombo.ListIndex)
        Case "Move Camera/Brush": Ed.Tools.Click "CAMERAMOVE"
        Case "Zoom Camera/Brush": Ed.Tools.Click "CAMERAZOOM"
        Case "Brush Rotate": Ed.Tools.Click "BRUSHROTATE"
        Case "Brush Scale": Ed.Tools.Click "BRUSHSCALE"
        Case "Brush Sheer": Ed.Tools.Click "BRUSHSHEER"
        Case "Brush Stretch": Ed.Tools.Click "BRUSHSTRETCH"
        Case "Brush SnapScale": Ed.Tools.Click "BRUSHSNAP"
        Case "Add Actor": Ed.Tools.Click "ADDACTOR"
        Case "Add Light": Ed.Tools.Click "ADDLIGHT"
        Case "Move Actor/Light": Ed.Tools.Click "MOVEACTOR"
        Case "Set/Pan/Scale Textures": Ed.Tools.Click "TEXTUREPAN"
        Case "Rotate Textures": Ed.Tools.Click "TEXTUREROTATE"
        Case "Terraform": Ed.Tools.Click "TERRAFORM"
        End Select
        GSettingMode = 0
    End If
    Call Ed.StatusText("Set mode to " & ModeCombo.List(ModeCombo.ListIndex))
End Sub

Private Sub New_Click()
    Ed.Server.Disable
    frmNewMap.Show 1 ' Model new-level dialog
    Ed.Server.Enable
End Sub

Private Sub ObjectProperties_Click()
   frmActorProperties.Show
End Sub

Private Sub Open_Click()
    '
    ' Dialog for "Open Map":
    '
    On Error GoTo Skip
    Ed.Server.Disable
    frmDialogs.MapOpen.filename = ""
    frmDialogs.MapOpen.ShowOpen
    '
    Call UpdateDialog(frmDialogs.MapOpen)
    If (frmDialogs.MapOpen.filename <> "") Then
        '
        Ed.CloseLevelWindows
        '
        ' Load the map, inhibiting redraw since we're
        ' about to resize everything anyway.
        '
        Ed.MapFname = frmDialogs.MapOpen.filename
        Caption = Ed.EditorAppName + " - " + Ed.MapFname
        '
        Ed.BeginSlowTask "Loading map"
        Ed.Server.SlowExec "MAP LOAD FILE=" & _
            Quotes(Ed.MapFname) & " REDRAW=OFF"
        Ed.EndSlowTask
        '
        If Ed.MapEdit Then Call Ed.Tools.Click("MAPEDIT")
        Ed.LoadParamsFromLevel
        ResizeAll (True)
        PostLoad
        '
    End If
    '
Skip: Ed.Server.Enable
End Sub

Private Sub PanelHolder_Mousedown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 2 Then
        PopupMenu frmPopups.Panel
    End If
End Sub

Private Sub ParSolCone_Click()
    frmParSolCone.Show
End Sub

Private Sub ParSolHeightMap_Click()
    frmParSolHeightMap.Show
End Sub

Private Sub ParSolLinearStair_Click()
    frmParSolLinearStair.Show
End Sub

Private Sub ParSolRect_Click()
    frmParSolRect.Show
End Sub

Private Sub ParSolSphereDome_Click()
    frmParSolSphere.Show
End Sub

Private Sub ParSolSpiralStair_Click()
    frmParSolSpiralStair.Show
End Sub

Private Sub ParSolTube_Click()
    frmParSolTube.Show
End Sub

Private Sub PolygonProperties_Click()
   frmSurfaceProps.Show
End Sub

Private Sub Preferences_Click()
   Ed.Server.Disable
   frmPreferences.Show 1
   Ed.Server.Enable
End Sub

Private Sub Project_Click()
    frmLevel.Show
End Sub

Private Sub Rebuild_Click()
    frmRebuilder.Show ' Rebuild dialog
End Sub

Private Sub Redo_Click()
    Ed.Server.Exec "TRANSACTION REDO"
End Sub

Private Sub RedoButton_Click()
    Ed.Tools.Click "TRANSACTION REDO"
End Sub

Private Sub RelNotes_Click()
    frmDialogs.RelNotes.ShowHelp ' Run WinHelp
End Sub

Private Sub ResetAll_Click()
    Ed.Server.Exec "BRUSH RESET"
End Sub

Private Sub ResetPosition_Click()
    Ed.Server.Exec "BRUSH MOVETO X=0 Y=0 Z=0"
End Sub

Private Sub ResetRotation_Click()
    Ed.Server.Exec "BRUSH ROTATETO PITCH=0 YAW=0 ROLL=0"
End Sub

Private Sub ResetScale_Click()
    Ed.Server.Exec "BRUSH SCALE X=1 Y=1 Z=1 SHEER=0"
End Sub

Private Sub Resize_Click()
    frmBrush.Show
End Sub

Private Sub Save_Click()
    On Error GoTo Skip
    If Ed.MapFname = "" Then
        '
        ' Prompt for filename
        '
        Ed.Server.Disable
        frmDialogs.MapSaveAs.Flags = 2 'Prompt if overwrite
        frmDialogs.MapSaveAs.ShowSave 'Modal Save-As Box
        Ed.MapFname = frmDialogs.MapSaveAs.filename
        Ed.Server.Enable
        Call UpdateDialog(frmDialogs.MapSaveAs)
    End If
    '
    If Ed.MapFname <> "" Then
        '
        ' Save the map
        '
        PreSave
        Caption = Ed.EditorAppName + " - " + Ed.MapFname
        Ed.BeginSlowTask ("Saving map")
        Ed.SaveParamsToLevel
        Ed.Server.SlowExec "MAP SAVE FILE=" & Quotes(Ed.MapFname)
        Ed.EndSlowTask
    End If
Skip: Ed.Server.Enable
End Sub

Private Sub SaveAs_Click()
    '
    ' Just set the default filename to empty and
    ' call Save_Click to do the normal "save" procedure.
    '
    Ed.MapFname = ""
    Save_Click
End Sub

Private Sub SaveBrush_Click()
    On Error GoTo Skip
    If Ed.BrushFname = "" Then
        '
        ' Prompt for filename
        '
        Ed.Server.Disable
        frmDialogs.BrushSave.Flags = 2 'Prompt if overwrite
        frmDialogs.BrushSave.ShowSave
        Ed.BrushFname = frmDialogs.BrushSave.filename
        Call UpdateDialog(frmDialogs.BrushSave)
    End If
    '
    If Ed.BrushFname <> "" Then
        '
        ' Save the brush
        '
        Ed.Server.Exec "BRUSH SAVE FILE=" & Quotes(Ed.BrushFname)
    End If
Skip: Ed.Server.Enable
End Sub

Private Sub SaveBrushAs_Click()
    '
    ' Just set the default filename to empty and
    ' call Save_Click to do the normal "save" procedure.
    '
    Ed.BrushFname = ""
    SaveBrush_Click
End Sub

Private Sub SelAdjFloors_Click()
    Ed.Server.Exec "POLY SELECT ADJACENT FLOORS"
End Sub

Private Sub SelAdjSlants_Click()
    Ed.Server.Exec "POLY SELECT ADJACENT SLANTS"
End Sub

Private Sub SelAdjWalls_Click()
    Ed.Server.Exec "POLY SELECT ADJACENT WALLS"
End Sub

Private Sub SelAllAdj_Click()
    Ed.Server.Exec "POLY SELECT ADJACENT ALL"
End Sub

Private Sub SelCoplAdj_Click()
    Ed.Server.Exec "POLY SELECT ADJACENT COPLANARS"
End Sub

Private Sub SelectAll_Click()
    Ed.Server.Exec "SELECT ALL"
End Sub

Private Sub SelectNone_Click()
    Ed.Server.Exec "SELECT NONE"
End Sub

Private Sub SelIntersection_Click()
    Ed.Server.Exec "POLY SELECT MEMORY INTERSECTION"
End Sub

Private Sub SelMatchBrush_Click()
    Ed.Server.Exec "POLY SELECT MATCHING BRUSH"
End Sub

Private Sub SelMatchGroups_Click()
    Ed.Server.Exec "POLY SELECT MATCHING GROUPS"
End Sub

Private Sub SelMatchItems_Click()
    Ed.Server.Exec "POLY SELECT MATCHING ITEMS"
End Sub

Private Sub SelMatchTex_Click()
    Ed.Server.Exec "POLY SELECT MATCHING TEXTURE"
End Sub

Private Sub SelMemorize_Click()
    Ed.Server.Exec "POLY SELECT MEMORY SET"
End Sub

Private Sub SelRecall_Click()
    Ed.Server.Exec "POLY SELECT MEMORY RECALL"
End Sub

Private Sub SelReverse_Click()
    Ed.Server.Exec "POLY SELECT REVERSE"
End Sub

Private Sub SelUnion_Click()
    Ed.Server.Exec "POLY SELECT MEMORY UNION"
End Sub

Private Sub SelXor_Click()
    Ed.Server.Exec "POLY SELECT MEMORY XOR"
End Sub

Private Sub ShowBackdrop_Click()
    Ed.Server.Exec "CAMERA SET BACKDROP=TOGGLE"
End Sub

Private Sub ShowBrush_Click()
    Ed.Server.Exec "CAMERA SET BRUSH=TOGGLE"
End Sub

Private Sub ShowGrid_Click()
    Ed.Server.Exec "CAMERA SET GRID=TOGGLE"
End Sub

Private Sub ShowOcclusion_Click()
    Ed.Server.Exec "CAMERA SET OCCLUSION=TOGGLE"
End Sub

Private Sub SSPanel5_Click()

End Sub
Private Sub TexBrows_Click()
    Ed.BrowserPos = 0
    ResizeAll (True)
End Sub

Private Sub TexPalette_Click()
    Ed.BrowserPos = 0
    ResizeAll (False)
End Sub

Private Sub TextureCombo_Click()
    If Ed.Startup Then
        ' Disregard
    ElseIf TextureCombo.ListIndex = 1 Then
        If Ed.BrowserPos = 2 Then
            Ed.BrowserPos = 0
            ResizeAll (True)
        End If
        Call Ed.SetBrowserTopic("Textures")
        TextureCombo.ListIndex = 0
    End If
End Sub

Private Sub Timer_Timer()
    Ed.AutoSaveCountup = Ed.AutoSaveCountup + 1
    If (Ed.AutoSaveCountup > Ed.AutoSaveTime) And _
        (Ed.AutoSaveTime <> 0) Then
        '
        Ed.AutoSaveCountup = 0
        Ed.BeginSlowTask ("Saving map")
        Ed.Server.SlowExec "MAP SAVE FILE=" & Quotes(Ed.BaseDir + Ed.MapDir + "\AutoSave.unr")
        Ed.EndSlowTask
    End If
End Sub

Private Sub TWODEE_Click()
    frmTwoDee.Show
End Sub

Private Sub Undo_Click()
    Ed.Server.Exec "TRANSACTION UNDO"
End Sub

Private Sub UndoButton_Click()
    Ed.Tools.Click "TRANSACTION UNDO"
End Sub

Private Sub ViewLevelLinks_Click()
    Call frmResults.UpdateStatus("Level links:")
    Ed.Server.Exec "LEVEL LINKS"
    frmResults.UpdateResults
    frmResults.Results_DblClick
End Sub

Private Sub WinBrowserHide_Click()
    Ed.BrowserPos = 2
    ResizeAll (True)
End Sub

Private Sub WinBrowserLeft_Click()
    Ed.BrowserPos = 1
    ResizeAll (True)
End Sub

Private Sub WinBrowserRight_Click()
    Ed.BrowserPos = 0
    ResizeAll (True)
End Sub

Private Sub WindowLog_Click()
    Ed.Server.Exec "APP SHOW"
End Sub

Private Sub WinToolbarLeft_Click()
    Ed.ToolbarPos = 0 ' left
    ResizeAll (True)
End Sub

Private Sub WinToolbarRight_Click()
    Ed.ToolbarPos = 1 ' right
    ResizeAll (True)
End Sub

Private Sub WorldBrowser_Click()
    frmResBrowse.Show
End Sub

'---------------------------------'
' All code related to the toolbar '
'---------------------------------'

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
    If KeyCode = &H70& Then ' Intercept F1
       KeyCode = 0
       Call Ed.Tools.Handlers(Ed.MRUTool).DoHelp(Ed.MRUTool, Ed)
    End If
End Sub

Private Sub ResizeToolbar()
    Dim W As Integer, H As Integer, MaxH As Integer
    '
    ToolbarCount = ToolGridX * ToolGridY
    '
    W = (ToolGridX * 35 + 6) * Screen.TwipsPerPixelX
    H = (ToolGridY * 35 + 32) * Screen.TwipsPerPixelY
    MaxH = Toolbar.Height - 32 * Screen.TwipsPerPixelY
    '
    Holder.Top = 1 * Screen.TwipsPerPixelY
    Holder.Height = H + 5 * Screen.TwipsPerPixelY
    Holder.Width = W - 3 * Screen.TwipsPerPixelX
    '
    If (H > MaxH) Then ' Must use scrollbar
        If Ed.ToolbarPos = 0 Then ' Left
            Holder.Left = 2 * Screen.TwipsPerPixelX
            Scroller.Left = W - 2 * Screen.TwipsPerPixelX
        Else ' Right
            Holder.Left = Scroller.Width + 2 * Screen.TwipsPerPixelX
            Scroller.Left = 1 * Screen.TwipsPerPixelX
        End If
        Toolbar.Width = W + 13 * Screen.TwipsPerPixelX
        Scroller.Height = Toolbar.Height
        Scroller.Min = 0
        Scroller.Max = H - MaxH ' - 14 * Screen.TwipsPerPixelY
        Scroller.Value = Scroller.Min
        Scroller.LargeChange = MaxH
        Scroller.SmallChange = MaxH / 8
        Scroller.Visible = True
    Else ' No scrollbar, everything fits nicely
        Holder.Left = 2 * Screen.TwipsPerPixelX
        Toolbar.Width = W
        Scroller.Visible = False
    End If
    '
    StatusText.Top = Holder.Height - 33 * Screen.TwipsPerPixelY
    StatusText.Left = 2 * Screen.TwipsPerPixelX
    StatusText.Width = Holder.Width - 10 * Screen.TwipsPerPixelX
End Sub

Private Sub InitToolbar()
    Dim i, j, N, V As Integer
    Dim Highlight As Boolean
    Dim Temp As String
    '
    ' Init defaults
    '
    Ed.GridMode = 1
    Ed.RotGridMode = 1
    Ed.SpeedMode = 1
    Ed.SnapVertex = 1
    Ed.ShowVertices = 1
    '
    ' Build grid
    '
    For N = 0 To ToolbarCount - 1
        '
        i = Int(N / 3)
        j = N Mod 3
        '
        If N <> 0 Then
            Load ToolIcons(N)
        End If
        '
        ToolIcons(N).Left = (j * 35) * Screen.TwipsPerPixelX
        ToolIcons(N).Top = (i * 35) * Screen.TwipsPerPixelY
        ToolIcons(N).Width = 35 * Screen.TwipsPerPixelX
        ToolIcons(N).Height = 35 * Screen.TwipsPerPixelY
        ToolIcons(N).GroupNumber = N
        '
    Next N
    '
    ' Set all tool names
    '
    ToolIcons(0).Tag = "CAMERAMOVE"
    ToolIcons(1).Tag = "CAMERAZOOM"
    '
    ToolIcons(3).Tag = "BRUSHROTATE"
    ToolIcons(4).Tag = "BRUSHSHEER"
    '
    ToolIcons(6).Tag = "BRUSHSCALE"
    ToolIcons(7).Tag = "BRUSHSTRETCH"
    '
    ToolIcons(9).Tag = "BRUSHSNAP"
    ToolIcons(10).Tag = "MOVEACTOR"
    '
    ToolIcons(12).Tag = "ADDLIGHT"
    ToolIcons(13).Tag = "ADDACTOR"
    '
    ToolIcons(15).Tag = "SELECT ALL"
    ToolIcons(16).Tag = "SELECT NONE"
    '
    ToolIcons(18).Tag = "TRANSACTION UNDO"
    ToolIcons(19).Tag = "TRANSACTION REDO"
    '
    ToolIcons(21).Tag = "TEXTURE RESET"
    ToolIcons(22).Tag = "BRUSH RESET"
    '
    ToolIcons(24).Tag = "TEXTUREPAN"
    ToolIcons(25).Tag = "TEXTUREROTATE"
    '
    ToolIcons(27).Tag = "BROKEN"
    ToolIcons(28).Tag = "BRUSH MIRROR"
    '
    ToolIcons(30).Tag = "MAPEDIT"
    ToolIcons(31).Tag = "TERRAFORM"
    '
    ToolIcons(33).Tag = "SHOWVERTICES"
    ToolIcons(34).Tag = "SNAPVERTEX"
    '
    ToolIcons(36).Tag = "HELP"
    ToolIcons(37).Tag = "SPEED"
    '
    ToolIcons(39).Tag = "GRID"
    ToolIcons(40).Tag = "ROTGRID"
    '
    ToolIcons(42).Tag = "ACTBROWSE"
    ToolIcons(43).Tag = "TEXBROWSE"
    '
    ' Brush tools:
    '
    ToolIcons(2).Tag = "BRUSH ADD"
    ToolIcons(5).Tag = "BRUSH SUBTRACT"
    ToolIcons(8).Tag = "BRUSH FROM INTERSECTION"
    ToolIcons(11).Tag = "BRUSH FROM DEINTERSECTION"
    ToolIcons(14).Tag = "BRUSH ADD SPECIAL"
    ToolIcons(17).Tag = "BRUSH ADDMOVER"
    ToolIcons(20).Tag = "RECTANGLE"
    ToolIcons(23).Tag = "SPHERE"
    ToolIcons(26).Tag = "CYLINDER"
    ToolIcons(29).Tag = "CONE"
    ToolIcons(32).Tag = "STAIR"
    ToolIcons(35).Tag = "SPIRAL"
    ToolIcons(38).Tag = "CURVEDSTAIR"
    ToolIcons(41).Tag = "SHEET"
    ToolIcons(44).Tag = "LOAD"
    '
    GToolClicking = 1
    For N = 0 To ToolbarCount - 1
        '
        ' Get picture
        '
        Call Ed.Tools.GetPicture(ToolIcons(N).Tag, ToolIcons(N))
        '
        ' Set highlighting
        '
        Call Ed.Tools.Handlers(ToolIcons(N).Tag).GetStatus(ToolIcons(N).Tag, Ed, Temp, Highlight)
        Call Ed.Tools.Highlight(ToolIcons(N).Tag, Highlight)
        '
        ' Make visible
        '
        ToolIcons(N).Visible = True
    Next N
    StatusText.ZOrder
    GToolClicking = 0
    '
    ' Set initial mode to first tool, and show picture:
    '
    Call Ed.Tools.Handlers("CAMERAMOVE").DoClick("CAMERAMOVE", Ed)
    '
End Sub

Private Sub Scroller_Scroll()
    Holder.Top = -Scroller.Value
End Sub

Private Sub Scroller_Change()
    Scroller_Scroll
End Sub

Private Sub ToolIcons_Click(index As Integer, Value As Integer)
    Dim i As Integer
    Dim Tool As String
    '
    If GToolClicking = 0 Then
        GToolClicking = 1
        Tool = ToolIcons(index).Tag
        Call Ed.Tools.Handlers(Tool).DoClick(Tool, Ed)
        Ed.MRUTool = Tool
        GToolClicking = 0
    End If
    '
End Sub

Private Sub ToolIcons_MouseDown(index As Integer, Button As Integer, Shift As Integer, X As Single, Y As Single)
    '
    Dim i As Integer
    Dim Icon As Integer
    Dim Temp As String
    Dim Highlight As Integer
    '
    If (Button And 2) <> 0 Then ' Left click
        Set PopupToolControl = ToolIcons(index)
        PopupToolMoveable = False
        PopupToolIndex = index
        Call LeftClickTool(ToolIcons(index).Tag, frmMain)
    End If
    '
End Sub

'
' Resize the entire screen: toolbar, panel,
' and browser.
'
Public Sub ResizeAll(Reopen As Boolean)
    Dim MustExit As Integer
    '
    If GInitialResized = 0 Then Exit Sub ' Just starting up
    If WindowState = 1 Then Exit Sub ' Minimized
    '
    If ScaleWidth < 480 * Screen.TwipsPerPixelX Then
        Width = Width - ScaleWidth + 480 * Screen.TwipsPerPixelX
        MustExit = True
    End If
    If ScaleHeight < 280 * Screen.TwipsPerPixelY Then
        Height = Height - ScaleHeight + 280 * Screen.TwipsPerPixelY
        MustExit = True
    End If
    If MustExit Then Exit Sub
    '
    GResizingAll = 1
    '
    ' Set visibility and positions:
    '
    CameraHolder.Visible = False
    MainBar.Visible = True
    '
    CX = 0
    CY = MainBar.Height
    CXL = ScaleWidth
    CYL = ScaleHeight - MainBar.Height
    '
    If Ed.BrowserPos = 2 Then ' hide
        BrowserPanel.Visible = False
    Else
        If BrowserPanel.Visible = False Then
            BrowserPanel.Align = 0
        End If
        BrowserPanel.Visible = True ' Must do before align
        '
        If Ed.BrowserPos = 0 Then ' right
            BrowserPanel.Align = 4
        Else ' left
            BrowserPanel.Align = 3
            CX = CX + BrowserPanel.Width
        End If
        CXL = CXL - (BrowserPanel.Width + 2 * Screen.TwipsPerPixelX)
        '
        If BrowserTopicCombo.Text <> "" Then
            Ed.ReloadBrowser
        End If
    End If
    '
    If Ed.ToolbarPos = 0 Then ' left
        Toolbar.Align = 3
    Else ' right
        Toolbar.Align = 4
    End If
    '
    ResizeToolbar
    Toolbar.Visible = True
    '
    If Ed.ToolbarPos = 0 Then CX = CX + Toolbar.Width
    CXL = CXL - Toolbar.Width
    '
    ' Resize Window Guides:
    '
    VBar.Visible = False
    LBar.Visible = False
    RBar.Visible = False
    LBox.Visible = False
    RBox.Visible = False
    '
    ' Resize camera holder:
    '
    CameraHolder.Visible = True
    '
    CameraHolder.Left = CX
    CameraHolder.Top = CY
    CameraHolder.Width = CXL
    CameraHolder.Height = CYL
    '
    If ScaleWidth >= 160 * Screen.TwipsPerPixelX And _
        ScaleHeight >= 120 * Screen.TwipsPerPixelY And _
        (Ed.Startup = 0) Then
        '
        VBar.Top = 0
        VBar.Height = CYL
        VBar.Left = Ed.CameraVertRatio * (CXL - VBar.Width)
        If VBar.Left < 0 Then VBar.Left = 0
        If VBar.Left > CXL - VBar.Width Then VBar.Left = CXL - VBar.Width
        '
        LBar.Left = 0
        LBar.Width = VBar.Left
        LBar.Top = Ed.CameraLeftRatio * (CYL - LBar.Height)
        If LBar.Top < 0 Then LBar.Top = 0
        If LBar.Top > CYL - LBar.Height Then LBar.Top = CYL - LBar.Height
        '
        RBar.Left = VBar.Left + VBar.Width
        RBar.Width = CXL - (VBar.Left + VBar.Width)
        RBar.Top = Ed.CameraRightRatio * (CYL - RBar.Height)
        If RBar.Top < 0 Then RBar.Top = 0
        If RBar.Top > CYL - RBar.Height Then RBar.Top = CYL - RBar.Height
        '
        LBox.Left = VBar.Left
        LBox.Top = LBar.Top
        '
        RBox.Left = VBar.Left
        RBox.Top = RBar.Top
        '
        VBar.Visible = True
        LBar.Visible = True
        RBar.Visible = True
        LBox.Visible = True
        RBox.Visible = True
    End If
    '
    ' Open cameras:
    '
    Call Ed.OpenCamera(Reopen, 0, 0, VBar.Left / Screen.TwipsPerPixelX, _
        LBar.Top / Screen.TwipsPerPixelY, _
        SHOW_NORMAL + SHOW_STANDARD_VIEW + SHOW_AS_CHILD + SHOW_MOVINGBRUSHES, _
        REN_ORTHXY, "Standard1")
    Call Ed.OpenCamera(Reopen, (VBar.Left + VBar.Width) / Screen.TwipsPerPixelX, _
        0, (CXL - VBar.Left - VBar.Width) / Screen.TwipsPerPixelX, _
        RBar.Top / Screen.TwipsPerPixelY, _
        SHOW_NORMAL + SHOW_STANDARD_VIEW + SHOW_AS_CHILD + SHOW_MOVINGBRUSHES, _
        REN_ORTHXZ, "Standard2")
    Call Ed.OpenCamera(Reopen, 0, (LBar.Top + LBar.Height) / Screen.TwipsPerPixelY, _
        VBar.Left / Screen.TwipsPerPixelX, (CYL - LBar.Top - LBar.Height) / Screen.TwipsPerPixelY, _
        SHOW_NORMAL + SHOW_STANDARD_VIEW + SHOW_MENU + SHOW_AS_CHILD + SHOW_MOVINGBRUSHES, _
        REN_PLAINTEX, "Standard3")
    Call Ed.OpenCamera(Reopen, (VBar.Left + VBar.Width) / Screen.TwipsPerPixelX, _
        (RBar.Top + RBar.Height) / Screen.TwipsPerPixelY, _
        (CXL - VBar.Left - VBar.Width) / Screen.TwipsPerPixelX, _
        (CYL - RBar.Top - RBar.Height) / Screen.TwipsPerPixelY, _
        SHOW_NORMAL + SHOW_STANDARD_VIEW + SHOW_AS_CHILD + SHOW_MOVINGBRUSHES, _
        REN_ORTHYZ, "Standard4")
    '
    ' Force all forms to be clipped to the newly-sized
    ' window and forced in front of cameras:
    '
    Ed.NoteResize
End Sub

Private Sub Form_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If (Button And 2) <> 0 Then ' Right click
        PopupMenu frmPopups.Window
    End If
End Sub

'
' Vertical bar
'

Private Sub VBar_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    ZOrder
    If Button = 1 Then
        Ed.Server.Exec "CAMERA HIDESTANDARD"
        VBarHold = 1
        VDelta = X
    End If
End Sub

Private Sub VBar_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim NewX As Integer
    If VBarHold And Button = 1 Then
        NewX = VBar.Left + X - VDelta
        If NewX < 0 Then
            VBar.Left = 0
        ElseIf NewX > CXL - VBar.Width Then
            VBar.Left = CXL - VBar.Width
        Else
            VBar.Left = NewX
        End If
    LBox.Left = VBar.Left
    RBox.Left = VBar.Left
    LBar.Width = VBar.Left
    RBar.Left = VBar.Left + VBar.Width
    RBar.Width = CXL - (VBar.Left + VBar.Width)
    Ed.CameraVertRatio = CSng(VBar.Left) / (CXL - VBar.Width)
    End If
End Sub

Private Sub VBar_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If VBarHold Then ResizeAll (True)
    VBarHold = 0
End Sub

'
' LBar
'

Private Sub LBar_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 1 Then
        Ed.Server.Exec "CAMERA HIDESTANDARD"
        LBarHold = 1
        HDelta = Y
    End If
End Sub

Private Sub LBar_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim NewY As Integer
    If LBarHold And Button = 1 Then
        NewY = LBar.Top + Y - HDelta
        If NewY < 0 Then
            LBar.Top = 0
        ElseIf NewY > CYL - LBar.Height Then
            LBar.Top = CYL - LBar.Height
        Else
            LBar.Top = NewY
        End If
        LBox.Top = LBar.Top
        Ed.CameraLeftRatio = CSng(LBar.Top) / CSng(CY + CYL - LBar.Height)
    End If
End Sub

Private Sub LBar_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If LBarHold Then ResizeAll (True)
    LBarHold = 0
End Sub

'
' RBar
'

Private Sub RBar_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 1 Then
        Ed.Server.Exec "CAMERA HIDESTANDARD"
        RBarHold = 1
        HDelta = Y
    End If
End Sub

Private Sub RBar_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim NewY As Integer
    If RBarHold And Button = 1 Then
        NewY = RBar.Top + Y - HDelta
        If NewY < 0 Then
            RBar.Top = 0
        ElseIf NewY > CYL - RBar.Height Then
            RBar.Top = CYL - RBar.Height
        Else
            RBar.Top = NewY
        End If
        RBox.Top = RBar.Top
        Ed.CameraRightRatio = CSng(RBar.Top) / CSng(CYL - RBar.Height)
    End If
End Sub

Private Sub RBar_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If RBarHold Then ResizeAll (True)
    RBarHold = 0
End Sub

'
' L box
'

Private Sub LBox_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 1 Then
        Ed.Server.Exec "CAMERA HIDESTANDARD"
        LBoxHold = 1
        LBarHold = 1
        VBarHold = 1
        '
        VDelta = X - LBox.Left + VBar.Left
        HDelta = Y - LBox.Top + LBar.Top
    End If
End Sub

Private Sub LBox_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If LBoxHold And Button = 1 Then
        Call VBar_MouseMove(Button, Shift, X, Y)
        Call LBar_MouseMove(Button, Shift, X, Y)
    End If
End Sub

Private Sub LBox_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If LBoxHold Then ResizeAll (True)
    '
    LBoxHold = 0
    LBarHold = 0
    VBarHold = 0
End Sub

'
' R box
'

Private Sub RBox_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 1 Then
        Ed.Server.Exec "CAMERA HIDESTANDARD"
        RBoxHold = 1
        RBarHold = 1
        VBarHold = 1
        '
        VDelta = X - RBox.Left + VBar.Left
        HDelta = Y - RBox.Top + RBar.Top
    End If
End Sub

Private Sub RBox_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If RBoxHold And Button = 1 Then
        Call VBar_MouseMove(Button, Shift, X, Y)
        Call RBar_MouseMove(Button, Shift, X, Y)
    End If
End Sub

Private Sub RBox_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If RBoxHold Then ResizeAll (True)
    RBoxHold = 0
    RBarHold = 0
    VBarHold = 0
End Sub

'
' UnrealEdServer callback dispatcher
'
Private Sub Callback_KeyPress(KeyAscii As Integer)
    '
    Dim N As Integer
    Dim S As String
    '
    Select Case KeyAscii - 32
    Case EDC_GENERAL:
    Case EDC_CURTEXCHANGE:
        frmMain.TextureCombo.List(0) = Ed.Server.GetProp("ED", "CURTEX")
        frmMain.TextureCombo.ListIndex = 0
        frmTexBrowser.BrowserRefresh
    Case EDC_CURCLASSCHANGE:
        frmMain.ActorCombo.List(0) = Ed.Server.GetProp("ED", "CURCLASS")
        frmMain.ActorCombo.ListIndex = 0
    Case EDC_SELPOLYCHANGE:
        If GPolyPropsAction = 1 Then
            frmSurfaceProps.GetSelectedPolys
        End If
    Case EDC_SELACTORCHANGE:
        If GActorPropsAction = 1 Then
            frmActorProperties.GetSelectedActors
        ElseIf GActorPropsAction = 2 Then
            frmActorProperties.NoteClassChange
        End If
    Case EDC_SELBRUSHCHANGE:
    Case EDC_RTCLICKTEXTURE:
        frmMain.TextureCombo.List(0) = Ed.Server.GetProp("ED", "CURTEX")
        frmMain.TextureCombo.ListIndex = 0
        frmTexBrowser.BrowserRefresh
        Call frmTexBrowser.TextureList_MouseDown(2, 0, 0, 0)
    Case EDC_RTCLICKPOLY:
        frmPopups2.prProperties.Caption = "Surface &Properties (" & Ed.Server.GetProp("Polys", "NumSelected") & " selected)..."
        frmPopups2.prApplyTex.Caption = "Apply &Texture " & frmMain.TextureCombo.List(0)
        PopupMenu frmPopups2.PolyRtClick
    Case EDC_RTCLICKACTOR:
        N = Val(Ed.Server.GetProp("Actor", "NumSelected"))
        GPopupActorClass = Ed.Server.GetProp("Actor", "ClassSelected")
        '
        frmPopups2.arMoverKeyframe.Visible = _
            (Val(Ed.Server.GetProp("Actor", "ISKINDOF CLASS=MOVER")) = 1)
        If GPopupActorClass <> "" Then
            frmPopups2.arProps.Caption = GPopupActorClass & " &Properties (" & Trim(Str(N)) & " selected)..."
            frmPopups2.arSelectAllOfType.Caption = "Select all " & GPopupActorClass & " actors"
            frmPopups2.arSelectAllOfType.Visible = True
            frmPopups2.arRememberClass.Visible = True
        Else
            frmPopups2.arProps.Caption = "Actor &Properties (" & Trim(Str(N)) & " selected)..."
            frmPopups2.arSelectAllOfType.Visible = False
            frmPopups2.arRememberClass.Visible = False
        End If
        '
        PopupMenu frmPopups2.ActorRtClick
    Case EDC_MODECHANGE:
    Case EDC_BRUSHCHANGE:
    Case EDC_MAPCHANGE:
    Case EDC_ACTORCHANGE:
    Case EDC_RTCLICKWINDOW:
        PopupMenu frmPopups.Window
    End Select
    '
End Sub

Public Sub PreferencesChange()
    WorldBrowser.Visible = Ed.GodMode
    MacroS.Visible = Ed.GodMode
End Sub

'
' Old code, no longer in use
'

Private Sub PlayLevel_Click()
    PreSave
    Ed.BeginSlowTask ("Saving map for play")
    Ed.Server.SlowExec "MAP SAVE FILE=" & Quotes(Ed.BaseDir & Ed.MapDir & "\PlayTemp.unr")
    Ed.Server.Exec "APP MINIMIZE"
    frmMain.WindowState = 1 ' Minimize
    Call Shell(Ed.BaseDir + "\Unreal.exe FILE=" & Quotes(Ed.BaseDir & Ed.MapDir & "\PlayTemp.unr") & " AUTODELETE=ON REFOCUS=" & Trim(Str(frmMain.hwnd)), 1)
    Ed.EndSlowTask
End Sub

