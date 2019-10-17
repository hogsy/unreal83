VERSION 1.0 CLASS
BEGIN
  MultiUse = 0   'False
END
Attribute VB_Name = "UnrealEdApp"
Attribute VB_Creatable = True
Attribute VB_Exposed = True
'---------------------------------------------
'    Ed OLE automation object
'---------------------------------------------
'
' This contains all public modules and
' variables other programs can access in
' order to create a new instance of the
' UnrealEditor for exclusive access
' via OLE.
'
' This is meant for utilities which want to
' load the editor, do something, and shut it
' down.  Add-on tools, which should coexist with
' other tools and a user, should create
' UnrealEditor objects instead.
'
' This is a creatable, single-use OLE object.
'
Option Explicit

'-----------------
' Public variables
'-----------------

'
' Embedded objects
'
Public Tools As UnrealTools
Public Server As UnrealEdServer

'
' App names
'
Public ServerAppName As String ' "Unreal"
Public EditorAppName As String ' "UnrealEd"

'
' Filenames
'
Public MacroFname As String
Public MacroRecording As Integer
Public MapFname As String ' Map filename
Public BrushFname As String ' Brush filename
Public Startup As Integer ' 1 when loading
Public AutoSaveCountup As Integer

'
' State info
'
Public Version As String
Public Licensed As Integer
Public GodMode As Integer
Public hWndEditor As Long

'
' Editor mode info
'
Public ToolMode As String
Public MRUTool As String
Public GridMode As Boolean
Public RotGridMode As Boolean
Public SpeedMode As Integer
Public SnapVertex As Boolean
Public ShowVertices As Boolean
Public MapEdit As Integer
Public ToolbarPos As Integer  '0=left, 1=rt
Public BrowserPos As Integer  '0=right, 1=left, 2=hide
Public PanelButton As Integer
Public BrowserButton As Integer
'
Public GridX As Integer
Public GridY As Integer
Public GridZ As Integer
Public AutoSaveTime As Integer ' Minutes
Public InitialFiles As String
Public InitialMacro As String

Public CameraVertRatio As Single
Public CameraLeftRatio As Single
Public CameraRightRatio As Single
Public FOV As Single

'
' Profile directories
'
Public BaseDir     As String ' Base editor directory
Public MapDir      As String ' Maps
Public TextureDir  As String ' Textures
Public ToolDir     As String ' Add-on tools (UTL)
Public ClassDir    As String ' Classes
Public ShapeDir    As String ' 2D Shapes
Public BrushDir    As String ' 3D Solids
Public ModelDir    As String ' 3D Model animations
Public SoundDir    As String ' Sounds
Public MusicDir    As String ' Music
Public MacroDir    As String ' Macros

'-----------------
' Private
'-----------------

'
' Default camera start locations to prevent newly
' opened cameras from being on top of old ones.
'
Private DefCamX As Integer
Private DefCamY As Integer
Private SettingGrid As Integer
Private Const GWW_HWNDPARENT As Integer = -8 ' For SetWindowWord

Private Type WindowRec
    Left As Long
    Top As Long
    Width As Long
    Height As Long
    Name As String
    Loaded As Long
    TheForm As Form
End Type

Private Const MAX_WINDOWS = 200 ' Maximum windows for SetOnTop to track
Dim NumWindowRecs As Integer
Dim WindowRecs(MAX_WINDOWS) As WindowRec

Private Type BrowserTopic
    BrowserForm As Variant
    TopicName As String
    Loaded As Boolean
End Type

Private Const MAX_TOPICS = 20
Private NumBrowserTopics As Integer
Private BrowserTopics(MAX_TOPICS) As BrowserTopic
Public CurBrowserTopic As String
Public InitialBrowserTopic As String
Private SettingTopic As Integer

'
' Initialize all class-specific information:
'
Private Sub Class_Initialize()
    '
    ' Create embedded objects:
    '
    Set Tools = New UnrealTools
    '
    ' Initialize globals:
    '
    Version = "0.48"
    ServerAppName = "Unreal"
    EditorAppName = "UnrealEd"
    hWndEditor = 0
    '
    ToolMode = "CAMERAMOVE"
    MRUTool = "CAMERAMOVE"
    SpeedMode = 1
    GridMode = False
    RotGridMode = False
    SnapVertex = True
    ShowVertices = True
    MapEdit = False
    '
    GridX = 10
    GridY = 10
    GridZ = 10
    '
