VERSION 4.00
Begin VB.Form frmEditVector 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Vector"
   ClientHeight    =   1185
   ClientLeft      =   8625
   ClientTop       =   2625
   ClientWidth     =   2700
   ControlBox      =   0   'False
   Height          =   1545
   Icon            =   "EditVect.frx":0000
   KeyPreview      =   -1  'True
   Left            =   8565
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1185
   ScaleWidth      =   2700
   ShowInTaskbar   =   0   'False
   Top             =   2325
   Width           =   2820
   Begin VB.CommandButton Cancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   315
      Left            =   480
      TabIndex        =   6
      Top             =   1500
      Width           =   675
   End
   Begin VB.TextBox EditY 
      Height          =   285
      Left            =   540
      TabIndex        =   2
      Text            =   "EditY"
      Top             =   480
      Width           =   1935
   End
   Begin VB.TextBox EditZ 
      Height          =   285
      Left            =   540
      TabIndex        =   3
      Text            =   "EditZ"
      Top             =   840
      Width           =   1935
   End
   Begin VB.TextBox EditX 
      Height          =   285
      Left            =   540
      TabIndex        =   1
      Text            =   "EditX"
      Top             =   120
      Width           =   1935
   End
   Begin VB.Label Label3 
      Alignment       =   1  'Right Justify
      Caption         =   "Y:"
      Height          =   255
      Left            =   120
      TabIndex        =   5
      Top             =   540
      Width           =   315
   End
   Begin VB.Label Label2 
      Alignment       =   1  'Right Justify
      Caption         =   "Z:"
      Height          =   255
      Left            =   120
      TabIndex        =   4
      Top             =   900
      Width           =   315
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Caption         =   "X:"
      Height          =   255
      Left            =   120
      TabIndex        =   0
      Top             =   180
      Width           =   315
   End
End
Attribute VB_Name = "frmEditVector"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim Owner As Form
Dim Begun As Boolean
Dim Updating As Boolean
Dim CurX As String, CurY As String, CurZ As String

'
' Public
'

Public Sub BeginEditVector(Name As String, Value As String, NewOwner As Form)
    '
    Begun = True
    Caption = Name
    UpdateValue (Value)
    Set Owner = NewOwner
    '
    Show
End Sub

Public Sub EndEditVector()
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
    Dim X As Double, Y As Double, Z As Double
    Dim i As Integer
    '
    Updating = True
    '
    i = InStr(Value, "(")
    CurX = Trim(Mid(Value, i + 1, InStr(Mid(Value, i + 1), ",") - 1))
    EditX.Text = CurX
    '
    i = InStr(i + 1, Value, ",")
    CurY = Trim(Mid(Value, i + 1, InStr(Mid(Value, i + 1), ",") - 1))
    EditY.Text = CurY
    '
    i = InStr(i + 1, Value, ",")
    CurZ = Trim(Mid(Value, i + 1, InStr(Mid(Value, i + 1), ")") - 1))
    EditZ.Text = CurZ
    '
    Updating = False
End Sub

Private Sub SendValue()
    Dim X As Double, Y As Double, Z As Double
    Dim S As String
    '
    If Not Updating Then
        '
        If EditX.Text <> CurX Or EditY.Text <> CurY Or EditZ.Text <> CurZ Then
            If Not Eval(EditX.Text, X) Then GoTo Out
            If Not Eval(EditY.Text, Y) Then GoTo Out
            If Not Eval(EditZ.Text, Z) Then GoTo Out
            '
            S = Owner.VectorChange( _
                "(" & Str(X) & _
                "," & Str(Y) & _
                "," & Str(Z) & ")")
            UpdateValue (S)
        End If
    End If
Out:
End Sub

Private Sub Cancel_Click()
    EndEditVector
End Sub

'
' Editing
'

Private Sub EditX_Click()
    SendValue
End Sub

Private Sub EditY_Click()
    SendValue
End Sub

Private Sub EditZ_Click()
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
    Call Ed.SetOnTop(Me, "EditVector", TOP_NORMAL)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

'
' Focus management
'

Private Sub Form_LostFocus()
    If Screen.ActiveForm.hwnd <> Me.hwnd Then EndEditVector
End Sub

Private Sub EditX_LostFocus()
    If Screen.ActiveForm.hwnd <> Me.hwnd Then EndEditVector
End Sub

Private Sub EditY_LostFocus()
    If Screen.ActiveForm.hwnd <> Me.hwnd Then EndEditVector
End Sub

Private Sub EditZ_LostFocus()
    If Screen.ActiveForm.hwnd <> Me.hwnd Then EndEditVector
End Sub

