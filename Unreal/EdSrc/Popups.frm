VERSION 4.00
Begin VB.Form frmPopups 
   Caption         =   "frmPopups"
   ClientHeight    =   870
   ClientLeft      =   4845
   ClientTop       =   3615
   ClientWidth     =   7560
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
   Height          =   1560
   Icon            =   "Popups.frx":0000
   Left            =   4785
   LinkTopic       =   "Form1"
   ScaleHeight     =   870
   ScaleWidth      =   7560
   ShowInTaskbar   =   0   'False
   Top             =   2985
   Width           =   7680
   Begin VB.Label Label1 
      Caption         =   "Misc popup menus that are brought up when you left-click on stuff"
      Height          =   495
      Left            =   1230
      TabIndex        =   0
      Top             =   90
      Width           =   3135
   End
   Begin VB.Menu Toolbar 
      Caption         =   "Toolbar"
      Begin VB.Menu ToolbarProperties 
         Caption         =   "&Properties..."
      End
      Begin VB.Menu ToolbarHelp 
         Caption         =   "&Help"
      End
      Begin VB.Menu MoveToolbar 
         Caption         =   "&Move toolbar"
      End
      Begin VB.Menu ZJIWZA 
         Caption         =   "-"
      End
      Begin VB.Menu ToolbarDo 
         Caption         =   "Do it"
      End
   End
   Begin VB.Menu BrowserWin 
      Caption         =   "Browser"
      Begin VB.Menu BroStats 
         Caption         =   "&Stats"
      End
      Begin VB.Menu BroProps 
         Caption         =   "&Properties"
      End
      Begin VB.Menu BroRename 
         Caption         =   "&Rename"
      End
      Begin VB.Menu BroDelete 
         Caption         =   "&Delete"
      End
      Begin VB.Menu WUZNQA 
         Caption         =   "-"
      End
      Begin VB.Menu BroSave 
         Caption         =   "&Save"
         Begin VB.Menu BroSaveThis 
            Caption         =   "This resource"
         End
         Begin VB.Menu BroSaveHierarch 
            Caption         =   "This hierarchy"
         End
      End
      Begin VB.Menu BroExport 
         Caption         =   "&Export..."
      End
   End
   Begin VB.Menu Window 
      Caption         =   "Window"
      Begin VB.Menu Rebuilder 
         Caption         =   "&Rebuilder"
      End
      Begin VB.Menu ResizeMvBrush 
         Caption         =   "Resize/Move &Brush"
      End
      Begin VB.Menu TwoDee 
         Caption         =   "&2D Shape Editor"
      End
      Begin VB.Menu Prefs 
         Caption         =   "Pr&eferences"
      End
      Begin VB.Menu ZOEEKW 
         Caption         =   "-"
      End
      Begin VB.Menu TexProp 
         Caption         =   "&Polygon Properties"
      End
      Begin VB.Menu ActProp 
         Caption         =   "&Actor Properties"
      End
      Begin VB.Menu ZUHWA 
         Caption         =   "-"
      End
      Begin VB.Menu TexBro 
         Caption         =   "&Texture Browser"
      End
      Begin VB.Menu ClassBro 
         Caption         =   "&Class Browser"
      End
   End
   Begin VB.Menu ResetBrush 
      Caption         =   "ResetBrush"
      Begin VB.Menu ResetBrushRotation 
         Caption         =   "Reset Brush &Rotation"
      End
      Begin VB.Menu ResetBrushScale 
         Caption         =   "Reset Brush &Scale"
      End
      Begin VB.Menu ResetBrushPosition 
         Caption         =   "Reset Brush &Position"
      End
      Begin VB.Menu ZZTRULZ 
         Caption         =   "-"
      End
      Begin VB.Menu ResetBrushAll 
         Caption         =   "&Reset All"
      End
   End
   Begin VB.Menu MirrorBrush 
      Caption         =   "&MirrorBrush"
      Begin VB.Menu MirrorX 
         Caption         =   "Mirror brush about &X Axis"
      End
      Begin VB.Menu MirrorY 
         Caption         =   "Mirror brush about &Y Axis"
      End
      Begin VB.Menu MirrorZ 
         Caption         =   "&Z Axis  (vertical)"
      End
   End
   Begin VB.Menu Help 
      Caption         =   "&Help"
      Begin VB.Menu HelpTopics 
         Caption         =   "&Help Topics"
         Shortcut        =   {F1}
      End
      Begin VB.Menu HelpCameras 
         Caption         =   "Help Using &Cameras"
         Shortcut        =   {F2}
      End
      Begin VB.Menu ZOAOE 
         Caption         =   "-"
      End
      Begin VB.Menu ReleaseNotes 
         Caption         =   "&Release Notes"
      End
      Begin VB.Menu ZOGMATISM 
         Caption         =   "-"
      End
      Begin VB.Menu About 
         Caption         =   "&About UnrealEd"
      End
   End
   Begin VB.Menu Panel 
      Caption         =   "Panel"
      Begin VB.Menu PanelBottom 
         Caption         =   "Panel on &Bottom"
      End
      Begin VB.Menu PanelTop 
         Caption         =   "Panel on &Top"
      End
      Begin VB.Menu ShowHidePanel 
         Caption         =   "&Hide Panel"
      End
   End
   Begin VB.Menu Browser 
      Caption         =   "&Browser"
      Begin VB.Menu BrowserRight 
         Caption         =   "Browser on &Right"
      End
      Begin VB.Menu BrowserLeft 
         Caption         =   "Browser on &Left"
      End
      Begin VB.Menu BrowserHide 
         Caption         =   "&Hide Browser"
      End
   End
   Begin VB.Menu TexBrowser 
      Caption         =   "TexBrowser"
      Begin VB.Menu TBProperties 
         Caption         =   "&Properties..."
      End
      Begin VB.Menu TBApply 
         Caption         =   "&Apply"
      End
      Begin VB.Menu ZTBXS 
         Caption         =   "-"
      End
      Begin VB.Menu TBExport 
         Caption         =   "Ex&port"
      End
      Begin VB.Menu TBDelete 
         Caption         =   "&Delete"
      End
   End