End Sub

'
' Launch the UnrealServer
'
Public Sub LaunchServer(hWndMain As Long)
    '
    Set Server = New UnrealEdServer
    Server.Exec "APP SET HWND=" & Trim(Str(hWndMain))
    hWndEditor = hWndMain
    '
End Sub

'
' Shut the object down
'
Private Sub Class_Terminate()
    '
End Sub

'
' CreateNewLevel: Close all level-related windows
' and create a new level.
'
Public Sub CreateNewLevel(Title As String, Creator As String)
    '
End Sub


'
' Make sure the editor is visible
'
Public Sub ShowEditor()
    '
End Sub

'
' Hide the editor windows
'
Public Sub HideEditor()
    '
End Sub

Sub CloseLevelWindows()
    '
    ' Close all windows related to the current level.
    ' Done before loading or creating new levels so that
    ' old information doesn't get sucked into new maps
    ' via DDE.
    '
    Unload frmLevel
    '
End Sub

'
' Open a camera with specified properties.
'    X,Y   = location on screen, in pixels
'    XR,YR = X,Y size of window, including border and title
'    Menu  = 1 for camera to have a menu, 0 for none.
'
Public Sub OpenCamera(Reopen As Boolean, _
    X As Integer, y As Integer, _
    XR As Integer, YR As Integer, _
    Flags As Long, _
    RenType As Long, Name As String)
    '
    Dim Cmd As String
    '
    Cmd = "CAMERA OPEN"
    '
    Cmd = Cmd & " HWND=" & Trim(Str(frmMain.CameraHolder.hwnd))
    Cmd = Cmd & " X=" & Trim(Str(X))
    Cmd = Cmd & " Y=" & Trim(Str(y))
    Cmd = Cmd & " XR=" & Trim(Str(XR))
    Cmd = Cmd & " YR=" & Trim(Str(YR))
    Cmd = Cmd & " FOV=" & Trim(Str(Ed.FOV))
    '
    If Not Reopen Then
        Cmd = Cmd & " FLAGS=" & Trim(Str(Flags))
        Cmd = Cmd & " REN=" & Trim(Str(RenType))
    End If
    '
    If Name <> "" Then
        Cmd = Cmd & " NAME=" & Quotes(Name)
    End If
    '
    Server.Exec Cmd
    '
End Sub

'
' Open a new camera with the default parameters and
' make it a child window of the editor's main window.
'
Public Sub OpenFreeCamera()
    '
    If Screen.Width = 640 * Screen.TwipsPerPixelX Then
        Call Ed.OpenCamera(0, DefCamX + 112 + 13, DefCamY + 76, 320, 200, SHOW_NORMAL + SHOW_MENU + SHOW_BACKDROP, REN_PLAINTEX, "")
    ElseIf Screen.Width = 800 * Screen.TwipsPerPixelX Then
        Call Ed.OpenCamera(0, DefCamX + 112 + 13, DefCamY + 76, 400, 300, SHOW_NORMAL + SHOW_MENU + SHOW_BACKDROP, REN_PLAINTEX, "")
    ElseIf Screen.Width = 1280 * Screen.TwipsPerPixelX Then
        Call Ed.OpenCamera(0, DefCamX + 112, DefCamY + 76, 640, 480, SHOW_NORMAL + SHOW_MENU + SHOW_BACKDROP, REN_PLAINTEX, "")
    Else ' Assume 1024x768
        Call Ed.OpenCamera(0, DefCamX + 112, DefCamY + 76, 560, 400, SHOW_NORMAL + SHOW_MENU + SHOW_BACKDROP, REN_PLAINTEX, "")
    End If ' 76
    '
    DefCamX = DefCamX + 32
    If DefCamX > 400 Then
        DefCamX = 0
    End If
    '
    DefCamY = DefCamY + 32
    If DefCamY > 300 Then
        DefCamX = 0
    End If
    '
End Sub

