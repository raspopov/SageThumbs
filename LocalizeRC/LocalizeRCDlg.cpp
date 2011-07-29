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

#include "stdafx.h"
#include "LocalizeRC.h"
#include "LocalizeRCDlg.h"

#define OLDFILEFORMAT (-2)

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define ClickButton(x) PostMessage( WM_COMMAND, MAKELONG( (x), BN_CLICKED ), (LPARAM)GetDlgItem( (x) )->GetSafeHwnd() )

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	enum { IDD = IDD_ABOUTBOX };

protected:
	CString m_strAbout;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg()
	: CDialog( CAboutDlg::IDD )
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ABOUT, m_strAbout);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_strAbout.LoadString( IDS_ABOUT );
	UpdateData( FALSE );

	return TRUE;
}

// CLocalizeRCDlg dialog

CLocalizeRCDlg::CLocalizeRCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLocalizeRCDlg::IDD, pParent)
	, m_hLargeIcon( NULL )
	, m_hSmallIcon( NULL )
	, m_bCopy( FALSE )
	, m_nObsoleteItems( 0 )
	, m_bNoSort( FALSE )
	, m_bRun( FALSE )
{
}

void CLocalizeRCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_COPY, m_bCopy);
	DDX_CBIndex(pDX, IDC_OBS_ITEMS, m_nObsoleteItems);
	DDX_Text(pDX, IDC_WORKSPACE, m_strWorkspace);
	DDX_Control(pDX, IDC_LANGUAGE, m_CtrlLanguage);
	DDX_Text(pDX, IDC_EDIT, m_strEdit);
	DDX_Text(pDX, IDC_TEXTMODE, m_strTextmode);
	DDX_Check(pDX, IDC_NOSORT, m_bNoSort);
	DDX_Text(pDX, IDC_INPUTRC, m_strInputRC);
	DDX_Text(pDX, IDC_LANGINI, m_strLangINI);
	DDX_Text(pDX, IDC_OUTPUTRC, m_strOutputRC);
	DDX_Text(pDX, IDC_ABOUT, m_strAbout);
}

BEGIN_MESSAGE_MAP(CLocalizeRCDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CREATEINI, OnBnClickedCreateini)
	ON_BN_CLICKED(IDC_OPENINI, OnBnClickedOpenini)
	ON_BN_CLICKED(IDC_CREATEOUTPUT, OnBnClickedCreateoutput)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_REVERSEINI, OnBnClickedReverseini)
	ON_BN_CLICKED(IDC_CHNG_WORKSPACE, OnBnClickedChngWorkspace)
	ON_CBN_SELCHANGE(IDC_LANGUAGE, OnCbnSelchangeLanguage)
	ON_CBN_SELCHANGE(IDC_OBS_ITEMS, OnCbnSelchangeObsItems)
	ON_BN_KILLFOCUS(IDC_COPY, OnBnKillfocusCopy)
	ON_BN_KILLFOCUS(IDC_NOSORT, OnBnKillfocusNosort)
	ON_BN_CLICKED(IDC_NEW_WORKSPACE, OnBnClickedNewWorkspace)
	ON_BN_CLICKED(IDC_CHNG_INPUTRC, OnBnClickedChngInputrc)
	ON_BN_CLICKED(IDC_CHNG_LANGINI, OnBnClickedChngLangini)
	ON_BN_CLICKED(IDC_CHNG_OUTPUTRC, OnBnClickedChngOutputrc)
END_MESSAGE_MAP()

// CLocalizeRCDlg message handlers

BOOL CLocalizeRCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	m_strWorkspace.Trim( _T(" \t\r\n\"") );
	if ( m_strWorkspace.IsEmpty() )
		// last used Workspace from registry
		m_strWorkspace = AfxGetApp()->GetProfileString( SEC_LASTPROJECT, ENT_WORKSPACE );

	// LoadIcon only loads the 32x32 icon, therefore use ::LoadImage for other sizes (16x16)
	m_hLargeIcon = AfxGetApp()->LoadIcon ( IDR_MAINFRAME );
	m_hSmallIcon = (HICON) ::LoadImage ( AfxGetResourceHandle(), 
                                         MAKEINTRESOURCE(IDR_MAINFRAME),
                                         IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hLargeIcon, TRUE);		// Set big icon
	SetIcon(m_hSmallIcon, FALSE);		// Set small icon

	// display text mode (UNICODE/ANSI)
#ifdef UNICODE
	m_strTextmode.LoadString( IDS_UNICODE );
#else
	m_strTextmode.LoadString( IDS_ANSI );
#endif
	m_strAbout.LoadString( IDS_ABOUT );

	// get installed languages
	CFileFind Finder;
	TCHAR szFilename[MAX_PATH];
	GetModuleFileName( NULL, szFilename, MAX_PATH );
	
	AddLanguage( &m_CtrlLanguage, _T("09"), m_LangID );
	
	// the last 2 chars are the LanguageID in hexadecimal form
	CString strSearch;
	strSearch.Format( _T("%sLocalizeRC??.dll"), GetFolder(szFilename) );

	BOOL bResult = Finder.FindFile( strSearch );
	CString strLangCode;

	while( bResult )
	{
		bResult = Finder.FindNextFile();
		// extract 2-digit language code
		strLangCode = Finder.GetFileTitle().Right(2);
		AddLanguage( &m_CtrlLanguage, strLangCode, m_LangID );
	}
	Finder.Close();

	// Load last workspace
	LoadWorkspace( FALSE );

	if ( m_bRun )
		ClickButton( IDC_CREATEINI );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLocalizeRCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLocalizeRCDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hSmallIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLocalizeRCDlg::OnQueryDragIcon()
{
	return static_cast< HCURSOR >( m_hSmallIcon );
}

