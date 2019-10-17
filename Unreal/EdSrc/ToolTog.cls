VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "ToolToggle"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
'-----------------------------------------
' Tool class for all toggle tools (TOGGLE)
'-----------------------------------------

Option Explicit

Public Sub GetStatus(ByVal Tool As String, ByRef UnrealEdApp As Object, ByRef Status As String, ByRef Value As Boolean)
    Select Case (Tool)
    Case "GRID":
        Value = Ed.GridMode
        Status = "Grid is " + OnOff(Value)
    Case "ROTGRID":
        Value = Ed.RotGridMode
        Status = "Rotation grid is " + OnOff(Value)
    Case "SPEED":
        If Ed.SpeedMode = 0 Then
            Status = "Slow speed"
        ElseIf Ed.SpeedMode = 1 Or Ed.SpeedMode = 3 Then
            Status = "Medium speed"
        Else
            Status = "Fast speed"
        End If
        Value = False
    Case "SNAPVERTEX":
        Value = Ed.SnapVertex
        Status = "Vertex snapping is " + OnOff(Value)
    Case "SHOWVERTICES":
        Value = Ed.ShowVertices
        Status = "Show vertices is " + OnOff(Value)
    Case "MAPEDIT":
        Value = Ed.MapEdit
        Status = "Map edit mode is " + OnOff(Value)
    End Select
    '
    Call Ed.StatusText(Status)
End Sub

Public Sub DoHelp(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    Case "GRID": ToolHelp (111)
    Case "ROTGRID": ToolHelp (112)
    Case "SNAPVERTEX": ToolHelp (405)
    Case "SHOWVERTICES": ToolHelp (405)
    Case "MAPEDIT": ToolHelp (114)
    Case "SPEED": ToolHelp (315)
    End Select
End Sub

Public Sub DoProperties(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    Case "GRID": frmGrid.Show
    Case "ROTGRID": frmRotGrid.Show
    Case "SNAPVERTEX": frmGrid.Show
    Case "SHOWVERTICES": frmGrid.Show
    Case "MAPEDIT": frmRebuilder.Show
    End Select
End Sub

Public Sub DoClick(ByVal Tool As String, ByRef UnrealEdApp As Object)
    '
    Dim Highlight As Boolean
    Dim Temp As String
    '
    Select Case (Tool)
    Case "GRID":
        Ed.GridMode = Not Ed.GridMode
        Ed.Server.Exec "MODE GRID=" + OnOff(Ed.GridMode)
        Ed.ResetGridSize
    Case "ROTGRID":
        Ed.RotGridMode = Not Ed.RotGridMode
        Ed.Server.Exec "MODE ROTGRID=" + OnOff(Ed.RotGridMode)
    Case "SNAPVERTEX":
        Ed.SnapVertex = Not Ed.SnapVertex
        Ed.Server.Exec "MODE SNAPVERTEX=" + OnOff(Ed.SnapVertex)
    Case "SHOWVERTICES":
        Ed.ShowVertices = Not Ed.ShowVertices
        Ed.Server.Exec "MODE SHOWVERTICES=" + OnOff(Ed.ShowVertices)
    Case "MAPEDIT":
        If Ed.MapEdit = 0 Then
            If (MsgBox("Editing the map will invalidate the BSP, so you will have to rebuild it.", 1 + 48, "Map Mode") = 1) Then
                Ed.MapEdit = 1
                frmMapToolbar.Show
                Ed.Server.Exec "MAP EDIT=ON"
            End If
        Else
            frmMapToolbar.Hide
            Ed.MapEdit = 0
            Ed.Server.Exec "MAP EDIT=OFF"
            frmRebuilder.Show
        End If
        frmMain.MapEditMode.Checked = Ed.MapEdit <> 0
    Case "SPEED":
        Ed.SpeedMode = Ed.SpeedMode + 1
        If Ed.SpeedMode > 2 Then Ed.SpeedMode = 0
        '
        If Ed.SpeedMode = 0 Then
            Ed.Server.Exec "MODE SPEED=1"
        ElseIf Ed.SpeedMode = 1 Or Ed.SpeedMode = 3 Then
            Ed.Server.Exec "MODE SPEED=4"
        Else
            Ed.Server.Exec "MODE SPEED=16"
        End If
        ' Update picture
        Call Ed.Tools.ForceNewPictures("SPEED", _
            frmMain.Pics.GraphicCell(29 + Ed.SpeedMode), _
            frmMain.HiPics.GraphicCell(29 + Ed.SpeedMode))
    End Select
    '
    ' Set new highlighting:
    '
    Call GetStatus(Tool, UnrealEdApp, Temp, Highlight)
    Call Ed.Tools.Highlight(Tool, Highlight)
    '
End Sub

Property Get HiPic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "GRID": V = 32
    Case "ROTGRID": V = 35
    Case "SPEED": V = 30
    Case "SNAPVERTEX": V = 26
    Case "SHOWVERTICES": V = 24
    Case "MAPEDIT": V = 23
    End Select
    '
    Set HiPic = frmMain.HiPics.GraphicCell(V)
End Property

Property Get Pic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "GRID": V = 32
    Case "ROTGRID": V = 35
    Case "SPEED": V = 30
    Case "SNAPVERTEX": V = 26
    Case "SHOWVERTICES": V = 24
    Case "MAPEDIT": V = 23
    End Select
    '
    Set Pic = frmMain.Pics.GraphicCell(V)
End Property

Public Sub InitTools(UnrealEdApp As UnrealEdApp)
    Call UnrealEdApp.Tools.Register(Me, "GRID", "Toggle grid mode", "Grid Settings...")
    Call UnrealEdApp.Tools.Register(Me, "ROTGRID", "Toggle rotation grid", "Rotation Grid Settings...")
    Call UnrealEdApp.Tools.Register(Me, "SNAPVERTEX", "Toggle vertex snap", "Grid Settings...")
    Call UnrealEdApp.Tools.Register(Me, "SHOWVERTICES", "Toggle vertex visiblity", "")
    Call UnrealEdApp.Tools.Register(Me, "MAPEDIT", "Toggle map edit mode", "Rebuild...")
    Call UnrealEdApp.Tools.Register(Me, "SPEED", "Camera speed", "")
End Sub