'
' Set the editor's grid size
'
Sub SetGridSize(X As Integer, y As Integer, _
    Z As Integer)
    '
    Server.Exec "MAP GRID X=" & Trim(Str(X)) & " Y=" & Trim(Str(y)) & " Z=" & Trim(Str(Z))
    '
    GridX = X
    GridY = y
    GridZ = Z
    '
    Call ResetGridSize
End Sub

'
' Update the user interface to reflect the current
' grid size
'
Sub ResetGridSize()
    If SettingGrid = 0 Then
        SettingGrid = 1
        '
        If Not GridMode Then frmMain.GridCombo.ListIndex = 0
        '
        If GridX = GridY And GridX = GridZ Then
            Select Case GridX
                Case 1:
                    frmGrid.G1.Value = True
                    If GridMode Then frmMain.GridCombo.ListIndex = 1
                Case 2:
                    frmGrid.G2.Value = True
                    If GridMode Then frmMain.GridCombo.ListIndex = 2
                Case 4:
                    frmGrid.G4.Value = True
                    If GridMode Then frmMain.GridCombo.ListIndex = 3
                Case 8:
                    frmGrid.G8.Value = True
                    If GridMode Then frmMain.GridCombo.ListIndex = 4
                Case 16:
                    frmGrid.G16.Value = True
                    If GridMode Then frmMain.GridCombo.ListIndex = 5
                Case 32:
                    frmGrid.G32.Value = True
                    If GridMode Then frmMain.GridCombo.ListIndex = 6
                Case 64:
                    frmGrid.G64.Value = True
                    If GridMode Then frmMain.GridCombo.ListIndex = 7
                Case 128:
                    frmGrid.G128.Value = True
                    If GridMode Then frmMain.GridCombo.ListIndex = 8
                Case 256:
                    frmGrid.G256.Value = True
                    If GridMode Then frmMain.GridCombo.ListIndex = 9
                Case Else:
                    frmGrid.Option2.Value = True
                    frmGrid.Size.Text = Trim(GridX)
                    If GridMode Then frmMain.GridCombo.ListIndex = 10
            End Select
        Else
            frmGrid.Option6.Value = True
            frmGrid.SX.Text = Trim(GridX)
            frmGrid.SY.Text = Trim(GridY)
            frmGrid.SZ.Text = Trim(GridZ)
            frmMain.GridCombo.ListIndex = 10
        End If
        SettingGrid = 0
     End If
End Sub

'
' Turn grid on or off
'
Sub SetGridMode(OnOff As Boolean)
    If GridMode <> OnOff Then
        Call Tools.Handlers("GRID").DoClick("GRID", Ed)
    Else
        Call ResetGridSize
    End If
End Sub

'
' Begin a slow task.  Sets the mouse cursor
' to the hourglass shape and raises the OLE
' timeout period.
'
Public Sub BeginSlowTask(Descr As String)
    StatusText (Descr)
    Screen.MousePointer = 11
    App.OleRequestPendingTimeout = 5# * 60# * 1000#
    App.OleServerBusyTimeout = 5# * 60# * 1000#
End Sub

'
' End a slow task.  Restores the mouse cursor
' and OLE timeout period.
'
Public Sub EndSlowTask()
    Screen.MousePointer = 0
    App.OleRequestPendingTimeout = 5# * 1000#
    App.OleServerBusyTimeout = 5# * 1000#
End Sub

