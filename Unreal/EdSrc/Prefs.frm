VERSION 4.00
Begin VB.Form frmPreferences 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Preferences"
   ClientHeight    =   6105
   ClientLeft      =   2100
   ClientTop       =   2310
   ClientWidth     =   6000
   ForeColor       =   &H80000008&
   Height          =   6510
   HelpContextID   =   116
   Icon            =   "Prefs.frx":0000
   Left            =   2040
   LinkTopic       =   "Form5"
   MaxButton       =   0   'False
   ScaleHeight     =   6105
   ScaleWidth      =   6000
   ShowInTaskbar   =   0   'False
   Top             =   1965
   Width           =   6120
   Begin VB.Frame Frame5 
      Caption         =   "Editing Features"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   855
      Left            =   120
      TabIndex        =   18
      Top             =   4680
      Width           =   5775
      Begin VB.CheckBox GodMode 
         Caption         =   "God Mode: Enables you to use options which may be dangerous"
         Height          =   255
         Left            =   180
         TabIndex        =   19
         Top             =   300
         Width           =   5475
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Field of view"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   735
      Left            =   4080
      TabIndex        =   15
      Top             =   120
      Width           =   1815
      Begin VB.ComboBox FOV 
         Height          =   315
         Left            =   120
         Style           =   2  'Dropdown List
         TabIndex        =   16
         Top             =   360
         Width           =   735
      End
      Begin VB.Label Label4 
         Caption         =   "Degrees"
         Height          =   255
         Left            =   960
         TabIndex        =   17
         Top             =   360
         Width           =   735
      End
   End
   Begin VB.Frame Frame4 
      Caption         =   "Autosave"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   855
      Left            =   120
      TabIndex        =   9
      Top             =   3720
      Width           =   5775
      Begin VB.ComboBox AutoTime 
         Height          =   315
         Left            =   1320
         TabIndex        =   11
         Text            =   "10"
         Top             =   360
         Width           =   855
      End
      Begin VB.Label Label3 
         Caption         =   "minutes."
         Height          =   255
         Left            =   2280
         TabIndex        =   12
         Top             =   360
         Width           =   855
      End
      Begin VB.Label Label2 
         Caption         =   "Autosave every"
         Height          =   255
         Left            =   120
         TabIndex        =   10
         Top             =   360
         Width           =   1215
      End
   End
   Begin VB.CommandButton Cancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   5040
      TabIndex        =   8
      Top             =   5640
      Width           =   855
   End
   Begin VB.CommandButton Ok 
      Caption         =   "&Ok"
      Default         =   -1  'True
      Height          =   375
      Left            =   120
      TabIndex        =   7
      Top             =   5640
      Width           =   855
   End
   Begin VB.Frame Frame3 
      Caption         =   "Startup Macro"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   735
      Left            =   120
      TabIndex        =   5
      Top             =   120
      Width           =   3855
      Begin VB.CommandButton NoStartupMac 
         Caption         =   "X"
         Height          =   255
         Left            =   3480
         TabIndex        =   14
         Top             =   360
         Width           =   255
      End
      Begin VB.CommandButton StartupMacPick 
         Caption         =   "..."
         Height          =   255
         Left            =   3120
         TabIndex        =   6
         Top             =   360
         Width           =   255
      End
      Begin VB.Label InitialMacroName 
         Caption         =   "(none)"
         Height          =   255
         Left            =   120
         TabIndex        =   13
         Top             =   360
         Width           =   3015
      End
   End
   Begin VB.Frame Frame2 
      Caption         =   "Initial files to load"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   2655
      Left            =   120
      TabIndex        =   0
      Top             =   960
      Width           =   5775
      Begin VB.CommandButton RemoveFile 
         Caption         =   "&Remove"
         Height          =   375
         Left            =   4800
         TabIndex        =   3
         Top             =   840
         Width           =   855
      End
      Begin VB.CommandButton AddFile 
         Caption         =   "&Add"
         Height          =   375
         Left            =   4800
         TabIndex        =   2
         Top             =   360
         Width           =   855
      End
      Begin VB.ListBox InitialFiles 
         Height          =   1620
         Left            =   120
         TabIndex        =   1
         Top             =   360
         Width           =   4575
      End
      Begin VB.Label Label1 
         Caption         =   "Supported file types are: UTX (Textures), UAX (Audio), UMX (Meshes), UNR (Unreal map), UCX (Class library)."
         Height          =   495
         Left            =   120
         TabIndex        =   4
         Top             =   2040
         Width           =   5055
      End
   End
