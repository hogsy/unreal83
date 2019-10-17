VERSION 4.00
Begin VB.Form frmNewClass 
   Caption         =   "Create a new actor class"
   ClientHeight    =   1755
   ClientLeft      =   3765
   ClientTop       =   8385
   ClientWidth     =   4590
   Height          =   2115
   Icon            =   "NewClass.frx":0000
   Left            =   3705
   LinkTopic       =   "Form1"
   ScaleHeight     =   1755
   ScaleWidth      =   4590
   ShowInTaskbar   =   0   'False
   Top             =   8085
   Width           =   4710
   Begin VB.Frame Frame1 
      Caption         =   "Actor Class"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   1095
      Left            =   120
      TabIndex        =   3
      Top             =   120
      Width           =   4335
      Begin VB.TextBox NewClassName 
         Height          =   285
         Left            =   2040
         TabIndex        =   0
         Top             =   720
         Width           =   2175
      End
      Begin VB.Label ParentClassName 
         Caption         =   "ParentClassName"
         Height          =   255
         Left            =   2040
         TabIndex        =   6
         Top             =   360
         Width           =   1695
      End
      Begin VB.Label Label2 
         Alignment       =   1  'Right Justify
         Caption         =   "New actor class name:"
         Height          =   255
         Left            =   240
         TabIndex        =   5
         Top             =   720
         Width           =   1695
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         Caption         =   "Parent class:"
         Height          =   255
         Left            =   360
         TabIndex        =   4
         Top             =   360
         Width           =   1575
      End
   End
   Begin VB.CommandButton Cancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   3600
      TabIndex        =   2
      Top             =   1320
      Width           =   855
   End
   Begin VB.CommandButton Ok 
      Caption         =   "C&reate this actor class"
      Default         =   -1  'True
      Height          =   375
      Left            =   120
      TabIndex        =   1
      Top             =   1320
      Width           =   1815
   End
End
Attribute VB_Name = "frmNewClass"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Private Sub Cancel_Click()
    GResult = 0
    Unload Me
End Sub

Private Sub Form_Load()
    Call Ed.MakeFormFit(Me)
End Sub

Private Sub Ok_Click()
    GResult = 1
    GString = NewClassName.Text
    Unload Me
End Sub