Public Sub SaveProfile()
    Dim S As String
    Dim X As Integer
    Dim i As Integer
    '
    ' Prototype:
    '
    OpenReg ("UnrealEd")
    '
    SetUserHeading ("Directories")
    Call SetUserInfo("Maps", MapDir)
    Call SetUserInfo("Textures", TextureDir)
    Call SetUserInfo("Tools", ToolDir)
    Call SetUserInfo("Classes", ClassDir)
    Call SetUserInfo("Shapes", ShapeDir)
    Call SetUserInfo("Brushes", BrushDir)
    Call SetUserInfo("Models", ModelDir)
    Call SetUserInfo("Sounds", SoundDir)
    Call SetUserInfo("Music", MusicDir)
    '
    Call SetUserHeading("Preferences")
    Call SetUserInfo("AutoSaveTime", Str(AutoSaveTime))
    Call SetUserInfo("InitialFiles060", InitialFiles)
    Call SetUserInfo("InitialMacro", InitialMacro)
    Call SetUserInfo("ToolbarPos", Str(ToolbarPos))
    Call SetUserInfo("BrowserPos", Str(BrowserPos))
    Call SetUserInfo("PanelButton", Str(PanelButton))
    Call SetUserInfo("BrowserButton", Str(BrowserButton))
    Call SetUserInfo("CameraVertRatio", Str(CameraVertRatio))
    Call SetUserInfo("CameraLeftRatio", Str(CameraLeftRatio))
    Call SetUserInfo("CameraRightRatio", Str(CameraRightRatio))
    Call SetUserInfo("FOV", Str(FOV))
    Call SetUserInfo("BrowserTopic", InitialBrowserTopic)
    Call SetUserInfo("GodMode", Str(GodMode))
    '
    S = ""
    Call SetUserHeading("Windows")
    For i = 1 To NumWindowRecs
        If i > 1 Then S = S & ";"
        S = S & _
            WindowRecs(i).Name & "=" & _
            Trim(Str(WindowRecs(i).Left)) & "," & _
            Trim(Str(WindowRecs(i).Top)) & "," & _
            Trim(Str(WindowRecs(i).Width)) & "," & _
            Trim(Str(WindowRecs(i).Height))
    Next i
    Call SetUserInfo("Settings", S)
    '
    CloseReg
    '
End Sub

Public Sub GetProfile()
    Dim S As String
    Dim T As String
    Dim i As Integer
    '
    ' Set up directories and load profile
    '
    If GetString(Command$, "PATH=", S) Then
        If (Mid$(S, 2, 1) = ":") Then
            ChDrive (Left$(S, 2))
        End If
        ChDir S ' Change to specified path
        BaseDir = S + "\"
    Else
        If (Mid$(App.Path, 2, 1) = ":") Then
            ChDrive (Left$(App.Path, 2))
        End If
        ChDir App.Path ' Change to EXE path
        BaseDir = App.Path + "\"
    End If
    App.HelpFile = BaseDir + "Help\" + EditorAppName + ".hlp"
    '
    ' Read registry info
    '
    Call OpenReg("UnrealEd")
    '
    SetUserHeading ("License")
    If GetUserInfo("License", "") <> "" Then
        Licensed = 1
    End If
    '
    SetUserHeading ("Directories")
    MapDir = GetUserInfo("Maps", "Maps")
    TextureDir = GetUserInfo("Textures", "Textures")
    ToolDir = GetUserInfo("Tools", "Tools")
    ClassDir = GetUserInfo("Classes", "Classes")
    ShapeDir = GetUserInfo("Shapes", "Shapes")
    BrushDir = GetUserInfo("Brushes", "Brushes")
    ModelDir = GetUserInfo("Models", "Models")
    SoundDir = GetUserInfo("Sounds", "Sounds")
    MusicDir = GetUserInfo("Music", "Music")
    MacroDir = GetUserInfo("Macros", "Macros")
    '
    SetUserHeading ("Preferences")
    AutoSaveTime = Val(GetUserInfo("AutoSaveTime", "10"))
    InitialFiles = GetUserInfo("InitialFiles075", "")
    'InitialFiles = GetUserInfo("InitialFiles075", BaseDir + "Textures\Wood.utx")
    InitialMacro = GetUserInfo("InitialMacro", "")
    ToolbarPos = Val(GetUserInfo("ToolbarPos", "0"))
    BrowserPos = Val(GetUserInfo("BrowserPos", "0"))
    PanelButton = Val(GetUserInfo("PanelButton", "2"))
    BrowserButton = Val(GetUserInfo("BrowserButton", "0"))
    CameraVertRatio = Val(GetUserInfo("CameraVertRatio", "0.66"))
    CameraLeftRatio = Val(GetUserInfo("CameraLeftRatio", "0.5"))
    CameraRightRatio = Val(GetUserInfo("CameraRightRatio", "0.5"))
    FOV = Val(GetUserInfo("FOV", "95"))
    InitialBrowserTopic = GetUserInfo("BrowserTopic", "Textures")
    GodMode = Val(GetUserInfo("GodMode", "0"))
    '
    SetUserHeading ("Windows")
    S = GetUserInfo("Settings", "")
    NumWindowRecs = 0
    While S <> ""
        i = InStr(S, ";")
        If i = 0 Then
            T = S
            S = ""
            If InStr(T, "=") = 0 Then GoTo DoneSettings
        Else
            T = Left(S, i - 1)
            S = Mid(S, i + 1)
        End If
        '
        NumWindowRecs = NumWindowRecs + 1
        i = InStr(T, "=")
        WindowRecs(NumWindowRecs).Name = Left(T, i - 1)
        T = Mid(T, i + 1)
        '
        i = InStr(T, ",")
        WindowRecs(NumWindowRecs).Left = Val(Left(T, i - 1))
        T = Mid(T, i + 1)
        '
        i = InStr(T, ",")
        WindowRecs(NumWindowRecs).Top = Val(Left(T, i - 1))
        T = Mid(T, i + 1)
        '
        i = InStr(T, ",")
        WindowRecs(NumWindowRecs).Width = Val(Left(T, i - 1))
        T = Mid(T, i + 1)
        '
        WindowRecs(NumWindowRecs).Height = Val(T)
        '
        WindowRecs(NumWindowRecs).Loaded = 0
    Wend