#define NUMKEYWORDS 8
LPCTSTR strKeyword[NUMKEYWORDS] = 
{
	_T("ACCELERATORS"),
	_T("DIALOG"),
	_T("DIALOGEX"),
	_T("MENU"),
	_T("MENUEX"),
	_T("STRINGTABLE"),
	_T("DLGINIT"),
	_T("code_page")
};
	
// random string, that indicates an error
#define ERR_STR _T("asfdshkagzuwrthgadsfhgkh12385143258")

BOOL CLocalizeRCDlg::OpenInputRC(BOOL bShowError)
{
	// The file from which to load the contents of rich edit control
	CStdioFile* pFile = NULL;
	
	if ( m_strInputRC.IsEmpty() )
	{
		if ( bShowError )
			AfxMessageBox( IDS_ERR_FILENAMEEMPTY );
		// clear contents in edit-control
		m_strEdit = _T("");
		UpdateData( FALSE );
		return FALSE;
	}

	BOOL bIsUnicode = CStdioUnicodeFile::IsUnicode( m_strInputRC );

	try
	{
	#ifdef UNICODE	
		if( bIsUnicode )
		{
			pFile = new CStdioUnicodeFile( m_strInputRC, CFile::modeRead | CFILEFLAG_UNICODEHELPER );
			// skip first two bytes
			pFile->Seek( 2, CFile::begin );
		}
		else
			pFile = new CStdioUnicodeFile( m_strInputRC, CFile::modeRead );
	#else
		if( bIsUnicode )
		{
			CString strErr;
			strErr.Format( IDS_UNICODEFILE, m_strInputRC );
			AfxMessageBox( strErr );

			// clear contents in edit-control
			m_strEdit = _T("");
			UpdateData( FALSE );
			return false;	// cancel
		}
		else
			pFile = new CStdioUnicodeFile( m_strInputRC, CFile::modeRead );
	#endif
	}
	catch( CFileException* e )
	{
		TCHAR szCause[255];
		e->GetErrorMessage(szCause, 255);
		AfxMessageBox(szCause);
		e->Delete();

		// clear contents in edit-control
		m_strEdit = _T("");
		UpdateData( FALSE );
		return FALSE;
	}
	// clear contents in edit-control
	m_strEdit = _T("");

	// fill edit-control with contents of the RC-file
	CString strLine;
	while( pFile->ReadString( strLine ) )
		m_strEdit += strLine + _T("\r\n");
	UpdateData( FALSE );

	UpdateWindow();

	pFile->Close();
	delete pFile;
	return TRUE;
}

void CLocalizeRCDlg::OnBnClickedCreateini()
{
	CWaitCursor wc;
	UpdateWindow();

	// Reload InputRC
	if ( ! OpenInputRC() )
	{
		m_bRun = FALSE;
		return;
	}

	// Write/Actualize INI-File
    if ( WriteReadIni( true ) == OLDFILEFORMAT )
	{
		m_bRun = FALSE;
		AfxMessageBox( IDS_OLDFILEFORMAT );
		return;
	}

	if ( m_bRun )
		ClickButton( IDC_CREATEOUTPUT );
}

