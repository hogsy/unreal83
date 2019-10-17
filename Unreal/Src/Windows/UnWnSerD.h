// UnWnSerD.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogServerProperties dialog

class CDialogServerProperties : public CDialog
{
// Construction
public:
	CDialogServerProperties(CWnd* pParent = NULL);   // standard constructor
    BOOL OnInitDialog();

// Dialog Data
	//{{AFX_DATA(CDialogServerProperties)
	enum { IDD = IDD_SERVER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogServerProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogServerProperties)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
