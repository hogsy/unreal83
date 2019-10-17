VERSION 4.00
Begin VB.Form frmClassBrowser 
   BorderStyle     =   0  'None
   Caption         =   "Class Browser"
   ClientHeight    =   6450
   ClientLeft      =   7365
   ClientTop       =   4305
   ClientWidth     =   2475
   Height          =   6855
   HelpContextID   =   329
   Icon            =   "BrClass.frx":0000
   Left            =   7305
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   6450
   ScaleWidth      =   2475
   ShowInTaskbar   =   0   'False
   Top             =   3960
   Width           =   2595
   Begin VB.PictureBox ClassHolder 
      BackColor       =   &H00C0C0C0&
      Height          =   5010
      Left            =   60
      ScaleHeight     =   4950
      ScaleWidth      =   2310
      TabIndex        =   0
      Top             =   45
      Width           =   2370
      Begin MSOutl.Outline Classes 
         Height          =   4935
         Left            =   0
         TabIndex        =   1
         Top             =   0
         Width           =   2295
         _Version        =   65536
         _ExtentX        =   4048
         _ExtentY        =   8705
         _StockProps     =   77
         ForeColor       =   -2147483640
         BackColor       =   12632256
         BorderStyle     =   0
         Style           =   2
         PicturePlus     =   "BrClass.frx":030A
         PictureMinus    =   "BrClass.frx":0404
      End
   End
   Begin Threed.SSPanel ClassPanel 
      Height          =   1335
      Left            =   0
      TabIndex        =   2
      Top             =   5100
      Width           =   2475
      _Version        =   65536
      _ExtentX        =   4366
      _ExtentY        =   2355
      _StockProps     =   15
      Begin VB.CommandButton Make 
         Caption         =   "&Make All"
         Height          =   255
         Left            =   1140
         TabIndex        =   3
         Top             =   1020
         Width           =   1275
      End
      Begin VB.CommandButton Refresh 
         Caption         =   "&Refresh"
         Height          =   255
         Left            =   60
         TabIndex        =   9
         Tag             =   "Refresh the class list"
         Top             =   1020
         Width           =   1095
      End
      Begin VB.CommandButton SaveClass 
         Caption         =   "&Save/Export"
         Height          =   255
         Left            =   1140
         TabIndex        =   4
         Tag             =   "Save actor classes"
         Top             =   780
         Width           =   1275
      End
      Begin VB.CommandButton LoadClass 
         Caption         =   "&Load/Import"
         Height          =   255
         Left            =   60
         TabIndex        =   5
         Tag             =   "Load actor classes"
         Top             =   780
         Width           =   1095
      End
      Begin VB.CommandButton NewClass 
         Caption         =   "Create &New Class Below"
         Height          =   255
         Left            =   60
         TabIndex        =   6
         Tag             =   "Creates a new actor class"
         Top             =   540
         Width           =   2355
      End
      Begin VB.CommandButton EditDefActor 
         Caption         =   "&Default Props"
         Height          =   255
         Left            =   1080
         TabIndex        =   7
         Tag             =   "Edit this class's default actor"
         Top             =   300
         Width           =   1335
      End
      Begin VB.CommandButton EditScript 
         Caption         =   "&Edit Script"
         Height          =   255
         Left            =   60
         TabIndex        =   8
         Tag             =   "Edit this class's script"
         Top             =   300
         Width           =   1035
      End
      Begin VB.Label ClassDescr 
         Alignment       =   2  'Center
         BackStyle       =   0  'Transparent
         Caption         =   "This class is built-in."
         Height          =   285
         Left            =   0
         TabIndex        =   10
         Top             =   60
         Width           =   2415
      End
   End
End
Attribute VB_Name = "frmClassBrowser"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
'
' Class browser: This is a form that implements
' the browser interface.
'
Option Explicit

'
' Public (Browser Interface)
'

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "ClassBrowser", TOP_BROWSER)
    ClassPanel.Top = Height - ClassPanel.Height
    ClassHolder.Height = Height - ClassPanel.Height
    Classes.Height = Height - ClassPanel.Height
    Refresh_Click
End Sub

Public Sub BrowserShow()
    Show
End Sub

Public Sub BrowserRefresh()
    '
End Sub

Public Sub BrowserHide()
    'Hide
    'Unload Me
End Sub

Public Function GetCurrent() As String
    Dim Result As String
    Result = Classes.List(Classes.ListIndex)
    If InStr(Result, Chr(9)) Then
        Result = Left(Result, InStr(Result, Chr(9)) - 1)
    End If
    If Left(Result, 1) = "*" Then
        Result = Mid(Result, 2)
    End If
    GetCurrent = Trim(Result)
End Function

'
' Public (other)
'

Public Sub EditDefActor_Click()
    frmActorProperties.GetClassDefaultActor (GetCurrent())
End Sub

Public Sub LaunchScriptEd(Classname As String, NewText As String, Cursor As Integer, ErrorLine As Integer)
    Dim F As frmScriptEd
    Dim S As String
    Dim i As Integer, j As Integer
    '
    If Ed.Server.GetProp("TEXTPOS", Classname) <> "" Then
        For i = 0 To GNumMiscForms - 1
            If GMiscForms(i).Caption = Classname Then
                Set F = GMiscForms(i)
                GoTo HaveIt
            End If
        Next i
        '
        Set F = New frmScriptEd
        F.Caption = Classname
        Call AddMiscForm(F)
        '
        If NewText = "" Then ' Get text from script
            S = Ed.Server.GetProp("TEXT", Classname)
            F.Editbox.Text = S
            F.Editbox.SelStart = Val(Ed.Server.GetProp("TEXTPOS", Classname))
        Else ' Newly created script
            S = NewText
            F.Editbox.Text = S
            F.Editbox.SelStart = Cursor
            Call Ed.Server.SetProp("TEXT", Classname, S)
        End If
        '
HaveIt: If ErrorLine <> 0 Then
            i = 1
            j = 1
            S = F.Editbox.Text
            While i < ErrorLine
                j = 1 + InStr(j, S, Chr(10))
                If j = 1 Then
                    GoTo ShowIt
                End If
                i = i + 1
            Wend
            F.Editbox.SelStart = j - 1
            i = InStr(j + 1, S, Chr(13))
            If i <> 0 Then F.Editbox.SelLength = i - j
        End If
ShowIt: F.Show
        F.SetFocus
    End If
End Sub

'
' Private
'

Private Sub Classes_DblClick()
    If ClassIsEditable() Then
        EditScript_Click
    Else
        EditDefActor_Click
    End If
End Sub

Public Sub Delete_Click()
    If ClassIsEditable() Then
        If MsgBox("Deleting class " & GetCurrent() & _
            " will delete all actors belonging to the class, " & _
            " as well as all child classes.  Are you sure you " & _
            " want to delete it?", 4, _
            "Delete Class " & GetCurrent()) = 6 Then
            Ed.Server.Exec "CLASS DELETE NAME=" & GetCurrent()
            Refresh_Click
        End If
    End If
End Sub

Private Sub Classes_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 2 Then
        If ClassIsEditable() Then
            frmPopups2.cbDelete.Visible = True
            frmPopups2.ClassEditScript.Visible = True
            frmPopups2.ClassEditScript.Caption = "&Edit " & GetCurrent() & " Script"
        Else
            frmPopups2.cbDelete.Visible = False
            frmPopups2.ClassEditScript.Visible = False
        End If
        frmPopups2.ClassEditActor.Caption = "Default " & GetCurrent() & " &Properties..."
        frmPopups2.ClassCreateNew.Caption = "Create new class below " & GetCurrent()
        PopupMenu frmPopups2.ClassBrowser
    End If
End Sub

