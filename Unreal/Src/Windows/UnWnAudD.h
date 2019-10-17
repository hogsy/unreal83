// UnWnAudD.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogAudio dialog

class CDialogAudio : public CDialog
{
// Construction
public:
	CDialogAudio(CWnd* pParent = NULL);   // standard constructor
    BOOL Accept(); 
    BOOL OnInitDialog();
        // If input is valid, save the changes and return TRUE. Otherwise,
        // notify the user of errors and return FALSE.

// Dialog Data
	//{{AFX_DATA(CDialogAudio)
	enum { IDD = IDD_AUDIO };
	CSliderCtrl	SoundVolume;
	CSliderCtrl	MusicVolume;
	int		SampleRate;
	BOOL	UseDirectSound;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogAudio)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogAudio)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
