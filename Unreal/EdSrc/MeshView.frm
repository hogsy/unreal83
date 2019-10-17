VERSION 4.00
Begin VB.Form frmMeshViewer 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Mesh Browser"
   ClientHeight    =   5490
   ClientLeft      =   5070
   ClientTop       =   4065
   ClientWidth     =   7050
   Height          =   5850
   Icon            =   "MeshView.frx":0000
   Left            =   5010
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5490
   ScaleWidth      =   7050
   ShowInTaskbar   =   0   'False
   Top             =   3765
   Width           =   7170
   Begin VB.Frame Frame1 
      Caption         =   "Animation Control"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   9.75
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   5295
      Left            =   4560
      TabIndex        =   1
      Top             =   120
      Width           =   2415
      Begin VB.OptionButton ViewPolys 
         Caption         =   "Polygons"
         Height          =   255
         Left            =   120
         TabIndex        =   22
         Tag             =   "See mesh polygons"
         Top             =   3000
         Width           =   1335
      End
      Begin VB.CommandButton Close 
         Cancel          =   -1  'True
         Caption         =   "&Close"
         Height          =   375
         Left            =   1560
         TabIndex        =   3
         Tag             =   "Close this window"
         Top             =   4800
         Width           =   735
      End
      Begin VB.CheckBox AutoDolly 
         Caption         =   "Auto Dolly"
         Height          =   255
         Left            =   120
         TabIndex        =   20
         Tag             =   "Continuously rotate the mesh"
         Top             =   3840
         Width           =   1095
      End
      Begin VB.CommandButton SeqMinus 
         Caption         =   "<"
         Height          =   315
         Left            =   60
         TabIndex        =   19
         Tag             =   "Previous sequence"
         Top             =   1320
         Width           =   210
      End
      Begin VB.CommandButton SeqPlus 
         Caption         =   ">"
         Height          =   315
         Left            =   270
         TabIndex        =   18
         Tag             =   "Next sequence"
         Top             =   1320
         Width           =   210
      End
      Begin VB.CommandButton MeshPlus 
         Caption         =   ">"
         Height          =   315
         Left            =   270
         TabIndex        =   16
         Tag             =   "Next mesh"
         Top             =   600
         Width           =   210
      End
      Begin VB.CommandButton MeshMinus 
         Caption         =   "<"
         Height          =   315
         Left            =   60
         TabIndex        =   17
         Tag             =   "Previous mesh"
         Top             =   600
         Width           =   210
      End
      Begin VB.CommandButton Refresh 
         Caption         =   "&Refresh"
         Height          =   375
         Left            =   840
         TabIndex        =   15
         Tag             =   "Refresh the mesh list"
         Top             =   4800
         Width           =   735
      End
      Begin VB.OptionButton ViewTextured 
         Caption         =   "Textured"
         Height          =   255
         Left            =   120
         TabIndex        =   14
         Tag             =   "See texture-mapped mesh"
         Top             =   2760
         Value           =   -1  'True
         Width           =   1335
      End
      Begin VB.CommandButton FramePlay 
         Caption         =   "Play >>"
         Height          =   255
         Left            =   1200
         TabIndex        =   12
         Tag             =   "Play/stop this animation"
         Top             =   2040
         Width           =   855
      End
      Begin VB.CommandButton FramePlus 
         Caption         =   ">"
         Height          =   255
         Left            =   840
         TabIndex        =   11
         Tag             =   "View next frame"
         Top             =   2040
         Width           =   255
      End
      Begin VB.CommandButton FrameMinus 
         Caption         =   "<"
         Height          =   255
         Left            =   600
         TabIndex        =   10
         Tag             =   "View previous frame"
         Top             =   2040
         Width           =   255
      End
      Begin VB.CommandButton FrameZero 
         Caption         =   "0"
         Height          =   255
         Left            =   120
         TabIndex        =   9
         Tag             =   "Go to first frame"
         Top             =   2040
         Width           =   375
      End
      Begin VB.ComboBox AnimSeqCombo 
         Height          =   315
         Left            =   510
         Style           =   2  'Dropdown List
         TabIndex        =   7
         Tag             =   "Animation sequence to view"
         Top             =   1320
         Width           =   1815
      End
      Begin VB.ComboBox MeshMapCombo 
         Height          =   315
         Left            =   540
         Sorted          =   -1  'True
         Style           =   2  'Dropdown List
         TabIndex        =   5
         Tag             =   "Mesh to view"
         Top             =   600
         Width           =   1815
      End
      Begin VB.CommandButton Help 
         Caption         =   "&Help"
         Height          =   375
         Left            =   120
         TabIndex        =   2
         Tag             =   "Get help"
         Top             =   4800
         Width           =   735
      End
      Begin VB.Label Label3 
         Caption         =   "Misc:"
         Height          =   255
         Left            =   120
         TabIndex        =   21
         Top             =   3600
         Width           =   1095
      End
      Begin VB.Label Label4 
         Caption         =   "View:"
         Height          =   255
         Left            =   120
         TabIndex        =   13
         Top             =   2520
         Width           =   1095
      End
      Begin VB.Label FrameLabel 
         Caption         =   "Frame 0 of 0"
         Height          =   255
         Left            =   120
         TabIndex        =   8
         Top             =   1800
         Width           =   1815
      End
      Begin VB.Label Label2 
         Caption         =   "Animation Sequence:"
         Height          =   255
         Left            =   120
         TabIndex        =   6
         Top             =   1080
         Width           =   1575
      End
      Begin VB.Label Label1 
         Caption         =   "MeshMap:"
         Height          =   255
         Left            =   120
         TabIndex        =   4
         Top             =   360
         Width           =   1815
      End
   End
   Begin VB.PictureBox MeshCamHolder 
      BackColor       =   &H00000000&
      Height          =   5310
      Left            =   45
      ScaleHeight     =   350
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   290
      TabIndex        =   0
      Top             =   120
      Width           =   4410
   End
