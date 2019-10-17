/*=============================================================================
	UnWnDlg.cpp: Server console's main dialog screen
	Used by: Windows console

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "StdAfx.h"
#include "UnWn.h"
#include "Unreal.h"
#include "UnWnCam.h"
#include "windowsx.h"

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnDlg dialog
/////////////////////////////////////////////////////////////////////////////

//
// Constructor.
//
CUnrealWnDlg::CUnrealWnDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUnrealWnDlg::IDD, pParent)
	{
	//{{AFX_DATA_INIT(CUnrealWnDlg)
	//}}AFX_DATA_INIT
	FocusEditControl=NULL;
	};

//
// Unused.
//
void CUnrealWnDlg::DoDataExchange(CDataExchange* pDX)
	{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnrealWnDlg)
	DDX_Control(pDX, IDC_CMDLINE, m_CmdLine);
	DDX_Control(pDX, IDC_CMDLOG, m_CmdLog);
	//}}AFX_DATA_MAP
	};

BEGIN_MESSAGE_MAP(CUnrealWnDlg, CDialog)
	ON_MESSAGE(WM_UNREALTIMER,OnUnrealTimer)
	//{{AFX_MSG_MAP(CUnrealWnDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDC_EXECUTE, OnExecute)
	ON_EN_SETFOCUS(IDC_CMDLINE, OnSetfocusCmdLine)
	ON_EN_SETFOCUS(IDC_CMDLOG, OnSetfocusCmdLog)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnDlg message handlers
/////////////////////////////////////////////////////////////////////////////

//
// Dialog initialization.  This dialog may maquerade as either a log
// window or as a splash screen.
//
BOOL CUnrealWnDlg::OnInitDialog()
	{
	GUARD;
	//
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//
	CDialog::OnInitDialog();
	//
	GetWindowRect(Rect);
	//
	// Add "About..." menu item to system menu.
	//
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	//
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	CString strAboutMenu;
	strAboutMenu.LoadString(IDS_ABOUTBOX);
	if (!strAboutMenu.IsEmpty())
		{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		};
	//
	// Set the icon for this dialog.  The framework does this automatically
	// when the application's main window is not a dialog
	//
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	//
	App.Platform.LogAlive = 1;
	App.Platform.hWndLog = (DWORD)m_hWnd;
	//
	if (App.Platform.LaunchWithoutLog)
		{
		// Hide log controls and show splash bitmap:
		GetDlgItem(IDC_CMDLOG   )->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CMDLINE  )->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EXECUTE  )->ShowWindow(SW_HIDE);
		//
		CRect R,Rect1,Rect2;
		GetClientRect(Rect1);
		//
		SetWindowLong(m_hWnd,GWL_STYLE,GetWindowLong(m_hWnd,GWL_STYLE) &
			~(WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU));
		GetWindowRect(R); R.top++; MoveWindow(R);
		//
		GetClientRect(Rect2);
		GetWindowRect(R);
		R.bottom += Rect1.bottom - Rect2.bottom;
		MoveWindow(R);
		}
	else
		{
		GetDlgItem(IDC_SPLASHER )->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COPYRT   )->ShowWindow(SW_HIDE);
		};
	return TRUE; // return TRUE unless you set the focus to a control
	//
	UNGUARD("CUnrealWnDlg::OnInitDialog");
	};

/////////////////////////////////////////////////////////////////////////////
// Command handlers
/////////////////////////////////////////////////////////////////////////////

//
// Destructor.  Unguarded.
//
CUnrealWnDlg::~CUnrealWnDlg(void)
	{
	// Does nothing.
	};

//
// Cancel button handler.
//
void CUnrealWnDlg::OnCancel(void)
	{
	GUARD;
	// Do nothing, preventing escape from closing
	UNGUARD("CUnrealWnDlg::OnCancel");
	};

//
// Ok button handler; hides the window.
//
void CUnrealWnDlg::OnOK(void)
	{
	GUARD;
	//
	GetWindowRect	(Rect);
	ShowWindow		(SW_HIDE);
	SetWindowPos	(NULL,0,0,0,0,0);
	//
	UNGUARD("CUnrealWnDlg::OnOK");
	};

//
// Exit handler.  Closes the window, which causes execution to unwind and
// return to CUnrealWnApp::InitInstance so that the application
// may terminate.
//
void CUnrealWnDlg::Exit(void)
	{
	GUARD;
	EndDialog(IDOK);
	PostQuitMessage(0);
	UNGUARD("CUnrealWnDlg::Exit");
	};

//
// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.
//
void CUnrealWnDlg::OnPaint() 
	{
	GUARD;
	//
	static int Initial=0;
	if (IsIconic())
		{
		CPaintDC dc(this); // device context for painting
		//
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);
		//
		// Center icon in client rectangle
		//
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		//
		// Draw the icon
		//
		dc.DrawIcon(x, y, m_hIcon);
		}
	else CDialog::OnPaint();
	if (!Initial)
		{
		Initial=1;
		//
		if (App.Platform.LaunchWithoutLog)
			{
			GetDlgItem(IDC_SPLASHER )->UpdateWindow();
			GetDlgItem(IDC_COPYRT   )->UpdateWindow();
			}
		else
			{
			GetDlgItem(IDC_CMDLOG   )->UpdateWindow();
			GetDlgItem(IDC_CMDLINE  )->UpdateWindow();
			GetDlgItem(IDC_EXECUTE  )->UpdateWindow();
			};
		App.Platform.ServerLaunched = 1;
		//
		App.Platform.OpenLog(LOG_PARTIAL);
		App.Platform.Log(LOG_Info,"Starting " GAME_NAME " " GAME_VERSION);
		App.Platform.Log(LOG_Info,"Using " ENGINE_NAME " " ENGINE_VERSION);
		App.Platform.Log(LOG_Info,COMPILER ", " __DATE__ " " __TIME__);
		//
		App.InitializeUnreal();
		//
		if (App.Platform.LaunchWithoutLog)
			{
			ShowWindow(SW_HIDE);
			//
			GetDlgItem(IDC_CMDLOG   )->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_CMDLINE  )->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EXECUTE  )->ShowWindow(SW_SHOW);
			//
			GetDlgItem(IDC_SPLASHER )->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_COPYRT   )->ShowWindow(SW_HIDE);
			//
			SetWindowLong(m_hWnd,GWL_STYLE,GetWindowLong(m_hWnd,GWL_STYLE) |
				(WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU));
			};
		SetWindowText("Unreal Log");
		//
		CRect Rect1,Rect2;
		GetClientRect(Rect1);
		//
		static CMenu TempMenu;
		TempMenu.LoadMenu(IDR_SERVERCON);
		SetMenu(&TempMenu);
		if (!GNetManager)
			{
			TempMenu.DeleteMenu(2,MF_BYPOSITION);
			TempMenu.DeleteMenu(ID_NETGAME,MF_BYCOMMAND);
			};
		GetClientRect(Rect2);
		GetWindowRect(Rect);
		Rect.bottom += Rect1.bottom - Rect2.bottom;
		//
		if (App.Platform.LaunchWithoutLog)	SetWindowPos(NULL,0,0,0,0,SWP_NOACTIVATE);
		else								MoveWindow(Rect);
		};
	UNGUARD("CUnrealWnDlg::OnPaint");
	};

//
// The system calls this to obtain the cursor to display while the user drags
// the minimized window.
//
HCURSOR CUnrealWnDlg::OnQueryDragIcon()
	{
	GUARD;
	return (HCURSOR) m_hIcon;
	UNGUARD("CUnrealWnDlg::OnQueryDragIcon");
	};

//
// The edit control (used for log output)
//

#define MAX_LINES		300
#define DELETE_LINES	100
#define VISIBLE_LINES	16

void CUnrealWnDlg::OnExecute()
	{
	GUARD;
	//
	CString CmdLine;
	//
	m_CmdLine.GetWindowText(CmdLine);
	CmdLine.TrimLeft();
	CmdLine.TrimRight();
	//
	if (CmdLine.GetLength())
		{
		App.Platform.Log(LOG_Console,CmdLine);
		GUnreal.Exec(LPCSTR(CmdLine));
		};
	m_CmdLine.SetWindowText("");
	//
	UNGUARD("CUnrealWnDlg::OnExecute");
	};

void CUnrealWnDlg::OnSetfocusCmdLine() 
	{FocusEditControl=&m_CmdLine;};

void CUnrealWnDlg::OnSetfocusCmdLog() 
	{FocusEditControl=&m_CmdLog;};

/////////////////////////////////////////////////////////////////////////////
// Windows events
/////////////////////////////////////////////////////////////////////////////

//
// Timer. Currently unused.
//
LONG CUnrealWnDlg::OnUnrealTimer(UINT wParam,LONG lParam)
	{
	GUARD;
	return 0;
	UNGUARD("CUnrealWnDlg::OnUnrealTimer");
	};

//
// System menu command handler.
//
void CUnrealWnDlg::OnSysCommand(UINT wParam, LPARAM lParam)
	{
	GUARD;
	//
	int nID = wParam & 0xFFF0;
	//
	if (nID==IDM_ABOUTBOX) // About
		{
		App.OnHelpAboutUnreal();
		}
	else if (nID==SC_CLOSE) // Close button
		{
		OnOK();
		}
	else // Another message we don't handle
		{
		CDialog::OnSysCommand(wParam,lParam);
		};
	//return 0;
	UNGUARD("CUnrealWnDlg::OnRawSysCommand");
	};

/////////////////////////////////////////////////////////////////////////////
// Custom members
/////////////////////////////////////////////////////////////////////////////

//
// Print a logging string in the log window
//
void CUnrealWnDlg::Log(const CString &Text)
	{
	GUARD;
	//
	int		TopEditLine,NumLines,LastLineIsOnScreen,LastChar;
	//
	if (m_CmdLog.GetLineCount()>MAX_LINES) // Delete starting stuff
		{
		m_CmdLog.SetSel     (0,m_CmdLog.LineIndex(DELETE_LINES),1);
		m_CmdLog.ReplaceSel ("");
		};
	NumLines           = m_CmdLog.GetLineCount();
	TopEditLine        = m_CmdLog.GetFirstVisibleLine();
	NumLines           = m_CmdLog.GetLineCount();
	//
	LastLineIsOnScreen = (NumLines - VISIBLE_LINES) >= TopEditLine;
	//
	if (Text.GetLength()>0)
		{
		LastChar=m_CmdLog.LineIndex(m_CmdLog.GetLineCount());
		//
		m_CmdLog.SetSel     (LastChar-1,LastChar-1,1);
		m_CmdLog.ReplaceSel ((CString)" "+Text+"\r\n");
		};
	UNGUARD("CUnrealWnDlg::Log");
	};

//
// Show/hide the dialog.
//
void CUnrealWnDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
	{
	GUARD;
	//
	CDialog::OnShowWindow(bShow, nStatus);
	//
	UNGUARD("CUnrealWnDlg::OnShowWindow");
	};

//
// Show the dialog.
//
void CUnrealWnDlg::ShowMe(void)
	{
	GUARD;
	//
	MoveWindow(Rect);
	ShowWindow(SW_SHOWNORMAL);
	//
	m_CmdLine.SetFocus();
	//
	int LastChar=m_CmdLog.LineIndex(m_CmdLog.GetLineCount());
	m_CmdLog.SetSel(LastChar-1,LastChar-1,0);
	//
	UNGUARD("CUnrealWnDlg::ShowMe");
	};

/////////////////////////////////////////////////////////////////////////////
// The End
/////////////////////////////////////////////////////////////////////////////
