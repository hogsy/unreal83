// UnWnKeyD.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogKeySelect dialog

class CDialogKeySelect : public CDialog
{
// Construction
public:
	CDialogKeySelect(CWnd* pParent = NULL);   // standard constructor
    BOOL OnInitDialog();

    BOOL * IsAllowed; // Set this to an array of 256 BOOL's, [x] is TRUE if virtual key x is allowed.
        // Leave this empty and any virtual key is allowed.
        // Esc is never allowed.
    int SelectedKey; // When the dialog ends, this has the valid selected virtual key, or 0.
// Dialog Data
	//{{AFX_DATA(CDialogKeySelect)
	enum { IDD = IDD_KEYBOARD_KEY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogKeySelect)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogKeySelect)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
