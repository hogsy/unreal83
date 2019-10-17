VERSION 4.00
Begin VB.Form frmPolyFlags 
   BorderStyle     =   0  'None
   Caption         =   "frmPolyFlags"
   ClientHeight    =   1950
   ClientLeft      =   4230
   ClientTop       =   5670
   ClientWidth     =   5010
   Height          =   2355
   Icon            =   "PolyFlgs.frx":0000
   Left            =   4170
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1950
   ScaleWidth      =   5010
   ShowInTaskbar   =   0   'False
   Tag             =   "y"
   Top             =   5325
   Width           =   5130
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Dirty Shadows"
      Height          =   255
      Index           =   19
      Left            =   1080
      TabIndex        =   19
      Tag             =   "&h40000"
      Top             =   1680
      Width           =   1635
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Lo Shadow Detail"
      Height          =   255
      Index           =   24
      Left            =   1080
      TabIndex        =   25
      Tag             =   "&h8000"
      Top             =   1440
      Width           =   1635
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Hurt"
      Height          =   255
      Index           =   23
      Left            =   4080
      TabIndex        =   24
      Tag             =   "&H40"
      Top             =   0
      Width           =   915
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Special Lit"
      Height          =   255
      Index           =   22
      Left            =   2760
      TabIndex        =   23
      Tag             =   "&H100000"
      Top             =   960
      Width           =   1275
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Nonsolid"
      Enabled         =   0   'False
      ForeColor       =   &H00404040&
      Height          =   255
      Index           =   21
      Left            =   2760
      TabIndex        =   22
      Tag             =   "&H8"
      Top             =   1680
      Width           =   1275
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Semisolid"
      Enabled         =   0   'False
      ForeColor       =   &H00404040&
      Height          =   255
      Index           =   20
      Left            =   2760
      TabIndex        =   21
      Tag             =   "&H20"
      Top             =   1440
      Width           =   1275
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Portal"
      ForeColor       =   &H00000000&
      Height          =   255
      Index           =   18
      Left            =   2760
      TabIndex        =   20
      Tag             =   "&H4000000"
      Top             =   1200
      Width           =   1275
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "No Auto-Look"
      Height          =   255
      Index           =   17
      Left            =   1080
      TabIndex        =   18
      Tag             =   "&H8000000"
      Top             =   720
      Width           =   1635
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Hi Shadow Detail"
      Height          =   255
      Index           =   16
      Left            =   1080
      TabIndex        =   17
      Tag             =   "&H800000"
      Top             =   1200
      Width           =   1635
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Unlit"
      Height          =   255
      Index           =   15
      Left            =   60
      TabIndex        =   16
      Tag             =   "&H400000"
      Top             =   960
      Width           =   975
   End
   Begin VB.CommandButton ClearAll 
      Caption         =   "&Clear All"
      Height          =   255
      Left            =   60
      TabIndex        =   15
      Top             =   1680
      Width           =   915
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "High Ledge Block"
      Height          =   255
      Index           =   14
      Left            =   1080
      TabIndex        =   14
      Tag             =   "&H80000"
      Top             =   480
      Width           =   1635
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "V-Pan"
      Height          =   255
      Index           =   13
      Left            =   4080
      TabIndex        =   13
      Tag             =   "&H400"
      Top             =   480
      Width           =   975
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "U-Pan"
      Height          =   255
      Index           =   12
      Left            =   4080
      TabIndex        =   12
      Tag             =   "&H200"
      Top             =   240
      Width           =   915
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Glow"
      Height          =   255
      Index           =   11
      Left            =   60
      TabIndex        =   3
      Tag             =   "&H200000"
      Top             =   480
      Width           =   975
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "No Smooth"
      Height          =   255
      Index           =   10
      Left            =   2760
      TabIndex        =   11
      Tag             =   "&H800"
      Top             =   0
      Width           =   1275
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Invisible"
      Height          =   255
      Index           =   9
      Left            =   60
      TabIndex        =   10
      Tag             =   "&H1"
      Top             =   1200
      Width           =   975
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Ghost"
      Height          =   255
      Index           =   8
      Left            =   60
      TabIndex        =   2
      Tag             =   "&H4000"
      Top             =   240
      Width           =   975
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Transparent"
      Height          =   255
      Index           =   7
      Left            =   2760
      TabIndex        =   9
      Tag             =   "&H4"
      Top             =   240
      Width           =   1275
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Masked"
      Height          =   255
      Index           =   6
      Left            =   60
      TabIndex        =   1
      Tag             =   "&H2"
      Top             =   0
      Width           =   975
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "2-Sided"
      Height          =   255
      Index           =   5
      Left            =   60
      TabIndex        =   8
      Tag             =   "&H100"
      Top             =   1440
      Width           =   975
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Small Wavy"
      Height          =   255
      Index           =   4
      Left            =   2760
      TabIndex        =   7
      Tag             =   "&H2000"
      Top             =   720
      Width           =   1275
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Big Wavy"
      Height          =   255
      Index           =   3
      Left            =   2760
      TabIndex        =   6
      Tag             =   "&H1000"
      Top             =   480
      Width           =   1275
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Far Parallax"
      Height          =   255
      Index           =   2
      Left            =   1080
      TabIndex        =   5
      Tag             =   "&H20000"
      Top             =   960
      Width           =   1635
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Fake Backdrop"
      Height          =   255
      Index           =   1
      Left            =   1080
      TabIndex        =   4
      Tag             =   "&H80"
      Top             =   240
      Width           =   1635
   End
   Begin VB.CheckBox PolyFlag 
      Caption         =   "Environment map"
      Height          =   255
      Index           =   0
      Left            =   1080
      TabIndex        =   0
      Tag             =   "&H10"
      Top             =   0
      Width           =   1635
   End
