VERSION 4.00
Begin VB.Form frmGrid 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Grid Settings"
   ClientHeight    =   1995
   ClientLeft      =   1305
   ClientTop       =   3690
   ClientWidth     =   6855
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
   Height          =   2355
   HelpContextID   =   111
   Icon            =   "Grid.frx":0000
   Left            =   1245
   LinkTopic       =   "Form8"
   MaxButton       =   0   'False
   ScaleHeight     =   1995
   ScaleWidth      =   6855
   ShowInTaskbar   =   0   'False
   Top             =   3390
   Width           =   6975
   Begin VB.CommandButton Command3 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
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
      Height          =   375
      Left            =   4200
      TabIndex        =   18
      Top             =   1560
      Width           =   855
   End
   Begin VB.Frame Frame1 
      Caption         =   "Grid Size"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   1935
      Left            =   120
      TabIndex        =   1
      Top             =   0
      Width           =   3975
      Begin VB.OptionButton G128 
         Caption         =   "128 inches"
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
         Left            =   2640
         TabIndex        =   17
         Top             =   600
         Width           =   1095
      End
      Begin VB.OptionButton G16 
         Caption         =   "16 inches"
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
         Left            =   1320
         TabIndex        =   16
         Top             =   600
         Value           =   -1  'True
         Width           =   1095
      End
      Begin VB.OptionButton G4 
         Caption         =   "4 inches"
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
         Top             =   840
         Width           =   1095
      End
      Begin VB.OptionButton G2 
         Caption         =   "2 inches"
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
         Top             =   600
         Width           =   1095
      End
      Begin VB.TextBox Size 
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
         Height          =   285
         Left            =   120
         TabIndex        =   4
         Text            =   "10"
         Top             =   1560
         Width           =   495
      End
      Begin VB.TextBox SX 
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
         Height          =   285
         Left            =   1800
         TabIndex        =   5
         Text            =   "16"
         Top             =   1560
         Width           =   615
      End
      Begin VB.TextBox SY 
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
         Height          =   285
         Left            =   2520
         TabIndex        =   3
         Text            =   "16"
         Top             =   1560
         Width           =   615
      End
      Begin VB.TextBox SZ 
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
         Height          =   285
         Left            =   3240
         TabIndex        =   2
         Text            =   "2"
         Top             =   1560
         Width           =   615
      End
      Begin VB.OptionButton Option6 
         Caption         =   "Irregular Size"
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
         TabIndex        =   12
         Top             =   1320
         Width           =   1455
      End
      Begin VB.OptionButton Option2 
         Caption         =   "Custom  Size"
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
         Top             =   1320
         Width           =   1455
      End
      Begin VB.OptionButton G64 
         Caption         =   "64 inches"
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
         Left            =   2640
         TabIndex        =   10
         Top             =   360
         Width           =   1215
      End
      Begin VB.OptionButton G32 
         Caption         =   "32 inches"
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
         Left            =   1320
         TabIndex        =   9
         Top             =   840
         Width           =   1095
      End
      Begin VB.OptionButton G8 
         Caption         =   "8 inches"
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
         Left            =   1320
         TabIndex        =   8
         Top             =   360
         Width           =   1095
      End
      Begin VB.OptionButton G256 
         Caption         =   "256 inches"
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
         Left            =   2640
         TabIndex        =   7
         Top             =   840
         Width           =   1215
      End
      Begin VB.OptionButton G1 
         Caption         =   "1 inch"
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
         TabIndex        =   6
         Top             =   360
         Width           =   1095
      End
      Begin VB.Line Line1 
         BorderColor     =   &H00000000&
         X1              =   240
         X2              =   3720
         Y1              =   1200
         Y2              =   1200
      End
   End
   Begin VB.Frame Frame2 
      Caption         =   "Vertex snap dist"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   1455
      Left            =   4200
      TabIndex        =   19
      Top             =   0
      Width           =   2535
      Begin VB.TextBox SnapVal 
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
         Height          =   285
         Left            =   1200
         TabIndex        =   22
         Text            =   "2"
         Top             =   1080
         Width           =   735
      End
      Begin VB.OptionButton Option7 
         Caption         =   "100 (10 ft)"
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
         Left            =   1320
         TabIndex        =   27
         Top             =   840
         Width           =   1120
      End
      Begin VB.OptionButton Option5 
         Caption         =   "50   (5 ft)"
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
         Left            =   1320
         TabIndex        =   26
         Top             =   600
         Width           =   1095
      End
      Begin VB.OptionButton Option4 
         Caption         =   "2    (2 in)"
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
         TabIndex        =   25
         Top             =   360
         Width           =   1095
      End
      Begin VB.OptionButton Option3 
         Caption         =   "20   (2 ft)"
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
         Left            =   1320
         TabIndex        =   24
         Top             =   360
         Width           =   1095
      End
      Begin VB.OptionButton Option1 
         Caption         =   "5    (5 in)"
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
         TabIndex        =   23
         Top             =   600
         Width           =   1095
      End
      Begin VB.OptionButton SnapCust 
         Caption         =   "Custom:"
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
         TabIndex        =   21
         Top             =   1080
         Width           =   975
      End
      Begin VB.OptionButton Snap10 
         Caption         =   "10  (1 ft)"
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
         TabIndex        =   20
         Top             =   840
         Value           =   -1  'True
         Width           =   1095
      End
   End
   Begin VB.CheckBox GridShow3D 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "&Show Grid in 3D views"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   120
      TabIndex        =   13
      Top             =   3360
      Visible         =   0   'False
      Width           =   2295
   End
   Begin VB.CommandButton Command1 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Cancel          =   -1  'True
      Caption         =   "&Close"
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
      Left            =   5880
      TabIndex        =   0
      Top             =   1560
      Width           =   855
   End
