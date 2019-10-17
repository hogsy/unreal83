VERSION 4.00
Begin VB.Form frmScriptEd 
   Caption         =   "Class Script Editor"
   ClientHeight    =   7725
   ClientLeft      =   7620
   ClientTop       =   7005
   ClientWidth     =   7935
   Height          =   8370
   Icon            =   "ClassSc.frx":0000
   Left            =   7560
   LinkTopic       =   "Form1"
   ScaleHeight     =   7725
   ScaleWidth      =   7935
   ShowInTaskbar   =   0   'False
   Top             =   6420
   Width           =   8055
   Begin VB.TextBox EditBox 
      BeginProperty Font 
         name            =   "Courier New"
         charset         =   0
         weight          =   400
         size            =   9
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00800000&
      Height          =   7335
      Left            =   0
      MultiLine       =   -1  'True
      ScrollBars      =   3  'Both
      TabIndex        =   0
      TabStop         =   0   'False
      Text            =   "ClassSc.frx":030A
      Top             =   0
      Width           =   7935
   End
   Begin VB.Menu FileMenu 
      Caption         =   "&File"
      Begin VB.Menu FileSave 
         Caption         =   "&Save"
         Shortcut        =   ^S
      End
      Begin VB.Menu EditRevert 
         Caption         =   "&Revert"
      End
   End
   Begin VB.Menu EditMenu 
      Caption         =   "&Edit"
      Begin VB.Menu EditCut 
         Caption         =   "Cu&t"
         Shortcut        =   ^X
      End
      Begin VB.Menu EditCopy 
         Caption         =   "&Copy"
         Shortcut        =   ^C
      End
      Begin VB.Menu EditPaste 
         Caption         =   "&Paste"
         Shortcut        =   ^V
      End
      Begin VB.Menu ZYPSY 
         Caption         =   "-"
      End
      Begin VB.Menu EditUndoRedo 
         Caption         =   "&Undo/Redo"
         Shortcut        =   ^Z
      End
      Begin VB.Menu ZBLAST 
         Caption         =   "-"
      End
      Begin VB.Menu EditFind 
         Caption         =   "&Find"
         Shortcut        =   ^F
      End
      Begin VB.Menu EditFindNext 
         Caption         =   "Find &Next"
         Shortcut        =   {F3}
      End
   End
   Begin VB.Menu ScriptMenu 
      Caption         =   "&Script"
      Begin VB.Menu ScriptCompile 
         Caption         =   "&Compile"
         Shortcut        =   {F5}
      End
      Begin VB.Menu ScriptMakeChanged 
         Caption         =   "&Make Changed Scripts"
         Shortcut        =   {F6}
      End
      Begin VB.Menu ScriptMakeAll 
         Caption         =   "Make &All Scripts"
         Shortcut        =   {F7}
      End
      Begin VB.Menu ZSWAY 
         Caption         =   "-"
      End
      Begin VB.Menu GoToNextError 
         Caption         =   "&Go to Next Error"
         Shortcut        =   {F4}
      End
      Begin VB.Menu ScriptResults 
         Caption         =   "&Show Results"
         Shortcut        =   {F8}
      End
      Begin VB.Menu ClassEditDefaults 
         Caption         =   "&Edit Default Actor Properties"
         Shortcut        =   {F9}
      End
   End
   Begin VB.Menu HelpMenu 
      Caption         =   "&Help"
      Begin VB.Menu HelpExamples 
         Caption         =   "&Examples"
      End
      Begin VB.Menu HelpOverview 
         Caption         =   "&Language Overview"
      End
      Begin VB.Menu ZOGGST 
         Caption         =   "-"
      End
      Begin VB.Menu HelpCommands 
         Caption         =   "&Command Reference"
      End
      Begin VB.Menu HelpEvents 
         Caption         =   "&Event Reference"
      End
   End
End
Attribute VB_Name = "frmScriptEd"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim ShiftPressed As Boolean

Private Sub ClassEditDefaults_Click()
    frmActorProperties.GetClassDefaultActor (Caption)
End Sub

Private Sub Editbox_KeyDown(KeyCode As Integer, Shift As Integer)
    If Shift Then
        ShiftPressed = True
    Else
        ShiftPressed = False
    End If
End Sub

Private Sub Editbox_KeyPress(KeyAscii As Integer)
    Dim TabStuff As String
    Dim NewText As String
    Dim i As Integer
    Dim c As Integer
    Dim S, e As Integer
    '
    If KeyAscii = 9 And Editbox.SelLength > 0 Then
        KeyAscii = 0
        S = Editbox.SelStart
        If ShiftPressed Then
            NewText = Editbox.SelText
            If Left(NewText, 1) = Chr(9) Then NewText = Mid(NewText, 2)
            For i = 1 To Len(NewText)
                If Mid(NewText, i, 2) = Chr(10) + Chr(9) Then
                    NewText = Left(NewText, i) + Mid(NewText, i + 2)
                End If
            Next i
        Else
            NewText = Chr(9) + Editbox.SelText
            For i = 1 To Len(NewText)
                If Mid(NewText, i, 1) = Chr(10) Then
                    NewText = Left(NewText, i) + Chr(9) + Mid(NewText, i + 1)
                    i = i + 1
                End If
            Next i
        End If
        Editbox.SelText = NewText
        Editbox.SelStart = S
        Editbox.SelLength = Len(NewText)
    End If
End Sub

Private Sub Editbox_KeyUp(KeyCode As Integer, Shift As Integer)
    ShiftPressed = False
End Sub

Private Sub EditCopy_Click()
    Editbox.SetFocus
    SendKeys "^{C}"
End Sub

Private Sub EditCut_Click()
    Editbox.SetFocus
    SendKeys "^{X}"
