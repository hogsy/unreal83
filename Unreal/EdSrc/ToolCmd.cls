VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "ToolCmd"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
'---------------------------------------
' Tool class for all command tools (CMD)
'
' These tools are names so that, in
' most cases, their tool name is
' identical to the UnrealEdServer
' command line they execute when
' clicked.
'---------------------------------------

Option Explicit

Public Sub GetStatus(ByVal Tool As String, ByRef UnrealEdApp As Object, ByRef Status As String, ByRef Value As Boolean)
    Status = ""
    Value = False
End Sub

Public Sub DoHelp(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    Case "ACTOR SELECT ALL": ToolHelp (130)
    Case "POLY SELECT ALL": ToolHelp (128)
    Case "SELECT ALL": ToolHelp (402)
    Case "SELECT NONE": ToolHelp (402)
    Case "TRANSACTION UNDO": ToolHelp (409)
    Case "TRANSACTION REDO": ToolHelp (409)
    Case "TEXTURE RESET": ToolHelp (403)
    Case "BRUSH RESET": ToolHelp (401)
    Case "BRUSH ADD": ToolHelp (401)
    Case "BRUSH SUBTRACT": ToolHelp (401)
    Case "BRUSH FROM INTERSECTION": ToolHelp (401)
    Case "BRUSH FROM DEINTERSECTION": ToolHelp (401)
    Case "BRUSH ADD SPECIAL": ToolHelp (316)
    Case "BRUSH ADDMOVER": ToolHelp (401)
    Case "BRUSH FROM HULL": ToolHelp (401)
    Case "BRUSH FROM INTERIOR": ToolHelp (401)
    Case "BRUSH RESTORE": ToolHelp (401)
    Case "BRUSH MIRROR": ToolHelp (401)
    Case "TEXBROWSE": ToolHelp (123)
    Case "ACTBROWSE": ToolHelp (329)
    Case "HELP": ToolHelp (126) ' Toolbar help
    Case "TOOLS": ToolHelp (408)
    End Select
End Sub

Public Sub DoProperties(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    Case "POLY SELECT ALL": PopupNext = 1
    Case "SELECT NONE": PopupNext = 1
    Case "SELECT ALL": PopupNext = 1
    Case "TEXTURE RESET": frmSurfaceProps.Show
    Case "BRUSH RESET": frmBrush.Show
    Case "BRUSH RESTORE": frmBrush.Show
    Case "BRUSH MIRROR": frmBrush.Show
    Case "TEXBROWSE": frmSurfaceProps.Show
    Case "ACTBROWSE": frmActorProperties.Show
    End Select
End Sub

Public Sub DoClick(ByVal Tool As String, ByRef UnrealEdApp As Object)
    '
    Call Ed.Tools.Highlight(Tool, True)
    '
    Select Case (Tool)
    Case "HELP": Call Ed.Tools.Handlers(Ed.MRUTool).DoHelp(Ed.MRUTool, Ed)
    Case "TEXBROWSE":
        If Ed.BrowserPos = 2 Then
            Ed.BrowserPos = 0
            frmMain.ResizeAll (True)
        End If
        Call Ed.SetBrowserTopic("Textures")
    Case "ACTBROWSE":
        Call Ed.SetBrowserTopic("Classes")
    Case "BRUSH ADD SPECIAL": frmAddSpecial.Show
    Case "TEXTURE RESET":
        Ed.Server.Exec "POLY TEXPAN RESET"
        Ed.Server.Exec "POLY TEXSCALE"
        Ed.Server.Exec "POLY TEXALIGN DEFAULT"
    Case "BRUSH RESET":
        frmMain.PopupMenu frmPopups.ResetBrush
    Case "BRUSH MIRROR":
        frmMain.PopupMenu frmPopups.MirrorBrush
    Case Else:
        Ed.Server.Exec (Tool)
    End Select
    '
    Select Case Tool
    Case "ACTOR SELECT ALL": Call Ed.StatusText("Select All Actors")
    Case "POLY SELECT ALL": Call Ed.StatusText("Select All Textures")
    Case "SELECT ALL": Call Ed.StatusText("Select All")
    Case "SELECT NONE": Call Ed.StatusText("Select None")
    Case "TRANSACTION UNDO": Call Ed.StatusText("Undo")
    Case "TRANSACTION REDO": Call Ed.StatusText("Redo")
    Case "TEXTURE RESET": Call Ed.StatusText("Reset Textures")
    Case "BRUSH RESET": Call Ed.StatusText("Reset Brush")
    Case "BRUSH ADD": Call Ed.StatusText("Add Brush To World")
    Case "BRUSH SUBTRACT": Call Ed.StatusText("Subtract Brush From World")
    Case "BRUSH FROM INTERSECTION": Call Ed.StatusText("Intersect Brush With World")
    Case "BRUSH FROM DEINTERSECTION": Call Ed.StatusText("Deintersect Brush With World")
    Case "BRUSH FROM HULL": Call Ed.StatusText("Brush From Hull")
    Case "BRUSH FROM INTERIOR": Call Ed.StatusText("Brush From Interior")
    Case "BRUSH RESTORE": Call Ed.StatusText("Restore Brush")
    Case "BRUSH ADDMOVER": Call Ed.StatusText("Add Movable Brush")
    End Select
    '
    Call Ed.Tools.Highlight(Tool, False)
End Sub

Property Get HiPic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "POLY SELECT ALL": V = 8
    Case "ACTOR SELECT ALL": V = 9
    Case "SELECT NONE": V = 10
    Case "SELECT ALL": V = 11
    Case "TRANSACTION UNDO": V = 38
    Case "TRANSACTION REDO": V = 39
    Case "BRUSH ADD": V = 40
    Case "BRUSH SUBTRACT": V = 41
    Case "BRUSH FROM INTERSECTION": V = 42
    Case "BRUSH FROM DEINTERSECTION": V = 43
    Case "BRUSH ADD SPECIAL": V = 46
    Case "BRUSH FROM HULL": V = 46
    Case "BRUSH FROM INTERIOR": V = 45
    Case "BRUSH RESTORE": V = 44
    Case "BRUSH MIRROR": V = 28
    Case "BRUSH ADDMOVER": V = 62
    Case "TEXTURE RESET": V = 20
    Case "BRUSH RESET": V = 21
    Case "HELP": V = 37
    Case "TOOLS": V = 22
    Case "TEXBROWSE": V = 58
    Case "ACTBROWSE": V = 59
    End Select
    '
    Set HiPic = frmMain.HiPics.GraphicCell(V)
End Property

Property Get Pic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "POLY SELECT ALL": V = 8
    Case "ACTOR SELECT ALL": V = 9
    Case "SELECT NONE": V = 10
    Case "SELECT ALL": V = 11
    Case "TRANSACTION UNDO": V = 38
    Case "TRANSACTION REDO": V = 39
    Case "BRUSH ADD": V = 40
    Case "BRUSH SUBTRACT": V = 41
    Case "BRUSH FROM INTERSECTION": V = 42
    Case "BRUSH FROM DEINTERSECTION": V = 43
    Case "BRUSH ADD SPECIAL": V = 46
    Case "BRUSH FROM HULL": V = 46
    Case "BRUSH FROM INTERIOR": V = 45
    Case "BRUSH RESTORE": V = 44
    Case "BRUSH MIRROR": V = 28
    Case "BRUSH ADDMOVER": V = 62
    Case "TEXTURE RESET": V = 20
    Case "BRUSH RESET": V = 21
    Case "HELP": V = 37
    Case "TOOLS": V = 22
    Case "TEXBROWSE": V = 58
    Case "ACTBROWSE": V = 59
    End Select
    '
    Set Pic = frmMain.Pics.GraphicCell(V)
End Property

Public Sub InitTools(UnrealEdApp As UnrealEdApp)
    Call UnrealEdApp.Tools.Register(Me, "ACTOR SELECT ALL", "Select all actors", "")
    Call UnrealEdApp.Tools.Register(Me, "POLY SELECT ALL", "Select all polys", "")
    Call UnrealEdApp.Tools.Register(Me, "SELECT ALL", "Select all", "")
    Call UnrealEdApp.Tools.Register(Me, "SELECT NONE", "Select none", "")
    Call UnrealEdApp.Tools.Register(Me, "TRANSACTION UNDO", "Undo last operation", "")
    Call UnrealEdApp.Tools.Register(Me, "TRANSACTION REDO", "Redo operation", "")
    Call UnrealEdApp.Tools.Register(Me, "TEXTURE RESET", "Reset textures", "Surface properties...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH RESET", "Reset brush", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH ADD", "Add brush to world", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH SUBTRACT", "Subtract brush from world", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH FROM INTERSECTION", "Brush intersection", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH FROM DEINTERSECTION", "Brush deintersection", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH ADD SPECIAL", "Add Special Brush", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH FROM HULL", "Brush hull", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH FROM INTERIOR", "Brush interior", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH RESTORE", "Restore brush", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH MIRROR", "Mirror brush", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH ADDMOVER", "Add movable brush", "")
    Call UnrealEdApp.Tools.Register(Me, "TEXBROWSE", "Texture browser", "Surface properties...")
    Call UnrealEdApp.Tools.Register(Me, "ACTBROWSE", "Actor Class browser", "Actor properties...")
    Call UnrealEdApp.Tools.Register(Me, "HELP", "Get Help", "")
    Call UnrealEdApp.Tools.Register(Me, "TOOLS", "Add-On Tools", "")
End Sub
