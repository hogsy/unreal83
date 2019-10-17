VERSION 4.00
Begin VB.Form frmTexProp 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Texture Properties"
   ClientHeight    =   4590
   ClientLeft      =   3630
   ClientTop       =   3945
   ClientWidth     =   5745
   ForeColor       =   &H80000008&
   Height          =   4950
   HelpContextID   =   123
   Icon            =   "TexProp.frx":0000
   Left            =   3570
   LinkTopic       =   "Form8"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4590
   ScaleWidth      =   5745
   ShowInTaskbar   =   0   'False
   Top             =   3645
   Width           =   5865
   Begin VB.PictureBox TexView 
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   700
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   3915
      Left            =   180
      ScaleHeight     =   257
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   257
      TabIndex        =   1
      Top             =   480
      Width           =   3915
   End
   Begin TabDlg.SSTab SSTab1 
      Height          =   4455
      Left            =   60
      TabIndex        =   0
      Top             =   60
      Width           =   5595
      _Version        =   65536
      _ExtentX        =   9869
      _ExtentY        =   7858
      _StockProps     =   15
      Caption         =   "Picture"
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   9
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      TabsPerRow      =   4
      Tab             =   0
      TabOrientation  =   0
      Tabs            =   4
      Style           =   1
      TabMaxWidth     =   0
      TabHeight       =   529
      TabCaption(0)   =   "Picture"
      Tab(0).ControlCount=   6
      Tab(0).ControlEnabled=   -1  'True
      Tab(0).Control(0)=   "Label4"
      Tab(0).Control(1)=   "TextureName"
      Tab(0).Control(2)=   "Label5"
      Tab(0).Control(3)=   "TextureFamily"
      Tab(0).Control(4)=   "Label7"
      Tab(0).Control(5)=   "TextureSize"
      TabCaption(1)   =   "Properties"
      Tab(1).ControlCount=   0
      Tab(1).ControlEnabled=   0   'False
      TabCaption(2)   =   "Polygon"
      Tab(2).ControlCount=   1
      Tab(2).ControlEnabled=   0   'False
      Tab(2).Control(0)=   "PolyFlagsHolder"
      TabCaption(3)   =   "Fire Engine"
      Tab(3).ControlCount=   10
      Tab(3).ControlEnabled=   0   'False
      Tab(3).Control(0)=   "Command2"
      Tab(3).Control(1)=   "SparkType"
      Tab(3).Control(2)=   "Command1"
      Tab(3).Control(3)=   "FractalType"
      Tab(3).Control(4)=   "Label3"
      Tab(3).Control(5)=   "Slider2"
      Tab(3).Control(6)=   "Label20"
      Tab(3).Control(7)=   "Label2"
      Tab(3).Control(8)=   "Slider1"
      Tab(3).Control(9)=   "Label1"
      Begin VB.PictureBox PolyFlagsHolder 
         Height          =   1935
         Left            =   -74760
         ScaleHeight     =   1875
         ScaleWidth      =   5055
         TabIndex        =   12
         Top             =   1380
         Width           =   5115
      End
      Begin VB.CommandButton Command2 
         Caption         =   "&New"
         Height          =   375
         Left            =   -70200
         TabIndex        =   9
         Top             =   3960
         Width           =   675
      End
      Begin VB.ComboBox SparkType 
         Height          =   315
         Left            =   -70920
         Style           =   2  'Dropdown List
         TabIndex        =   7
         Top             =   2040
         Width           =   1395
      End
      Begin VB.CommandButton Command1 
         Caption         =   "&Clear"
         Height          =   375
         Left            =   -70920
         TabIndex        =   4
         Top             =   3960
         Width           =   675
      End
      Begin VB.ComboBox FractalType 
         Height          =   315
         Left            =   -70920
         Style           =   2  'Dropdown List
         TabIndex        =   2
         Top             =   720
         Width           =   1395
      End
      Begin VB.Label TextureSize 
         Alignment       =   2  'Center
         Caption         =   "N x M"
         Height          =   255
         Left            =   4140
         TabIndex        =   18
         Top             =   1980
         Width           =   1335
      End
      Begin VB.Label Label7 
         Caption         =   "Size:"
         Height          =   255
         Left            =   4140
         TabIndex        =   17
         Top             =   1740
         Width           =   1335
      End
      Begin VB.Label TextureFamily 
         Alignment       =   2  'Center
         Caption         =   "N x M"
         Height          =   255
         Left            =   4140
         TabIndex        =   16
         Top             =   1320
         Width           =   1335
      End
      Begin VB.Label Label5 
         Caption         =   "Family:"
         Height          =   255
         Left            =   4140
         TabIndex        =   15
         Top             =   1080
         Width           =   1335
      End
      Begin VB.Label TextureName 
         Alignment       =   2  'Center
         Caption         =   "N x M"
         Height          =   255
         Left            =   4140
         TabIndex        =   14
         Top             =   660
         Width           =   1335
      End
      Begin VB.Label Label4 
         Caption         =   "Name:"
         Height          =   255
         Left            =   4140
         TabIndex        =   13
         Top             =   420
         Width           =   1335
      End
      Begin VB.Label Label3 
         Caption         =   "Jitter:"
         Height          =   255
         Left            =   -70800
         TabIndex        =   11
         Top             =   2460
         Width           =   855
      End
      Begin ComctlLib.Slider Slider2 
         Height          =   255
         Left            =   -70860
         TabIndex        =   10
         Top             =   2700
         Width           =   1215
         _Version        =   65536
         _ExtentX        =   2143
         _ExtentY        =   450
         _StockProps     =   64
      End
      Begin VB.Label Label20 
         Caption         =   "Spark Type:"
         Height          =   255
         Left            =   -70920
         TabIndex        =   8
         Top             =   1800
         Width           =   1335
      End
      Begin VB.Label Label2 
         Caption         =   "Heat:"
         Height          =   255
         Left            =   -70740
         TabIndex        =   6
         Top             =   1140
         Width           =   855
      End
      Begin ComctlLib.Slider Slider1 
         Height          =   255
         Left            =   -70800
         TabIndex        =   5
         Top             =   1380
         Width           =   1215
         _Version        =   65536
         _ExtentX        =   2143
         _ExtentY        =   450
         _StockProps     =   64
      End
      Begin VB.Label Label1 
         Caption         =   "Fractal Type:"
         Height          =   255
         Left            =   -70920
         TabIndex        =   3
         Top             =   480
         Width           =   1335
      End
   End
