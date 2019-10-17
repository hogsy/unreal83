VERSION 4.00
Begin VB.Form frmCameraHolder 
   BackColor       =   &H00000000&
   Caption         =   "CameraHolder"
   ClientHeight    =   6990
   ClientLeft      =   4095
   ClientTop       =   4215
   ClientWidth     =   6690
   Height          =   7395
   Left            =   4035
   LinkTopic       =   "Form2"
   ScaleHeight     =   6990
   ScaleWidth      =   6690
   Top             =   3870
   Width           =   6810
   Begin VB.OLE RBox 
      BackColor       =   &H00400000&
      Height          =   90
      Left            =   0
      MousePointer    =   15  'Size All
      TabIndex        =   0
      Top             =   495
      Visible         =   0   'False
      Width           =   90
   End
   Begin VB.OLE LBox 
      BackColor       =   &H00400000&
      Height          =   90
      Left            =   45
      MousePointer    =   15  'Size All
      TabIndex        =   1
      Top             =   285
      Visible         =   0   'False
      Width           =   90
   End
   Begin VB.OLE RBar 
      BackColor       =   &H00C0C0C0&
      BorderStyle     =   0  'None
      Height          =   90
      Left            =   105
      MousePointer    =   7  'Size N S
      TabIndex        =   4
      Top             =   495
      Visible         =   0   'False
      Width           =   1440
   End
   Begin VB.OLE VBar 
      BackColor       =   &H00C0C0C0&
      BorderStyle     =   0  'None
      Height          =   1020
      Left            =   600
      MousePointer    =   9  'Size W E
      TabIndex        =   3
      Top             =   120
      Visible         =   0   'False
      Width           =   90
   End
   Begin VB.OLE LBar 
      BackColor       =   &H00C0C0C0&
      BorderStyle     =   0  'None
      Height          =   90
      Left            =   150
      MousePointer    =   7  'Size N S
      TabIndex        =   2
      Top             =   285
      Visible         =   0   'False
      Width           =   1440
   End
End
Attribute VB_Name = "frmCameraHolder"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Dim VBarHold As Integer, LBarHold As Integer, RBarHold
Dim LBoxHold As Integer, RBoxHold As Integer
Dim VDelta As Integer, HDelta As Integer

'
' Vertical bar
'

Private Sub VBar_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    ZOrder
    If Button = 1 Then
        Ed.Server.Exec "CAMERA HIDESTANDARD"
        VBarHold = 1
        VDelta = X
    End If
End Sub

Private Sub VBar_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim NewX As Integer
    If VBarHold And Button = 1 Then
        NewX = VBar.Left + X - VDelta
        If NewX < 0 Then
            VBar.Left = 0
        ElseIf NewX > frmMain.CXL - VBar.Width Then
            VBar.Left = frmMain.CXL - VBar.Width
        Else
            VBar.Left = NewX
        End If
    LBox.Left = VBar.Left
    RBox.Left = VBar.Left
    LBar.Width = VBar.Left
    RBar.Left = VBar.Left + VBar.Width
    RBar.Width = frmMain.CXL - (VBar.Left + VBar.Width)
    Ed.CameraVertRatio = CSng(VBar.Left) / (frmMain.CXL - VBar.Width)
    End If
End Sub

Private Sub VBar_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If VBarHold Then frmMain.ResizeAll (True)
    VBarHold = 0
End Sub

'
' LBar
'

Private Sub LBar_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 1 Then
        Ed.Server.Exec "CAMERA HIDESTANDARD"
        LBarHold = 1
        HDelta = Y
    End If
End Sub

Private Sub LBar_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim NewY As Integer
    If LBarHold And Button = 1 Then
        NewY = LBar.Top + Y - HDelta
        If NewY < 0 Then
            LBar.Top = 0
        ElseIf NewY > frmMain.CYL - LBar.Height Then
            LBar.Top = frmMain.CYL - LBar.Height
        Else
            LBar.Top = NewY
        End If
        LBox.Top = LBar.Top
        Ed.CameraLeftRatio = CSng(LBar.Top) / CSng(frmMain.CY + frmMain.CYL - LBar.Height)
    End If
End Sub

Private Sub LBar_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If LBarHold Then frmMain.ResizeAll (True)
    LBarHold = 0
End Sub

'
' RBar
'

Private Sub RBar_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 1 Then
        Ed.Server.Exec "CAMERA HIDESTANDARD"
        RBarHold = 1
        HDelta = Y
    End If
End Sub

Private Sub RBar_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim NewY As Integer
    If RBarHold And Button = 1 Then
        NewY = RBar.Top + Y - HDelta
        If NewY < 0 Then
            RBar.Top = 0
        ElseIf NewY > frmMain.CYL - RBar.Height Then
            RBar.Top = frmMain.CYL - RBar.Height
        Else
            RBar.Top = NewY
        End If
        RBox.Top = RBar.Top
        Ed.CameraRightRatio = CSng(RBar.Top) / CSng(frmMain.CYL - RBar.Height)
    End If
End Sub

Private Sub RBar_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If RBarHold Then frmMain.ResizeAll (True)
    RBarHold = 0
End Sub

'
' L box
'

Private Sub LBox_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 1 Then
        Ed.Server.Exec "CAMERA HIDESTANDARD"
        LBoxHold = 1
        LBarHold = 1
        VBarHold = 1
        '
        VDelta = X - LBox.Left + VBar.Left
        HDelta = Y - LBox.Top + LBar.Top
    End If
End Sub

Private Sub LBox_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If LBoxHold And Button = 1 Then
        Call VBar_MouseMove(Button, Shift, X, Y)
        Call LBar_MouseMove(Button, Shift, X, Y)
    End If
End Sub

Private Sub LBox_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If LBoxHold Then frmMain.ResizeAll (True)
    '
    LBoxHold = 0
    LBarHold = 0
    VBarHold = 0
End Sub

'
' R box
'

Private Sub RBox_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 1 Then
        Ed.Server.Exec "CAMERA HIDESTANDARD"
        RBoxHold = 1
        RBarHold = 1
        VBarHold = 1
        '
        VDelta = X - RBox.Left + VBar.Left
        HDelta = Y - RBox.Top + RBar.Top
    End If
End Sub

Private Sub RBox_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If RBoxHold And Button = 1 Then
        Call VBar_MouseMove(Button, Shift, X, Y)
        Call RBar_MouseMove(Button, Shift, X, Y)
    End If
End Sub

Private Sub RBox_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If RBoxHold Then frmMain.ResizeAll (True)
    RBoxHold = 0
    RBarHold = 0
    VBarHold = 0
End Sub

