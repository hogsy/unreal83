VERSION 4.00
Begin VB.Form frmResBrowse 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Resource Browser"
   ClientHeight    =   4710
   ClientLeft      =   5985
   ClientTop       =   6090
   ClientWidth     =   3750
   ClipControls    =   0   'False
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
   Height          =   5070
   HelpContextID   =   122
   Icon            =   "ResBrws.frx":0000
   Left            =   5925
   LinkTopic       =   "Form8"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4710
   ScaleWidth      =   3750
   ShowInTaskbar   =   0   'False
   Top             =   5790
   Width           =   3870
   Begin VB.PictureBox Picture1 
      Height          =   4275
      Left            =   0
      ScaleHeight     =   4215
      ScaleWidth      =   3675
      TabIndex        =   4
      Top             =   0
      Width           =   3735
      Begin MSOutl.Outline List 
         Height          =   4215
         Left            =   0
         TabIndex        =   5
         Top             =   0
         Width           =   3675
         _Version        =   65536
         _ExtentX        =   6482
         _ExtentY        =   7435
         _StockProps     =   77
         BackColor       =   -2147483643
         BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         BorderStyle     =   0
         PicturePlus     =   "ResBrws.frx":030A
         PictureMinus    =   "ResBrws.frx":0404
      End
   End
   Begin VB.CommandButton Command4 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Purge"
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
      Left            =   1920
      TabIndex        =   3
      Top             =   4320
      Width           =   855
   End
   Begin VB.CommandButton Command3 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Debug"
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
      Left            =   960
      TabIndex        =   2
      Top             =   4320
      Width           =   855
   End
   Begin VB.CommandButton Command2 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
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
      Left            =   2880
      TabIndex        =   1
      Top             =   4320
      Width           =   855
   End
   Begin VB.CommandButton Refresh 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Refresh"
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
      Left            =   0
      TabIndex        =   0
      Top             =   4320
      Width           =   855
   End
End
Attribute VB_Name = "frmResBrowse"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub Command2_Click()
    Unload frmResBrowse ' Unload to reset, don't just hide
End Sub

Private Sub Command3_Click()
    Ed.Server.Exec "RESOURCE DEBUG"
    Refresh_Click
End Sub

Private Sub Command4_Click()
    Ed.Server.Exec "RESOURCE PURGE"
    Refresh_Click
End Sub

Private Sub Form_Load()
    Call InitOutline
    Call Ed.SetOnTop(Me, "ResourceBrowser", TOP_NORMAL)
    Call Refresh_Click
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub List_Expand(ListIndex As Integer)
    Dim Text As String
    Dim Name As String
    Dim ResType As String
    '
    Text = List.List(ListIndex)
    ResType = GrabString(Text)
    Name = GrabString(Text)
    '
    Call ExpandOutline(List, "Res", "QueryRes", "RESOURCE QUERY TYPE=" & ResType & " NAME=" & Name, ListIndex, 0)
End Sub

Private Sub List_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button And 2 Then
        'PopupMenu frmPopups.BrowserWin
    End If
End Sub

Private Sub Refresh_Click()
    QuerySource = -1
    List.Clear
    List.AddItem "Unreal Server on this PC"
    List.ListIndex = 0
    '
    QuerySource = List.ListIndex
    Ed.Server.Exec "RESOURCE QUERY NAME=ROOT TYPE=Array"
    Call UpdateOutline(List, "Res", "QueryRes")
End Sub