Private Sub Command1_Click()
    Dim R As RECT
    Call GetClientRect(frmGeneric.hwnd, R)
    Call SetWindowPos(Me.hwnd, 0, 0, 0, R.Right - R.Left, R.Bottom - R.Top, 0)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub SelOpt_Click()
    If Height <> 6765 Then
        Height = 6765
    Else
        Height = 7935
    End If
End Sub

Public Sub EditScript_Click()
    If GetCurrent() <> "" And ClassIsEditable() Then
        Call LaunchScriptEd(GetCurrent(), "", 0, 0)
    End If
End Sub

Private Sub Classes_Click()
    Dim Temp As String
    '
    Temp = GetCurrent()
    If Temp <> "" Then
        frmMain.ActorCombo.List(0) = Temp
        frmMain.ActorCombo.ListIndex = 0
        Ed.Server.Exec "ACTOR SET ADDCLASS=" & Temp
    End If
    SetActorCaption
End Sub

Private Sub Classes_Expand(ListIndex As Integer)
    Dim Text As String
    Dim Classname As String
    Text = Classes.List(ListIndex)
    Classname = GrabString(Text)
    If Left(Classname, 1) = "*" Then Classname = Mid(Classname, 2)
    Call ExpandOutline(Classes, "Class", "QueryRes", "CLASS QUERY PARENT=" & Classname, ListIndex, 0)
End Sub

Public Sub LoadClass_Click()
    Dim Fnames As String
    Dim Fname As String
    '
    ' Dialog for "Load Class":
    '
    On Error GoTo Skip
    Ed.Server.Disable
    frmDialogs.ClassLoad.filename = ""
    frmDialogs.ClassLoad.ShowOpen
    '
    Call UpdateDialog(frmDialogs.ClassLoad)
    If (frmDialogs.ClassLoad.filename <> "") Then
        Ed.Server.Exec "TASK BEGIN MESSAGE=" & Quotes("Loading and compiling classes")
        Screen.MousePointer = 11
        Fnames = Trim(frmDialogs.ClassLoad.filename)
        While (Fnames <> "")
            Fname = GrabFname(Fnames)
            Ed.Server.Exec "CLASS LOAD FILE=" & Quotes(Fname)
        Wend
        Make_Click
        Refresh_Click
        PostLoad
        Screen.MousePointer = 0
        Ed.Server.Exec "TASK END"
    End If
Skip:
    Ed.Server.Enable
End Sub

Private Sub Make_Click()
    Ed.Server.Exec "TASK BEGIN MESSAGE=" & Quotes("Making all classes")
    Ed.Server.Exec "SCRIPT MAKE ALL"
    frmResults.UpdateResults
    Ed.Server.Exec "TASK END"
End Sub

Public Sub NewClass_Click()
    Dim S As String
    If Classes.ListIndex >= 0 Then
        frmNewClass.ParentClassName = GetCurrent()
        frmNewClass.NewClassName = "My" & GetCurrent()
        Ed.Server.Disable
        frmNewClass.Show 1
        Ed.Server.Enable
        If GResult = 1 Then
            If Val(Ed.Server.GetProp("CLASS", "EXISTS NAME=" & Quotes(GString))) = 1 Then
                If MsgBox( _
                    "An actor class named " & GString & _
                    " Already exists.  Do you want to replace it?  Replacing an " & _
                    "existing class causes all of its child classes to be copied " & _
                    "over to the new class.", 4, _
                    "Class already exists") = 7 Then
                    Exit Sub
                End If
            End If
            Ed.Server.Exec "CLASS NEW NAME=" & Quotes(GString) & " PARENT=" & Quotes(GetCurrent())
            S = "Class " & GString & " Expands " & GetCurrent() & _
                Chr(13) & Chr(10) & Chr(13) & Chr(10)
            frmMain.ActorCombo.List(0) = GString
            Call ExpandOutline(frmClassBrowser.Classes, "Class", "QueryRes", "CLASS QUERY PARENT=" & GetCurrent(), frmClassBrowser.Classes.ListIndex, 1)
            Call SetCurrent
            Call LaunchScriptEd(GString, S, Len(S), 0)
        End If
    End If