End
Attribute VB_Name = "frmPolyFlags"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Const NumFlags = 25
Public HolderForm As Form
Dim Updating As Integer

'
' Public
'

Public Sub SetFormParent(ParentForm As Form, ParentControl As Control)
    Dim R As RECT
    '
    Call SetParent(Me.hwnd, ParentControl.hwnd)
    Call GetClientRect(ParentControl.hwnd, R)
    Call SetWindowPos(Me.hwnd, 0, 0, 0, R.Right - R.Left, R.Bottom - R.Top, 0)
    '
    Set HolderForm = ParentForm
End Sub

Public Sub SetFlagBits(FlagsOn As Long, FlagsOff As Long)
    Dim i As Integer, b As Long
    '
    Updating = 1
    For i = 0 To NumFlags - 1
        b = CLng(PolyFlag(i).Tag)
        '
        If (FlagsOn And b) = (FlagsOff And b) Then
            PolyFlag(i).Value = 2
        ElseIf ((FlagsOn And b) <> 0) Then
            PolyFlag(i).Value = 1
        Else
            PolyFlag(i).Value = 0
        End If
    Next i
    Updating = 0
End Sub

Private Sub ClearAll_Click()
    Dim i As Integer
    '
    Updating = 1
    For i = 0 To NumFlags - 1
         PolyFlag(i).Value = 0
    Next i
    Updating = 0
    PolyFlagsUpdate
End Sub

Private Sub PolyFlagsUpdate()
    Dim OnFlags As Long, OffFlags As Long
    Dim i As Integer
    '
    For i = 0 To NumFlags - 1
        If PolyFlag(i).Value = 1 Then
            OnFlags = OnFlags + CLng(PolyFlag(i).Tag)
        ElseIf PolyFlag(i).Value = 0 Then
            OffFlags = OffFlags + CLng(PolyFlag(i).Tag)
        End If
    Next i
    '
    Call HolderForm.PolyFlagsUpdate(OnFlags, OffFlags)
End Sub

'
' Private
'

Private Sub Form_Unload(Cancel As Integer)
    Set HolderForm = Nothing
End Sub

Private Sub PolyFlag_Click(index As Integer)
    If Updating = 0 Then
        PolyFlagsUpdate
    End If
End Sub
