VERSION 4.00
Begin VB.Form frmLevel 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Unreal Level"
   ClientHeight    =   4035
   ClientLeft      =   2490
   ClientTop       =   6330
   ClientWidth     =   7455
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
   Height          =   4395
   HelpContextID   =   117
   Icon            =   "Level.frx":0000
   Left            =   2430
   LinkTopic       =   "Form1"
   ScaleHeight     =   4035
   ScaleWidth      =   7455
   ShowInTaskbar   =   0   'False
   Top             =   6030
   Width           =   7575
   Begin VB.CommandButton Command6 
      Caption         =   "&View Links..."
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
      Left            =   1200
      TabIndex        =   24
      Top             =   3600
      Width           =   1275
   End
   Begin VB.CommandButton Cancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
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
      Left            =   6420
      TabIndex        =   1
      Top             =   3600
      Width           =   975
   End
   Begin VB.CommandButton Ok 
      Caption         =   "&Ok"
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
      Left            =   120
      TabIndex        =   0
      Top             =   3600
      Width           =   975
   End
   Begin TabDlg.SSTab SSTab2 
      Height          =   3375
      Left            =   60
      TabIndex        =   2
      Top             =   120
      Width           =   7335
      _Version        =   65536
      _ExtentX        =   12938
      _ExtentY        =   5953
      _StockProps     =   15
      Caption         =   "Summary "
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      TabsPerRow      =   3
      Tab             =   0
      TabOrientation  =   0
      Tabs            =   3
      Style           =   1
      TabMaxWidth     =   0
      TabHeight       =   529
      TabCaption(0)   =   "Summary "
      Tab(0).ControlCount=   8
      Tab(0).ControlEnabled=   -1  'True
      Tab(0).Control(0)=   "Label3"
      Tab(0).Control(1)=   "Label4"
      Tab(0).Control(2)=   "Label6"
      Tab(0).Control(3)=   "Label8"
      Tab(0).Control(4)=   "Title"
      Tab(0).Control(5)=   "Project"
      Tab(0).Control(6)=   "Creator"
      Tab(0).Control(7)=   "Release"
      TabCaption(1)   =   "Properties "
      Tab(1).ControlCount=   2
      Tab(1).ControlEnabled=   0   'False
      Tab(1).Control(0)=   "Frame1"
      Tab(1).Control(1)=   "Frame3"
      TabCaption(2)   =   "Design Notes "
      Tab(2).ControlCount=   1
      Tab(2).ControlEnabled=   0   'False
      Tab(2).Control(0)=   "DesignNotes"
      Begin VB.TextBox Release 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   2160
         TabIndex        =   18
         Top             =   2040
         Width           =   3255
      End
      Begin VB.TextBox Creator 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   2160
         TabIndex        =   17
         Top             =   1200
         Width           =   3255
      End
      Begin VB.TextBox Project 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   2160
         TabIndex        =   15
         Top             =   1680
         Width           =   3255
      End
      Begin VB.Frame Frame3 
         Caption         =   "Backdrop Textures"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   1275
         Left            =   -71460
         TabIndex        =   10
         Top             =   540
         Width           =   3675
         Begin VB.CommandButton Command5 
            Caption         =   "Browse"
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
            TabIndex        =   23
            Top             =   900
            Width           =   975
         End
         Begin VB.CommandButton Command3 
            Caption         =   "<- Current"
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
            TabIndex        =   21
            Top             =   540
            Width           =   975
         End
         Begin VB.CommandButton Command4 
            Caption         =   "<- Current"
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
            TabIndex        =   22
            Top             =   300
            Width           =   975
         End
         Begin VB.Label Label7 
            Caption         =   "(default)"
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
            Left            =   900
            TabIndex        =   14
            Top             =   540
            Width           =   1575
         End
         Begin VB.Label Label5 
            Alignment       =   1  'Right Justify
            Caption         =   "Horizon:"
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
            TabIndex        =   13
            Top             =   540
            Width           =   675
         End
         Begin VB.Label Label2 
            Alignment       =   1  'Right Justify
            Caption         =   "Sky:"
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
            Top             =   300
            Width           =   675
         End
         Begin VB.Label Label1 
            Caption         =   "(default)"
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
            Left            =   900
            TabIndex        =   11
            Top             =   300
            Width           =   1575
         End
      End
      Begin VB.Frame Frame1 
         Caption         =   "Ambient Song"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   975
         Left            =   -74880
         TabIndex        =   8
         Top             =   540
         Width           =   3255
         Begin VB.CommandButton Command2 
            Caption         =   "<- Current"
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
            Left            =   2160
            TabIndex        =   20
            Top             =   300
            Width           =   975
         End
         Begin VB.CommandButton Command1 
            Caption         =   "Browse"
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
            Left            =   2160
            TabIndex        =   19
            Top             =   600
            Width           =   975
         End
         Begin VB.Label SongName 
            Caption         =   "(default)"
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
            Top             =   300
            Width           =   1935
         End
      End
      Begin VB.TextBox Title 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   2160
         TabIndex        =   4
         Top             =   840
         Width           =   3255
      End
      Begin VB.TextBox DesignNotes 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   2775
         Left            =   -74880
         MousePointer    =   3  'I-Beam
         MultiLine       =   -1  'True
         ScrollBars      =   2  'Vertical
         TabIndex        =   3
         Top             =   480
         Width           =   7095
      End
      Begin VB.Label Label8 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Project:"
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
         Top             =   1680
         Width           =   1215
      End
      Begin VB.Label Label6 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Release Status:"
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
         TabIndex        =   7
         Top             =   2040
         Width           =   1215
      End
      Begin VB.Label Label4 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Creator:"
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
         TabIndex        =   6
         Top             =   1200
         Width           =   1215
      End
      Begin VB.Label Label3 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Level Name:"
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
         Left            =   720
         TabIndex        =   5
         Top             =   840
         Width           =   1335
      End
   End
End
Attribute VB_Name = "frmLevel"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub Ok_Click()
    '
    Call Ed.Server.SetProp("Lev", "0", Title.Text)
    Call Ed.Server.SetProp("Lev", "1", Creator.Text)
    Call Ed.Server.SetProp("Lev", "2", Release.Text)
    Call Ed.Server.SetProp("Lev", "3", DesignNotes.Text)
    Call Ed.Server.SetProp("Lev", "4", Project.Text)
    '
    Unload Me
    '
End Sub

Private Sub Cancel_Click()
    '
    ' Unload form without sending updated info.
    '
    Unload Me
    '
End Sub

Private Sub Form_Load()
    '
    Call Ed.SetOnTop(Me, "LevelProperties", TOP_NORMAL)
    '
    Title.Text = Ed.Server.GetProp("Lev", "0")
    Creator.Text = Ed.Server.GetProp("Lev", "1")
    Release.Text = Ed.Server.GetProp("Lev", "2")
    DesignNotes.Text = Ed.Server.GetProp("Lev", "3")
    Project.Text = Ed.Server.GetProp("Lev", "4")
    ' 6 = encoded editor params
    '
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub


