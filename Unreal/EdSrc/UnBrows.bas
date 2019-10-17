Attribute VB_Name = "Browsers"
'
' Various support routines for browsers
'
Option Explicit
'
Dim Bogus As String
Dim Spacer As String
Dim Remove As String
Dim TempExpand As Integer
'
Public QuerySource As Integer

Sub InitOutline()
    Bogus = "Loading..." + Chr(9) + Chr(9) + Chr(9) + Chr(9) + Chr(9) + "*"
    Spacer = Chr(9) + Chr(9) + Chr(9) + Chr(9) + Chr(9)
End Sub

Sub ExpandOutline(List As Control, _
    Topic As String, Item As String, _
    Query As String, _
    ListIndex As Integer, _
    ForceExpand)
    '
    Dim i As Integer, j As Integer
    Dim Text As String, Temp As String
    '
    If TempExpand Then
        Exit Sub ' Ignore
    ElseIf Left(List.List(ListIndex), 1) = Bogus Then
        Exit Sub ' Ignore
    End If
    '
    If ForceExpand Then
        Text = List.FullPath(ListIndex) + List.PathSeparator
        j = Len(Text)
        For i = List.ListCount - 1 To 0 Step -1
            If Left(List.FullPath(i), j) = Text Then
                List.RemoveItem i
            End If
        Next
        GoTo DoExpand
    End If
    '
    ' See if this node has a bogus child.  If not,
    ' it has already been expanded:
    '
    Temp = Bogus + List.List(ListIndex)
    For j = 0 To List.ListCount - 1
        If List.List(j) = Temp Then
            GoTo DoExpand
        End If
    Next j
    GoTo Out ' Didn't find bogus child; already expanded
    '
DoExpand:
    '
    DisableRedraw (List.hwnd)
    Screen.MousePointer = 11
    List.Expand(ListIndex) = False
    '
    Text = List.List(ListIndex)
    '
    ' Remember to replace bogus child
    '
    Remove = Bogus & Text
    QuerySource = ListIndex
    '
    Ed.Server.Exec Query
    Call UpdateOutline(List, Topic, Item)
Out:
    Screen.MousePointer = 0
    EnableRedraw (List.hwnd)
End Sub

Private Sub ProcessOutlineResult(List As Control, Source As String)
    Dim j As Integer
    Dim c As Integer
    Dim Descr As String
    '
    If (QuerySource <> -1) Then
        If (Trim(Source) <> "") Then
            '
            Descr = Source
            j = InStr(Descr, "|")
            If j <> 0 Then
                Descr = Left(Descr, j - 1) + Chr(9) + Chr(9) + Chr(9) + Chr(9) + Chr(9) + Mid(Descr, j + 1)
            End If
            '
            If (Remove <> "") Then
                For j = 0 To List.ListCount - 1
                    If List.List(j) = Remove Then
                        List.List(j) = Descr
                        Remove = ""
                        c = j
                        GoTo Skip
                    End If
                Next j
            End If
            '
            ' Add new item:
            '
            c = List.ListCount
            List.ListIndex = QuerySource
            List.AddItem Descr
            '
            For j = 0 To List.ListCount - 1
                If List.List(j) = Descr Then
                    c = j
                    GoTo Skip
                End If
            Next
Skip:       '
            ' If resource has children, add a bogus entry
            ' starting with a '*' to catch them when
            ' the list is expanded:
            '
            If Right(Source, 1) = "C" Then
                List.ListIndex = c
                List.AddItem Bogus & List.List(c)
            End If
        Else
            '
            ' Expand list, preventing recursion:
            '
            TempExpand = TempExpand + 1
            List.Expand(QuerySource) = True
            List.ListIndex = QuerySource
            TempExpand = TempExpand - 1
        End If
    End If
End Sub

Sub UpdateOutline(List As Control, Topic As String, Item As String)
    Dim Text As String
    '
    Do
        Text = Trim(Ed.Server.GetProp(Topic, Item))
        Call ProcessOutlineResult(List, Text)
    Loop Until Text = ""
End Sub