// extract first caption after *pnPosition
CString CLocalizeRCDlg::ExtractCaption(CString& strText, int* pnPosition, CString strKeyword, CString &strIDC )
{
	CString strCaption, strLine;
	int nStart, nEnd, nStartQuote, nEndQuote;

	nStart = *pnPosition;

	// DLGINIT: Microsoft-Format to store data for comboboxes
	if( strKeyword == _T("DLGINIT") )
	{		
		if( nStart == 0 )
		{
			// remove BEGIN and END
			nStart = FindSeperateWord( strText, _T("BEGIN"), 0 );
			if( nStart != -1 )
				nStart += 7;	
		}

		// FORMAT: <IDC>, 0x403, <LENGTH IN BYTES>, 0
		// FORMAT: <WORD> as Hex Wert chars in LOBYTE und HIGHBYTE
		// comma seperated, null-terminated, ends with comma

		// comma seperated, null-terminated, ends with comma
		CString strHelp;
		LPTSTR pHelp;
		int nWord, nToken = 0, nLength = 0;

		CString strToken = StringTokenize( strText, _T(",\r\n"), &nStart);
		while (nStart != -1)
		{
			strToken.TrimLeft();
			strToken.TrimRight();
			if( strToken.IsEmpty() )
			{
				strToken = StringTokenize( strText, _T(",\r\n"),&nStart);
				continue;
			}

			switch( nToken )
			{
				case 0:		// IDC	oder 0 bei Ende von Liste
					if( strToken == "0" )
					{
						*pnPosition = nStart;
						return strCaption;
					}
					else // IDC
					{
						if( strIDC.IsEmpty() )
						{
							strIDC = strToken; 	// new IDC
						}
						else
						{
							if( strIDC != strToken )
								return strCaption;       
						}
					}

					break;
				case 1:		// 0x403 for Combo-Boxes
					if( strToken != "0x403" )
						return ERR_STR;
					break;
				case 2:		// Length of String
					nLength = _ttoi( strToken );
					nLength = (int)floor((nLength / 2.0)+0.5);
					break;
				case 3:		// always 0
					break;
				default:	// everything above 3 is word values
					if( nToken <= nLength + 3 )
					{
						// convert string to hexadecimal integer
						nWord = _tcstol( strToken, &pHelp, 16 );
						strHelp.Format( _T("%c%c"), LOBYTE(nWord), HIBYTE(nWord) );
						strHelp.Remove( '\0' );
						strCaption += strHelp;
						if( nToken == nLength + 3 )	// last word
						{
							strCaption += ";";
							nToken = -1;
						}
					}
			}
			nToken++;
			*pnPosition = nStart;
			strToken = StringTokenize(strText, _T(",\r\n"), &nStart);
		}
		return ERR_STR;
	}	

	// find end of line
	while( (nEnd = strText.Find( _T("\r\n"), nStart )) != -1 )
	{
		// strLine contents one command line
		strLine += strText.Mid( nStart, nEnd - nStart );
					
		if( strLine.GetLength() <= 0 )
		{
			// next line
			nStart = nEnd+2;
			*pnPosition = nStart;
			continue;
		}
		
		
		int rPos = strLine.GetLength()-1;
		// skip spaces at the end of line
		while (rPos >= 0 && strLine[rPos] == ' ')
		{
			rPos--;
		}

		// merge multiline commands to one line
		if( (strLine[rPos] == '|') || 
			(strLine[rPos] == ',') )
		{
			// next line
			nStart = nEnd+2;
			continue;
		}

		TCHAR chHelp;
		if( nStart > 0 )
			chHelp = strText[nStart-1];

		// search for first " (is never within "") so even empty strings ("") are found
		nStartQuote = strLine.Find( _T('"') );
		if ( nStartQuote >= 0 )
			nStartQuote++;

		if( nStartQuote == -1  || !MustBeTranslated(strLine, strKeyword) )	// not found
		{
			strLine = "";
			nStart = *pnPosition = nEnd+2;
			continue;
		}
		// find last " and ignore "" (quotation marks within strings)
		nEndQuote = FindQuote( strLine, nStartQuote );
		if( nEndQuote == -1 )
		{
			strLine = "";
			nStart = *pnPosition = nEnd+2;
			continue;
		}

		nStartQuote--;
		strCaption = strLine.Mid( nStartQuote, nEndQuote-nStartQuote );
		*pnPosition += nEndQuote;
		chHelp = strText[*pnPosition];
		return strCaption;
	}
	return ERR_STR;
}

void CLocalizeRCDlg::OnBnClickedOpenini()
{
	if( m_strLangINI.IsEmpty() )
		AfxMessageBox( IDS_ERR_FILENAMEEMPTY );
	else
	{
		// open lang.ini with standard association
		HINSTANCE hInstance = ShellExecute( m_hWnd, _T("open"), m_strLangINI, NULL, NULL, SW_SHOWNORMAL );
		if( (int)hInstance <= 32 )
			ShowError( IDS_ERR_OPENINI, false, (DWORD)hInstance );
	}
}

void CLocalizeRCDlg::OnBnClickedCreateoutput()
{	
	CWaitCursor wc;
	UpdateWindow();

	if ( ! UpdateData() )
	{
		m_bRun = FALSE;
		return;
	}

	int nResult = WriteReadIni( false );
	if ( nResult == OLDFILEFORMAT )
	{
		m_bRun = FALSE;
		AfxMessageBox( IDS_OLDFILEFORMAT );
		return;
	}
	if ( nResult <= 0 )
	{
		m_bRun = FALSE;
		return; 
	}

	// create output RC
	if ( m_strOutputRC.IsEmpty() )
	{
		m_bRun = FALSE;
		AfxMessageBox( IDS_ERR_FILENAMEEMPTY );
		return;
	}

	try
	{
		CFile File( m_strOutputRC, CFILEFLAG_UNICODEHELPER |
			CFile::modeWrite | CFile::modeCreate );
	#ifdef UNICODE
		// write 0xFF 0xFE
		BYTE fffe[ 2 ] = { 0xFF, 0xFE };
		File.Write(fffe, 2);
	#endif
		File.Write( m_strEdit, sizeof(TCHAR) * m_strEdit.GetLength() );
		File.Close();
	}
	catch ( CFileException* e )
	{
		m_bRun = FALSE;
		TCHAR szCause[ 255 ] = {};
		e->GetErrorMessage(szCause, 255);
		AfxMessageBox(szCause);
		e->Delete();
	}

	// copy header and res-Folder
	if ( m_bCopy )
	{
		CString strOutputFolder = GetFolder( m_strOutputRC );
		CString strInputFolder = GetFolder( m_strInputRC );

		// copy header
		if( ! CopyFile( strInputFolder + _T("resource.h"), strOutputFolder + _T("resource.h"), false ) )
		{
			m_bRun = FALSE;
			ShowError( IDS_ERR_FILECOPY, true );
			return;
		}

		// create folder
		if ( ! CreateDirectory( strOutputFolder + _T("res"), NULL ) )
		{
			DWORD dwErrCode = GetLastError();
			if ( dwErrCode != ERROR_ALREADY_EXISTS )
			{
				m_bRun = FALSE;
				ShowError( IDS_ERR_FOLDERCREATE, false, dwErrCode );
				return;
			}
		}

		CFileFind FFind;
		BOOL bWorking = FFind.FindFile( strInputFolder+_T("res\\*.*"), 0 );
		while ( bWorking ) 
		{
			bWorking = FFind.FindNextFile();

			// skip . and .. files; otherwise, we'd
			// recur infinitely!

			if ( FFind.IsDots() || FFind.IsDirectory() )
				continue;
			else
			{
				// copy file
				if ( ! CopyFile( FFind.GetFilePath(), strOutputFolder + _T("res\\") + FFind.GetFileName(), false ) )
				{
					m_bRun = FALSE;
					ShowError( IDS_ERR_FILECOPY, true );
					return;
				}
			}
		}
		FFind.Close();
	}

	// Fill Edit with InputRC again
	OpenInputRC();

	if ( m_bRun )
		EndDialog( 0 );
}

