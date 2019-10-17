VERSION 4.00
Begin VB.Form frmResults 
   BorderStyle     =   5  'Sizable ToolWindow
   Caption         =   "Results Window"
   ClientHeight    =   2160
   ClientLeft      =   4920
   ClientTop       =   10200
   ClientWidth     =   8190
   Height          =   2565
   Icon            =   "Results.frx":0000
   Left            =   4860
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2160
   ScaleWidth      =   8190
   ShowInTaskbar   =   0   'False
   Top             =   9855
   Width           =   8310
   Begin VB.ListBox Results 
      BackColor       =   &H00000000&
      BeginProperty Font 
         name            =   "Courier New"
         charset         =   0
         weight          =   400
         size            =   9
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H0000FFFF&
      Height          =   1860
      IntegralHeight  =   0   'False
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   8175
   End
End
Attribute VB_Name = "frmResults"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

'
' Public
'

'
' Update the compile results by snagging the
' script error property from the server
'
Sub UpdateResults()
    Dim S As String, L As String
    S = Ed.Server.GetProp("TEXT", "ScriptError")
    '
    Results.Clear
    L = GrabLine(S)
    While L <> "" Or S <> ""
        Results.AddItem L
        L = GrabLine(S)
    Wend
    '
    If (Results.ListCount > 1) Then
        Results.ListIndex = Results.ListCount - 2
    End If
    '
    Show
End Sub

Public Sub GoToNext()
    If Not Visible Then UpdateResults
    If Results.ListCount > 0 Then
        If Results.ListIndex + 1 < Results.ListCount Then
            Results.ListIndex = Results.ListIndex + 1
        Else
            Results.ListIndex = 0
        End If
        Results_DblClick
    End If
End Sub

'
' Print a status message
'
Sub UpdateStatus(S As String)
    Results.Clear
    Results.AddItem S
    Show
End Sub

Private Sub Results_KeyDown(KeyCode As Integer, Shift As Integer)
    If KeyCode = vbKeyF4 Then GoToNext
End Sub

'
' Private
'

Private Sub Form_Load()
    '
    Call Ed.SetOnTop(Me, "CompileResults", TOP_PANEL)
    '
    Dim R As RECT
    Call GetWindowRect(frmMain.hwnd, R)
    Left = frmMain.CX
    Width = frmMain.CXL
    Top = frmMain.ScaleHeight - Height
    '
End Sub

Private Sub Form_Resize()
    Results.Width = ScaleWidth
    Results.Height = ScaleHeight
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Public Sub Results_DblClick()
    Dim S As String, T As String, i As Integer, Line As Integer
    If Results.ListIndex >= 0 Then
        S = Results.List(Results.ListIndex)
        '
        If Left(S, 10) = "Compiling " Then
            S = Mid(S, 11)
            i = InStr(S, "...")
            If i <> 0 Then
                S = Left(S, i - 1) ' Class name
                Call frmClassBrowser.LaunchScriptEd(S, "", 0, 0)
            End If
        ElseIf Left(S, 9) = "Error in " Then
            S = Mid(S, 10)
            i = InStr(S, ", Line ")
            If i <> 0 Then
                Line = Val(Mid(S, i + 7)) ' Line number
                S = Left(S, i - 1) ' Class name
                Call frmClassBrowser.LaunchScriptEd(S, "", 0, Line)
            End If
        End If
    End If
End Sub
