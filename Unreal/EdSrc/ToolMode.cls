VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "ToolMode"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
'---------------------------------------
' Tool class for all mode tools (MODE)
'---------------------------------------

Option Explicit

Public Sub GetStatus(ByVal Tool As String, ByRef UnrealEdApp As Object, ByRef Status As String, ByRef Value As Boolean)
    Status = ""
    '
    ' !!Should set value of this is the current mode!!
    '
    Value = False
End Sub

Public Sub DoHelp(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    Case "CAMERAMOVE": ToolHelp (301)
    Case "CAMERAZOOM": ToolHelp (302)
    Case "BRUSHFREE": ToolHelp (303)
    Case "BRUSHMOVE": ToolHelp (304)
    Case "BRUSHROTATE": ToolHelp (305)
    Case "BRUSHSHEER": ToolHelp (306)
    Case "BRUSHSCALE": ToolHelp (307)
    Case "BRUSHSNAP": ToolHelp (331)
    Case "BRUSHSTRETCH": ToolHelp (308)
    Case "ADDACTOR": ToolHelp (309)
    Case "ADDLIGHT": ToolHelp (309)
    Case "MOVEACTOR": ToolHelp (310)
    Case "TEXTUREPAN": ToolHelp (311)
    Case "TEXTURESET": ToolHelp (312)
    Case "TEXTUREROTATE": ToolHelp (313)
    Case "TEXTURESCALE": ToolHelp (314)
    Case "TERRAFORM": ToolHelp (323)
    End Select
End Sub

Public Sub DoProperties(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    Case "BRUSHFREE": frmBrush.Show
    Case "BRUSHMOVE": frmBrush.Show
    Case "BRUSHROTATE": frmBrush.Show
    Case "BRUSHSHEER": frmBrush.Show
    Case "BRUSHSCALE": frmBrush.Show
    Case "BRUSHSNAP": frmBrush.Show
    Case "BRUSHSTRETCH": frmBrush.Show
    Case "TEXTUREPAN": frmSurfaceProps.Show
    Case "TEXTURESET": frmSurfaceProps.Show
    Case "TEXTUREROTATE": frmSurfaceProps.Show
    Case "TEXTURESCALE": frmSurfaceProps.Show
    Case "ADDACTOR": frmClassBrowser.Show
    Case "ADDLIGHT": frmClassBrowser.Show
    Case "MOVEACTOR": frmActorProperties.Show
    Case "TERRAFORM": frmTerrain.Show
    End Select
End Sub

Public Sub DoClick(ByVal Tool As String, ByRef UnrealEdApp As Object)
    '
    Dim i As Integer
    '
    ' Unhighlight previous mode:
    '
    Call Ed.Tools.Highlight(Ed.ToolMode, False)
    '
    Select Case (Tool)
    Case "ADDLIGHT":
        Ed.Server.Exec "MODE ADDACTOR"
        Ed.ToolMode = "MODE:" + Tool
        '
        frmMain.ActorCombo.List(0) = "Light"
        frmMain.ActorCombo.ListIndex = 0
        Ed.Server.Exec "ACTOR SET ADDCLASS=LIGHT"
    Case Else:
        Ed.Server.Exec "MODE " + Tool
        Ed.ToolMode = "MODE:" + Tool
    End Select
    '
    ' Set and highlight new mode:
    '
    Ed.ToolMode = Tool
    Call Ed.Tools.Highlight(Ed.ToolMode, True)
    '
    Select Case (Tool)
        Case "CAMERAMOVE": i = 0
        Case "CAMERAZOOM": i = 1
        Case "BRUSHFREE": i = 0
        Case "BRUSHMOVE": i = 0
        Case "BRUSHROTATE": i = 2
        Case "BRUSHSHEER": i = 4
        Case "BRUSHSCALE": i = 3
        Case "BRUSHSTRETCH": i = 5
        Case "BRUSHSNAP": i = 6
        Case "ADDACTOR": i = 7
            If Ed.CurBrowserTopic = "Classes" Then
                Ed.Server.Exec "ACTOR SET ADDCLASS=" & frmClassBrowser.GetCurrent()
            Else
                Ed.Server.Exec "ACTOR SET ADDCLASS=" & frmMain.ActorCombo.List(0)
            End If
        Case "ADDLIGHT": i = 8
        Case "MOVEACTOR": i = 9
        Case "TEXTUREPAN": i = 10
        Case "TEXTURESET": i = 10
        Case "TEXTUREROTATE": i = 11
        Case "TEXTURESCALE": i = 10
        Case "TERRAFORM": i = 12
    End Select
    '
    If Not GSettingMode Then
        GSettingMode = 1
        frmMain.ModeCombo.ListIndex = i
        GSettingMode = 0
    End If
End Sub

Property Get HiPic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "CAMERAMOVE": V = 0
    Case "CAMERAZOOM": V = 1
    Case "BRUSHFREE": V = 2
    Case "BRUSHMOVE": V = 3
    Case "BRUSHROTATE": V = 4
    Case "BRUSHSHEER": V = 5
    Case "BRUSHSCALE": V = 6
    Case "BRUSHSTRETCH": V = 7
    Case "BRUSHSNAP": V = 27
    Case "ADDACTOR": V = 15
    Case "ADDLIGHT": V = 14
    Case "MOVEACTOR": V = 13
    Case "TEXTUREPAN": V = 16
    Case "TEXTURESET": V = 17
    Case "TEXTUREROTATE": V = 18
    Case "TEXTURESCALE": V = 19
    Case "TERRAFORM": V = 55
    End Select
    '
    Set HiPic = frmMain.HiPics.GraphicCell(V)
End Property

Property Get Pic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "CAMERAMOVE": V = 0
    Case "CAMERAZOOM": V = 1
    Case "BRUSHFREE": V = 2
    Case "BRUSHMOVE": V = 3
    Case "BRUSHROTATE": V = 4
    Case "BRUSHSHEER": V = 5
    Case "BRUSHSCALE": V = 6
    Case "BRUSHSTRETCH": V = 7
    Case "BRUSHSNAP": V = 27
    Case "ADDACTOR": V = 15
    Case "ADDLIGHT": V = 14
    Case "MOVEACTOR": V = 13
    Case "TEXTUREPAN": V = 16
    Case "TEXTURESET": V = 17
    Case "TEXTUREROTATE": V = 18
    Case "TEXTURESCALE": V = 19
    Case "TERRAFORM": V = 55
    End Select
    '
    Set Pic = frmMain.Pics.GraphicCell(V)
End Property

Public Sub InitTools(UnrealEdApp As UnrealEdApp)
    Call UnrealEdApp.Tools.Register(Me, "CAMERAMOVE", "Move camera", "")
    Call UnrealEdApp.Tools.Register(Me, "CAMERAZOOM", "Zoom camera", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSHFREE", "Move brush freeform", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSHMOVE", "Move brush axially", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSHROTATE", "Rotate brush", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSHSHEER", "Sheer brush", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSHSCALE", "Scale brush", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSHSTRETCH", "Stretch brush", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "ADDACTOR", "Add new actor", "Class browser...")
    Call UnrealEdApp.Tools.Register(Me, "ADDLIGHT", "Add new light", "Class browser...")
    Call UnrealEdApp.Tools.Register(Me, "MOVEACTOR", "Move actor/light", "Actor properties...")
    Call UnrealEdApp.Tools.Register(Me, "TEXTUREPAN", "Pan textures", "Surface properties...")
    Call UnrealEdApp.Tools.Register(Me, "TEXTURESET", "Set textures", "Surface properties...")
    Call UnrealEdApp.Tools.Register(Me, "TEXTUREROTATE", "Rotate textures", "Surface properties...")
    Call UnrealEdApp.Tools.Register(Me, "TEXTURESCALE", "Scale textures", "Surface properties...")
    Call UnrealEdApp.Tools.Register(Me, "TERRAFORM", "Edit Terrain", "Terrain properties...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSHSNAP", "SnapScale Brush", "Brush properties...")
End Sub