// checks if line contents strings that have to be translated
bool CLocalizeRCDlg::MustBeTranslated(CString strLine, CString strKeyword )
{
	// if it is stringtable -> translate
	if( strKeyword == ::strKeyword[5] )
		return true;

	bool bTranslate = true;

	strLine.TrimLeft(_T(" "));
	strLine.TrimRight(_T(" "));
		
	if(strLine[0] == '#') // Preprocessor line
		bTranslate = false;

	if(strLine[0] == '/') // Comment line
		bTranslate = false;

	//--- Exclude following controls from translation ---------------------
	if(strLine.Find(_T("msctls_updown32"), 0) > 0) // Spin control
		bTranslate = false;

	if(strLine.Find(_T("SysTreeView32"), 0) > 0) // Tree view control
		bTranslate = false;

	if(strLine.Find(_T("msctls_trackbar32"), 0) > 0) // Slider control
		bTranslate = false;

	if(strLine.Find(_T("SysIPAddress32"), 0) > 0) // IP adress
		bTranslate = false;

	if(strLine.Find(_T("msctls_hotkey32"), 0) > 0) // Hot key
		bTranslate = false;

	if(strLine.Find(_T("SysListView32"), 0) > 0) // List view control
		bTranslate = false;

	if(strLine.Find(_T("SysAnimate32"), 0) > 0) // Animate control
		bTranslate = false;

	if(strLine.Find(_T("SysMonthCal32"), 0) > 0) // Month calendar
		bTranslate = false;

	if(strLine.Find(_T("ComboBoxEx32"), 0) > 0) // Extended combo box
		bTranslate = false;

	if( !bTranslate )
		return false;

	bTranslate = false;

	//--- Include following controls into translation --------------- 
	if(strLine.Find(_T("CAPTION"), 0) == 0)
		bTranslate = true;					// Dialog box caption

	if(strLine.Find(_T("POPUP"), 0) == 0)
		bTranslate = true;

	if(strLine.Find(_T("MENUITEM"), 0) == 0)
		bTranslate = true;	

	if(strLine.Find(_T("PUSHBUTTON"), 0) == 0)
		bTranslate = true;

	if(strLine.Find(_T("DEFPUSHBUTTON"), 0) == 0)
		bTranslate = true;

	if(strLine.Find(_T("LTEXT"), 0) == 0)
		bTranslate = true;

	if(strLine.Find(_T("RTEXT"), 0) == 0)
		bTranslate = true;

	if(strLine.Find(_T("CTEXT"), 0) == 0)
		bTranslate = true;

	if(strLine.Find(_T("GROUPBOX"), 0) == 0)
		bTranslate = true;

	if(strLine.Find(_T("CONTROL"), 0) == 0)
	{ 
		if(strLine.Find(_T("BS_AUTORADIOBUTTON"), 0) != -1 )
			bTranslate = true;

		if(strLine.Find(_T("BS_AUTOCHECKBOX"), 0) != -1 )
			bTranslate = true;

		if( strLine.Find(_T("Button"), 0) != -1 )
			bTranslate = true;

		// statics with SS_LEFTNOWORDWRAP are controls
		if(strLine.Find(_T("SS_LEFTNOWORDWRAP"), 0) != -1 )
			bTranslate = true;
	}
	if( strLine.Find( _T("ID"), 0) == 0)	// stringtable
		bTranslate = true;
	
	if(strLine.Find( _T("AFX_IDS_"), 0) == 0)
		bTranslate = true;

	if( strLine[0] == '"' )				// accelerator
		bTranslate = true;

    return bTranslate;
}

// return the position where '"' was found
int CLocalizeRCDlg::FindQuote(CString strLine, int nStartPos)
{
	int nPosition = nStartPos;
	
	// search for '"', that is not a quotation mark inside the text ("")
	do 
	{
		nPosition = strLine.Find( '"', nPosition );
		// no '"' was found
		if( nPosition == -1 )
			return nPosition;

		nPosition++;

		if( nPosition >= strLine.GetLength()-1 )
			return nPosition;
	}
	while( strLine[nPosition++] == '"' );

	return nPosition-1;
}

#define PREFIX_CHANGEDITEM _T("*")

