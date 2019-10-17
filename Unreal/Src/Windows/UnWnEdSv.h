/*=============================================================================
	UnWnEdSv.h: Header file for CUnrealEdServer OLE class
	Used by: Unreal/Windows interface

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/////////////////////////////////////////////////////////////////////////////
// CUnrealEdServer window
/////////////////////////////////////////////////////////////////////////////

class CUnrealEdServer : public CWnd
	{
	DECLARE_DYNCREATE(CUnrealEdServer)
	//
	// Construction
	//
	public:
	CUnrealEdServer();
	//
	// Attributes
	//
	public:
	//
	// Operations
	//
	public:
	//
	// Overrides
	//
	// ClassWizard-generated virtual function overrides
	//{{AFX_VIRTUAL(CUnrealEdServer)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL
	//
	// Implementation
	//
	public:
	virtual ~CUnrealEdServer();
	//
	// Generated message map functions
	//
	protected:
	//{{AFX_MSG(CUnrealEdServer)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(CUnrealEdServer)
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CUnrealEdServer)
	afx_msg void Exec(LPCTSTR Cmd);
	afx_msg void SlowExec(LPCTSTR Cmd);
	afx_msg BSTR GetProp(LPCTSTR Topic, LPCTSTR Item);
	afx_msg void SetProp(LPCTSTR Topic, LPCTSTR Item, LPCTSTR NewValue);
	afx_msg void Enable();
	afx_msg void Disable();
	afx_msg void Init(long hWndMain, long hWndEdCallback);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
	};

/////////////////////////////////////////////////////////////////////////////
