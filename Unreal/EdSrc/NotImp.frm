VERSION 4.00
Begin VB.Form frmNotImp 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Work In Progress"
   ClientHeight    =   960
   ClientLeft      =   3240
   ClientTop       =   3150
   ClientWidth     =   4560
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
   Height          =   1320
   Icon            =   "NotImp.frx":0000
   Left            =   3180
   LinkTopic       =   "Form4"
   ScaleHeight     =   960
   ScaleWidth      =   4560
   ShowInTaskbar   =   0   'False
   Top             =   2850
   Width           =   4680
   Begin VB.CommandButton Command1 
      Caption         =   "&OK"
      Default         =   -1  'True
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
      Left            =   1800
      TabIndex        =   1
      Top             =   480
      Width           =   1095
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Sorry, this feature has not been implemented yet."
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
      Left            =   600
      TabIndex        =   0
      Top             =   120
      Width           =   3615
   End
End
Attribute VB_Name = "frmNotImp"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub Command1_Click()
   Unload Me
End Sub