int CLocalizeRCDlg::WriteReadIni(bool bWrite)
{
	BYTE nKeyword;
	
	if( m_strLangINI.IsEmpty() )
	{
		AfxMessageBox( IDS_ERR_FILENAMEEMPTY );
		return false;
	}

	// Create or Open INI-File
	CIniEx IniEx;
	if( !IniEx.Open( m_strLangINI ) )
		return false;

	// old sectionnames in INI file ?
	LPCTSTR strOldKeywords[5] =
	{
		_T(" ACCELERATORS "),
		_T(" DIALOG "),
		_T(" DIALOGEX "),
		_T(" MENU "),
		_T(" MENUEX ")
	};

	for( nKeyword = 0; nKeyword < 5; nKeyword++ )
	{
		CString strHelp = strOldKeywords[nKeyword];
		if( IniEx.LookupSection(&strHelp) != -1 )
			return OLDFILEFORMAT;
	}

	CString strCaption, strHelp, strValue, strIDC;
	int nStart, nHelp, nPosition, nSelStart, nSelEnd, nOldLength, nNewLength, nPrevHelp;

	for( nKeyword = 0; nKeyword < NUMKEYWORDS-1; nKeyword++ )
	{
		nPosition = 0;
		// search for keyword section in RC-file
		while( (nPosition = FindSeperateWord( m_strEdit, strKeyword[nKeyword], nPosition )) != -1 )
		{
			nStart = nPosition;
			
			// find BEGIN
			nHelp = FindSeperateWord( m_strEdit, _T("BEGIN"), nPosition );
			
			// find related END (ignore interlocked BEGINs-ENDs)
			nPosition = nHelp-1;
			do
			{
				nPosition = FindSeperateWord( m_strEdit, _T("END"), nPosition+1);
				nHelp = FindSeperateWord( m_strEdit, _T("BEGIN"), nHelp+1 );
			}
			while( nHelp < nPosition && nHelp != -1 );
			
			// save SECTION in strHelp
			strHelp = m_strEdit.Mid( nStart, nPosition-nStart );
			
			nHelp = nPrevHelp = 0;

			// extract caption
			while( (strCaption = ExtractCaption( strHelp, &nHelp, strKeyword[nKeyword], strIDC )) != ERR_STR )
			{
				if( bWrite )
				{
					// check if key in Lang.INI already exists
					strValue = IniEx.GetValue( strKeyword[nKeyword], strCaption );
					if( strValue.IsEmpty() )
					{
						// insert line in Lang.INI
						IniEx.SetValue( strKeyword[nKeyword], strCaption, PREFIX_CHANGEDITEM+strCaption );
					}
					else
					{
						// insert line in Lang.INI
						IniEx.SetValue( strKeyword[nKeyword], strCaption, PREFIX_CHANGEDITEM+strValue );
					}
				}
				else
				{
					// check if key in Lang.INI exists
					strValue = IniEx.GetValue( strKeyword[nKeyword], strCaption );
					if( !strValue.IsEmpty() )
					{
						// if it is DLGINIT
						if( nKeyword == 6 )
						{
							CString strInsert, strToken, strHelp2;
							WORD wData;
							int n;
							int nTokenPos = 0;

							while( (strToken = StringTokenize( strValue, _T(";"), &nTokenPos )) != "" )
							{
								// first line: header
								strHelp2.Format(_T("\t%s, 0x403, %d, 0\r\n"), strIDC, strToken.GetLength()+1 );
								strInsert += strHelp2;

								// second line: data
								for( n=0; n < strToken.GetLength(); n+=2 )
								{
									if( n == (strToken.GetLength() - 1) )
										wData = MAKEWORD( strToken.GetAt(n), 0 );
									else
										wData = MAKEWORD( strToken.GetAt(n), strToken.GetAt(n+1) );
									strHelp2.Format( _T("0x%04x, "), wData );
									strInsert += strHelp2;
								}

								// eventually add null character
								if( n == strToken.GetLength() )
								{
									strInsert += _T("\"\\000\"");
								}
								strInsert += _T("\r\n");
							}
							// only add 0 if DLGINIT block is at the end
							if( nHelp >= strHelp.GetLength() -1 )
							{
								strInsert += _T("\t0\r\n");
								nSelEnd = strHelp.GetLength();
							}
							else
								nSelEnd = nHelp;

							// find BEGIN
							if( nPrevHelp == 0 )
							{
								nSelStart = FindSeperateWord(strHelp, _T("BEGIN"), 0 );
								nSelStart += 7;
							}
							else
								nSelStart = nPrevHelp;

							nOldLength = nSelEnd-nSelStart;
							strHelp.Delete( nSelStart, nOldLength  );
							nNewLength = strInsert.GetLength();
							strHelp.Insert( nSelStart, strInsert );	
						}
						else
						{
							nSelStart = nHelp-strCaption.GetLength();
							nSelEnd = nHelp;

							nOldLength = nSelEnd-nSelStart;
							strHelp.Delete( nSelStart, nOldLength  );
							nNewLength = strValue.GetLength();
							strHelp.Insert( nSelStart, strValue );	
						}
						nHelp += nNewLength - nOldLength;
						if( nHelp < 0 )
							nHelp = 0;
					}
					nPrevHelp = nHelp;
				}
				strIDC.Empty();
			}
			if( !bWrite )
			{
				m_strEdit.Delete( nStart, nPosition-nStart );
				m_strEdit.Insert( nStart, strHelp );
				nPosition = nStart + strHelp.GetLength();
			}
		}
	}
#define CODEPAGE _T("code_page")
	// search for codepage
	////////////////////////////////////////////
	nPosition = 0;
	CString strCodepage;
	while( (nPosition = FindSeperateWord( m_strEdit, CODEPAGE, nPosition )) != -1 )
	{
		nSelEnd = m_strEdit.Find( ')', nPosition );
		nSelStart = m_strEdit.Find( '(', nPosition ) + 1;

		strCaption = m_strEdit.Mid( nSelStart, nSelEnd-nSelStart );
		
		// check if key in Lang.INI already exists
		strValue = IniEx.GetValue( CODEPAGE, strCaption );

		if( bWrite )
		{
	
			if( strValue.IsEmpty() )
			{
				// insert line in Lang.INI
				IniEx.SetValue( CODEPAGE, strCaption, PREFIX_CHANGEDITEM+strCaption );
			}
			else
			{
				// insert line in Lang.INI
				IniEx.SetValue( CODEPAGE, strCaption, PREFIX_CHANGEDITEM+strValue );
			}
		}
		else
		{
			if( !strValue.IsEmpty() )
			{
				m_strEdit.Delete( nSelStart, nSelEnd-nSelStart );
				m_strEdit.Insert( nSelStart, strValue );
				nPosition = nSelStart + strValue.GetLength();
			}
		}
		nPosition++;
	}

	if( bWrite )
	{
		// remove unnecessary lines in Lang.INI
		CStringArray strSections, strKeys;
		IniEx.GetSections( strSections );

		// go through every section
		int nSection=0;
		while( nSection < strSections.GetSize() )
		{
			strKeys.RemoveAll();
			IniEx.GetKeysInSection( strSections[nSection], strKeys );
			for( nKeyword = 0; nKeyword < NUMKEYWORDS; nKeyword++ )
			{
				// dont touch unknown sections
				if( _tcscmp(strSections[nSection], strKeyword[nKeyword]) != 0 )
					continue;

				// Go through every key in this section
				int nKey=0;
				while( nKey < strKeys.GetSize() )
				{
					// check if key is necessary for this RC
					strValue = IniEx.GetValue( strSections[nSection], strKeys[nKey] );
					if( strValue[0] == '*' )
					{
						strValue.TrimLeft( _T(" *") );
						IniEx.SetValue( strSections[nSection], strKeys[nKey], strValue );
					}
					else
					{
						// key is obsolete and no longer needed
						switch( m_nObsoleteItems )
						{
							case 0:	// delete item	
								IniEx.RemoveKey( strSections[nSection], strKeys[nKey]);
								break;
							case 1:
								if( strValue[0] != '#' )
									IniEx.SetValue( strSections[nSection], strKeys[nKey], _T("#") + strValue );
								break;
						}
					}
					nKey++;
				}
				break;
			}
			if( nKeyword == NUMKEYWORDS && strSections[nSection] != CODEPAGE  )
				IniEx.SetValue( strSections[nSection], _T("!!! This sectionname isn't recognized by LocalizeRC. Probably you can delete the whole section !!!"), _T("") );

			strKeys.RemoveAll();
			IniEx.GetKeysInSection( strSections[nSection], strKeys );
			
			// remove empty sections
			if( strKeys.GetSize() == 0 )
			{
				if( IniEx.RemoveSection( strSections[nSection] ) )
					strSections.RemoveAt( nSection );
				else
					nSection++;
			}
			else
				nSection++;
		}
		if( !m_bNoSort )
			IniEx.SortIniValues();
		// save changes
		IniEx.WriteFile();
	}
	return true;
}