DoneSettings:
    '
    Call CloseReg
    '
    ' Set common dialog directories:
    '
    frmDialogs.ExportBrush.InitDir = Ed.BaseDir + Ed.BrushDir
    frmDialogs.ImportBrush.InitDir = Ed.BaseDir + Ed.BrushDir
    frmDialogs.MacroPick.InitDir = Ed.BaseDir + Ed.MacroDir
    frmDialogs.ClassLoad.InitDir = Ed.BaseDir + Ed.ClassDir
    frmDialogs.ClassSave.InitDir = Ed.BaseDir + Ed.ClassDir
    frmDialogs.TexFamLoad.InitDir = Ed.BaseDir + Ed.TextureDir
    frmDialogs.TexFamSave.InitDir = Ed.BaseDir + Ed.TextureDir
    frmDialogs.TexImport.InitDir = Ed.BaseDir + Ed.TextureDir
    frmDialogs.ExportTex.InitDir = Ed.BaseDir + Ed.TextureDir
    frmDialogs.BrushOpen.InitDir = Ed.BaseDir + Ed.BrushDir
    frmDialogs.ExportMap.InitDir = Ed.BaseDir + Ed.MapDir
    frmDialogs.ImportMap.InitDir = Ed.BaseDir + Ed.MapDir
    frmDialogs.MacroLoad.InitDir = Ed.BaseDir + Ed.MacroDir
    frmDialogs.MacroSaveAs.InitDir = Ed.BaseDir + Ed.MacroDir
    frmDialogs.MapOpen.InitDir = Ed.BaseDir + Ed.MapDir
    frmDialogs.MapSaveAs.InitDir = Ed.BaseDir + Ed.MapDir
    frmDialogs.BrushSave.InitDir = Ed.BaseDir + Ed.BrushDir
    frmDialogs.TwoDeeOpen.InitDir = Ed.BaseDir + Ed.ShapeDir
    frmDialogs.TwoDeeSave.InitDir = Ed.BaseDir + Ed.ShapeDir
    frmDialogs.AddFile.InitDir = Ed.BaseDir
    frmDialogs.SoundImportDlg.InitDir = Ed.BaseDir + Ed.SoundDir
    frmDialogs.SoundExportDlg.InitDir = Ed.BaseDir + Ed.SoundDir
    frmDialogs.SoundSaveFamilyDlg.InitDir = Ed.BaseDir + Ed.SoundDir
    frmDialogs.SoundLoadFamilyDlg.InitDir = Ed.BaseDir + Ed.SoundDir
End Sub

Public Sub StatusText(S As String)
    'frmMain.StatusBar1.Panels(1).Text = " " & S
    frmMain.StatusText.Caption = S
End Sub

