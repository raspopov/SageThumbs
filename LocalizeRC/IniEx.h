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

#ifdef UNICODE
	#define CFILEFLAG_UNICODEHELPER	(CFile::typeBinary)
#else
	#define CFILEFLAG_UNICODEHELPER	(0)
#endif

class CStdioUnicodeFile: public CStdioFile
{
public:
	CStdioUnicodeFile();
	CStdioUnicodeFile(LPCTSTR lpszFileName, UINT nOpenFlags);
	BOOL ReadString(CString& cstr);

	static BOOL CStdioUnicodeFile::IsUnicode(LPCTSTR lpszFileName);
};

#define MAX_SECTION_COUNT	512
class CIniEx  
{
private:
	//Functions
	int		LookupKey		(int nSectionIndex,CString *Key);
	
	int		CompareStrings	(const CString *str1,CString *str2);
	void    GrowIfNecessary	(void);
	void	FindBackupFile	(void);
private:
	//Variables
	//CMapStringToString	*m_tmpLines;
	CString				m_ErrStr;
	CStringArray    	**m_Keys;
	CStringArray    	**m_Values;
	CStringArray		m_Sections;

	int		m_SectionNo;
	int		m_GrowSize;
	int		m_allocatedObjectCount;
	BOOL	m_NoCaseSensitive;
	BOOL	m_writeWhenChange;
	CString	m_BackupFileName;
	CString m_FileName;
	BOOL	m_makeBackup;
	BOOL	m_Changed;	

public:
	int		LookupSection	(CString *Section);
	void GetKeysInSection(CString section,CStringArray &keys);
	void GetSections(CStringArray &sections);

	CIniEx(int GrowSize=4);
	virtual ~CIniEx();

	void ParseLine (const CString& line, CString& key, CString& val);

	BOOL Open(LPCTSTR pFileName,
			  BOOL writeWhenChange=FALSE,
			  BOOL createIfNotExist=TRUE,
			  BOOL noCaseSensitive=FALSE,
			  BOOL makeBackup=FALSE);
	BOOL OpenAtExeDirectory(LPCTSTR pFileName,
			  BOOL writeWhenChange=FALSE,
			  BOOL createIfNotExist=TRUE,
			  BOOL noCaseSensitive=FALSE,
			  BOOL makeBackup=FALSE);
	void ResetContent();

	CString WriteFile(BOOL makeBackup=FALSE);

	CString GetValue(CString Section,CString Key,CString DefaultValue=_T(""));
	CString GetValue(CString Key);

	void SetValue(CString Key,CString Value);
	void SetValue(CString Section,CString Key,CString Value);

	BOOL RemoveKey(CString Key);
	BOOL RemoveKey(CString Section,CString Key);

	BOOL RemoveSection(CString Section);

	BOOL GetWriteWhenChange();
	void SetWriteWhenChange(BOOL WriteWhenChange);

	void SetBackupFileName(CString &backupFile);

	void SortIniValues();
	int CompareItems( CString str1, CString str2 );
	BOOL CIniEx::Swap( int nSection, int nLeftIndex, int nRightIndex );
	void CIniEx::QuickSortRecursive(int nSection, int iLow, int iHigh, BOOL bAscending);

};