void CLocalizeRCDlg::OnDestroy()
{
	CDialog::OnDestroy();

	DestroyIcon( m_hSmallIcon );
	DestroyIcon( m_hLargeIcon );

	AfxGetApp()->WriteProfileString( SEC_LASTPROJECT, ENT_WORKSPACE, m_strWorkspace );

	SaveWorkspace();
}	

CString CLocalizeRCDlg::GetFolder(CString strPath)
{
	TCHAR path[_MAX_PATH] = {};
	TCHAR drive[_MAX_DRIVE] = {};   
	TCHAR dir[_MAX_DIR] = {};

	_tcscpy_s( path, _MAX_PATH, strPath );
	
	// Trenne Pfad von Anwendungsnamen
	_tsplitpath_s( path, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0 );

	CString strReturn;
	strReturn.Format( _T("%s%s"), drive, dir);
	return strReturn;
}

void CLocalizeRCDlg::OnBnClickedReverseini()
{
	// Create or Open INI-File
	CIniEx IniEx;
	IniEx.Open( m_strLangINI );

	CStringArray strSections, strKeys;
	CString strHelp;
	IniEx.GetSections( strSections );
	int nKey, nSec;
	
	// go through every section
	for( nSec=0; nSec < strSections.GetSize(); nSec++ )
	{
		// go through every key
		IniEx.GetKeysInSection( strSections[nSec], strKeys );
		for( nKey=0; nKey < strKeys.GetSize(); nKey++ )
		{
			strHelp = IniEx.GetValue( strSections[nSec], strKeys[nKey] );	
			if( strHelp != strKeys[nKey] )
			{
				IniEx.SetValue( strSections[nSec], strHelp, strKeys[nKey] );
				IniEx.RemoveKey( strSections[nSec], strKeys[nKey] );
			}
		}
	}
	// save changes
	IniEx.WriteFile();
}

void CLocalizeRCDlg::OnBnClickedNewWorkspace()
{
	if ( OpenSaveDialog( false, false, IDS_EXTLWS, IDS_EXTLWSDESCRIPTION,
		m_strWorkspace, _T("") ) )
	{
		m_strLangINI.Empty();
		m_strInputRC.Empty();
		m_strOutputRC.Empty();
		UpdateData( FALSE );
	}
}

void CLocalizeRCDlg::OnBnClickedChngWorkspace()
{
	if ( OpenSaveDialog( true, false, IDS_EXTLWS, IDS_EXTLWSDESCRIPTION,
		m_strWorkspace, _T("") ) )
	{
		if( ! LoadWorkspace() )
			AfxMessageBox( IDS_ERR_OPENWORKSPACE );
	}
}