End Sub

Private Sub EditFind_Click()
    '
    frmScriptFind.ScriptName = Caption
    frmScriptFind.Show 1
    '
    EditFindNext_Click
End Sub

Private Sub EditFindNext_Click()
    Dim Find As String
    Dim Replace As String
    Dim Test As String
    Dim Length As Integer
    Dim P As Integer
    Dim DoCaps As Integer
    Dim OldStart As Integer
    '
    Find = frmScriptFind.FindText
    Replace = frmScriptFind.ReplaceText
    '
    If Trim(Find) = "" Then Exit Sub
    '
    DoCaps = IIf(frmScriptFind.CaseSensitive.Value, 0, 1)
    '
    If DoCaps = 1 Then Find = UCase(Find)
    '
    If GFindResult = 1 Then
        '
        ' Find:
DoFind: '
        P = InStr( _
            Editbox.SelStart + Editbox.SelLength + 1, _
            Editbox.Text, _
            Find, _
            DoCaps)
        If P > 0 Then
            Editbox.SelStart = P - 1
            Editbox.SelLength = Len(Find)
        Else
            Editbox.SelStart = Len(Editbox.Text)
        End If
        If GFindResult = 3 Then GoTo DoRep
        '
    ElseIf GFindResult = 2 Then
        '
        ' Replace:
DoRep:  '
        Test = Mid(Editbox.Text, Editbox.SelStart + 1, Len(Find))
        If DoCaps = 1 Then Test = UCase(Test)
        '
        If Test = Find Then
            OldStart = Editbox.SelStart
            Editbox.Text = _
                Left(Editbox.Text, Editbox.SelStart) & _
                Replace & _
                Mid(Editbox.Text, Editbox.SelStart + Len(Find) + 1)
            Editbox.SelStart = OldStart + Len(Replace)
            Editbox.SelLength = 0
            GoTo DoFind
        Else
            If GFindResult <> 3 Then GoTo DoFind
        End If
        Editbox.SelStart = Len(Editbox.Text)
        '
    ElseIf GFindResult = 3 Then
        '
        ' Replace All
        '
        GoTo DoFind
        '
    End If
End Sub

Private Sub EditPaste_Click()
    Editbox.SetFocus
    SendKeys "^{V}"
End Sub

Private Sub EditRevert_Click()
    Editbox.SetFocus
    Editbox.Text = Ed.Server.GetProp("TEXT", Caption)
End Sub

Private Sub EditUndoRedo_Click()
    Editbox.SetFocus
    Call SendMessage(Editbox.hwnd, EM_UNDO, 0, 0)
End Sub

Private Sub FileSave_Click()
    Editbox.SetFocus
    DoFileSave
End Sub

Sub DoFileSave()
    Call Ed.Server.SetProp("TEXT", Caption, Editbox.Text)
    Call Ed.Server.SetProp("TEXTPOS", Caption, Str(Editbox.SelStart))
End Sub

Private Sub Form_Load()
    Dim i As Integer
    Dim Tabs As DWORDREC
    '
    Me.Left = (ScriptEdLeft + 140) * Screen.TwipsPerPixelX: ScriptEdLeft = (ScriptEdLeft + 32) Mod 300
    Me.Top = (ScriptEdTop + 140) * Screen.TwipsPerPixelY: ScriptEdTop = (ScriptEdTop + 32) Mod 200
    '
    Call Ed.SetOnTop(Me, "ClassScriptEditor" & Str(GNumMiscForms), TOP_NORMAL)
    '
    Tabs.Value = 12
    Call SendTabsMessage(Editbox.hwnd, EM_SETTABSTOPS, 1, Tabs)
    '
    DoResize
End Sub

Private Sub Form_Resize()
    DoResize
End Sub

Private Sub DoResize()
    Dim MustExit As Boolean
    If WindowState = 1 Then Exit Sub
    If Width < 320 * Screen.TwipsPerPixelX Then
        Width = 320 * Screen.TwipsPerPixelX
        MustExit = True
    End If
    If Height < 240 * Screen.TwipsPerPixelY Then
        Height = 240 * Screen.TwipsPerPixelY
        MustExit = True
    End If
    If MustExit Then Exit Sub
    '
    Editbox.Width = ScaleWidth
    Editbox.Height = ScaleHeight - Editbox.Top
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
    Call RemoveMiscForm(Me)
    DoFileSave
End Sub

Sub StartCompile()
    Call frmResults.UpdateStatus("Compiling...")
    Call Ed.Server.SetProp("TEXT", Caption, Editbox.Text)
    Call Ed.Server.SetProp("TEXTPOS", Caption, Str(Editbox.SelStart))
End Sub

Sub EndCompile()
    frmResults.UpdateResults
    frmActorProperties.NoteClassChange
End Sub

Private Sub GoToNextError_Click()
    frmResults.GoToNext
End Sub

Private Sub ScriptCompile_Click()
    StartCompile
    Ed.Server.Exec "SCRIPT COMPILE CLASS=" & Caption
    EndCompile
    frmResults.Results_DblClick
End Sub

Public Sub PreSave()
    Call Ed.Server.SetProp("TEXT", Caption, Editbox.Text)
End Sub

Public Sub PostLoad()
    Editbox.SetFocus
    Editbox.Text = Ed.Server.GetProp("TEXT", Caption)
End Sub

Private Sub ScriptMakeAll_Click()
    StartCompile
    Ed.Server.Exec "SCRIPT MAKE ALL"
    EndCompile
End Sub

Private Sub ScriptMakeChanged_Click()
    StartCompile
    Ed.Server.Exec "SCRIPT MAKE"
    EndCompile
End Sub

Private Sub ScriptResults_Click()
    frmResults.UpdateResults
End Sub
