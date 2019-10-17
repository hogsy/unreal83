VERSION 4.00
Begin VB.Form frmGeneric 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Temp Junk"
   ClientHeight    =   9690
   ClientLeft      =   2985
   ClientTop       =   3945
   ClientWidth     =   9075
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
   Height          =   10050
   HelpContextID   =   330
   Icon            =   "Generic.frx":0000
   Left            =   2925
   LinkTopic       =   "Form8"
   MaxButton       =   0   'False
   ScaleHeight     =   9690
   ScaleWidth      =   9075
   ShowInTaskbar   =   0   'False
   Top             =   3645
   Width           =   9195
End
Attribute VB_Name = "frmGeneric"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "Temp", TOP_NORMAL)
End Sub
