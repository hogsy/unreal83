VERSION 4.00
Begin VB.Form frmRotGrid 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Rotation Grid Settings"
   ClientHeight    =   2595
   ClientLeft      =   3525
   ClientTop       =   2505
   ClientWidth     =   4350
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
   Height          =   2955
   HelpContextID   =   112
   Icon            =   "GridRot.frx":0000
   Left            =   3465
   LinkTopic       =   "Form8"
   MaxButton       =   0   'False
   ScaleHeight     =   2595
   ScaleWidth      =   4350
   ShowInTaskbar   =   0   'False
   Top             =   2205
   Width           =   4470
   Begin VB.Frame Frame1 
      Caption         =   "Size"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   2055
      Left            =   120
      TabIndex        =   1
      Top             =   120
      Width           =   4095
      Begin VB.OptionButton G1 
         Caption         =   "1   (1/256 circle)"
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
         Top             =   360
         Width           =   1935
      End
      Begin VB.OptionButton G2 
         Caption         =   "2   (1/128 circle)"
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
         Width           =   1935
      End
      Begin VB.OptionButton G4 
         Caption         =   "4   (1/64 circle)"
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
         Top             =   840
         Value           =   -1  'True
         Width           =   1935
      End
      Begin VB.OptionButton G8 
         Caption         =   "8   (1/32 circle)"
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
         TabIndex        =   12
         Top             =   360
         Width           =   1695
      End
      Begin VB.OptionButton G32 
         Caption         =   "32 (1/8 circle)"
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
         TabIndex        =   11
         Top             =   840
         Width           =   1575
      End
      Begin VB.OptionButton G16 
         Caption         =   "16 (1/16 circle)"
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
         TabIndex        =   10
         Top             =   600
         Width           =   1695
      End
      Begin VB.TextBox Roll 
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
         Left            =   3360
         TabIndex        =   5
         Text            =   "10"
         Top             =   1650
         Width           =   615
      End
      Begin VB.TextBox Yaw 
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
         Left            =   2040
         TabIndex        =   4
         Text            =   "10"
         Top             =   1650
         Width           =   615
      End
      Begin VB.TextBox Pitch 
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
         Left            =   720
         TabIndex        =   3
         Text            =   "10"
         Top             =   1650
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
         Left            =   120
         TabIndex        =   2
         Top             =   1320
         Width           =   1455
      End
      Begin VB.Label Label4 
         Alignment       =   1  'Right Justify
         Caption         =   "Roll"
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
         Left            =   2805
         TabIndex        =   8
         Top             =   1695
         Width           =   495
      End
      Begin VB.Label Label3 
         Alignment       =   1  'Right Justify
         Caption         =   "Yaw"
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
         Left            =   1485
         TabIndex        =   9
         Top             =   1695
         Width           =   495
      End
      Begin VB.Label Label2 
         Alignment       =   1  'Right Justify
         Caption         =   "Pitch"
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
         Left            =   150
         TabIndex        =   7
         Top             =   1695
         Width           =   495
      End
      Begin VB.Line Line1 
         BorderColor     =   &H00000000&
         X1              =   240
         X2              =   3960
         Y1              =   1200
         Y2              =   1200
      End
   End
   Begin VB.CommandButton Command1 
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
      Height          =   255
      Left            =   3240
      TabIndex        =   0
      Top             =   2280
      Width           =   975
   End
   Begin VB.Label Label1 
      Caption         =   "There are 256 degrees in a circle."
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
      Top             =   2280
      Width           =   3015
   End
End
Attribute VB_Name = "frmRotGrid"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub Command1_Click()
    frmRotGrid.Hide
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "RotGridSettings", TOP_PANEL)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub G1_Click()
    Ed.Server.Exec "MAP ROTGRID PITCH=1 YAW=1 ROLL=1"
End Sub

Private Sub G16_Click()
    Ed.Server.Exec "MAP ROTGRID PITCH=16 YAW=16 ROLL=16"
End Sub

Private Sub G2_Click()
    Ed.Server.Exec "MAP ROTGRID PITCH=2 YAW=2 ROLL=2"
End Sub

Private Sub G32_Click()
    Ed.Server.Exec "MAP ROTGRID PITCH=32 YAW=32 ROLL=32"
End Sub

Private Sub G4_Click()
    Ed.Server.Exec "MAP ROTGRID PITCH=4 YAW=4 ROLL=4"
End Sub

Private Sub G8_Click()
    Ed.Server.Exec "MAP ROTGRID PITCH=8 YAW=8 ROLL=8"
End Sub

Private Sub Option6_Click()
    SendIrregularSize
End Sub

Private Sub Pitch_LostFocus()
    SendIrregularSize
End Sub

Private Sub Roll_LostFocus()
    SendIrregularSize
End Sub

Private Sub SendIrregularSize()
    Dim P, Y, R As Integer
    '
    P = Val(Pitch.Text)
    Y = Val(Yaw.Text)
    R = Val(Roll.Text)
    '
    If (P <> 0) Or (Y <> 0) Or (R <> 0) Then
        Ed.Server.Exec "MAP ROTGRID PITCH=" & Trim(Str(P)) & " YAW=" & Trim(Str(Y)) & " ROLL=" & Trim(Str(R))
    End If
    '
End Sub

Private Sub Yaw_LostFocus()
    SendIrregularSize
End Sub

