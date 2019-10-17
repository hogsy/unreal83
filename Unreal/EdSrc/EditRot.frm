VERSION 4.00
Begin VB.Form frmEditRotation 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Rotation"
   ClientHeight    =   1170
   ClientLeft      =   8055
   ClientTop       =   4980
   ClientWidth     =   2970
   ControlBox      =   0   'False
   Height          =   1530
   Icon            =   "EditRot.frx":0000
   KeyPreview      =   -1  'True
   Left            =   7995
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1170
   ScaleWidth      =   2970
   ShowInTaskbar   =   0   'False
   Top             =   4680
   Width           =   3090
   Begin VB.CommandButton Cancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   315
      Left            =   480
      TabIndex        =   6
      Top             =   1680
      Width           =   675
   End
   Begin VB.TextBox EditYaw 
      Height          =   285
      Left            =   780
      TabIndex        =   5
      Text            =   "EditYaw"
      Top             =   480
      Width           =   1935
   End
   Begin VB.TextBox EditRoll 
      Height          =   285
      Left            =   780
      TabIndex        =   4
      Text            =   "EditRoll"
      Top             =   840
      Width           =   1935
   End
   Begin VB.TextBox EditPitch 
      Height          =   285
      Left            =   780
      TabIndex        =   3
      Text            =   "EditPitch"
      Top             =   120
      Width           =   1935
   End
   Begin VB.Label Label3 
      Alignment       =   1  'Right Justify
      Caption         =   "Yaw:"
      Height          =   255
      Left            =   120
      TabIndex        =   2
      Top             =   540
      Width           =   555
   End
   Begin VB.Label Label2 
      Alignment       =   1  'Right Justify
      Caption         =   "Roll:"
      Height          =   255
      Left            =   120
      TabIndex        =   1
      Top             =   900
      Width           =   555
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Caption         =   "Pitch:"
      Height          =   255
      Left            =   120
      TabIndex        =   0
      Top             =   180
      Width           =   555
   End
End
Attribute VB_Name = "frmEditRotation"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim Owner As Form
Dim Begun As Boolean
Dim Updating As Boolean
Dim CurPitch As String, CurYaw As String, CurRoll As String

'
' Public
'

Public Sub BeginEditRotation(Name As String, Value As String, NewOwner As Form)
    '
    Begun = True
    Caption = Name
    UpdateValue (Value)
    Set Owner = NewOwner
    '
    Show
End Sub

Public Sub EndEditRotation()
    If Begun Then
        SendValue
        Updating = True ' Prevent additional updates during unload
        Unload Me
    End If
End Sub

'
' Updating
'

Private Sub UpdateValue(Value As String)
    Dim Pitch As Long, Yaw As Long, Roll As Long
    Dim i As Integer
    '
    Updating = True
    '
    i = InStr(Value, "(")
    CurPitch = Trim(Mid(Value, i + 1, InStr(Mid(Value, i + 1), ",") - 1))
    EditPitch.Text = CurPitch
    '
    i = InStr(i + 1, Value, ",")
    CurYaw = Trim(Mid(Value, i + 1, InStr(Mid(Value, i + 1), ",") - 1))
    EditYaw.Text = CurYaw
    '
    i = InStr(i + 1, Value, ",")
    CurRoll = Trim(Mid(Value, i + 1, InStr(Mid(Value, i + 1), ")") - 1))
    EditRoll.Text = CurRoll
    '
    Updating = False
End Sub

Private Sub SendValue()
    Dim Pitch As Double, Yaw As Double, Roll As Double
    Dim S As String
    '
    If Not Updating Then
        '
        If EditPitch.Text <> CurPitch Or _
            EditYaw.Text <> CurYaw Or _
            EditRoll.Text <> CurRoll Then
            '
            If Not Eval(EditPitch.Text, Pitch) Then GoTo Out
            If Not Eval(EditYaw.Text, Yaw) Then GoTo Out
            If Not Eval(EditRoll.Text, Roll) Then GoTo Out
            '
            S = Owner.RotationChange( _
                "(" & Str(Pitch) & _
                "," & Str(Yaw) & _
                "," & Str(Roll) & ")")
            UpdateValue (S)
        End If
    End If
Out:
End Sub

Private Sub Cancel_Click()
    EndEditRotation
End Sub

'
' Editing
'

Private Sub EditPitch_Click()
    SendValue
End Sub

Private Sub EditYaw_Click()
    SendValue
End Sub

Private Sub EditRoll_Click()
    SendValue
End Sub

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
    If KeyCode = vbKeyReturn Then
        SendValue
    End If
End Sub

'
' Loading/unloading
'

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "EditRotation", TOP_NORMAL)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

'
' Focus management
'

Private Sub Form_LostFocus()
    If Screen.ActiveForm.hwnd <> Me.hwnd Then EndEditRotation
End Sub

Private Sub EditX_LostFocus()
    If Screen.ActiveForm.hwnd <> Me.hwnd Then EndEditRotation
End Sub

Private Sub EditY_LostFocus()
    If Screen.ActiveForm.hwnd <> Me.hwnd Then EndEditRotation
End Sub

Private Sub EditZ_LostFocus()
    If Screen.ActiveForm.hwnd <> Me.hwnd Then EndEditRotation
End Sub


