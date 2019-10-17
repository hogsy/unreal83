VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "ToolBrush"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
'---------------------------------------
' Tool class for all brush tools (BRUSH)
'---------------------------------------

Option Explicit
Private Count As Integer ' Hack for triggering brush forms

Public Sub GetStatus(ByVal Tool As String, ByRef UnrealEdApp As Object, ByRef Status As String, ByRef Value As Boolean)
    Status = ""
    Value = False
End Sub

Public Sub DoHelp(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    Case "RECTANGLE": ToolHelp (153)
    Case "SPHERE": ToolHelp (154)
    Case "CYLINDER": ToolHelp (156)
    Case "CONE": ToolHelp (150)
    Case "STAIR": ToolHelp (152)
    Case "SPIRAL": ToolHelp (155)
    Case "LANDSCAPE": ToolHelp (300) ' Unused
    Case "2DEDITOR": ToolHelp (127) ' 2D editor overview
    Case "CURVEDSTAIR": ToolHelp (151)
    Case "SHEET": ToolHelp (330)
    Case "LOAD": ToolHelp (401) ' Brush overview
    End Select
End Sub

Public Sub DoProperties(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    Case "RECTANGLE": frmParSolRect.Show
    Case "SPHERE": frmParSolSphere.Show
    Case "CYLINDER": frmParSolTube.Show
    Case "CONE": frmParSolCone.Show
    Case "STAIR": frmParSolLinearStair.Show
    Case "SPIRAL": frmParSolSpiralStair.Show
    Case "LANDSCAPE": frmParSolHeightMap.Show
    Case "2DEDITOR": frmTwoDee.Show
    Case "CURVEDSTAIR": frmParSolCurvedStair.Show
    Case "SHEET": frmPsSheet.Show
    End Select
End Sub

Public Sub DoClick(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Dim BrushFname As String
    '
    ' Uses a hack to trigger the solid form's "Build"
    ' button:
    '
    Call Ed.Tools.Highlight(Tool, True)
    '
    Select Case (Tool)
    Case "RECTANGLE": frmParSolRect.Trigger = Count
    Case "SPHERE": frmParSolSphere.Trigger = Count
    Case "CYLINDER": frmParSolTube.Trigger = Count
    Case "CONE": frmParSolCone.Trigger = Count
    Case "STAIR": frmParSolLinearStair.Trigger = Count
    Case "SPIRAL": frmParSolSpiralStair.Trigger = Count
    Case "LANDSCAPE": frmParSolHeightMap.Trigger = Count
    Case "CURVEDSTAIR": frmParSolCurvedStair.Trigger = Count
    Case "SHEET": frmPsSheet.Trigger = Count
    Case "LOAD":
        '
        ' Dialog for "Open Brush":
        '
        On Error GoTo Skip
        Ed.Server.Disable
        frmDialogs.BrushOpen.filename = ""
        frmDialogs.BrushOpen.ShowOpen
        '
        Call UpdateDialog(frmDialogs.BrushOpen)
        If (frmDialogs.BrushOpen.filename <> "") Then
            '
            ' Load the brush
            '
            BrushFname = frmDialogs.BrushOpen.filename
            Ed.Server.Exec "BRUSH LOAD FILE=" & Quotes(BrushFname)
Skip:
        Ed.Server.Enable
        End If
    Case "2DEDITOR": frmTwoDee.Show
    End Select
    '
    Call Ed.Tools.Highlight(Tool, False)
    Count = Count + 1
End Sub

Property Get HiPic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "RECTANGLE": V = 49
    Case "SPHERE": V = 50
    Case "CYLINDER": V = 51
    Case "CONE": V = 52
    Case "STAIR": V = 53
    Case "SPIRAL": V = 54
    Case "LANDSCAPE": V = 55
    Case "2DEDITOR": V = 55 ' TEMPORARY
    Case "CURVEDSTAIR": V = 56
    Case "SHEET": V = 60
    Case "LOAD": V = 57
    End Select
    '
    Set HiPic = frmMain.HiPics.GraphicCell(V)
End Property

Property Get Pic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "RECTANGLE": V = 49
    Case "SPHERE": V = 50
    Case "CYLINDER": V = 51
    Case "CONE": V = 52
    Case "STAIR": V = 53
    Case "SPIRAL": V = 54
    Case "LANDSCAPE": V = 55
    Case "2DEDITOR": V = 55 ' TEMPORARY
    Case "CURVEDSTAIR": V = 56
    Case "SHEET": V = 60
    Case "LOAD": V = 57
    End Select
    '
    Set Pic = frmMain.Pics.GraphicCell(V)
End Property

Public Sub InitTools(ByRef UnrealEdApp As Object)
    Call UnrealEdApp.Tools.Register(Me, "RECTANGLE", "Build a rectangle", "Rectangle Properties...")
    Call UnrealEdApp.Tools.Register(Me, "SPHERE", "Build a sphere", "Sphere Properties...")
    Call UnrealEdApp.Tools.Register(Me, "CYLINDER", "Build a cylinder", "Cylinder Properties...")
    Call UnrealEdApp.Tools.Register(Me, "CONE", "Build a cone", "Cone Properties...")
    Call UnrealEdApp.Tools.Register(Me, "STAIR", "Build a stair", "Stair Properties...")
    Call UnrealEdApp.Tools.Register(Me, "SPIRAL", "Build a spiral stair", "Spiral Stair Properties...")
    Call UnrealEdApp.Tools.Register(Me, "LANDSCAPE", "Build landscape", "Landscape properties...")
    Call UnrealEdApp.Tools.Register(Me, "2DEDITOR", "2D Shape Editor", "")
    Call UnrealEdApp.Tools.Register(Me, "CURVEDSTAIR", "Build a curved stair", "Curved Stair Properties...")
    Call UnrealEdApp.Tools.Register(Me, "SHEET", "Build a sheet", "Sheet Properties...")
    Call UnrealEdApp.Tools.Register(Me, "LOAD", "Load a brush", "")
End Sub