BOOL CLocalizeRCDlg::LoadWorkspace(BOOL bShowError)
{
	CIniEx IniEx;
	if ( ! IniEx.Open( m_strWorkspace, 1, 0 ) )
		return FALSE;

	CString strPath = GetFolder(m_strWorkspace);
	m_strInputRC = GetAbsolutePathFromIni( &IniEx, ENT_INPUTRC, strPath );
	m_strLangINI = GetAbsolutePathFromIni( &IniEx, ENT_LANGINI, strPath );
	m_strOutputRC = GetAbsolutePathFromIni( &IniEx, ENT_OUTPUTRC, strPath );
	m_bCopy = _ttoi( IniEx.GetValue(ENT_COPY) );
	m_nObsoleteItems = _ttoi( IniEx.GetValue(ENT_OBSITEMS) );
	m_bNoSort = _ttoi( IniEx.GetValue(ENT_NOSORT) );
	OpenInputRC(bShowError);

	UpdateData( FALSE );

	return TRUE;
}

BOOL CLocalizeRCDlg::SaveWorkspace()
{
	CIniEx IniEx;
	if ( ! IniEx.Open( m_strWorkspace ) )
		return FALSE;

	// save settings to Workspace
	CString strHelp;
	strHelp.Format( _T("%d"), m_bCopy );
	IniEx.SetValue( ENT_COPY, strHelp );
	strHelp.Format( _T("%d"), m_nObsoleteItems );
	IniEx.SetValue( ENT_OBSITEMS, strHelp );
	strHelp.Format( _T("%d"), m_bNoSort );
	IniEx.SetValue( ENT_NOSORT, strHelp );
	
	// save changes
	IniEx.WriteFile();

	return TRUE;
}

CString CLocalizeRCDlg::GetAbsolutePathFromIni(CIniEx* pIniEx, CString strKey, CString strPath)
{
	CString strHelp = pIniEx->GetValue( strKey );
	CString strValue;
	if( strHelp.IsEmpty() )
		return CString();

	if( PathIsRelative( strHelp ) )
	{
		TCHAR szPath[MAX_PATH];
		PathCombine( szPath, strPath, strHelp );
		strHelp.Format(_T("%s"), szPath);
		return strHelp;
	}
	return strHelp;
}

void CLocalizeRCDlg::OnCbnSelchangeLanguage()
{
	// Save Changes
	AfxGetApp()->WriteProfileInt( SEC_LASTPROJECT, ENT_LANGID, m_CtrlLanguage.GetItemData(m_CtrlLanguage.GetCurSel()) );
	AfxMessageBox( IDS_RESTARTAPP );
}

#define STR_LEN 64

int CLocalizeRCDlg::AddLanguage(CComboBox* pComboBox, LPCTSTR strLangCode, LANGID SelectedID)
{
	LCID lcID;
	TCHAR szLangName[STR_LEN] = {};
	LPTSTR pHelp;
	int nIndex;

	lcID =  MAKELCID( MAKELANGID(_tcstoul( strLangCode, &pHelp, 16 ), SUBLANG_NEUTRAL ), SORT_DEFAULT );
	GetLocaleInfo( lcID, LOCALE_SNATIVELANGNAME , szLangName, STR_LEN);
	nIndex = pComboBox->AddString( szLangName );
	pComboBox->SetItemData( nIndex, lcID );
	if( lcID == SelectedID )
		pComboBox->SetCurSel( nIndex );
	return nIndex;
}

CString CLocalizeRCDlg::StringTokenize(CString strSource, LPCTSTR pszTokens, int* pnStart)
{
#if _MFC_VER >= 0x0700
	return strSource.Tokenize( pszTokens, *pnStart );
#else
	// original code CString::Tokenize from MFC7
	/////////////////////////////////////////////////

	if( pszTokens == NULL )
	{
		return( strSource );
	}

	// (LPCSTR)(LPCTSTR) hack for VC6.
	LPCTSTR pszPlace = (LPCTSTR)strSource+*pnStart;
	LPCTSTR pszEnd = (LPCTSTR)strSource+strSource.GetLength();
	if( pszPlace < pszEnd )
	{
		int nIncluding = StringSpanIncluding( pszPlace, pszTokens );

		if( (pszPlace+nIncluding) < pszEnd )
		{
			pszPlace += nIncluding;
			int nExcluding = StringSpanExcluding( pszPlace, pszTokens );

			int iFrom = *pnStart+nIncluding;
			int nUntil = nExcluding;
			*pnStart = iFrom+nUntil+1;

			return( strSource.Mid( iFrom, nUntil ) );
		}
	}

	// return empty string, done tokenizing
	*pnStart = -1;

	return( CString( "" ) );
#endif
}

#define SEPERATORS _T(" \r\n,\t()")

int CLocalizeRCDlg::FindSeperateWord(CString strText, LPCTSTR strWord, int nStartPos)
{
	int nFoundPos;
	while( (nFoundPos = strText.Find( strWord, nStartPos )) != -1 )
	{
        CString strSeperator;

		// look for preceding character
		if( nFoundPos > 0 )
		{
			strSeperator = strText[nFoundPos-1];
			// if preceding character isn't a separator, continue search
			if( strSeperator.FindOneOf( SEPERATORS ) == -1 )
			{
				strSeperator = strText.Mid( nFoundPos-1, nFoundPos + 40 );
				
				nStartPos = nFoundPos+1;
				continue;
			}
		}

		// look for successing character
		int nSuccessingPos =  nFoundPos+_tcslen(strWord);
		if( nSuccessingPos < strText.GetLength() )
		{
			strSeperator = strText[nSuccessingPos];

			// if successing character isn't a separator, continue search
			if( strSeperator.FindOneOf( SEPERATORS ) == -1 )
			{
				nStartPos = nFoundPos+1;
				continue;
			}
		}
		break;
		
	}
	return nFoundPos;
}

