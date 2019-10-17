VERSION 4.00
Begin VB.Form frmSaveClass 
   Caption         =   "Save Actor Classes"
   ClientHeight    =   2535
   ClientLeft      =   2820
   ClientTop       =   10140
   ClientWidth     =   5220
   Height          =   2895
   Icon            =   "SaveClas.frx":0000
   Left            =   2760
   LinkTopic       =   "Form1"
   ScaleHeight     =   2535
   ScaleWidth      =   5220
   ShowInTaskbar   =   0   'False
   Top             =   9840
   Width           =   5340
   Begin VB.Frame Frame2 
      Caption         =   "File Type"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   1275
      Left            =   120
      TabIndex        =   6
      Top             =   1200
      Width           =   3975
      Begin VB.OptionButton SaveH 
         Caption         =   "C++ Header file (.h)"
         Height          =   195
         Left            =   120
         TabIndex        =   9
         Top             =   900
         Width           =   2355
      End
      Begin VB.OptionButton SaveTCX 
         Caption         =   "Text Actor Classes (.tcx)"
         Height          =   255
         Left            =   120
         TabIndex        =   8
         Top             =   540
         Width           =   2355
      End
      Begin VB.OptionButton SaveUCX 
         Caption         =   "Unreal Actor Classes (.ucx)"
         Height          =   255
         Left            =   120
         TabIndex        =   7
         Top             =   300
         Value           =   -1  'True
         Width           =   2355
      End
   End
   Begin VB.CommandButton Cancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   4200
      TabIndex        =   4
      Top             =   600
      Width           =   975
   End
   Begin VB.CommandButton Save 
      Caption         =   "&Save As..."
      Default         =   -1  'True
      Height          =   375
      Left            =   4200
      TabIndex        =   3
      Top             =   180
      Width           =   975
   End
   Begin VB.Frame Frame1 
      Caption         =   "Classes To Save"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   1035
      Left            =   120
      TabIndex        =   0
      Top             =   60
      Width           =   3975
      Begin VB.OptionButton ClassThis 
         Caption         =   "Just class ..."
         Height          =   255
         Left            =   120
         TabIndex        =   2
         Top             =   600
         Width           =   3795
      End
      Begin VB.OptionButton ClassBelow 
         Caption         =   "Class ... and all child classes"
         Height          =   255
         Left            =   120
         TabIndex        =   1
         Top             =   360
         Value           =   -1  'True
         Width           =   3795
      End
   End
   Begin VB.Label ClassName 
      Caption         =   "ClassName (invisible)"
      Height          =   735
      Left            =   4200
      TabIndex        =   5
      Top             =   1440
      Visible         =   0   'False
      Width           =   975
   End
End
Attribute VB_Name = "frmSaveClass"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
'
' Modal save actor class form called from frmClassBrowser
'
Option Explicit

Private Sub Cancel_Click()
    GlobalAbortedModal = 1
    Unload Me
End Sub

Private Sub Form_Load()
    Call Ed.MakeFormFit(Me)
End Sub

Private Sub Save_Click()
    GResult = 0
    '
    If SaveUCX.Value Then
        frmDialogs.ClassSave.FilterIndex = 1
    ElseIf SaveTCX.Value Then
        frmDialogs.ClassSave.FilterIndex = 2
    Else ' SaveH.Value
        frmDialogs.ClassSave.FilterIndex = 3
    End If
    '
    If ClassThis.Value Then GResult = 1
    GlobalAbortedModal = 0
    Unload Me
End Sub
