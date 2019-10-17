VERSION 4.00
Begin VB.Form frmAmbientBrowser 
   BorderStyle     =   0  'None
   Caption         =   "Ambient Browser"
   ClientHeight    =   6465
   ClientLeft      =   6975
   ClientTop       =   2850
   ClientWidth     =   2445
   Height          =   6825
   Icon            =   "BrAmbi.frx":0000
   Left            =   6915
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   6465
   ScaleWidth      =   2445
   ShowInTaskbar   =   0   'False
   Top             =   2550
   Width           =   2565
   Begin Threed.SSPanel SSPanel1 
      Align           =   2  'Align Bottom
      Height          =   1170
      Left            =   0
      TabIndex        =   0
      Top             =   5295
      Width           =   2445
      _Version        =   65536
      _ExtentX        =   4313
      _ExtentY        =   2064
      _StockProps     =   15
      Caption         =   "Ambient Browser... To be expanded."
      BackColor       =   14198960
   End
End
Attribute VB_Name = "frmAmbientBrowser"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
'
' Sound Browser: This is a form that implements
' the browser interface.
'
' See ClassBr.frm for an example implementation
'
Option Explicit

'
' Public (Browser Interface)
'

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "SoundBrowser", TOP_BROWSER)
End Sub

Public Sub BrowserShow()
    Show
End Sub

Public Sub BrowserRefresh()
End Sub

Public Sub BrowserHide()
    Unload Me
End Sub

Public Function GetCurrent() As String
    GetCurrent = ""
End Function

'
' Private
'