End
Attribute VB_Name = "frmPopups"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub About_Click()
    frmDialogs.About.ShowHelp
End Sub

Private Sub ActProp_Click()
    frmActorProperties.GetSelectedActors
End Sub

Private Sub BrowserHide_Click()
    Ed.BrowserPos = 2
    frmMain.ResizeAll (True)
End Sub

Private Sub BrowserLeft_Click()
    Ed.BrowserPos = 1
    frmMain.ResizeAll (True)
End Sub

Private Sub BrowserRight_Click()
    Ed.BrowserPos = 0
    frmMain.ResizeAll (True)
End Sub

Private Sub ClassBro_Click()
    Ed.SetBrowserTopic ("Classes")
End Sub

Private Sub HelpCameras_Click()
    Call Ed.Tools.Handlers(Ed.ToolMode).DoHelp(Ed.ToolMode, Ed)
End Sub

Private Sub HelpTopics_Click()
    frmDialogs.HelpContents.HelpFile = App.Path + "\\help\\unrealed.hlp"
    frmDialogs.HelpContents.ShowHelp
End Sub

Private Sub MirrorX_Click()
    Ed.Server.Exec "BRUSH MIRROR X"
End Sub

Private Sub MirrorY_Click()
    Ed.Server.Exec "BRUSH MIRROR Y"
End Sub

Private Sub MirrorZ_Click()
    Ed.Server.Exec "BRUSH MIRROR Z"
End Sub

Private Sub MoveToolbar_Click()
    Ed.ToolbarPos = 1 - Ed.ToolbarPos
    frmMain.ResizeAll (True)
End Sub

Private Sub Prefs_Click()
   frmPreferences.Show
End Sub

Private Sub Rebuilder_Click()
    frmRebuilder.Show ' Rebuild dialog
End Sub

Private Sub ReleaseNotes_Click()
    frmDialogs.RelNotes.ShowHelp
End Sub

Private Sub ResBro_Click()
    frmResBrowse.Show
End Sub

Private Sub ResetBrushAll_Click()
    Ed.Server.Exec "BRUSH RESET"
End Sub

Private Sub ResetBrushPosition_Click()
    Ed.Server.Exec "BRUSH MOVETO X=0 Y=0 Z=0"
End Sub

Private Sub ResetBrushRotation_Click()
    Ed.Server.Exec "BRUSH ROTATETO PITCH=0 YAW=0 ROLL=0"
End Sub

Private Sub ResetBrushScale_Click()
    Ed.Server.Exec "BRUSH SCALE RESET"
End Sub

Private Sub ResizeMvBrush_Click()
    frmBrush.Show
End Sub

Private Sub TexBro_Click()
    Ed.SetBrowserTopic ("Textures")
End Sub

Private Sub TexProp_Click()
   frmSurfaceProps.Show
End Sub

Private Sub ToolbarDo_Click()
    Call Ed.Tools.Handlers(PopupToolName).DoClick(PopupToolName, Ed)
End Sub

Private Sub ToolbarHelp_Click()
    Call Ed.Tools.Handlers(PopupToolName).DoHelp(PopupToolName, Ed)
End Sub

Private Sub ToolbarProperties_Click()
    Call Ed.Tools.Handlers(PopupToolName).DoProperties(PopupToolName, Ed)
End Sub

Private Sub TWODEE_Click()
    frmTwoDee.Show
End Sub

'
' TexBrowser
'

Private Sub TBApply_Click()
    frmTexBrowser.BroApply_Click
End Sub

Private Sub TBDelete_Click()
    frmTexBrowser.BroDelete_Click
End Sub

Private Sub TBExport_Click()
    frmTexBrowser.BroExport_Click
End Sub

Private Sub TBProperties_Click()
    frmTexBrowser.BroEdit_Click
End Sub
