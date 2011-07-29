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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CLocalizeRCApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

CLocalizeRCApp::CLocalizeRCApp()
{
}

CLocalizeRCApp theApp;

BOOL CLocalizeRCApp::InitInstance()
{
	InitCommonControls();
	
	AfxInitRichEdit();

	CWinApp::InitInstance();

	SetRegistryKey( _T("LocalizeRC") );

	CCommandLineInfo rCmdInfo;
	ParseCommandLine( rCmdInfo );
	rCmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	CurLangID = STANDARD_LANGID;

	if( ! LoadLangIDDLL( static_cast< LANGID >( GetProfileInt( SEC_LASTPROJECT, ENT_LANGID, 0 ) )) )
		if( ! LoadLangIDDLL( GetUserDefaultLangID() ) )
			LoadLangIDDLL( GetSystemDefaultLangID() );

	{
		CLocalizeRCDlg dlg;
		dlg.m_LangID = CurLangID;
		dlg.m_bRun = rCmdInfo.m_bRunAutomated;
		dlg.m_strWorkspace = rCmdInfo.m_strFileName;

		m_pMainWnd = &dlg;
		dlg.DoModal();
	}

	if ( CurLangID != STANDARD_LANGID )
		FreeLibrary( AfxGetResourceHandle() );

	return FALSE;
}

BOOL CLocalizeRCApp::LoadLangIDDLL(LANGID LangID)
{
	if ( LangID == STANDARD_LANGID )	// integrated language is the right one
		return true;
	
	CString strLangIDDLL;
	strLangIDDLL.Format( _T("LocalizeRC%.2x.dll"), LangID );

	if ( HINSTANCE hInstance = LoadLibrary( strLangIDDLL ) )
	{
		AfxSetResourceHandle( hInstance );
		CurLangID = LangID;
		return true;
	}

	return false;
}

int CLocalizeRCApp::ExitInstance()
{
	CWinApp::ExitInstance();

	return 0;
}