End
Attribute VB_Name = "frmMeshViewer"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim FramePos As Integer
Dim NumFrames As Integer
Dim Playing As Integer

'
' Public:
'

Public Function GetCurrent() As String
    GetCurrent = MeshMapCombo.Text
End Function

'
' Startup/shutdown
'

Private Sub Close_Click()
    Unload Me
End Sub

Private Sub Form_Load()
    Dim S As String
    Call Ed.SetOnTop(Me, "MeshViewer", TOP_PANEL)
    UpdateMeshList
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
    Ed.Server.Exec "CAMERA CLOSE NAME=MeshViewCam"
End Sub

'
' List refreshing
'

Sub UpdateMeshList()
    Dim S As String
    MeshMapCombo.Clear
    Ed.Server.Exec "RESOURCE QUERY TYPE=MESHMAP"
    Do
        S = Trim(Ed.Server.GetProp("Res", "QueryRes"))
        If S = "" Then Exit Do
        Debug.Print "<" & S & ">"
        S = Trim(Mid(S, InStr(S, " ")))
        S = Trim(Left(S, InStr(S, "|") - 1))
        
        MeshMapCombo.AddItem S
    Loop
    If MeshMapCombo.ListCount > 0 Then MeshMapCombo.ListIndex = 0
    UpdateFrameList
End Sub

Sub UpdateFrameList()
    Dim i As Integer
    Dim N As Integer
    Dim S As String
    '
    AnimSeqCombo.Clear
    If MeshMapCombo.ListIndex >= 0 Then
        N = Val(Ed.Server.GetProp("MESH", "NUMANIMSEQS" & _
            " NAME=" & Quotes(MeshMapCombo.List(MeshMapCombo.ListIndex))))
        For i = 0 To N - 1
            S = Trim(Ed.Server.GetProp("MESH", "ANIMSEQ" & _
                " NAME=" & Quotes(MeshMapCombo.List(MeshMapCombo.ListIndex)) & _
                " NUM=" & Trim(Str(i))))
            If S <> "" Then
                AnimSeqCombo.AddItem S
            End If
        Next i
        If AnimSeqCombo.ListCount > 0 Then AnimSeqCombo.ListIndex = 0
    End If
    UpdateFrame
End Sub

Sub UpdateFrame()
    FramePos = 0
    If AnimSeqCombo.ListCount > 0 Then
        NumFrames = Val(Right(AnimSeqCombo.Text, 3))
    Else
        NumFrames = 0
    End If
    UpdateFrameCaption
End Sub

