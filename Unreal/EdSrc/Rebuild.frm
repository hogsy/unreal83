VERSION 4.00
Begin VB.Form frmRebuilder 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Rebuilder"
   ClientHeight    =   6375
   ClientLeft      =   5670
   ClientTop       =   3915
   ClientWidth     =   3855
   BeginProperty Font 
      name            =   "MS Sans Serif"
      charset         =   0
      weight          =   700
      size            =   8.25
      underline       =   0   'False
      italic          =   0   'False
      strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H00808080&
   Height          =   6735
   HelpContextID   =   118
   Icon            =   "Rebuild.frx":0000
   Left            =   5610
   LinkTopic       =   "Form4"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   6375
   ScaleWidth      =   3855
   ShowInTaskbar   =   0   'False
   Top             =   3615
   Width           =   3975
   Begin VB.CommandButton UpdStats 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&Upd Stats"
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
      Left            =   1740
      TabIndex        =   2
      Top             =   5940
      Width           =   1095
   End
   Begin VB.CommandButton Command2 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
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
      Left            =   2940
      TabIndex        =   0
      Top             =   5940
      Width           =   855
   End
   Begin VB.CommandButton Go 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&Rebuild Geometry"
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
      TabIndex        =   1
      Top             =   5940
      Width           =   1575
   End
   Begin TabDlg.SSTab Tab1 
      Height          =   5775
      Left            =   60
      TabIndex        =   3
      Top             =   60
      Width           =   3735
      _Version        =   65536
      _ExtentX        =   6588
      _ExtentY        =   10186
      _StockProps     =   15
      Caption         =   "BSP  "
      ForeColor       =   0
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      TabsPerRow      =   3
      Tab             =   1
      TabOrientation  =   0
      Tabs            =   3
      Style           =   1
      TabMaxWidth     =   0
      TabHeight       =   564
      ShowFocusRect   =   0   'False
      TabCaption(0)   =   "Geometry  "
      Tab(0).ControlCount=   2
      Tab(0).ControlEnabled=   0   'False
      Tab(0).Control(0)=   "Frame8"
      Tab(0).Control(1)=   "AutoBSP"
      TabCaption(1)   =   "BSP  "
      Tab(1).ControlCount=   4
      Tab(1).ControlEnabled=   -1  'True
      Tab(1).Control(0)=   "Frame4"
      Tab(1).Control(1)=   "Frame1"
      Tab(1).Control(2)=   "Frame3"
      Tab(1).Control(3)=   "AutoLights"
      TabCaption(2)   =   "Lighting  "
      Tab(2).ControlCount=   2
      Tab(2).ControlEnabled=   0   'False
      Tab(2).Control(0)=   "Frame6"
      Tab(2).Control(1)=   "Frame5"
      Begin VB.CheckBox AutoLights 
         Caption         =   "&Auto rebuild lighting"
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
         Left            =   720
         TabIndex        =   71
         Top             =   5340
         Value           =   1  'Checked
         Width           =   2055
      End
      Begin VB.Frame Frame3 
         Caption         =   "BSP Rebuild options"
         BeginProperty Font 
            name            =   "Arial"
            charset         =   0
            weight          =   700
            size            =   12
            underline       =   0   'False
            italic          =   -1  'True
            strikethrough   =   0   'False
         EndProperty
         Height          =   975
         Left            =   120
         TabIndex        =   65
         Top             =   4320
         Width           =   3495
         Begin VB.CheckBox BuildZones 
            Caption         =   "Build Visibility Zones"
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
            Left            =   180
            TabIndex        =   72
            Top             =   600
            Value           =   1  'Checked
            Width           =   1875
         End
         Begin VB.CheckBox OptGeom 
            Caption         =   "&Optimize Geometry"
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
            Left            =   180
            TabIndex        =   66
            Top             =   360
            Value           =   1  'Checked
            Width           =   1815
         End
      End
      Begin VB.Frame Frame1 
         Caption         =   "Optimization"
         BeginProperty Font 
            name            =   "Arial"
            charset         =   0
            weight          =   700
            size            =   12
            underline       =   0   'False
            italic          =   -1  'True
            strikethrough   =   0   'False
         EndProperty
         Height          =   1935
         Left            =   120
         TabIndex        =   61
         Top             =   2280
         Width           =   3495
         Begin VB.OptionButton Lame 
            Caption         =   "&Lame    (fast rebuild)"
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
            Left            =   240
            TabIndex        =   64
            Top             =   360
            Width           =   3135
         End
         Begin VB.OptionButton Good 
            Caption         =   "&Good    (medium rebuild)"
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
            Left            =   240
            TabIndex        =   63
            Top             =   600
            Value           =   -1  'True
            Width           =   3135
         End
         Begin VB.OptionButton Optimal 
            Caption         =   "&Optimal (Go grab a cup of coffee)"
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
            Left            =   240
            TabIndex        =   62
            Top             =   840
            Width           =   3135
         End
         Begin ComctlLib.Slider Balance 
            Height          =   375
            Left            =   120
            TabIndex        =   70
            Top             =   1140
            Width           =   3255
            _Version        =   65536
            _ExtentX        =   5741
            _ExtentY        =   661
            _StockProps     =   64
            Max             =   100
            SelStart        =   15
            TickFrequency   =   5
            Value           =   15
         End
         Begin VB.Label BalanceValue 
            Alignment       =   2  'Center
            Caption         =   "15"
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
            Left            =   1440
            TabIndex        =   69
            Top             =   1620
            Width           =   495
         End
         Begin VB.Label Label2 
            Alignment       =   1  'Right Justify
            Caption         =   "Balance Tree"
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
            Left            =   2040
            TabIndex        =   68
            Top             =   1620
            Width           =   1215
         End
         Begin VB.Label Label1 
            Caption         =   "Minimize cuts"
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
            Left            =   240
            TabIndex        =   67
            Top             =   1620
            Width           =   1215
         End
      End
      Begin VB.Frame Frame4 
         Caption         =   "BSP Stats"
         BeginProperty Font 
            name            =   "Arial"
            charset         =   0
            weight          =   700
            size            =   12
            underline       =   0   'False
            italic          =   -1  'True
            strikethrough   =   0   'False
         EndProperty
         Height          =   1695
         Left            =   120
         TabIndex        =   35
         Top             =   480
         Width           =   3495
         Begin VB.Label Label3 
            Alignment       =   1  'Right Justify
            Caption         =   "Polys"
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
            Left            =   240
            TabIndex        =   60
            Top             =   345
            Width           =   615
         End
         Begin VB.Label Label4 
            Alignment       =   1  'Right Justify
            Caption         =   "Nodes"
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
            Left            =   240
            TabIndex        =   59
            Top             =   585
            Width           =   615
         End
         Begin VB.Label Label5 
            Alignment       =   1  'Right Justify
            Caption         =   "Ratio"
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
            Left            =   240
            TabIndex        =   58
            Top             =   840
            Width           =   615
         End
         Begin VB.Label Label6 
            Alignment       =   1  'Right Justify
            Caption         =   "Leaves"
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
            TabIndex        =   57
            Top             =   1320
            Width           =   615
         End
         Begin VB.Label Label7 
            Alignment       =   1  'Right Justify
            Caption         =   "Branches"
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
            Left            =   1320
            TabIndex        =   56
            Top             =   360
            Width           =   975
         End
         Begin VB.Label Label8 
            Alignment       =   1  'Right Justify
            Caption         =   "Fronts"
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
            Left            =   1560
            TabIndex        =   55
            Top             =   840
            Width           =   735
         End
         Begin VB.Label Label9 
            Alignment       =   1  'Right Justify
            Caption         =   "Backs"
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
            Left            =   1560
            TabIndex        =   54
            Top             =   1080
            Width           =   735
         End
         Begin VB.Label Label10 
            Alignment       =   1  'Right Justify
            Caption         =   "Coplanars"
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
            Left            =   1560
            TabIndex        =   53
            Top             =   600
            Width           =   735
         End
         Begin VB.Label Label12 
            Alignment       =   1  'Right Justify
            Caption         =   "Avg Dep"
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
            TabIndex        =   52
            Top             =   1320
            Width           =   735
         End
         Begin VB.Label Label13 
            Alignment       =   1  'Right Justify
            Caption         =   "Max Dep"
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
            TabIndex        =   51
            Top             =   1080
            Width           =   735
         End
         Begin VB.Label EdPolys 
            Caption         =   "?"
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
            TabIndex        =   50
            Top             =   360
            Width           =   495
         End
         Begin VB.Label Leaves 
            Caption         =   "?"
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
            Left            =   2400
            TabIndex        =   49
            Top             =   1320
            Width           =   495
         End
         Begin VB.Label Backs 
            Caption         =   "?"
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
            Left            =   2375
            TabIndex        =   48
            Top             =   1080
            Width           =   495
         End
         Begin VB.Label Fronts 
            Caption         =   "?"
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
            Left            =   2375
            TabIndex        =   47
            Top             =   840
            Width           =   495
         End
         Begin VB.Label Coplanars 
            Caption         =   "?"
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
            Left            =   2375
            TabIndex        =   46
            Top             =   600
            Width           =   495
         End
         Begin VB.Label Branches 
            Caption         =   "?"
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
            Left            =   2375
            TabIndex        =   45
            Top             =   360
            Width           =   495
         End
         Begin VB.Label AvgDepth 
            Caption         =   "?"
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
            TabIndex        =   44
            Top             =   1320
            Width           =   495
         End
         Begin VB.Label MaxDepth 
            Caption         =   "?"
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
            TabIndex        =   43
            Top             =   1080
            Width           =   495
         End
         Begin VB.Label Ratio 
            Caption         =   "?"
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
            TabIndex        =   42
            Top             =   840
            Width           =   495
         End
         Begin VB.Label Nodes 
            Caption         =   "?"
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
            TabIndex        =   41
            Top             =   600
            Width           =   495
         End
         Begin VB.Label pBranches 
            Caption         =   "?"
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
            Left            =   2880
            TabIndex        =   40
            Top             =   360
            Width           =   495
         End
         Begin VB.Label pLeaves 
            Caption         =   "?"
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
            Left            =   2880
            TabIndex        =   39
            Top             =   1320
            Width           =   495
         End
         Begin VB.Label pBacks 
            Caption         =   "?"
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
            Left            =   2880
            TabIndex        =   38
            Top             =   1080
            Width           =   495
         End
         Begin VB.Label pFronts 
            Caption         =   "?"
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
            Left            =   2880
            TabIndex        =   37
            Top             =   840
            Width           =   495
         End
         Begin VB.Label pCoplanars 
            Caption         =   "?"
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
            Left            =   2880
            TabIndex        =   36
            Top             =   600
            Width           =   495
         End
      End
      Begin VB.Frame Frame8 
         Caption         =   "Map Geometry Stats"
         BeginProperty Font 
            name            =   "Arial"
            charset         =   0
            weight          =   700
            size            =   12
            underline       =   0   'False
            italic          =   -1  'True
            strikethrough   =   0   'False
         EndProperty
         Height          =   3675
         Left            =   -74880
         TabIndex        =   22
         Top             =   480
         Width           =   3495
         Begin VB.Label Label32 
            Alignment       =   1  'Right Justify
            Caption         =   "Planes"
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
            TabIndex        =   84
            Top             =   3240
            Width           =   975
         End
         Begin VB.Label MapPlanes 
            Caption         =   "?"
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
            Left            =   1320
            TabIndex        =   83
            Top             =   3240
            Width           =   1215
         End
         Begin VB.Label Label28 
            Alignment       =   1  'Right Justify
            Caption         =   "Sides"
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
            TabIndex        =   82
            Top             =   2520
            Width           =   975
         End
         Begin VB.Label MapSides 
            Caption         =   "?"
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
            Left            =   1320
            TabIndex        =   81
            Top             =   2520
            Width           =   1215
         End
         Begin VB.Label MapPoints 
            Caption         =   "?"
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
            Left            =   1320
            TabIndex        =   80
            Top             =   2040
            Width           =   1215
         End
         Begin VB.Label MapVectors 
            Caption         =   "?"
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
            Left            =   1320
            TabIndex        =   79
            Top             =   2280
            Width           =   1215
         End
         Begin VB.Label MapBounds 
            Caption         =   "?"
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
            Left            =   1320
            TabIndex        =   78
            Top             =   2760
            Width           =   1215
         End
         Begin VB.Label MapZones 
            Caption         =   "?"
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
            Left            =   1320
            TabIndex        =   77
            Top             =   3000
            Width           =   1215
         End
         Begin VB.Label Label31 
            Alignment       =   1  'Right Justify
            Caption         =   "Vectors"
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
            TabIndex        =   76
            Top             =   2280
            Width           =   975
         End
         Begin VB.Label Label30 
            Alignment       =   1  'Right Justify
            Caption         =   "Bounds"
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
            TabIndex        =   75
            Top             =   2760
            Width           =   975
         End
         Begin VB.Label Label29 
            Alignment       =   1  'Right Justify
            Caption         =   "Zones"
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
            TabIndex        =   74
            Top             =   3000
            Width           =   975
         End
         Begin VB.Label Label26 
            Alignment       =   1  'Right Justify
            Caption         =   "Points"
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
            TabIndex        =   73
            Top             =   2040
            Width           =   975
         End
         Begin VB.Label MapTotalPolys 
            Caption         =   "?"
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
            Left            =   1320
            TabIndex        =   34
            Top             =   1560
            Width           =   1215
         End
         Begin VB.Label MapAvgPolys 
            Caption         =   "?"
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
            Left            =   1320
            TabIndex        =   33
            Top             =   1320
            Width           =   1215
         End
         Begin VB.Label MapSpecial 
            Caption         =   "?"
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
            Left            =   1320
            TabIndex        =   32
            Top             =   1080
            Width           =   1215
         End
         Begin VB.Label MapSubtract 
            Caption         =   "?"
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
            Left            =   1320
            TabIndex        =   31
            Top             =   840
            Width           =   1215
         End
         Begin VB.Label MapAdd 
            Caption         =   "?"
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
            Left            =   1320
            TabIndex        =   30
            Top             =   600
            Width           =   1215
         End
         Begin VB.Label MapBrushes 
            Caption         =   "?"
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
            Left            =   1320
            TabIndex        =   29
            Top             =   360
            Width           =   1215
         End
         Begin VB.Label Label18 
            Alignment       =   1  'Right Justify
            Caption         =   "Special"
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
            TabIndex        =   28
            Top             =   1080
            Width           =   735
         End
         Begin VB.Label Label19 
            Alignment       =   1  'Right Justify
            Caption         =   "Average Polys"
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
            TabIndex        =   27
            Top             =   1320
            Width           =   1095
         End
         Begin VB.Label Label20 
            Alignment       =   1  'Right Justify
            Caption         =   "Subtract"
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
            Left            =   240
            TabIndex        =   26
            Top             =   840
            Width           =   975
         End
         Begin VB.Label Label21 
            Alignment       =   1  'Right Justify
            Caption         =   "Add"
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
            TabIndex        =   25
            Top             =   600
            Width           =   1095
         End
         Begin VB.Label Label22 
            Alignment       =   1  'Right Justify
            Caption         =   "Brushes"
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
            TabIndex        =   24
            Top             =   360
            Width           =   735
         End
         Begin VB.Label Label23 
            Alignment       =   1  'Right Justify
            Caption         =   "Total Polys"
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
            TabIndex        =   23
            Top             =   1560
            Width           =   1095
         End
      End
      Begin VB.CheckBox AutoBSP 
         Caption         =   "&Auto rebuild BSP"
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
         Left            =   -74040
         TabIndex        =   21
         Top             =   5280
         Value           =   1  'Checked
         Width           =   1575
      End
      Begin VB.Frame Frame5 
         Caption         =   "Lighting Stats"
         BeginProperty Font 
            name            =   "Arial"
            charset         =   0
            weight          =   700
            size            =   12
            underline       =   0   'False
            italic          =   -1  'True
            strikethrough   =   0   'False
         EndProperty
         Height          =   2175
         Left            =   -74880
         TabIndex        =   6
         Top             =   480
         Width           =   3375
         Begin VB.Label Label17 
            Alignment       =   1  'Right Justify
            Caption         =   "Avg Size"
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
            Left            =   660
            TabIndex        =   20
            Top             =   1080
            Width           =   735
         End
         Begin VB.Label Label16 
            Alignment       =   1  'Right Justify
            Caption         =   "Max Size"
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
            Left            =   660
            TabIndex        =   19
            Top             =   1320
            Width           =   735
         End
         Begin VB.Label Label15 
            Alignment       =   1  'Right Justify
            Caption         =   "Texel coverage"
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
            Left            =   180
            TabIndex        =   18
            Top             =   1560
            Width           =   1215
         End
         Begin VB.Label Label14 
            Alignment       =   1  'Right Justify
            Caption         =   "Mesh Points"
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
            Left            =   300
            TabIndex        =   17
            Top             =   840
            Width           =   1095
         End
         Begin VB.Label Label11 
            Alignment       =   1  'Right Justify
            Caption         =   "Meshes"
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
            Left            =   660
            TabIndex        =   16
            Top             =   600
            Width           =   735
         End
         Begin VB.Label LightMeshes 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "Meshes"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   15
            Top             =   600
            Width           =   975
         End
         Begin VB.Label LightMeshPts 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "MeshPts"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   14
            Top             =   840
            Width           =   975
         End
         Begin VB.Label LightAvgSize 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "AvgSize"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   13
            Top             =   1080
            Width           =   975
         End
         Begin VB.Label LightMaxSize 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "MaxSize"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   12
            Top             =   1320
            Width           =   975
         End
         Begin VB.Label LightCacheSize 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "CacheSize"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   11
            Top             =   1560
            Width           =   975
         End
         Begin VB.Label Label24 
            Alignment       =   1  'Right Justify
            Caption         =   "Lights"
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
            Left            =   420
            TabIndex        =   10
            Top             =   360
            Width           =   975
         End
         Begin VB.Label Label25 
            Alignment       =   1  'Right Justify
            Caption         =   "Square Meters"
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
            Left            =   300
            TabIndex        =   9
            Top             =   1800
            Width           =   1095
         End
         Begin VB.Label LightMeters 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "Meters"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   8
            Top             =   1800
            Width           =   975
         End
         Begin VB.Label LightCount 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "Count"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   7
            Top             =   360
            Width           =   975
         End
      End
      Begin VB.Frame Frame6 
         Caption         =   "Raytracing Options"
         BeginProperty Font 
            name            =   "Arial"
            charset         =   0
            weight          =   700
            size            =   12
            underline       =   0   'False
            italic          =   -1  'True
            strikethrough   =   0   'False
         EndProperty
         Height          =   1095
         Left            =   -74880
         TabIndex        =   4
         Top             =   2760
         Width           =   3375
         Begin VB.CheckBox LightSel 
            Caption         =   "Apply selected lights only"
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
            Left            =   240
            TabIndex        =   5
            Top             =   360
            Width           =   2775
         End
      End
   End