'
' Force a window to be on top of the editor,
' and make sure it fits on the screen properly.
' F is a form.  Will fail if called from external
' OLE servers.
'
Public Sub SetOnTop(F As Variant, Name As String, Pos As Integer)
    Dim X As Integer, y As Integer, i As Integer
    Dim R As RECT
    '
    Select Case Pos
    Case TOP_NORMAL:
        '
    Case TOP_PANEL:
        '
    Case TOP_BROWSER:
        Call SetParent(F.hwnd, frmMain.BrowserPanel.hwnd)
        Call GetClientRect(frmMain.BrowserPanel.hwnd, R)
        Call SetWindowPos(F.hwnd, 0, 0, 24, R.Right - R.Left, R.Bottom - R.Top - 24, 0)
        F.Height = frmMain.BrowserPanel.Height - 24 * Screen.TwipsPerPixelY
        Exit Sub
    End Select
    '
    ' See if we've recorded a position for it:
    '
    For i = 1 To NumWindowRecs
        If Name = WindowRecs(i).Name Then
            F.Left = WindowRecs(i).Left
            F.Top = WindowRecs(i).Top
            If F.BorderStyle = 2 Or F.BorderStyle = 5 Then ' Is resizable
                F.Width = WindowRecs(i).Width
                F.Height = WindowRecs(i).Height
            End If
            F.Left = WindowRecs(i).Left
            F.Top = WindowRecs(i).Top
            GoTo MakeItFit
        End If
    Next i
    NumWindowRecs = NumWindowRecs + 1
    i = NumWindowRecs
    '
    WindowRecs(i).Left = F.Left
    WindowRecs(i).Width = F.Width
    WindowRecs(i).Top = F.Top
    WindowRecs(i).Height = F.Height
    '
    ' Make it fit:
    '
MakeItFit:
    F.Tag = Name
    WindowRecs(i).Name = Name
    Set WindowRecs(i).TheForm = F
    WindowRecs(i).Loaded = 1
    '
    Call ToppenFormWindow(F)
    '
End Sub

Public Sub ToppenFormWindow(F As Variant)
    Dim Temp As Long
    '
    Call MakeFormFit(F)
    '
    ' Put it on top:
    '
    'Temp = GetWindowLong(F.hwnd, -16) 'MsgBox (Hex(Temp))
    'Temp = SetWindowLong(F.hwnd, -16, Temp Or WS_OVERLAPPED Or WS_POPUP)
    'Temp = GetWindowLong(F.hwnd, -20) 'MsgBox (Hex(Temp))
    'Temp = SetWindowLong(F.hwnd, -20, Temp Or WS_EX_NOPARENTNOTIFY)
    'Call SetParent(F.hwnd, frmMain.hwnd)
    '
    ' This works in Windows 95 only:
    '
    Call SetWindowLong(F.hwnd, -8, frmMain.hwnd)
    '
End Sub

Public Sub MakeFormFit(F As Variant)
    '
    If (F.Width > frmMain.ScaleWidth) And _
        (F.BorderStyle = 2 Or F.BorderStyle = 5) Then
        ' Is resizable and overflowing
        F.Left = 0
        F.Width = frmMain.ScaleWidth
    ElseIf F.WindowState = 0 Then
        ' Not overflowing, not minimized
        If F.Left + F.Width > frmMain.ScaleWidth Then F.Left = frmMain.ScaleWidth - F.Width
        If F.Top < 0 Then F.Top = 0
    End If
    '
    If (F.Height > frmMain.ScaleHeight) And _
        (F.BorderStyle = 2 Or F.BorderStyle = 5) Then
        ' Is resizable and overflowing
        F.Top = 0
        F.Height = frmMain.ScaleHeight
    Else
        ' Not overflowing
        If F.Top + F.Height > frmMain.ScaleHeight Then F.Top = frmMain.ScaleHeight - F.Height
        If F.Top < 0 Then F.Top = 0
    End If
    '
End Sub

Public Sub NoteResize()
   Dim i As Integer
    '
    ' See if we've recorded a position for it:
    '
    For i = 1 To NumWindowRecs
        If WindowRecs(i).Loaded = 1 Then
            Call ToppenFormWindow(WindowRecs(i).TheForm)
        End If
    Next i
 End Sub

Public Function MakeParams() As String
    Dim Params As String
    '
    Params = Params & "CAMERAVERT=" & Trim(Str(CameraVertRatio)) & Chr(13)
    Params = Params & "CAMERALEFT=" & Trim(Str(CameraLeftRatio)) & Chr(13)
    Params = Params & "CAMERARIGHT=" & Trim(Str(CameraRightRatio)) & Chr(13)
    '
    MakeParams = Params
End Function