End
Attribute VB_Name = "frmGrid"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub BaseAbsolute_Click()
    Ed.Server.Exec "MAP GRID BASE=ABSOLUTE"
End Sub

Private Sub BaseCustom_Click()
    Ed.Server.Exec "MAP GRID BASE=CUSTOM"
End Sub

Private Sub BaseRelative_Click()
    Ed.Server.Exec "MAP GRID BASE=RELATIVE"
End Sub

Private Sub Command1_Click()
    frmGrid.Hide
End Sub

Private Sub Command3_Click()
    SendKeys "{F1}"
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "GridSettings", TOP_PANEL)
End Sub

Private Sub G1_Click()
    Call Ed.SetGridSize(1, 1, 1)
End Sub

Private Sub G2_Click()
    Call Ed.SetGridSize(2, 2, 2)
End Sub

Private Sub G4_Click()
    Call Ed.SetGridSize(4, 4, 4)
End Sub

Private Sub G8_Click()
    Call Ed.SetGridSize(8, 8, 8)
End Sub

Private Sub G16_Click()
    Call Ed.SetGridSize(16, 16, 16)
End Sub

Private Sub G32_Click()
    Call Ed.SetGridSize(32, 32, 32)
End Sub

Private Sub G64_Click()
    Call Ed.SetGridSize(64, 64, 64)
End Sub

Private Sub G128_Click()
    Call Ed.SetGridSize(128, 128, 128)
End Sub

Private Sub G256_Click()
    Call Ed.SetGridSize(256, 256, 256)
End Sub

Private Sub Option1_Click()
    Ed.Server.Exec "MODE SNAPDIST=5"
End Sub

Private Sub Option2_Click()
    Dim S As Integer
    S = Val(Size.Text)
    If (S <> 0) Then
        Call Ed.SetGridSize(S, S, S)
    End If
End Sub

Private Sub Option3_Click()
    Ed.Server.Exec "MODE SNAPDIST=20"
End Sub

Private Sub Option6_Click()
    Dim X As Integer
    Dim Y As Integer
    Dim Z As Integer
    '
    X = Val(SX.Text)
    Y = Val(SY.Text)
    Z = Val(SZ.Text)
    '
    If (X <> 0) And (Y <> 0) And (Z <> 0) Then
        Call Ed.SetGridSize(X, Y, Z)
    End If
    '
End Sub

Private Sub SetGridBase_Click()
    Ed.Server.Exec "MAP SETGRIDBASE"
End Sub

Private Sub Size_LostFocus()
    Option2_Click
End Sub

Private Sub Snap10_Click()
    Ed.Server.Exec "MODE SNAPDIST=10"
End Sub

Private Sub SnapCust_Click()
    Dim S As Integer
    '
    S = Val(SnapVal.Text)
    If (S <> 0) Then
        Ed.Server.Exec "MODE SNAPDIST=" & Trim(Str(S))
    End If
    '
End Sub

Private Sub SnapToBrushOrigin_Click()
    Ed.Server.Exec "MODE SNAPTOPIVOT=OFF"
End Sub

Private Sub SnapToBrushPivot_Click()
    Ed.Server.Exec "MODE SNAPTOPIVOT=ON"
End Sub

Private Sub SnapVal_Change()
    Dim S As Integer
    '
    S = Val(SnapVal.Text)
    If (S <> 0) Then
        Ed.Server.Exec "MODE SNAPDIST=" & Trim(Str(S))
    End If
    '
End Sub

Private Sub SX_LostFocus()
    Option6_Click
End Sub

Private Sub SY_LostFocus()
    Option6_Click
End Sub

Private Sub SZ_LostFocus()
    Option6_Click
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub


