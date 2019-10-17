VERSION 4.00
Begin VB.Form frmSoundImportDlg 
   Caption         =   "Import a sound"
   ClientHeight    =   2145
   ClientLeft      =   6285
   ClientTop       =   10755
   ClientWidth     =   4935
   Height          =   2550
   Icon            =   "ImprtSnd.frx":0000
   Left            =   6225
   LinkTopic       =   "Form2"
   ScaleHeight     =   2145
   ScaleWidth      =   4935
   Top             =   10410
   Width           =   5055
   Begin VB.CommandButton Cancel 
      BackColor       =   &H00C0C0C0&
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   3660
      TabIndex        =   10
      Top             =   1740
      Width           =   1095
   End
   Begin VB.CommandButton OK 
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Ok"
      Default         =   -1  'True
      Height          =   375
      Left            =   0
      TabIndex        =   9
      Top             =   1740
      Width           =   1095
   End
   Begin VB.CommandButton Command1 
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Skip"
      Height          =   375
      Left            =   2520
      TabIndex        =   8
      Top             =   1740
      Width           =   1095
   End
   Begin VB.Frame Frame1 
      Caption         =   "Sound name"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   1515
      Left            =   60
      TabIndex        =   1
      Top             =   60
      Width           =   4755
      Begin VB.TextBox SoundFamily 
         BackColor       =   &H00FFFFFF&
         Height          =   285
         Left            =   2040
         MaxLength       =   15
         TabIndex        =   3
         Text            =   "General"
         Top             =   1080
         Width           =   2415
      End
      Begin VB.TextBox SoundName 
         BackColor       =   &H00FFFFFF&
         Height          =   285
         Left            =   2040
         MaxLength       =   15
         TabIndex        =   2
         Text            =   "???"
         Top             =   660
         Width           =   2415
      End
      Begin VB.Label Label4 
         Alignment       =   1  'Right Justify
         Caption         =   "Sound Family:"
         Height          =   255
         Left            =   480
         TabIndex        =   7
         Top             =   1080
         Width           =   1455
      End
      Begin VB.Label Label3 
         Alignment       =   1  'Right Justify
         Caption         =   "Sound Name:"
         Height          =   255
         Left            =   600
         TabIndex        =   6
         Top             =   660
         Width           =   1335
      End
      Begin VB.Label Fname 
         Caption         =   "Fname"
         Height          =   255
         Left            =   960
         TabIndex        =   5
         Top             =   360
         Width           =   3495
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         Caption         =   "File:"
         Height          =   255
         Left            =   120
         TabIndex        =   4
         Top             =   360
         Width           =   735
      End
   End
   Begin VB.CommandButton OkAll 
      Caption         =   "Ok to &All"
      Height          =   375
      Left            =   1200
      TabIndex        =   0
      Top             =   1740
      Width           =   1215
   End
End
Attribute VB_Name = "frmSoundImportDlg"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim ThisFname
Dim AutoOk As Integer

Private Sub Cancel_Click()
    GlobalAbortedModal = 1
    AutoOk = 0
    Hide
End Sub


Private Sub Form_Activate()
    DoNext
End Sub

Private Sub Form_Load()
    AutoOk = 0
    Call Ed.MakeFormFit(Me)
End Sub

Public Sub DoNext()
    Fname = GrabFname(GString)
    If frmSoundImportDlg.Fname = "" Then
        GlobalAbortedModal = 0
        AutoOk = 0
        Unload Me ' !!!!!!!!!WAS HIDE ME -- A BUG!!!!!!!!!!!!!!
    End If
    '
    frmSoundImportDlg.SoundName = GetFileNameOnly(Fname)
    '
    If AutoOk = 1 Then Ok_Click
End Sub

Private Sub Ok_Click()
    Dim tmpFname As String
    Dim tmpName As String
    Dim tmpFamily As String
    
    ' Get name of file that was chosen by user
    tmpFname = frmSoundImportDlg.Fname
    
    tmpName = Trim(frmSoundImportDlg.SoundName)
    tmpFamily = Trim(frmSoundImportDlg.SoundFamily)
    
    GlobalAbortedModal = 0
    
    If Len(tmpName) = 0 Or Len(tmpName) > 15 Or Len(tmpFamily) = 0 Or Len(tmpFamily) > 15 Then
        MsgBox "Sound name and family name must be given, and must be 1-31 characters each", 16
    Else
        ' Tell the engine to import the specified sound
        Ed.Server.Exec "AUDIO IMPORT FILE=" & Quotes(tmpFname) & _
            " NAME=" & Quotes(tmpName) & _
            " FAMILY=" & Quotes(tmpFamily)
        DoNext
    End If
    '
    GString = SoundFamily.Text
    '
End Sub


Private Sub OkAll_Click()
    AutoOk = 1
    Ok_Click
End Sub


