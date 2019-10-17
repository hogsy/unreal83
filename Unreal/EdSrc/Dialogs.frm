VERSION 4.00
Begin VB.Form frmDialogs 
   AutoRedraw      =   -1  'True
   BorderStyle     =   0  'None
   Caption         =   "Common dialogs used throughout"
   ClientHeight    =   4095
   ClientLeft      =   525
   ClientTop       =   1185
   ClientWidth     =   10275
   Enabled         =   0   'False
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
   Height          =   4500
   Icon            =   "Dialogs.frx":0000
   Left            =   465
   LinkTopic       =   "Form1"
   ScaleHeight     =   4095
   ScaleWidth      =   10275
   ShowInTaskbar   =   0   'False
   Top             =   840
   Width           =   10395
   Begin MSComDlg.CommonDialog TwoDeeTexture 
      Left            =   2820
      Top             =   2100
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "bmp"
      DialogTitle     =   "Load Texture in Background"
      Filter          =   "BMP files (*.bmp)|*.bmp|All Files (*.*)|*.*"
   End
   Begin VB.Label Label2 
      Caption         =   "TwoDeeTexture"
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
      Left            =   2580
      TabIndex        =   28
      Top             =   2640
      Width           =   1335
   End
   Begin MSComDlg.CommonDialog TwoDeeOpen 
      Left            =   420
      Top             =   2100
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "u2d"
      DialogTitle     =   "Open 2D Shape"
      Filter          =   "2D Shapes (*.u2d)|*.u2d|All Files (*.*)|*.*"
      Flags           =   2097152
   End
   Begin MSComDlg.CommonDialog TwoDeeSave 
      Left            =   1620
      Top             =   2040
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "u2d"
      DialogTitle     =   "Save 2D Shape"
      Filter          =   "2D Shapes (*.u2d)|*.u2d|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin VB.Label Label30 
      Caption         =   "SoundLoadFamilyDlg"
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
      Left            =   4740
      TabIndex        =   27
      Top             =   3540
      Width           =   1755
   End
   Begin MSComDlg.CommonDialog SoundLoadFamilyDlg 
      Left            =   5340
      Top             =   3000
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "uax"
      DialogTitle     =   "Load Sound Family"
      Filter          =   "Unreal Sounds (*.uax)|*.uax|All Files (*.*)|*.*"
      Flags           =   33280
      MaxFileSize     =   5000
   End
   Begin VB.Label Label29 
      Caption         =   "SoundSaveFamilyDlg"
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
      Left            =   3000
      TabIndex        =   26
      Top             =   3540
      Width           =   1575
   End
   Begin MSComDlg.CommonDialog SoundSaveFamilyDlg 
      Left            =   3480
      Top             =   3000
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "uax"
      DialogTitle     =   "Load Sound Family"
      Filter          =   "Unreal Sounds (*.uax)|*.uax|All Files (*.*)|*.*"
      Flags           =   33280
      MaxFileSize     =   5000
   End
   Begin VB.Label Label9 
      Caption         =   "SoundExportDlg"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   1560
      TabIndex        =   25
      Top             =   3540
      Width           =   1335
   End
   Begin MSComDlg.CommonDialog SoundExportDlg 
      Left            =   1860
      Top             =   3000
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "ufx"
      DialogTitle     =   "Export A Sound"
      Filter          =   "Unreal sound effect (*.ufx)|*.ufx"
      Flags           =   2097152
   End
   Begin VB.Label Label8 
      Caption         =   "SoundImportDlg"
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
      TabIndex        =   24
      Top             =   3600
      Width           =   1275
   End
   Begin MSComDlg.CommonDialog SoundImportDlg 
      Left            =   420
      Top             =   3060
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "ufx"
      DialogTitle     =   "Import sound"
      Filter          =   "Unreal sound effect (*.ufx)|*.ufx|All Files (*.*)|*.*"
      Flags           =   2097664
      MaxFileSize     =   5000
   End
   Begin VB.Label Label28 
      Caption         =   "ClassLoad"
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
      Left            =   3360
      TabIndex        =   23
      Top             =   720
      Width           =   855
   End
   Begin VB.Label Label27 
      Caption         =   "ClassSave"
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
      Left            =   4320
      TabIndex        =   22
      Top             =   720
      Width           =   855
   End
   Begin MSComDlg.CommonDialog ClassLoad 
      Left            =   3600
      Top             =   120
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "ucx"
      DialogTitle     =   "Load Actor Class File"
      Filter          =   "Unreal Actor Class (*.ucx)|*.ucx|Actor class text files (*.tcx)|*.tcx|All Files (*.*)|*.*"
      Flags           =   33280
   End
   Begin MSComDlg.CommonDialog ClassSave 
      Left            =   4560
      Top             =   120
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "ucx"
      DialogTitle     =   "Save Actor Class File"
      Filter          =   "Unreal Actor Class (*.ucx)|*.ucx|Actor class text file (*.tcx)|*.tcx|C++ header (*.h)|*.h"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog ExportTex 
      Left            =   8820
      Top             =   120
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "pcx"
      DialogTitle     =   "Export A Texture"
      Filter          =   "Standard PCX file (*.pcx)|*.pcx"
      Flags           =   2097154
   End
   Begin VB.Label Label26 
      Caption         =   "ExportTex"
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
      Left            =   8700
      TabIndex        =   21
      Top             =   720
      Width           =   1095
   End
   Begin VB.Label Label24 
      Caption         =   "AddFile"
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
      Left            =   4440
      TabIndex        =   20
      Top             =   2640
      Width           =   735
   End
   Begin MSComDlg.CommonDialog AddFile 
      Left            =   4440
      Top             =   2100
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "unr"
      DialogTitle     =   "Load an UnrealEd file at startup"
      Filter          =   $"Dialogs.frx":030A
      Flags           =   2129920
   End
   Begin VB.Label Label23 
      Caption         =   "MacroPick"
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
      Left            =   2400
      TabIndex        =   19
      Top             =   720
      Width           =   855
   End
   Begin MSComDlg.CommonDialog MacroPick 
      Left            =   2400
      Top             =   180
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "mac"
      DialogTitle     =   "Set Initial UnrealEd Macro"
      Filter          =   "Unreal Macros (*.mac)|*.mac|All Files (*.*)|*.*"
      Flags           =   2097152
   End
   Begin VB.Label Label22 
      Caption         =   "ToolHelp"
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
      Left            =   9240
      TabIndex        =   18
      Top             =   2700
      Width           =   855
   End
   Begin MSComDlg.CommonDialog ToolHelp 
      Left            =   9360
      Top             =   2100
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      HelpCommand     =   1
      HelpContext     =   105
      HelpFile        =   "help\unrealed.hlp"
   End
   Begin MSComDlg.CommonDialog RelNotes 
      Left            =   6300
      Top             =   2100
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      HelpCommand     =   1
      HelpContext     =   105
      HelpFile        =   "help\unrealed.hlp"
   End
   Begin VB.Label Label21 
      Caption         =   "RelNotes"
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
      Left            =   6300
      TabIndex        =   17
      Top             =   2700
      Width           =   855
   End
   Begin VB.Label Label20 
      Caption         =   "About"
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
      Left            =   8460
      TabIndex        =   16
      Top             =   2700
      Width           =   615
   End
   Begin MSComDlg.CommonDialog About 
      Left            =   8460
      Top             =   2100
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      HelpCommand     =   1
      HelpContext     =   104
      HelpFile        =   "help\unrealed.hlp"
   End
   Begin VB.Label Label19 
      Caption         =   "HelpContents"
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
      Left            =   7260
      TabIndex        =   15
      Top             =   2700
      Width           =   1095
   End
   Begin MSComDlg.CommonDialog HelpContents 
      Left            =   7500
      Top             =   2100
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      HelpCommand     =   11
      HelpFile        =   "help\unrealed.hlp"
   End
   Begin MSComDlg.CommonDialog MacroLoad 
      Left            =   3600
      Top             =   1140
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "mac"
      DialogTitle     =   "Load Macro"
      Filter          =   "Unreal Macros (*.mac)|*.mac|All Files (*.*)|*.*"
      Flags           =   2097152
   End
   Begin MSComDlg.CommonDialog MacroSaveAs 
      Left            =   4860
      Top             =   1140
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "mac"
      DialogTitle     =   "Save Macro As"
      Filter          =   "Unreal Macros (*.mac)|*mac|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog ImportMap 
      Left            =   2400
      Top             =   1140
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "t3d"
      DialogTitle     =   "Import Map"
      Filter          =   "Unreal Text (*.t3d)|*.t3d|All Files (*.*)|*.*"
      Flags           =   2097152
   End
   Begin MSComDlg.CommonDialog ExportMap 
      Left            =   1440
      Top             =   1140
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "t3d"
      DialogTitle     =   "Export Map"
      Filter          =   "Unreal Text (*.t3d)|*.t3d|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog TexFamLoad 
      Left            =   5520
      Top             =   120
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "utx"
      DialogTitle     =   "Load Texture Family"
      Filter          =   "Unreal Textures (*.utx)|*.utx|All Files (*.*)|*.*"
      Flags           =   33280
      MaxFileSize     =   5000
   End
   Begin MSComDlg.CommonDialog TexFamSave 
      Left            =   6600
      Top             =   120
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "utx"
      DialogTitle     =   "Save Texture Family"
      Filter          =   "Unreal Textures (*.utx)|*.utx|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog TexImport 
      Left            =   7740
      Top             =   120
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "pcx"
      DialogTitle     =   "Import texture map"
      Filter          =   "PCX files (*.pcx)|*.pcx|All Files (*.*)|*.*"
      Flags           =   2097664
      MaxFileSize     =   5000
   End
   Begin MSComDlg.CommonDialog BrushSave 
      Left            =   8220
      Top             =   1200
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "u3d"
      DialogTitle     =   "Save Brush"
      Filter          =   "3D Solids (*.u3d)|*.u3d|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog BrushOpen 
      Left            =   120
      Top             =   1200
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "u3d"
      DialogTitle     =   "Open Brush"
      Filter          =   "3D Solids (*.u3d)|*.u3d|All Files (*.*)|*.*"
      Flags           =   2097152
   End
   Begin MSComDlg.CommonDialog ExportBrush 
      Left            =   120
      Top             =   180
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "t3d"
      DialogTitle     =   "Export A Brush"
      Filter          =   "Unreal Text (*.t3d)|*.t3d|DXF Files (*.dxf)|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog ImportBrush 
      Left            =   1320
      Top             =   180
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "t3d"
      DialogTitle     =   "Import A Brush"
      Filter          =   "Importable (*.t3d; *.dxf; *.asc)|*.t3d;*.dxf;*.asc|All Files (*.*)|*.*"
      Flags           =   2097152
   End
   Begin MSComDlg.CommonDialog MapSaveAs 
      Left            =   6900
      Top             =   1200
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "unr"
      DialogTitle     =   "Save Map As"
      Filter          =   "Unreal Maps (*.unr)|*.unr|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog MapOpen 
      Left            =   5700
      Top             =   1200
      _Version        =   65536
      _ExtentX        =   847
      _ExtentY        =   847
      _StockProps     =   0
      CancelError     =   -1  'True
      DefaultExt      =   "unr"
      DialogTitle     =   "Open Map File"
      Filter          =   "Unreal Maps (*.unr)|*.unr|All Files (*.*)|*.*"
      Flags           =   2129920
   End
   Begin VB.Label Label18 
      Caption         =   "MacroLoad"
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
      Left            =   3360
      TabIndex        =   14
      Top             =   1740
      Width           =   975
   End
   Begin VB.Label Label17 
      Caption         =   "MacroSaveAs"
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
      Left            =   4500
      TabIndex        =   13
      Top             =   1740
      Width           =   1155
   End
   Begin VB.Label Label16 
      Caption         =   "ImportMap"
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
      TabIndex        =   12
      Top             =   1740
      Width           =   975
   End
   Begin VB.Label Label15 
      Caption         =   "ExportMap"
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
      Left            =   1200
      TabIndex        =   11
      Top             =   1740
      Width           =   975
   End
   Begin VB.Label Label14 
      Caption         =   "TexFamLoad"
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
      Left            =   5280
      TabIndex        =   10
      Top             =   720
      Width           =   1035
   End
   Begin VB.Label Label13 
      Caption         =   "TexFamSave"
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
      Left            =   6420
      TabIndex        =   9
      Top             =   720
      Width           =   1095
   End
   Begin VB.Label Label12 
      Caption         =   "TexImport"
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
      Left            =   7620
      TabIndex        =   8
      Top             =   720
      Width           =   975
   End
   Begin VB.Label Label11 
      Caption         =   "TwoDeeSave"
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
      Left            =   1440
      TabIndex        =   7
      Top             =   2640
      Width           =   1095
   End
   Begin VB.Label Label10 
      Caption         =   "TwoDeeOpen"
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
      Left            =   180
      TabIndex        =   6
      Top             =   2640
      Width           =   1155
   End
   Begin VB.Label Label7 
      Caption         =   "BrushSave"
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
      Left            =   8220
      TabIndex        =   5
      Top             =   1740
      Width           =   915
   End
   Begin VB.Label Label6 
      Caption         =   "BrushOpen"
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
      Top             =   1740
      Width           =   975
   End
   Begin VB.Label Label5 
      Caption         =   "ExportBrush"
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
      TabIndex        =   3
      Top             =   720
      Width           =   975
   End
   Begin VB.Label Label4 
      Caption         =   "ImportBrush"
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
      Left            =   1260
      TabIndex        =   2
      Top             =   720
      Width           =   975
   End
   Begin VB.Label Label3 
      Caption         =   "MapSaveAs"
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
      Left            =   6900
      TabIndex        =   1
      Top             =   1740
      Width           =   1215
   End
   Begin VB.Label Label1 
      Caption         =   "MapOpen"
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
      Left            =   5760
      TabIndex        =   0
      Top             =   1740
      Width           =   975
   End
End
Attribute VB_Name = "frmDialogs"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit


