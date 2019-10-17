/*=============================================================================
	UnWnDlg.h: Header file for main server log window
	Used by: Unreal/Windows interface

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/////////////////////////////////////////////////////////////////////////////
// CUnrealWnDlg dialog
/////////////////////////////////////////////////////////////////////////////

class CUnrealWnDlg : public CDialog
	{
	//
	// Construction:
	//
	public:
	CUnrealWnDlg(CWnd* pParent = NULL);	// standard constructor
	~CUnrealWnDlg(void); // destructor
	//
	// Custom:
	//
	void Exit(void);
	void Log(const CString &Text);
	void ShowMe(void);
	CEdit *FocusEditControl;
	CRect Rect;
	//
	// Dialog Data:
	//
	//{{AFX_DATA(CUnrealWnDlg)
	enum { IDD = IDD_UNWN_DIALOG };
	CEdit	m_CmdLine;
	CEdit	m_CmdLog;
	//}}AFX_DATA
	//
	// Virtual function overrides:
	//
	virtual void OnCancel(void);
	virtual void OnOK(void);
	//
	// ClassWizard-generated virtual function overrides:
	//
	//{{AFX_VIRTUAL(CUnrealWnDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
	//
	// Implementation
	//
	protected:
	HICON m_hIcon;
	//
	// Handmade message map functions
	afx_msg LONG OnUnrealTimer(UINT,LONG);
	//
	// Generated message map functions
	//{{AFX_MSG(CUnrealWnDlg)
	afx_msg void OnSysCommand(UINT,LONG);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnExecute();
	afx_msg void OnSetfocusCmdLine();
	afx_msg void OnSetfocusCmdLog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	//
	DECLARE_MESSAGE_MAP()
	};

