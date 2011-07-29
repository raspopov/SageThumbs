#pragma once
#include "afxwin.h"


// CCreateWorkspace dialog

class CCreateWorkspace : public CDialog
{
	DECLARE_DYNAMIC(CCreateWorkspace)

public:
	CCreateWorkspace(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCreateWorkspace();

// Dialog Data
	enum { IDD = IDD_CREATEWORKSPACE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedChngInputrc();
	afx_msg void OnBnClickedChngLangini();
	afx_msg void OnBnClickedChngOutputrc();
	afx_msg void OnBnClickedOk();

	virtual BOOL OnInitDialog();
	BOOL OpenSaveDialog(bool bOpen, UINT nExtID, UINT nExtDescriptionID, CString& strEdit);
	
	CString m_strInputRC;
	CString m_strLangINI;
	CString m_strOutputRC;
	CString strWorkspace;
};