End
Attribute VB_Name = "frmRebuilder"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim UpdatingTab As Integer

Private Sub Backs_Change()
    pBacks.Caption = ShowPercentz(Caption)
End Sub

Private Sub Balance_Change()
    BalanceValue.Caption = Trim(Str(Balance.Value))
End Sub

Private Sub Branches_Change()
    pBranches.Caption = ShowPercentz(Caption)
End Sub

Private Sub Command2_Click()
    Hide
End Sub

Private Sub Form_Paint()
    UpdStats_Click
End Sub

Private Sub UpdStats_Click()
    Select Case Tab1.Tab
        Case 0: RefreshGeomStats
        Case 1: RefreshBSPStats
        Case 2: RefreshLightStats
    End Select
End Sub

Private Sub Coplanars_Change()
    pCoplanars.Caption = ShowPercentz(Caption)
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "Rebuilder", TOP_PANEL)
    RefreshGeomStats
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub Fronts_Change()
    pFronts.Caption = ShowPercentz(Caption)
End Sub

Private Sub Go_Click()
    Ed.Server.Exec "TASK BEGIN MESSAGE=" & Quotes("Rebuilding Level")
    Select Case Tab1.Tab
        Case 0: Call GoGeometry
        Case 1: Call GoBSP
        Case 2: Call GoLights
    End Select
    Ed.Server.Exec "TASK END"
    Beep
End Sub

Private Sub GoBSP()
    Dim Cmd As String
    '
    ' Build BSP rebuld command:
    '
    Cmd = "BSP REBUILD"
    '
    If Lame.Value Then
        Cmd = Cmd & " LAME"
    ElseIf Good.Value Then
        Cmd = Cmd & " GOOD"
    ElseIf Optimal.Value Then
        Cmd = Cmd & " OPTIMAL"
    End If
    '
    Cmd = Cmd & " BALANCE=" & Trim(Str(Balance.Value))
    '
    If BuildZones.Value Then
        Cmd = Cmd & " ZONES"
    End If
    '
    If OptGeom.Value Then
        Cmd = Cmd & " OPTGEOM"
    End If
    '
    Ed.BeginSlowTask ("Rebuilding BSP")
    Ed.Server.SlowExec Cmd
    Ed.EndSlowTask
    '
    If (Tab1.Tab = 1) Then UpdateTab (1)
    '
    If AutoLights.Value Then
        GoLights
    End If
    '
End Sub

Private Sub GoGeometry()
    '
    Dim TempResult As Integer
    '
    Ed.BeginSlowTask "Rebuilding Map"
    Ed.Server.SlowExec "MAP REBUILD"
    Ed.EndSlowTask
    '
    If (Tab1.Tab = 0) Then UpdateTab (0)
    '
    ' If map editing was on, turn it off now:
    '
    If Ed.MapEdit Then Call Ed.Tools.Click("MAPEDIT")
    '
    If AutoBSP Then
        GoBSP
    End If
    '
End Sub

Private Sub GoLights()
    Dim Cmd As String
    '
    Cmd = "LIGHT APPLY"
    '
    Cmd = Cmd & " SELECTED=" & OnOff(LightSel.Value)
    '
    Ed.BeginSlowTask "Illuminating world"
    Ed.Server.SlowExec Cmd
    Ed.EndSlowTask
    '
    If (Tab1.Tab = 2) Then UpdateTab (2)
    '
End Sub

Private Sub RefreshBSPStats()
    EdPolys.Caption = Ed.Server.GetProp("BSP", "Polys")
    Nodes.Caption = Ed.Server.GetProp("BSP", "Nodes")
    MaxDepth.Caption = Ed.Server.GetProp("BSP", "MaxDepth")
    AvgDepth.Caption = Ed.Server.GetProp("BSP", "AvgDepth")
    Branches.Caption = Ed.Server.GetProp("BSP", "Branches")
    Coplanars.Caption = Ed.Server.GetProp("BSP", "Coplanars")
    Fronts.Caption = Ed.Server.GetProp("BSP", "Fronts")
    Backs.Caption = Ed.Server.GetProp("BSP", "Backs")
    Leaves.Caption = Ed.Server.GetProp("BSP", "Leaves")
    '
    ' Calculate percents:
    '
    pLeaves.Caption = ShowPercentz(Leaves.Caption)
    pBacks.Caption = ShowPercentz(Backs.Caption)
    pFronts.Caption = ShowPercentz(Fronts.Caption)
    pCoplanars.Caption = ShowPercentz(Coplanars.Caption)
    pBranches.Caption = ShowPercentz(Branches.Caption)
    '
    If Val(EdPolys.Caption) = 0 Then
        Ratio.Caption = "0"
    Else
        Ratio.Caption = Trim(Str(Int(100 * Val(Nodes.Caption) / Val(EdPolys.Caption)) / 100))
    End If
