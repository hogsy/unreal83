/*=============================================================================
	UnPswd.h: Password dialog header

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/////////////////////////////////////////////////////////////////////////////
// CPasswordDlg dialog
/////////////////////////////////////////////////////////////////////////////

class CPasswordDlg : public CDialog
	{
	// Construction
	public:
	CPasswordDlg(CWnd* pParent = NULL);   // standard constructor
	char Title[256];
	char Prompt[256];
	char Name[256];
	char Password[256];
	char Encryption[256];
	//
	// Dialog Data
	//
	//{{AFX_DATA(CPasswordDlg)
	enum { IDD = IDD_PASSWORD };
	CStatic	m_Prompt;
	CEdit	m_Password;
	CEdit	m_Name;
	//}}AFX_DATA
	//
	// Overrides
	//
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPasswordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	//
	// Implementation
	//
	protected:
	//
	// Generated message map functions
	//{{AFX_MSG(CPasswordDlg)
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	};

/////////////////////////////////////////////////////////////////////////////
// The End
/////////////////////////////////////////////////////////////////////////////