Sub UpdateFrameCaption()
    If Playing Then
        FrameLabel.Caption = "Playing " & Trim(Str(NumFrames)) & " frames..."
    Else
        FrameLabel.Caption = "Frame " & Trim(Str(FramePos + 0)) & " of " & Trim(Str(NumFrames))
    End If
    '
    If MeshMapCombo.ListCount <= 0 Then
        Ed.Server.Exec "CAMERA CLOSE NAME=MeshViewCam"
    Else
        Ed.Server.Exec "CAMERA OPEN " & _
            " NAME=MeshViewCam X=0 Y=0 FOV=75" & _
            " XR=" & Trim(Str(MeshCamHolder.ScaleWidth)) & _
            " YR=" & Trim(Str(MeshCamHolder.ScaleHeight)) & _
            " MESH=" & Quotes(MeshMapCombo.Text) & _
            " REN=" & Trim(Str(REN_MESHVIEW)) & _
            " FLAGS=" & Trim(Str(SHOW_AS_CHILD + SHOW_NOBUTTONS + IIf(Playing, SHOW_BACKDROP, 0) + IIf(Playing = 1 Or AutoDolly.Value, SHOW_REALTIME, 0) + IIf(AutoDolly.Value, SHOW_BRUSH, 0) + IIf(ViewPolys.Value, SHOW_COORDS, 0))) & _
            " HWND=" & Trim(Str(MeshCamHolder.hwnd)) & _
            " MISC1=" & Trim(Left(Right(AnimSeqCombo.Text, 7), 3)) & _
            " MISC2=" & Trim(Str(FramePos))
    End If
End Sub

'
' Buttons
'

Private Sub FramePlay_Click()
    If Playing = 1 Then
        FramePlay.Caption = "Play >>"
        FrameZero.Enabled = True
        FrameMinus.Enabled = True
        FramePlus.Enabled = True
        Playing = 0
    Else
        FramePlay.Caption = "Stop"
        FrameZero.Enabled = False
        FrameMinus.Enabled = False
        FramePlus.Enabled = False
        Playing = 1
    End If
    UpdateFrameCaption
End Sub

Private Sub FrameZero_Click()
    FramePos = 0
    UpdateFrameCaption
End Sub

Private Sub FrameMinus_Click()
    FramePos = FramePos - 1
    If FramePos < 0 Then FramePos = NumFrames - 1
    If FramePos < 0 Then FramePos = 0
    UpdateFrameCaption
End Sub

Private Sub FramePlus_Click()
    FramePos = FramePos + 1
    If FramePos >= NumFrames Then FramePos = 0
    UpdateFrameCaption
End Sub

Private Sub Refresh_Click()
    UpdateMeshList
End Sub

Private Sub MeshMapCombo_Click()
    UpdateFrameList
End Sub

Private Sub AnimSeqCombo_Click()
    UpdateFrame
End Sub

Private Sub MeshMinus_Click()
    If MeshMapCombo.ListIndex >= 0 Then
        If MeshMapCombo.ListIndex > 0 Then
            MeshMapCombo.ListIndex = MeshMapCombo.ListIndex - 1
        ElseIf MeshMapCombo.ListCount > 0 Then
            MeshMapCombo.ListIndex = MeshMapCombo.ListCount - 1
        End If
    End If
End Sub

Private Sub MeshPlus_Click()
    If MeshMapCombo.ListIndex >= 0 Then
        If MeshMapCombo.ListIndex + 1 < MeshMapCombo.ListCount Then
            MeshMapCombo.ListIndex = MeshMapCombo.ListIndex + 1
        Else
            MeshMapCombo.ListIndex = 0
        End If
    End If
End Sub


Private Sub SeqMinus_Click()
    If AnimSeqCombo.ListIndex >= 0 Then
        If AnimSeqCombo.ListIndex > 0 Then
            AnimSeqCombo.ListIndex = AnimSeqCombo.ListIndex - 1
        ElseIf AnimSeqCombo.ListCount > 0 Then
            AnimSeqCombo.ListIndex = AnimSeqCombo.ListCount - 1
        End If
    End If
End Sub

Private Sub SeqPlus_Click()
    If AnimSeqCombo.ListIndex >= 0 Then
        If AnimSeqCombo.ListIndex + 1 < AnimSeqCombo.ListCount Then
            AnimSeqCombo.ListIndex = AnimSeqCombo.ListIndex + 1
        Else
            AnimSeqCombo.ListIndex = 0
        End If
    End If
End Sub

Private Sub ViewPolys_Click()
    UpdateFrameCaption
End Sub

Private Sub ViewTextured_Click()
    UpdateFrameCaption
End Sub

Private Sub ViewWire_Click()
    UpdateFrameCaption
End Sub

Private Sub AutoDolly_Click()
    UpdateFrameCaption
End Sub

