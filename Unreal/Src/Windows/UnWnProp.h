// UnWnProp.h : header file

/////////////////////////////////////////////////////////////////////////////
// CDialogProperties dialog

class CDialogProperties : public CDialog
{
public:
	CDialogProperties(CWnd* pParent = NULL);   // standard constructor
    BOOL OnInitDialog();
    void DestroyCurrentPage(); // Destroy any existing window in PageDialog.
    void StartNewPage(); // Show a new page for the currently selected tab.
    BOOL Accept();
        // Invoke the Accept() routine for the current page, if appropriate.
        // Returns TRUE if the input was accepted and saved, FALSE otherwise
        // (in which case an error message should have been reported to the
        // user).

    static CDialog * LatestInstance;
        // This is set during InitDialog to the latest instance of this dialog class.
        // Use this if you want to force only a single instance (which is suggested).
        // This is set to 0 when the latest instance is deleted.

// Dialog Data
	//{{AFX_DATA(CDialogProperties)
	enum { IDD = IDD_PROPERTIES };
	CTabCtrl	TabControl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogProperties)
	afx_msg void OnSelchangeTabs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangingTabs(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnOpen();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    typedef enum // The various pages for the tab group, in the order they appear.
    {
        NoPage              // Indicates no page (always 0)
    ,   PreferencesPage
    ,   AudioPage
    ,   VideoPage
    ,   DevicesPage
    ,   ActionsPage
    ,   SensitivityPage
//todo:add    ,   ServerPage
    }
    PageType;
    PageType CurrentPage;
    PageType SelectedPage() const; // Returns the currently select page, or NoPage.
    CDialog * PageDialog; // The dialog used to show the current page, or 0 if none.
    int PageDialogId; // Control Id of the current page, undefined if PageDialog==0.
};
