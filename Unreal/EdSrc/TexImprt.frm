VERSION 4.00
Begin VB.Form frmTexImport 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Import Texture"
   ClientHeight    =   3465
   ClientLeft      =   2565
   ClientTop       =   2910
   ClientWidth     =   4920
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
   Height          =   3825
   HelpContextID   =   125
   Icon            =   "TexImprt.frx":0000
   Left            =   2505
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3465
   ScaleWidth      =   4920
   ShowInTaskbar   =   0   'False
   Top             =   2610
   Width           =   5040
   Begin VB.CommandButton OkAll 
      Caption         =   "Ok to &All"
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
      Left            =   1260
      TabIndex        =   15
      Top             =   3060
      Width           =   1215
   End
   Begin VB.Frame Frame2 
      Caption         =   "Properties"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   1335
      Left            =   60
      TabIndex        =   10
      Top             =   1620
      Width           =   4755
      Begin VB.CheckBox DoMips 
         Caption         =   "Generate Mipmaps"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   195
         Left            =   240
         TabIndex        =   14
         Top             =   600
         Value           =   1  'Checked
         Width           =   1875
      End
      Begin VB.CheckBox DoMasked 
         Caption         =   "Masked"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   195
         Left            =   240
         TabIndex        =   13
         Top             =   360
         Width           =   1875
      End
      Begin VB.TextBox DispersionC 
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   285
         Left            =   2940
         TabIndex        =   11
         Text            =   "0.50"
         Top             =   900
         Width           =   915
      End
      Begin VB.Label Label5 
         Caption         =   "Diffusion in 256-color mode:"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   540
         TabIndex        =   12
         Top             =   900
         Width           =   2175
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Name"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   1455
      Left            =   60
      TabIndex        =   3
      Top             =   120
      Width           =   4755
      Begin VB.TextBox TexName 
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
         Height          =   285
         Left            =   2040
         MaxLength       =   15
         TabIndex        =   5
         Text            =   "TexName"
         Top             =   660
         Width           =   2415
      End
      Begin VB.TextBox TexFamily 
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
         Height          =   285
         Left            =   2040
         MaxLength       =   15
         TabIndex        =   4
         Text            =   "FamilyName"
         Top             =   1020
         Width           =   2415
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         Caption         =   "File:"
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
         TabIndex        =   9
         Top             =   300
         Width           =   735
      End
      Begin VB.Label Fname 
         Caption         =   "Fname"
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
         Left            =   960
         TabIndex        =   8
         Top             =   300
         Width           =   3495
      End
      Begin VB.Label Label3 
         Alignment       =   1  'Right Justify
         Caption         =   "Texture Name:"
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
         TabIndex        =   7
         Top             =   660
         Width           =   1335
      End
      Begin VB.Label Label4 
         Alignment       =   1  'Right Justify
         Caption         =   "Texture Family:"
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
         Left            =   480
         TabIndex        =   6
         Top             =   1020
         Width           =   1455
      End
   End
   Begin VB.CommandButton Command1 
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Skip"
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
      Left            =   2580
      TabIndex        =   2
      Top             =   3060
      Width           =   1095
   End
   Begin VB.CommandButton OK 
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Ok"
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
      Left            =   60
      TabIndex        =   0
      Top             =   3060
      Width           =   1095
   End
   Begin VB.CommandButton Cancel 
      BackColor       =   &H00C0C0C0&
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
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
      Left            =   3720
      TabIndex        =   1
      Top             =   3060
      Width           =   1095
   End
End
Attribute VB_Name = "frmTexImport"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim ThisFname
Dim AutoOk As Integer

Private Sub Cancel_Click()
    GlobalAbortedModal = 1
    AutoOk = 0
    Hide
End Sub

Public Sub DoNext()
    Fname = GrabFname(GString)
    If frmTexImport.Fname = "" Then
        GlobalAbortedModal = 0
        AutoOk = 0
        Hide
    End If
    '
    frmTexImport.TexName = GetFileNameOnly(Fname)
    '
    If AutoOk = 1 Then Ok_Click
End Sub

Private Sub Form_Activate()
    DoNext
End Sub

Private Sub Form_Load()
    AutoOk = 0
    Call Ed.MakeFormFit(Me)
End Sub

Private Sub Ok_Click()
    '
    Dim TexFname As String
    Dim TexName As String
    Dim TexFamily As String
    '
    TexFname = frmTexImport.Fname
    TexName = Trim(frmTexImport.TexName)
    TexFamily = Trim(frmTexImport.TexFamily)
    '
    GlobalAbortedModal = 0
    '
    If Len(TexName) = 0 Or Len(TexName) > 15 Or Len(TexFamily) = 0 Or Len(TexFamily) > 15 Then
        MsgBox "Texture name and family name must be given, and must be 1-31 characters each", 16
    Else
        Ed.Server.Exec "TEXTURE IMPORT FILE=" & Quotes(TexFname) & _
            " NAME=" & Quotes(TexName) & _
            " FAMILY=" & Quotes(TexFamily) & _
            " MIPS=" & OnOff(Int(DoMips.Value)) & _
            " PALDIFFUSION=" & Trim(DispersionC.Text) & _
            " FLAGS=" & Trim(Str(IIf(DoMasked.Value, PF_MASKED, 0)))
        DoNext
    End If
    '
End Sub

Private Sub OkAll_Click()
    AutoOk = 1
    Ok_Click
End Sub
