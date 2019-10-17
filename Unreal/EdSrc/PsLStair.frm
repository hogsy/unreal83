VERSION 4.00
Begin VB.Form frmParSolLinearStair 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Build a Linear Staircase"
   ClientHeight    =   5295
   ClientLeft      =   6690
   ClientTop       =   1110
   ClientWidth     =   2490
   ControlBox      =   0   'False
   BeginProperty Font 
      name            =   "MS Sans Serif"
      charset         =   0
      weight          =   700
      size            =   8.25
      underline       =   0   'False
      italic          =   0   'False
      strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H80000008&
   Height          =   5655
   HelpContextID   =   152
   Icon            =   "PsLStair.frx":0000
   Left            =   6630
   LinkTopic       =   "Form3"
   MaxButton       =   0   'False
   ScaleHeight     =   5295
   ScaleWidth      =   2490
   ShowInTaskbar   =   0   'False
   Top             =   810
   Width           =   2610
   Begin VB.TextBox FirstStep 
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   288
      Left            =   1320
      TabIndex        =   3
      Text            =   "0"
      Top             =   2160
      Width           =   1092
   End
   Begin Threed.SSPanel SSPanel1 
      Height          =   1815
      Left            =   120
      TabIndex        =   16
      Top             =   120
      Width           =   2295
      _Version        =   65536
      _ExtentX        =   4048
      _ExtentY        =   3201
      _StockProps     =   15
      BackColor       =   12632256
      BevelInner      =   1
      Begin VB.Line Line1 
         BorderColor     =   &H0000FFFF&
         X1              =   480
         X2              =   840
         Y1              =   1080
         Y2              =   1440
      End
      Begin VB.Line Line2 
         BorderColor     =   &H0000FFFF&
         X1              =   840
         X2              =   1080
         Y1              =   1440
         Y2              =   1440
      End
      Begin VB.Line Line3 
         BorderColor     =   &H0000FFFF&
         X1              =   840
         X2              =   840
         Y1              =   1680
         Y2              =   1440
      End
      Begin VB.Line Line4 
         BorderColor     =   &H0000FFFF&
         X1              =   480
         X2              =   480
         Y1              =   1320
         Y2              =   1080
      End
      Begin VB.Line Line5 
         BorderColor     =   &H0000FFFF&
         X1              =   720
         X2              =   480
         Y1              =   1080
         Y2              =   1080
      End
      Begin VB.Line Line6 
         BorderColor     =   &H0000FFFF&
         X1              =   720
         X2              =   1080
         Y1              =   1080
         Y2              =   1440
      End
      Begin VB.Line Line7 
         BorderColor     =   &H0000FFFF&
         X1              =   720
         X2              =   1080
         Y1              =   840
         Y2              =   1200
      End
      Begin VB.Line Line8 
         BorderColor     =   &H0000FFFF&
         X1              =   1080
         X2              =   1320
         Y1              =   1200
         Y2              =   1200
      End
      Begin VB.Line Line9 
         BorderColor     =   &H0000FFFF&
         X1              =   1080
         X2              =   1080
         Y1              =   1440
         Y2              =   1200
      End
      Begin VB.Line Line10 
         BorderColor     =   &H0000FFFF&
         X1              =   720
         X2              =   720
         Y1              =   1080
         Y2              =   840
      End
      Begin VB.Line Line11 
         BorderColor     =   &H0000FFFF&
         X1              =   960
         X2              =   720
         Y1              =   840
         Y2              =   840
      End
      Begin VB.Line Line12 
         BorderColor     =   &H0000FFFF&
         X1              =   960
         X2              =   1320
         Y1              =   840
         Y2              =   1200
      End
      Begin VB.Line Line13 
         BorderColor     =   &H0000FFFF&
         X1              =   960
         X2              =   1320
         Y1              =   600
         Y2              =   960
      End
      Begin VB.Line Line14 
         BorderColor     =   &H0000FFFF&
         X1              =   1320
         X2              =   1560
         Y1              =   960
         Y2              =   960
      End
      Begin VB.Line Line15 
         BorderColor     =   &H0000FFFF&
         X1              =   1320
         X2              =   1320
         Y1              =   1200
         Y2              =   960
      End
      Begin VB.Line Line16 
         BorderColor     =   &H0000FFFF&
         X1              =   960
         X2              =   960
         Y1              =   840
         Y2              =   600
      End
      Begin VB.Line Line17 
         BorderColor     =   &H0000FFFF&
         X1              =   1200
         X2              =   960
         Y1              =   600
         Y2              =   600
      End
      Begin VB.Line Line18 
         BorderColor     =   &H0000FFFF&
         X1              =   1200
         X2              =   1560
         Y1              =   600
         Y2              =   960
      End
      Begin VB.Line Line19 
         BorderColor     =   &H0000FFFF&
         X1              =   1200
         X2              =   1560
         Y1              =   360
         Y2              =   720
      End
      Begin VB.Line Line20 
         BorderColor     =   &H0000FFFF&
         X1              =   1560
         X2              =   1800
         Y1              =   720
         Y2              =   720
      End
      Begin VB.Line Line21 
         BorderColor     =   &H0000FFFF&
         X1              =   1560
         X2              =   1560
         Y1              =   960
         Y2              =   720
      End
      Begin VB.Line Line22 
         BorderColor     =   &H0000FFFF&
         X1              =   1200
         X2              =   1200
         Y1              =   600
         Y2              =   360
      End
      Begin VB.Line Line24 
         BorderColor     =   &H0000FFFF&
         X1              =   1440
         X2              =   1800
         Y1              =   360
         Y2              =   720
      End
      Begin VB.Line Line25 
         BorderColor     =   &H0000FFFF&
         X1              =   1200
         X2              =   1440
         Y1              =   360
         Y2              =   360
      End
      Begin VB.Line Line26 
         BorderColor     =   &H0000FFFF&
         X1              =   480
         X2              =   840
         Y1              =   1320
         Y2              =   1680
      End
      Begin VB.Line Line27 
         BorderColor     =   &H0000FFFF&
         X1              =   1800
         X2              =   1800
         Y1              =   720
         Y2              =   1680
      End
      Begin VB.Line Line28 
         BorderColor     =   &H0000FFFF&
         X1              =   840
         X2              =   1800
         Y1              =   1680
         Y2              =   1680
      End
      Begin VB.Line Line29 
         X1              =   360
         X2              =   360
         Y1              =   1080
         Y2              =   1320
      End
      Begin VB.Line Line30 
         X1              =   480
         X2              =   360
         Y1              =   1080
         Y2              =   1080
      End
      Begin VB.Line Line31 
         X1              =   480
         X2              =   360
         Y1              =   1320
         Y2              =   1320
      End
      Begin VB.Line Line32 
         X1              =   1200
         X2              =   1200
         Y1              =   240
         Y2              =   120
      End
      Begin VB.Line Line33 
         X1              =   1440
         X2              =   1200
         Y1              =   120
         Y2              =   120
      End
      Begin VB.Line Line34 
         X1              =   1440
         X2              =   1440
         Y1              =   240
         Y2              =   120
      End
      Begin VB.Line Line35 
         X1              =   1560
         X2              =   1920
         Y1              =   360
         Y2              =   720
      End
      Begin VB.Line Line36 
         X1              =   1440
         X2              =   1560
         Y1              =   360
         Y2              =   360
      End
      Begin VB.Line Line37 
         X1              =   1800
         X2              =   1920
         Y1              =   720
         Y2              =   720
      End
      Begin VB.Label Label4 
         BackStyle       =   0  'Transparent
         Caption         =   "Width"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   1680
         TabIndex        =   19
         Top             =   240
         Width           =   495
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Height"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   18
         Top             =   840
         Width           =   495
      End
      Begin VB.Label Label2 
         BackStyle       =   0  'Transparent
         Caption         =   "Length"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   600
         TabIndex        =   17
         Top             =   120
         Width           =   495
      End
   End
   Begin VB.CommandButton Help 
      BackColor       =   &H00C0C0C0&
      Cancel          =   -1  'True
      Caption         =   "&Help"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   960
      TabIndex        =   1
      Top             =   4800
      Width           =   615
   End
   Begin VB.TextBox StepLength 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1320
      TabIndex        =   4
      Text            =   "32"
      Top             =   2520
      Width           =   1095
   End
   Begin VB.TextBox NumSteps 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1320
      TabIndex        =   7
      Text            =   "8"
      Top             =   3600
      Width           =   1095
   End
   Begin VB.TextBox StepWidth 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1320
      TabIndex        =   6
      Text            =   "256"
      Top             =   3240
      Width           =   1095
   End
   Begin VB.TextBox StepHeight 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1320
      TabIndex        =   5
      Text            =   "16"
      Top             =   2880
      Width           =   1095
   End
   Begin VB.TextBox Group 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1320
      TabIndex        =   8
      Text            =   "Group"
      Top             =   3960
      Width           =   1095
   End
   Begin VB.CommandButton Command2 
      Caption         =   "&Close"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   1680
      TabIndex        =   2
      Top             =   4800
      Width           =   735
   End
   Begin VB.CommandButton Build 
      Caption         =   "&Build"
      Default         =   -1  'True
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   120
      TabIndex        =   0
      Top             =   4800
      Width           =   735
   End
   Begin VB.Label Label3 
      Alignment       =   1  'Right Justify
      Caption         =   "Add to First Step"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   252
      Left            =   0
      TabIndex        =   20
      Top             =   2160
      Width           =   1212
   End
   Begin VB.Label Trigger 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Trigger"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   0
      TabIndex        =   15
      Top             =   4560
      Visible         =   0   'False
      Width           =   612
   End
   Begin VB.Label Label5 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Step Width"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   252
      Left            =   0
      TabIndex        =   11
      Top             =   3240
      Width           =   1212
   End
   Begin VB.Label Label6 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Step Height"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   252
      Left            =   0
      TabIndex        =   12
      Top             =   2880
      Width           =   1212
   End
   Begin VB.Label Label7 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Step Length"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   252
      Left            =   240
      TabIndex        =   13
      Top             =   2520
      Width           =   972
   End
   Begin VB.Label Label8 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Number of steps"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   252
      Left            =   0
      TabIndex        =   14
      Top             =   3600
      Width           =   1212
   End
   Begin VB.Label Label10 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Group Name"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   252
      Left            =   0
      TabIndex        =   10
      Top             =   3960
      Width           =   1212
   End
   Begin VB.Label Label11 
      Alignment       =   2  'Center
      BackStyle       =   0  'Transparent
      Caption         =   "Item Names are: Step, Rise, Side, Base, Back"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   492
      Left            =   120
      TabIndex        =   9
      Top             =   4320
      Width           =   2292
   End
   Begin VB.Line Line23 
      X1              =   1440
      X2              =   1200
      Y1              =   -120
      Y2              =   -120
   End