void CLocalizeRCDlg::OnCbnSelchangeObsItems()
{
	UpdateData();
}

void CLocalizeRCDlg::OnBnKillfocusCopy()
{
	UpdateData();
}

void CLocalizeRCDlg::OnBnKillfocusNosort()
{
	UpdateData();
}

void CLocalizeRCDlg::OnBnClickedChngInputrc()
{
	// Change Input RC
	if( OpenSaveDialog( true, true, IDS_EXTRC, IDS_EXTRCDESCRIPTION, m_strInputRC, ENT_INPUTRC ) )
		OpenInputRC();
}

void CLocalizeRCDlg::OnBnClickedChngLangini()
{
	// Change Lang INI
	OpenSaveDialog( false, true, IDS_EXTINI, IDS_EXTINIDESCRIPTION, m_strLangINI, ENT_LANGINI );	
}

void CLocalizeRCDlg::OnBnClickedChngOutputrc()
{
	// Change Output RC
	OpenSaveDialog( false, true, IDS_EXTRC, IDS_EXTRCDESCRIPTION, m_strOutputRC, ENT_OUTPUTRC );
}

BOOL CLocalizeRCDlg::OpenSaveDialog(BOOL bOpen, BOOL bRelative, UINT nExtID, UINT nExtDescriptionID, CString& strEdit, CString strIniEntry)
{
	CString strExtension, strExtensionInfo;
	strExtension.LoadString( nExtID );
	strExtensionInfo.LoadString( nExtDescriptionID ); 

	DWORD dwFlags = OFN_HIDEREADONLY|OFN_ENABLESIZING|OFN_EXPLORER;
	if ( bOpen )
		dwFlags |= OFN_FILEMUSTEXIST;
	else
		dwFlags |= OFN_OVERWRITEPROMPT;

#if _MFC_VER >= 0x0700
	CFileDialog FileDialog( bOpen, strExtension, NULL, dwFlags,	strExtensionInfo, this, 0 );
#else
	CFileDialog FileDialog( bOpen, strExtension, NULL, dwFlags,	strExtensionInfo, this );
#endif
	TCHAR szFile[ MAX_PATH ] = {};
	_tcscpy_s( szFile, strEdit );
	FileDialog.m_ofn.lpstrFile = szFile;

	if ( FileDialog.DoModal() != IDOK )
		return FALSE;

	if ( bRelative )
	{
		// change to relative path
		TCHAR szOut[MAX_PATH] = {};
		if( ! PathRelativePathTo( szOut, m_strWorkspace, FILE_ATTRIBUTE_NORMAL, FileDialog.GetPathName(), FILE_ATTRIBUTE_NORMAL ) )
			_tcscpy_s( szOut, FileDialog.GetFileName() );
		if ( strIniEntry )
		{
			CIniEx IniEx;
			if( ! IniEx.Open( m_strWorkspace ) )
				return FALSE;

			CString strValue = szOut;
			IniEx.SetValue( strIniEntry, strValue );
			IniEx.WriteFile();
		}
	}
	strEdit = FileDialog.GetPathName();

	UpdateData( FALSE );

	return TRUE;	
}

BOOL CLocalizeRCDlg::ShowError(UINT nIDString1, bool bGetLastError, DWORD dwErrCode )
{
	CString strString1, strLastError;
	
	if( dwErrCode == 0 )
		dwErrCode = GetLastError();

	strString1.LoadString( nIDString1 );
	if( bGetLastError )	
	{
		strLastError = ConvertErrorToString( dwErrCode );
		strString1 += _T(" ") + strLastError;
	}
	AfxMessageBox( strString1 );

	return TRUE;
}

CString CLocalizeRCDlg::ConvertErrorToString( DWORD dwErrCode )
{	
	HMODULE 	hModule = NULL;		// default to system source
	LPTSTR		MessageBuffer = NULL;
	DWORD		dwBufferLength;
	CString		strError;
 
	// Always start off with an empty string
	strError.Empty();

   	// if error_code is in the network range, load the message source
	if (dwErrCode >= NERR_BASE && dwErrCode <= MAX_NERR) 
	{
	    hModule = ::LoadLibraryEx( _TEXT("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE );
	}
 
	
	// call FormatMessage() to allow for message text to be acquired
	// from the system or the supplied module handle
	dwBufferLength = ::FormatMessage( 
	    FORMAT_MESSAGE_ALLOCATE_BUFFER |
	    FORMAT_MESSAGE_IGNORE_INSERTS |
	    FORMAT_MESSAGE_FROM_SYSTEM |		// always consider system table
	    ((hModule != NULL) ? FORMAT_MESSAGE_FROM_HMODULE : 0),
	    hModule,					// module to get message from (NULL == system)
	    dwErrCode,
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// default language
	    (LPTSTR) &MessageBuffer, 0, NULL );

    if ( MessageBuffer ) 
	{
	    if ( dwBufferLength )
			strError = (LPCTSTR)MessageBuffer;
 
	    // free the buffer allocated by the system
	    ::LocalFree(MessageBuffer);
	}
 
	// if you loaded a message source, unload it
	if (hModule != NULL)
	    ::FreeLibrary(hModule);

    if ( strError.GetLength() == 0 )
	    strError.Format( IDS_ERR_UNKNOWN, dwErrCode );
	
	return strError;
}
