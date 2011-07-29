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

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

// Workspace / Registry keys
#define SEC_LASTPROJECT _T("Last Project")

#define ENT_WORKSPACE _T("Workspace")
#define ENT_LANGID _T("LanguageID")

#define ENT_OUTPUTRC _T("OutputRC")
#define ENT_LANGINI _T("LangINI")
#define ENT_INPUTRC _T("InputRC")
#define ENT_COPY _T("Copy")
#define ENT_OBSITEMS _T("ObsoleteItems")
#define ENT_NOSORT _T("NoSort")

#define STANDARD_LANGID 0x09	// English

class CLocalizeRCApp : public CWinApp
{
public:
	CLocalizeRCApp();

private:
	LANGID CurLangID;

	BOOL LoadLangIDDLL(LANGID LangID);

	virtual BOOL InitInstance();
	virtual int ExitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CLocalizeRCApp theApp;