End
Attribute VB_Name = "frmParSolLinearStair"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub Build_Click()
    '
    Dim StepLoop As Integer
    Dim N As Integer
    Dim Group As String
    'Dim V As Integer         ' Vertex Accumulator
    Dim i As Integer
    Dim BasePointer As Integer
    Dim MaxLoop As Integer
    Dim Temp As Double
    
    Dim SolidCount As Integer
    Dim SolidLoop As Integer

    Dim SWidth As Integer
    Dim SLength As Integer
    Dim SHeight As Integer
    Dim SSteps As Integer
    Dim SFirstStep As Integer
    Dim ZBase As Single
    
    Dim CurrentX As Single
    Dim CurrentY As Single
    Dim CurrentZ As Single
    
    Call InitBrush("Stair")
    Group = UCase$(frmParSolLinearStair.Group)

    '
    ' Initialize the starting point of the first stair
    '
    If Not Eval(StepWidth, Temp) Then Exit Sub
    SWidth = Int(Temp)
    '
    If Not Eval(StepLength, Temp) Then Exit Sub
    SLength = Int(Temp)
    '
    If Not Eval(StepHeight, Temp) Then Exit Sub
    SHeight = Int(Temp)
    '
    If Not Eval(NumSteps, Temp) Then Exit Sub
    SSteps = Int(Temp)
    '
    If Not Eval(FirstStep, Temp) Then Exit Sub
    SFirstStep = Int(Temp)
    
    '
    CurrentX = Int(-(SWidth / 2))               '
    CurrentY = Int(-(((SLength * SSteps)) / 2)) '
    CurrentZ = Int(-(((SHeight * SSteps) - SFirstStep) / 2)) '
       ZBase = Int(-(((SHeight * SSteps) + SFirstStep) / 2)) '
    
    '
    'Check For errors
    '
    If (SWidth <= 0) Or (SLength <= 0) Or (SHeight <= 0) Or (SSteps <= 0) Then
        MsgBox ("You must use positive, non-zero values for Length, Width, Height, and Number of Steps.")
        Exit Sub
    End If
    If SSteps = 1 Then
        MsgBox ("A 1-step Staircase is a Rectangle. Use the Rectangle Brush.")
        Exit Sub
    End If
    If SSteps > 45 Then
        MsgBox ("Sorry, you must have 45 or fewer steps.")
        Exit Sub
    End If

    
    'Brush.NumPolys = (SSteps * 2) ' 2 Rectangles per Step + base and back
    For StepLoop = 1 To SSteps
        '
        ' Single Step  (starting at 0,0,0)
        '
        
        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = "Rise"
        Brush.Polys(N).NumVertices = 4
        
        If StepLoop = 1 Then
            Call PutVertex(N, 4, CurrentX, CurrentY, ZBase)
            Call PutVertex(N, 3, CurrentX, CurrentY, CurrentZ + SHeight)
            Call PutVertex(N, 2, CurrentX + SWidth, CurrentY, CurrentZ + SHeight)
            Call PutVertex(N, 1, CurrentX + SWidth, CurrentY, ZBase)
        Else
            Call PutVertex(N, 4, CurrentX, CurrentY, CurrentZ)
            Call PutVertex(N, 3, CurrentX, CurrentY, CurrentZ + SHeight)
            Call PutVertex(N, 2, CurrentX + SWidth, CurrentY, CurrentZ + SHeight)
            Call PutVertex(N, 1, CurrentX + SWidth, CurrentY, CurrentZ)
        End If
        
        CurrentZ = CurrentZ + SHeight

        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = "Step"
        Brush.Polys(N).NumVertices = 4

        Call PutVertex(N, 4, CurrentX, CurrentY, CurrentZ)
        Call PutVertex(N, 3, CurrentX, CurrentY + SLength, CurrentZ)
        Call PutVertex(N, 2, CurrentX + SWidth, CurrentY + SLength, CurrentZ)
        Call PutVertex(N, 1, CurrentX + SWidth, CurrentY, CurrentZ)
        CurrentY = CurrentY + SLength
    Next StepLoop
    '
    ' Draw Back
    '
    N = N + 1
    InitBrushPoly (N)
    Brush.Polys(N).Group = Group
    Brush.Polys(N).Item = "Back"
    Brush.Polys(N).NumVertices = 4
    CurrentZ = CurrentZ - SHeight
    Call PutVertex(N, 1, CurrentX, CurrentY, ZBase)
    Call PutVertex(N, 2, CurrentX, CurrentY, CurrentZ + SHeight)
    Call PutVertex(N, 3, CurrentX + SWidth, CurrentY, CurrentZ + SHeight)
    Call PutVertex(N, 4, CurrentX + SWidth, CurrentY, ZBase)
    
    
    ' Move to Position
    ' Draw Base
    '
    N = N + 1
    InitBrushPoly (N)
    Brush.Polys(N).Group = Group
    Brush.Polys(N).Item = "Base"
    Brush.Polys(N).NumVertices = 4
    
    Call PutVertex(N, 1, CurrentX, CurrentY, ZBase)
    Call PutVertex(N, 2, CurrentX + SWidth, CurrentY, ZBase)
    Call PutVertex(N, 3, CurrentX + SWidth, CurrentY - (SLength * NumSteps), ZBase)
    Call PutVertex(N, 4, CurrentX, CurrentY - (SLength * NumSteps), ZBase)


    
    '
    ' Init and draw the side
    '
    
    CurrentX = Int(-(SWidth / 2))               '
    CurrentY = Int(-(((SLength * SSteps)) / 2)) '
    CurrentZ = Int(-(((SHeight * SSteps) - SFirstStep) / 2)) '
    
    For StepLoop = 1 To NumSteps
        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = "Side"
        Brush.Polys(N).NumVertices = 4

        If StepLoop = 1 Then
            Call PutVertex(N, 1, CurrentX, CurrentY, ZBase)
            Call PutVertex(N, 2, CurrentX, CurrentY, CurrentZ + SHeight)
            Call PutVertex(N, 3, CurrentX, Int((SLength * (NumSteps)) / 2), CurrentZ + SHeight)
            Call PutVertex(N, 4, CurrentX, Int((SLength * (NumSteps)) / 2), ZBase)
        Else
            Call PutVertex(N, 1, CurrentX, CurrentY, CurrentZ)
            Call PutVertex(N, 2, CurrentX, CurrentY, CurrentZ + SHeight)
            Call PutVertex(N, 3, CurrentX, Int((SLength * NumSteps) / 2), CurrentZ + SHeight)
            Call PutVertex(N, 4, CurrentX, Int((SLength * NumSteps) / 2), CurrentZ)
        
        End If
        CurrentZ = CurrentZ + SHeight
        CurrentY = CurrentY + SLength
    Next StepLoop
    
    CurrentX = Int(SWidth / 2)                  '
    CurrentY = Int(-(((SLength * SSteps)) / 2)) '
    CurrentZ = Int(-(((SHeight * SSteps) - SFirstStep) / 2)) '
    
    For StepLoop = 1 To NumSteps
        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = "Side"
        Brush.Polys(N).NumVertices = 4

        If StepLoop = 1 Then
            Call PutVertex(N, 4, CurrentX, CurrentY, ZBase)
            Call PutVertex(N, 3, CurrentX, CurrentY, CurrentZ + SHeight)
            Call PutVertex(N, 2, CurrentX, Int((SLength * (NumSteps)) / 2), CurrentZ + SHeight)
            Call PutVertex(N, 1, CurrentX, Int((SLength * (NumSteps)) / 2), ZBase)
        Else
            Call PutVertex(N, 4, CurrentX, CurrentY, CurrentZ)
            Call PutVertex(N, 3, CurrentX, CurrentY, CurrentZ + SHeight)
            Call PutVertex(N, 2, CurrentX, Int((SLength * NumSteps) / 2), CurrentZ + SHeight)
            Call PutVertex(N, 1, CurrentX, Int((SLength * NumSteps) / 2), CurrentZ)
        
        End If
        CurrentZ = CurrentZ + SHeight
        CurrentY = CurrentY + SLength
    Next StepLoop

    
    Brush.NumPolys = N
    Call SendBrush(Ed.Server)
    Call Ed.StatusText("Built a Stair")
'
End Sub

Private Sub Command2_Click()
    Hide
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "BuildStair", TOP_NORMAL)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub Help_Click()
    ToolHelp (152)
End Sub

Private Sub Trigger_Change()
    Build_Click
End Sub

