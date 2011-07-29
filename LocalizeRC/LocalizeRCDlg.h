/*
LocalizeRC - A tool for localizing/translating Resource Scripts (RC files).

Copyright (C) Konrad Windszus, 2003-2004.
Copyright (C) Nikolay Raspopov, 2011.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#pragma once

class CLocalizeRCDlg : public CDialog
{
public:
	CLocalizeRCDlg(CWnd* pParent = NULL);	// standard constructor

	enum { IDD = IDD_LOCALIZERC_DIALOG };

	LANGID		m_LangID;
	BOOL		m_bRun;
	CString		m_strWorkspace;

protected:
	HICON		m_hLargeIcon;
	HICON		m_hSmallIcon;
	// content of rich edit control (input RC)
	CString		m_strEdit;
	// filename of resource-file with source language 
	CString		m_strInputRC;
	// file of the INI File with IDC and text-strings
	CString		m_strLangINI;
	// filename of the generated RC
	CString		m_strOutputRC;
	BOOL		m_bCopy;
	int			m_nObsoleteItems;
	CComboBox	m_CtrlLanguage;
	CString		m_strTextmode;
	CString		m_strAbout;
	BOOL		m_bNoSort;	

	// checks if line contents strings that have to be translated
	bool MustBeTranslated(CString strLine, CString strKeyword);
	// search for '"' that is not a quotation mark inside the text ("")
	int FindQuote(CString strLine, int nStartPos = 0);
	int WriteReadIni(bool bWrite);
	CString ExtractCaption(CString& strText, int* pnPosition, CString strKeyword, CString &strIDC);
	static CString GetFolder(CString strPath);

	BOOL OpenInputRC(BOOL bShowError = TRUE);
	BOOL LoadWorkspace(BOOL bShowError = TRUE);
	BOOL SaveWorkspace();
	CString GetAbsolutePathFromIni(CIniEx* pIniEx, CString strKey, CString strPath);
	
	static int AddLanguage(CComboBox* pComboBox, LPCTSTR strLangCode, LANGID SelectedID);
	static CString StringTokenize(CString strSource, LPCTSTR strDelimiters, int* pnStart);
	static int FindSeperateWord(CString strText, LPCTSTR strWord, int nStartPos);

	static int StringSpanIncluding( LPCTSTR pszBlock, LPCTSTR pszSet ) throw()
	{
		return (int)_tcsspn( ( pszBlock ), ( pszSet ) );

	}

	static int StringSpanExcluding( LPCTSTR pszBlock, LPCTSTR pszSet ) throw()
	{
		return (int)_tcscspn( ( pszBlock ), ( pszSet ) );
	}

	BOOL OpenSaveDialog(BOOL bOpen, BOOL bRelative, UINT nExtID, UINT nExtDescriptionID, CString& strEdit, CString strIniEntry);
	BOOL ShowError(UINT nIDString1, bool bGetLastError = false, DWORD dwErrCode = 0);

	static CString ConvertErrorToString( DWORD dwErrCode );

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedNewWorkspace();
	afx_msg void OnBnClickedChngWorkspace();
	afx_msg void OnBnClickedReverseini();
	afx_msg void OnCbnSelchangeLanguage();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedCreateini();
	afx_msg void OnBnClickedOpenini();
	afx_msg void OnBnClickedCreateoutput();
	afx_msg void OnCbnSelchangeObsItems();
	afx_msg void OnBnKillfocusCopy();
	afx_msg void OnBnKillfocusNosort();
	afx_msg void OnBnClickedChngInputrc();
	afx_msg void OnBnClickedChngLangini();
	afx_msg void OnBnClickedChngOutputrc();

	DECLARE_MESSAGE_MAP()
};