End
Attribute VB_Name = "frmPreferences"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim Starting As Boolean

Private Sub AddFile_Click()
    '
    ' Dialog for "Add Resource":
    '
    frmDialogs.AddFile.filename = ""
    '
    On Error GoTo Skip
    Ed.Server.Disable
    frmDialogs.AddFile.ShowOpen 'Modal File-Open Box
    '
    Call UpdateDialog(frmDialogs.AddFile)
    If (frmDialogs.AddFile.filename <> "") Then
        InitialFiles.AddItem frmDialogs.AddFile.filename
    End If
Skip: Ed.Server.Enable
End Sub

Private Sub Cancel_Click()
    Unload Me
End Sub

Private Sub FOV_Click()
    If Not Starting Then
        Ed.FOV = Val(FOV.Text)
    End If
End Sub

Private Sub NoStartupMac_Click()
    InitialMacroName.Caption = "(none)"
End Sub

Private Sub Ok_Click()
    Dim i As Integer
    '
    Ed.AutoSaveTime = Val(AutoTime.Text) ' May be 0
    '
    Ed.InitialMacro = InitialMacroName.Caption
    If InitialMacroName.Caption = "(none)" Then Ed.InitialMacro = ""
    '
    Ed.InitialFiles = ""
    For i = 0 To InitialFiles.ListCount - 1
        Ed.InitialFiles = Ed.InitialFiles & InitialFiles.List(i)
        If i < InitialFiles.ListCount Then Ed.InitialFiles = Ed.InitialFiles & " "
    Next i
    '
    Ed.GodMode = GodMode.Value
    '
    Ed.SaveProfile
    Unload Me
    '
    Call frmMain.PreferencesChange
    Call frmMain.ResizeAll(True)
End Sub

Private Sub Form_Load()
    Dim S As String, T As String
    Call Ed.MakeFormFit(Me)
    '
    Starting = True
    '
    S = Ed.InitialFiles
    Do
        If InStr(S, " ") Then
            T = Trim(Left(S, InStr(S, " ") - 1))
            S = Trim(Mid(S, InStr(S, " ") + 1))
        Else
            T = S
            S = ""
        End If
        If T <> "" Then InitialFiles.AddItem (T)
    Loop Until T = ""
    '
    InitialMacroName.Caption = Ed.InitialMacro
    If Ed.InitialMacro = "" Then InitialMacroName.Caption = "(none)"
    '
    AutoTime.AddItem "(Never)"
    AutoTime.AddItem "5"
    AutoTime.AddItem "10"
    AutoTime.AddItem "20"
    AutoTime.AddItem "30"
    AutoTime.AddItem "60"
    '
    FOV.AddItem "95"
    FOV.AddItem "60"
    If Ed.FOV = 95 Then
        FOV.ListIndex = 0
    Else
        FOV.ListIndex = 1
    End If
    '
    GodMode.Value = Ed.GodMode
    '
    If Ed.AutoSaveTime = 0 Then
        AutoTime.Text = "(Never)"
    Else
        AutoTime.Text = Str(Ed.AutoSaveTime)
    End If
    '
    Starting = False
    '
End Sub

Private Sub RemoveFile_Click()
    If InitialFiles.List(InitialFiles.ListIndex) <> "" Then
        InitialFiles.RemoveItem (InitialFiles.ListIndex)
    End If
End Sub

Private Sub StartupMacPick_Click()
    '
    ' Dialog for "Pick Macro":
    '
    frmDialogs.MacroPick.filename = InitialMacroName.Caption
    If InitialMacroName.Caption = "" Then frmDialogs.MacroPick.filename = ""
    '
    On Error GoTo Skip
    Ed.Server.Disable
    frmDialogs.MacroPick.ShowOpen 'Modal File-Open Box
    '
    Call UpdateDialog(frmDialogs.MacroPick)
    If (frmDialogs.MacroPick.filename <> "") Then
        InitialMacroName.Caption = frmDialogs.MacroPick.filename
    Else
        InitialMacroName.Caption = "(none)"
    End If
Skip:
    Ed.Server.Enable
End Sub
