VERSION 4.00
Begin VB.Form frmScriptFind 
   Caption         =   "Find Text"
   ClientHeight    =   2115
   ClientLeft      =   6300
   ClientTop       =   1470
   ClientWidth     =   5505
   Height          =   2475
   Icon            =   "ScrFind.frx":0000
   Left            =   6240
   LinkTopic       =   "Form2"
   ScaleHeight     =   2115
   ScaleWidth      =   5505
   Top             =   1170
   Width           =   5625
   Begin VB.CommandButton DoReplace 
      Caption         =   "&Replace"
      Height          =   315
      Left            =   4260
      TabIndex        =   5
      Top             =   480
      Width           =   1155
   End
   Begin VB.Frame Frame2 
      Caption         =   "Options"
      Height          =   1155
      Left            =   2460
      TabIndex        =   14
      Top             =   900
      Width           =   1695
      Begin VB.CheckBox CaseSensitive 
         Caption         =   "Match Ca&se"
         Height          =   255
         Left            =   120
         TabIndex        =   11
         Top             =   300
         Width           =   1515
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Search"
      Height          =   1155
      Left            =   60
      TabIndex        =   12
      Top             =   900
      Width           =   2295
      Begin VB.OptionButton SearchAll 
         Caption         =   "&All Scripts"
         Enabled         =   0   'False
         Height          =   255
         Left            =   120
         TabIndex        =   10
         Top             =   780
         Width           =   2115
      End
      Begin VB.OptionButton SearchParents 
         Caption         =   "Current + &Parent Scripts"
         Enabled         =   0   'False
         Height          =   195
         Left            =   120
         TabIndex        =   9
         Top             =   540
         Width           =   2115
      End
      Begin VB.OptionButton SearchCurrent 
         Caption         =   "&Current Script"
         Height          =   255
         Left            =   120
         TabIndex        =   8
         Top             =   240
         Value           =   -1  'True
         Width           =   1815
      End
   End
   Begin VB.CommandButton DoCancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   315
      Left            =   4260
      TabIndex        =   7
      Top             =   1740
      Width           =   1155
   End
   Begin VB.CommandButton DoReplaceAll 
      Caption         =   "Replace &All"
      Height          =   315
      Left            =   4260
      TabIndex        =   6
      Top             =   840
      Width           =   1155
   End
   Begin VB.CommandButton DoFind 
      Caption         =   "Find &Next"
      Default         =   -1  'True
      Height          =   315
      Left            =   4260
      TabIndex        =   4
      Top             =   120
      Width           =   1155
   End
   Begin VB.TextBox ReplaceText 
      Height          =   315
      Left            =   1320
      TabIndex        =   3
      Top             =   480
      Width           =   2835
   End
   Begin VB.TextBox FindText 
      Height          =   315
      Left            =   1320
      TabIndex        =   1
      Top             =   120
      Width           =   2835
   End
   Begin VB.Label ScriptName 
      Caption         =   "ScriptName"
      Height          =   255
      Left            =   4260
      TabIndex        =   13
      Top             =   1380
      Visible         =   0   'False
      Width           =   1215
   End
   Begin VB.Label Label2 
      Alignment       =   1  'Right Justify
      Caption         =   "Replace &With:"
      Height          =   255
      Left            =   120
      TabIndex        =   2
      Top             =   540
      Width           =   1035
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Caption         =   "&Find Text:"
      Height          =   255
      Left            =   60
      TabIndex        =   0
      Top             =   180
      Width           =   1095
   End
End
Attribute VB_Name = "frmScriptFind"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub DoCancel_Click()
    GResult = 0
    Me.Hide
End Sub

Private Sub DoFind_Click()
    GFindResult = 1
    Me.Hide
End Sub

Private Sub DoReplace_Click()
    GFindResult = 2
    Me.Hide
End Sub

Private Sub DoReplaceAll_Click()
    GFindResult = 3
    Me.Hide
End Sub

Private Sub FindText_GotFocus()
    FindText.SelStart = 0
    FindText.SelLength = Len(FindText.Text)
End Sub

Private Sub ReplaceText_GotFocus()
    ReplaceText.SelStart = 0
    ReplaceText.SelLength = Len(ReplaceText.Text)
End Sub

Private Sub Form_Activate()
    Call Ed.MakeFormFit(Me)
    FindText.SelStart = 0
    FindText.SelLength = Len(frmScriptFind.FindText.Text)
    FindText.SetFocus
End Sub