End Sub

Private Sub RefreshGeomStats()
    MapBrushes.Caption = Ed.Server.GetProp("Map", "Brushes")
    MapAdd.Caption = Ed.Server.GetProp("Map", "Add")
    MapSubtract.Caption = Ed.Server.GetProp("Map", "Subtract")
    MapSpecial.Caption = Ed.Server.GetProp("Map", "Special")
    MapAvgPolys.Caption = Ed.Server.GetProp("Map", "AvgPolys")
    MapTotalPolys.Caption = Ed.Server.GetProp("Map", "TotalPolys")
    MapPoints.Caption = Ed.Server.GetProp("Map", "Points")
    MapVectors.Caption = Ed.Server.GetProp("Map", "Vectors")
    MapSides.Caption = Ed.Server.GetProp("Map", "Sides")
    MapBounds.Caption = Ed.Server.GetProp("Map", "Bounds")
    MapZones.Caption = Ed.Server.GetProp("Map", "Zones")
    MapPlanes.Caption = Ed.Server.GetProp("Map", "Planes")
End Sub

Private Sub RefreshLightStats()
    LightCount.Caption = Ed.Server.GetProp("Light", "Count")
    LightMeshes.Caption = Ed.Server.GetProp("Light", "Meshes")
    LightMeshPts.Caption = Ed.Server.GetProp("Light", "MeshPts")
    LightCacheSize.Caption = Ed.Server.GetProp("Light", "CacheSize")
    LightAvgSize.Caption = Ed.Server.GetProp("Light", "AvgSize")
    LightMaxSize.Caption = Ed.Server.GetProp("Light", "MaxSize")
    LightMeters.Caption = Ed.Server.GetProp("Light", "Meters")