Public Sub SaveParamsToLevel()
    Dim Params As String
    Params = MakeParams()
    Call Ed.Server.SetProp("Lev", "6", Params)
End Sub

Public Sub SetParams(Params As String)
    Dim S As String
    If GetString(Params, "CAMERAVERT=", S) Then CameraVertRatio = Val(S)
    If GetString(Params, "CAMERALEFT=", S) Then CameraLeftRatio = Val(S)
    If GetString(Params, "CAMERARIGHT=", S) Then CameraRightRatio = Val(S)
End Sub

Public Sub LoadParamsFromLevel()
    Dim Params As String
    Params = Ed.Server.GetProp("Lev", "6")
    SetParams (Params)
End Sub

Public Sub EndOnTop(F As Variant) ' F = form
    Dim i As Integer
    '
    ' See if we've recorded a position for it:
    '
    For i = 1 To NumWindowRecs
        If F.Tag = WindowRecs(i).Name Then
            If WindowRecs(i).Loaded = 1 Then
                WindowRecs(i).TheForm.Hide
                Call SetParent(F.hwnd, 0)
                WindowRecs(i).Left = F.Left
                WindowRecs(i).Top = F.Top
                WindowRecs(i).Height = F.Height
                WindowRecs(i).Width = F.Width
                Set WindowRecs(i).TheForm = Nothing
                WindowRecs(i).Loaded = 0
            End If
            Exit Sub
        End If
    Next i
    Debug.Print "EndOnTop: Form " & F.Caption & " not found!"
End Sub

'
' Register a browser topic.  BrowserForm must be a form.
'
' BrowserForm must have:
'    Public BrowswerShow(HolderForm as Form,Top as Integer)
'    Public BrowserHide()
'
Public Sub RegisterBrowserTopic(BrowserForm As Variant, TopicName As String)
    frmMain.BrowserTopicCombo.AddItem TopicName
    '
    Set BrowserTopics(NumBrowserTopics).BrowserForm = BrowserForm
    BrowserTopics(NumBrowserTopics).TopicName = TopicName
    BrowserTopics(NumBrowserTopics).Loaded = False
    '
    NumBrowserTopics = NumBrowserTopics + 1
End Sub

Public Sub SetBrowserTopic(TopicName As String)
    Dim i
    If SettingTopic = 0 Then
        SettingTopic = 1
        If TopicName <> CurBrowserTopic Then
            For i = 0 To NumBrowserTopics - 1
                If BrowserTopics(i).TopicName = CurBrowserTopic Then
                    Call BrowserTopics(i).BrowserForm.BrowserHide
                End If
            Next i
        End If
        For i = 0 To NumBrowserTopics - 1
            If BrowserTopics(i).TopicName = TopicName Then
                Call BrowserTopics(i).BrowserForm.BrowserShow
                BrowserTopics(i).Loaded = True
                frmMain.BrowserTopicCombo.ListIndex = i
                CurBrowserTopic = TopicName
                InitialBrowserTopic = TopicName
                SettingTopic = 0
                Exit Sub
            End If
        Next i
        Call MsgBox("Topic " & TopicName & " doesn't exist", , "SetBrowserTopic problem")
    End If
    SettingTopic = 0
End Sub

Public Sub ReloadBrowser()
    Dim i As Integer
    For i = 0 To NumBrowserTopics - 1
        Unload BrowserTopics(i).BrowserForm
        If BrowserTopics(i).TopicName = CurBrowserTopic Then
            Call BrowserTopics(i).BrowserForm.BrowserShow
        End If
    Next i
End Sub

Public Sub UnloadBrowser()
    Dim i
    For i = 0 To NumBrowserTopics - 1
        Unload BrowserTopics(i).BrowserForm
        Set BrowserTopics(i).BrowserForm = Nothing
    Next i
End Sub

Public Function GetBrowserCurrentItem(TopicName As String) As String
    Dim i As Integer
    '
    GetBrowserCurrentItem = ""
    If CurBrowserTopic = TopicName Then
        For i = 0 To NumBrowserTopics - 1
            If BrowserTopics(i).TopicName = TopicName Then
                GetBrowserCurrentItem = BrowserTopics(i).BrowserForm.GetCurrent
                Exit Function
            End If
        Next i
    End If
End Function