End Sub

Private Sub Refresh_Click()
    Dim i As Integer
    Dim N As Integer
    Call InitOutline
    '
    QuerySource = -1
    Classes.Clear
    Classes.AddItem "Root"
    Classes.ListIndex = 0
    '
    QuerySource = Classes.ListIndex
    Ed.Server.Exec "CLASS QUERY PARENT=ROOT"
    Call UpdateOutline(Classes, "Class", "QueryRes")
    '
    SetCurrent
End Sub

Sub SetCurrent()
    Dim N As Integer
    Dim i As Integer
    '
    N = Len(frmMain.ActorCombo.Text)
    For i = 0 To Classes.ListCount - 1
        If Mid(Classes.List(i), 1, N) = frmMain.ActorCombo.Text Then
            Classes.ListIndex = i
        ElseIf Mid(Classes.List(i), 2, N) = frmMain.ActorCombo.Text Then
            Classes.ListIndex = i
        End If
    Next i
    SetActorCaption
End Sub

Function ClassIsEditable() As Boolean
    If Classes.ListIndex < 0 Then
        ClassIsEditable = False
    ElseIf Left(Classes.List(Classes.ListIndex), 1) <> "*" Then
        ClassIsEditable = True
    ElseIf Ed.GodMode Then
        ClassIsEditable = True
    Else
        ClassIsEditable = False
    End If
End Function

Sub SetActorCaption()
    If ClassIsEditable() Then
        EditScript.Enabled = True
        ClassDescr.Caption = "This class is scripted."
    Else
        EditScript.Enabled = False
        ClassDescr.Caption = "This class is built-in."
    End If
End Sub

Public Sub SaveClass_Click()
    Dim CurClass As String
    Dim Fname As String
    '
    PreSave
    CurClass = GetCurrent()
    If CurClass = "" Then Exit Sub
    '
    frmSaveClass.ClassThis.Caption = "Just class " & CurClass
    frmSaveClass.ClassBelow.Caption = "Class " & CurClass & " and all classes beneath it"
    frmSaveClass.Classname = CurClass
    Ed.Server.Disable
    frmSaveClass.Show 1
    Ed.Server.Enable
    If GlobalAbortedModal Then Exit Sub
    '
    If InStr(CurClass, " ") > 0 Then
        Fname = Left(CurClass, InStr(CurClass, " ") - 1)
    Else
        Fname = CurClass
    End If
    '
    If frmDialogs.ClassSave.FilterIndex = 1 Then
        frmDialogs.ClassSave.filename = Trim(Fname) & ".ucx"
    ElseIf frmDialogs.ClassSave.FilterIndex = 2 Then
        frmDialogs.ClassSave.filename = Trim(Fname) & ".tcx"
    Else
        frmDialogs.ClassSave.filename = Trim(Fname) & ".h"
    End If
    On Error GoTo BadFilename
TryAgain:
    On Error GoTo Skip
    Ed.Server.Disable
    frmDialogs.ClassSave.DialogTitle = "Save " & CurClass & " actor class"
    frmDialogs.ClassSave.ShowSave
    Ed.Server.Enable
    '
    Call UpdateDialog(frmDialogs.ClassSave)
    On Error GoTo 0
    Fname = frmDialogs.ClassSave.filename
    If (Fname <> "") Then
        PreSave
        If GResult = 1 Then
            Ed.Server.Exec "CLASS SAVE NAME=" & Quotes(CurClass) & " FILE=" & Quotes(Fname)
        Else
            Ed.Server.Exec "CLASS SAVEBELOW NAME=" & Quotes(CurClass) & " FILE=" & Quotes(Fname)
        End If
    End If
    Exit Sub
BadFilename: ' Bad filename handler:
    Fname = ""
    frmDialogs.ClassSave.filename = ""
    On Error GoTo 0
    GoTo TryAgain
Skip:
    Ed.Server.Enable
End Sub