End Sub

Private Function ShowPercentz(ByVal Caption As String) As String
    '
    If Val(Nodes.Caption) = 0 Then
        ShowPercentz = "(0%)"
    Else
        ShowPercentz = "(" & Trim(Str(Int(100 * Val(Caption) / Val(Nodes.Caption)))) & "%)"
    End If
    '
End Function


Private Sub UpdateGoButton(N As Integer)
    Select Case N
        Case 0:
            Go.Caption = "&Rebuild Geometry"
        Case 1:
            Go.Caption = "&Rebuild BSP"
        Case 2:
            Go.Caption = "&Apply Lights"
    End Select
End Sub

Private Sub UpdateTab(N As Integer)
    If UpdatingTab = 0 Then
        UpdatingTab = 1 ' Prevent recursion
        Select Case N
            Case 0: RefreshGeomStats
            Case 1: RefreshBSPStats
            Case 2: RefreshLightStats
        End Select
        '
        If Tab1.Tab <> N Then
            Tab1.Tab = N
        End If
        '
        UpdateGoButton (N)
        UpdatingTab = 0
    End If
End Sub

Private Sub Tab_Click(PreviousTab As Integer)

End Sub

Private Sub Tab_DblClick()

End Sub

Private Sub Tab_DragDrop(Source As Control, X As Single, Y As Single)

End Sub


Private Sub Tab1_Click(PreviousTab As Integer)
    UpdateTab (Tab1.Tab)
End Sub