End
Attribute VB_Name = "frmTexProp"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim TexName As String
Dim PolyFlagsForm As Form

Private Sub Close_Click()
    Unload Me
End Sub

Sub OpenTexCam()
    If TexName = "" Then
        Ed.Server.Exec "CAMERA CLOSE NAME=TexPropCam"
    Else
        Ed.Server.Exec "CAMERA OPEN " & _
            " TEXTURE=" & Quotes(TexName) & _
            " NAME=TexPropCam X=0 Y=0 XR=136 YR=144 " & _
            " REN=" & Trim(Str(REN_TEXVIEW)) & _
            " FLAGS=" & Trim(Str(SHOW_AS_CHILD + SHOW_NOCAPTURE + SHOW_NOBUTTONS + SHOW_REALTIME)) & _
            " HWND=" & Trim(Str(TexView.hwnd))
    End If
End Sub

Sub CloseTexCam()
    Ed.Server.Exec "CAMERA CLOSE NAME=TexPropCam"
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "TextureProperties", TOP_PANEL)
    '
    Set PolyFlagsForm = New frmPolyFlags
    Call PolyFlagsForm.SetFormParent(Me, PolyFlagsHolder)
    PolyFlagsForm.Show
    '
    ' Init fire engine combos:
    '
    FractalType.AddItem "Smoke & Fire"
    FractalType.AddItem "Water"
    '
    SparkType.AddItem "SimpleFlame"
    SparkType.AddItem "JitterFlame"
    SparkType.AddItem "SliceFlame"
    SparkType.AddItem "PulseFlame"
    SparkType.AddItem "Fireball"
    SparkType.AddItem "Streamer"
    SparkType.AddItem "Aladdin"
    SparkType.AddItem "Popper"
    SparkType.AddItem "Spermatazoa"
    SparkType.AddItem "Popper"
    SparkType.AddItem "Popper"
    '
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Unload PolyFlagsForm
    Set PolyFlagsForm = Nothing
    '
    Call Ed.EndOnTop(Me)
    Ed.Server.Exec "CAMERA CLOSE NAME=TexPropCam"
End Sub

Public Sub SetTexture(Name As String)
    TexName = Name
    Caption = "Texture Properties for " & TexName
    OpenTexCam
End Sub

Private Sub Realtime_Click()
    OpenTexCam
End Sub

Private Sub Sphere_Click()
    OpenTexCam
End Sub

Public Sub PolyFlagsUpdate(NewOnFlags As Long, NewOffFlags As Long)
    'Ed.Server.Exec "POLY SET SETFLAGS=" & Trim(Str(NewOnFlags)) & " CLEARFLAGS=" & Trim(Str(NewOffFlags))
End Sub
