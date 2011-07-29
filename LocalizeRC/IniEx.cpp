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
#include "IniEx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// UNICODE File Reader

CStdioUnicodeFile::CStdioUnicodeFile()
	: CStdioFile()
{
}

CStdioUnicodeFile::CStdioUnicodeFile(LPCTSTR lpszFileName, UINT nOpenFlags)
	: CStdioFile( lpszFileName, nOpenFlags )
{
}

// static method to check if file is unicode
BOOL CStdioUnicodeFile::IsUnicode(LPCTSTR lpszFileName)
{
	CStdioUnicodeFile File;
	if( !File.Open( lpszFileName, CFile::modeRead | CFile::typeBinary ) )
		return false;
	// check first two bytes (BOT)
	BYTE fffe[2] = {0, 0};
	if (2 == File.Read(&fffe, 2)) 
	{
		if (fffe[0] == 0xFF || fffe[1] == 0xFE)
		{
			File.Close();
			return true;
		}
	}
	File.Close();
	return false;
}

BOOL CStdioUnicodeFile::ReadString(CString& rString)
{
	ASSERT_VALID(this);
	rString = _T("");    // empty string without deallocating
	const int nMaxSize = 128;
	LPTSTR lpsz = rString.GetBuffer(nMaxSize);
	LPTSTR lpszResult;
	int nLen = 0;
	for (;;)
	{
		lpszResult = _fgetts(lpsz, nMaxSize+1, m_pStream);
		rString.ReleaseBuffer();
		// handle error/eof case
		if (lpszResult == NULL && !feof(m_pStream))
		{
			clearerr(m_pStream);
			AfxThrowFileException(CFileException::genericException, _doserrno,
				m_strFileName);
		}
		// if string is read completely or EOF
		if (lpszResult == NULL ||
			(nLen = lstrlen(lpsz)) < nMaxSize ||
			lpsz[nLen-1] == '\n')
			break;
		nLen = rString.GetLength();
		lpsz = rString.GetBuffer(nMaxSize + nLen) + nLen;
	}
	// remove '\n' from end of string if present
	lpsz = rString.GetBuffer(0);
	nLen = rString.GetLength();
	if (nLen != 0 && lpsz[nLen-1] == '\n') {
		rString.GetBufferSetLength(nLen-1);
		nLen = rString.GetLength();
		if (nLen != 0 && lpsz[nLen-1] == '\r')
			rString.GetBufferSetLength(nLen-1);
	}
	return lpszResult != NULL;
}

//GrowSize for Dynmiz Section Allocation
CIniEx::CIniEx(int GrowSize/*=4*/)
{
	m_GrowSize=GrowSize;

	m_SectionNo=0;
	m_writeWhenChange=FALSE;
	m_makeBackup=FALSE;
	m_NoCaseSensitive=TRUE;
	m_Changed=FALSE;
	m_Keys=NULL;
	m_Values=NULL;
	m_allocatedObjectCount=0;
}

CIniEx::~CIniEx()
{
	if (m_writeWhenChange)
		WriteFile(m_makeBackup);
	ResetContent();
}

void CIniEx::ParseLine (const CString& line, CString& key, CString& val)
{
	int iStart = 0, iEnd;
	bool quoted = false;

	//Clear params
	key.Empty();
	val.Empty();

	//First eat whitespaces
	while ( line[iStart] && _istspace(line[iStart]) )
		iStart++;
	
	// return if empty input string
	if ( line[iStart] == _T('\0') )
		return;

	//Save starting position and continue
	iEnd = iStart;

	// First character is " -> everything is quoted (always)
	if ( line[iStart] == _T('\"') )
	{
		quoted = true;
		iEnd++;
	}

	//Walk until first quote skipping pairs
	while ( line[iEnd] )
	{
		if ( !quoted && (line[iEnd] == _T('=')) )
		{
			break;
		}
		else if ( quoted && line[iEnd] == _T('\"') )
		{
			iEnd++;
			if ( line[iEnd] != _T('\"') )
			{
				break;
			}
		}
		iEnd++;
	}

	//We have our key
	key = line.Mid(iStart, iEnd - iStart);

	//Remove spaces only from right
	key.TrimRight();

	//Find the equal.. we should NOT have any remaining chars here...
	//but we could if we have something like:
	//"key" extra = value
	//In that case we discard the extra data
	while ( line[iEnd] && line[iEnd] != _T('=') )
		iEnd++;

	if ( !line[iEnd] )
		return;

	//Onto the value
	iStart = iEnd + 1;

	//Eat whitespaces again
	while ( line[iStart] && _istspace(line[iStart]) )
		iStart++;

	val = line.Mid(iStart);
	val.TrimRight();
}


BOOL CIniEx::OpenAtExeDirectory(LPCTSTR pFileName,
							BOOL writeWhenChange,/*=TRUE*/
							BOOL createIfNotExist/*=TRUE*/,
							BOOL noCaseSensitive /*=TRUE*/,
							BOOL makeBackup      /*=FALSE*/)
{

	CString filePath;
//if it's a dll argv will be NULL and it may cause memory leak	
#ifndef _USRDLL
	CString tmpFilePath;
	int nPlace=0;
	tmpFilePath=__argv[0];
	nPlace=tmpFilePath.ReverseFind('\\');
	
	
	if (nPlace!=-1)
	{
		filePath=tmpFilePath.Left(nPlace);
	}
	else
	{
		TCHAR curDir[MAX_PATH];
		GetCurrentDirectory(MAX_PATH,curDir);
		filePath=curDir;
	}
#else
	//it must be safe for dll's
	TCHAR curDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,curDir);
	filePath=curDir;
#endif
	filePath+="\\";
	filePath+=pFileName;
	return Open(filePath,writeWhenChange,createIfNotExist,noCaseSensitive,makeBackup);
}

BOOL CIniEx::Open(LPCTSTR pFileName,
				  BOOL writeWhenChange,/*=FALSE*/
				  BOOL createIfNotExist/*=TRUE*/,
				  BOOL noCaseSensitive /*=FALSE*/,
				  BOOL makeBackup      /*=FALSE*/)
{

	CFileException e;
	BOOL bRet;
	CString Line;
	CString sectionStr;
	//int nPlace;
	UINT mode=CFile::modeReadWrite;

	//if it's second ini file for this instance
	//we have to save it and delete member variables contents
	if (!m_FileName.IsEmpty()) 
	{
		if( m_writeWhenChange )
			WriteFile();
		ResetContent();
	}

	m_NoCaseSensitive=noCaseSensitive;
	m_writeWhenChange=writeWhenChange;
	m_makeBackup=makeBackup;

	CString tmpKey;
	CString tmpValue;
	if (createIfNotExist)
		mode= mode | CFile::modeCreate | CFile::modeNoTruncate;

	BOOL bIsUnicode = CStdioUnicodeFile::IsUnicode( pFileName );

	try
	{
		m_FileName=pFileName;

		//Grow the arrays as GrowSize(which given constructor)
		m_allocatedObjectCount = m_GrowSize;
		m_Keys=(CStringArray **)malloc( m_allocatedObjectCount * sizeof(CStringArray *) );
		m_Values=(CStringArray **)malloc( m_allocatedObjectCount * sizeof(CStringArray *) );
		for (int i=0;i<m_GrowSize;i++)
		{
			m_Keys[m_SectionNo+i]=NULL;
			m_Values[m_SectionNo+i]=NULL;
		}		

		CStdioUnicodeFile file;

		#ifdef UNICODE	
			if( bIsUnicode )
			{	
				if( !file.Open( pFileName, mode | CFILEFLAG_UNICODEHELPER, &e ) )
				{
					return false;
				}
				// skip first two bytes
				file.Seek( 2, CFile::begin );
			}
			else
			{
				if( !file.Open( pFileName, mode, &e ) )
					return false;
			}
		#else
			if( bIsUnicode )
			{
				CString strErr;
				strErr.Format( IDS_UNICODEFILE, pFileName );
				AfxMessageBox( strErr );
				return false;	// cancel
			}
			else
			{
				if( !file.Open( pFileName, mode, &e ) )
					return false;
			}
		#endif

		for(;;)
		{
			//Read one line from given ini file
			bRet=file.ReadString(Line);
			if (!bRet) 
				break;
			Line.TrimRight();
			if (Line=="") 
				continue;
					
			//if line's first character = '[' 
			//and last character = ']' it's section 
			if (Line.Left(1)=="[" && Line.Right(1)=="]")
			{
				m_Keys[m_SectionNo]=new CStringArray;
				m_Values[m_SectionNo]=new CStringArray;
				m_SectionNo++;		
				GrowIfNecessary();
				
				sectionStr=Line.Mid(1,Line.GetLength()-2);
				m_Sections.SetAtGrow(m_SectionNo-1,sectionStr);
				continue;
			}
			
			ParseLine( Line, tmpKey, tmpValue );

			/*nPlace=Line.Find(_T("="));
			if (nPlace==-1)
			{
				tmpKey=Line;
				tmpValue="";
			}
			else
			{
				tmpKey=Line.Left(nPlace);
				tmpValue=Line.Mid(nPlace+1);
			}*/

			// create Section of no one exists in file
			if( m_SectionNo == 0 )
			{	
				m_Keys[m_SectionNo]=new CStringArray;
				m_Values[m_SectionNo]=new CStringArray;
				m_SectionNo++;
			}
			m_Keys[m_SectionNo-1]->Add(tmpKey);
			m_Values[m_SectionNo-1]->Add(tmpValue);
			m_Sections.SetAtGrow(m_SectionNo-1,sectionStr);
		}
		file.Close();
	}
	catch (CFileException *e)
	{
		m_ErrStr.Format( _T("%d"), e->m_cause );
	}
	

	return TRUE;
}

CString CIniEx::GetValue(CString Key)
{
	return GetValue(_T(""),Key);
}

//if Section Name="" -> looking up key for witout section
CString CIniEx::GetValue(CString Section,CString Key,CString DefaultValue/*=""*/)
{
	int nIndex=LookupSection(&Section);
	if (nIndex==-1) return DefaultValue;
	int nRet;
	CString retStr;
	for (INT_PTR i=m_Keys[nIndex]->GetUpperBound();i>=0;i--)
	{
		nRet=CompareStrings(&(m_Keys[nIndex]->GetAt(i)),&Key);
		if (nRet==0)
		{
			retStr=m_Values[nIndex]->GetAt(i);
			/*int nPlace=retStr.ReverseFind(';');
			if (nPlace!=-1) 
				retStr.Delete(nPlace,retStr.GetLength()-nPlace);*/
			return retStr;
		}
	}
	return DefaultValue;
}



//returns index of key for given section
//if no result returns -1
int CIniEx::LookupKey(int nSectionIndex,CString *Key)
{
	ASSERT(nSectionIndex<=m_SectionNo);
	int nRet;
	for (INT_PTR i=m_Keys[nSectionIndex]->GetUpperBound();i>=0;i--)
	{
		nRet=CompareStrings(&m_Keys[nSectionIndex]->GetAt(i),Key);
		if (nRet==0) 
			return (int)i;
	}
	return -1;
}

//return given sections index in array
int CIniEx::LookupSection(CString *Section)
{
	int nRet;
	for (int i=0;i<m_Sections.GetSize();i++)
	{
		nRet=CompareStrings(&m_Sections.GetAt(i),Section);
		if (nRet==0) return i;
	}
	return -1;
}

//Sets for Key=Value for without section
void CIniEx::SetValue(CString Key,CString Value)
{
	SetValue(_T(""),Key,Value);
}

//writes Key=value given section
void CIniEx::SetValue(CString Section,CString Key,CString Value)
{
	//file opened?
	ASSERT(!m_FileName.IsEmpty());

	//if given key already existing, overwrite it
	int nIndex=LookupSection(&Section);
	int nKeyIndex;
	if (nIndex==-1)
	{
		//if key not exist grow arrays (if necessary)
		m_Changed=TRUE;
		m_SectionNo++;
		GrowIfNecessary();
		m_Keys[m_SectionNo-1]=new CStringArray;
		m_Values[m_SectionNo-1]=new CStringArray;
		nIndex=m_SectionNo-1;
		m_Sections.SetAtGrow(m_SectionNo-1,Section);
	}

	
	//looking up keys for section
	nKeyIndex=LookupKey(nIndex,&Key);
	
	//if key exist -> overwrite it
	//if not add to end of array
	if (nKeyIndex!=-1) 
	{
		if (CompareStrings(&m_Values[nIndex]->GetAt(nKeyIndex),&Value)!=0)
			m_Changed=TRUE;
		m_Values[nIndex]->SetAt(nKeyIndex,Value);
	}
	else	//if not exist
	{
		m_Changed=TRUE;
		m_Keys[nIndex]->Add(Key);
		m_Values[nIndex]->Add(Value);
	}
}


//returns backup file name
//if you didn't want backup (when openning file) it returns ""
CString CIniEx::WriteFile(BOOL makeBackup/*=FALSE*/)
{
	if (!m_Changed) 
		return CString();
	CString tmpFileName=m_FileName;

	if (makeBackup)
	{
		if (m_BackupFileName.IsEmpty())
		{
			FindBackupFile();
		}
		CopyFile(m_FileName,m_BackupFileName,FALSE);
	}
	
	CStdioUnicodeFile file;
	if (!file.Open(m_FileName, CFILEFLAG_UNICODEHELPER | CFile::modeCreate | CFile::modeWrite)) 
	{
		return CString();
	}

	CString strNewline;

#ifdef UNICODE
	// write 0xFF 0xFE
	BYTE fffe[2] = { 0xFF, 0xFE };
	file.Write(fffe, 2);
	strNewline = _T("\r\n");
#else
	strNewline = _T("\n");
#endif

	CString tmpLine;
	for ( int i = 0 ; i < m_Sections.GetSize(); i++ )
	{
		if ( ! m_Sections.GetAt(i).IsEmpty() )
		{
			tmpLine.Format(_T("[%s]%s"),
				(LPCTSTR)m_Sections.GetAt(i),
				(LPCTSTR)strNewline );
			file.WriteString( tmpLine );
		}

		if ( ! m_Keys[i] )
			continue;

		for ( int j = 0; j <= m_Keys[i]->GetUpperBound(); j++ )
		{
			// if key is empty we don't write "="
			tmpLine.Format( _T("%s%s%s%s"),
				(LPCTSTR)m_Keys[i]->GetAt(j),
				( m_Keys[i]->GetAt(j).IsEmpty() ? _T("") : _T("=") ),
				(LPCTSTR)m_Values[i]->GetAt(j),
				(LPCTSTR)strNewline );
		
			file.WriteString(tmpLine);
 
		}
	}

	file.Close();

	return m_BackupFileName;
}

BOOL CIniEx::GetWriteWhenChange(void)
{
	return m_writeWhenChange;
}


void CIniEx::SetWriteWhenChange(BOOL WriteWhenChange)
{
	m_writeWhenChange=WriteWhenChange;
}


void CIniEx::SetBackupFileName(CString &backupFile)
{
	m_BackupFileName=backupFile;
}


void CIniEx::FindBackupFile(void)
{
	WIN32_FIND_DATA ffData;
	BOOL bContinue=TRUE;
	CString filePath(m_FileName);
	CString ext;
	int nPlace=filePath.ReverseFind('.');
	filePath.Delete(nPlace,filePath.GetLength()-nPlace);
	filePath+="*.*";
	int extNo=0;
	LPTSTR p;
	HANDLE handle=FindFirstFile(filePath,&ffData);
	while (bContinue)
	{
		bContinue=FindNextFile(handle,&ffData);
		p=ffData.cFileName;
		p+=_tcslen(ffData.cFileName)-3;
		if (_ttoi(p)>extNo) extNo=_ttoi(p);
	}
	m_BackupFileName.Format(_T("%s.%.3d"),m_FileName,extNo+1);

}


void CIniEx::ResetContent()
{
	if (!m_Keys) return;
	if ( m_Keys)
	{
		for (int i=0;i<m_allocatedObjectCount;i++)
		{
			if (m_Keys[i])
				delete m_Keys[i];
			if (m_Values[i])
				delete m_Values[i];
		}
		free(m_Keys);
		free(m_Values);
	}
	m_Keys=NULL;
	m_Values=NULL;
	m_Sections.RemoveAll();
	m_SectionNo=0;
	m_FileName="";
	m_Changed=FALSE;

}


//Removes key and it's value from given section
BOOL CIniEx::RemoveKey(CString Section,CString Key)
{
	int nIndex=LookupSection(&Section);
	if (nIndex==-1) return FALSE;

	int nKeyIndex=LookupKey(nIndex,&Key);
	if (nKeyIndex==-1) return FALSE;

	m_Keys[nIndex]->RemoveAt(nKeyIndex);
	m_Values[nIndex]->RemoveAt(nKeyIndex);
	m_Changed=TRUE;
	return TRUE;
}

//Removes key and it's value 
BOOL CIniEx::RemoveKey(CString Key)
{
	return RemoveKey(_T(""),Key);
}


//Removes given section(including all keys and values)
//return FALSE when given section not found
//It won't couse memory leak because when deleting object
//the msize (malloc size) checking
BOOL CIniEx::RemoveSection(CString Section)
{
	int nIndex=LookupSection(&Section);
	if (nIndex==-1) return FALSE;

	m_Keys[nIndex]->RemoveAll();
	m_Values[nIndex]->RemoveAll();

	delete m_Keys[nIndex];
	delete m_Values[nIndex];

	m_Keys[nIndex]=NULL;
	m_Values[nIndex]=NULL;

	m_Sections.RemoveAt(nIndex);
	m_SectionNo--;
	m_Changed=TRUE;
	return TRUE;
}

int CIniEx::CompareStrings(const CString *str1, CString *str2)
{
	if (m_NoCaseSensitive)
		return str1->CompareNoCase(*str2);
	else
		return str1->Compare(*str2);
}

void CIniEx::GrowIfNecessary(void)
{
	//for first gives GrowSize
	if (m_SectionNo>=m_allocatedObjectCount)
	{
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//realloc for GrowSize
		m_allocatedObjectCount += m_GrowSize;
		m_Keys=(CStringArray **)realloc(m_Keys,sizeof(CStringArray*) * (m_allocatedObjectCount) );
		m_Values=(CStringArray **)realloc(m_Values,sizeof(CStringArray*) * (m_allocatedObjectCount) );
		//allocated + GrowSize
		//zero memory for new allocation
		for (int i=0;i<m_GrowSize;i++)
		{
			m_Keys[m_SectionNo+i]=NULL;
			m_Values[m_SectionNo+i]=NULL;
		}
	}
}

//copy each string (section name) because 
//if sections parametter be a pointer it may clear content of member
void CIniEx::GetSections(CStringArray &sections)
{
	for (int i=0;i<m_Sections.GetSize();i++)
		sections.Add(m_Sections.GetAt(i));

}

void CIniEx::GetKeysInSection(CString section,CStringArray &keys)
{
	int nIndex=LookupSection(&section);
	if (nIndex==-1) return;

	for (int i=0;i<m_Keys[nIndex]->GetSize();i++)
	{
		keys.Add(m_Keys[nIndex]->GetAt(i));
	}
}

void CIniEx::SortIniValues()
{
	for (int i=0;i<m_Sections.GetSize();i++)
	{
		if (!m_Keys[i]) 
			continue;

		// Quicksort
		QuickSortRecursive( i, 0, m_Keys[i]->GetUpperBound(), true );
	}
}

int CIniEx::CompareItems( CString str1, CString str2 )
{
	return str1.CompareNoCase(str2);
}

BOOL CIniEx::Swap( int nSection, int nLeftIndex, int nRightIndex )
{
	CString strHelp = m_Keys[nSection]->GetAt(nLeftIndex);
	m_Keys[nSection]->SetAt(nLeftIndex, m_Keys[nSection]->GetAt(nRightIndex) );
	m_Keys[nSection]->SetAt(nRightIndex, strHelp );

	strHelp = m_Values[nSection]->GetAt(nLeftIndex);
	m_Values[nSection]->SetAt(nLeftIndex, m_Values[nSection]->GetAt(nRightIndex));
	m_Values[nSection]->SetAt(nRightIndex, strHelp);

	return true;
}

void CIniEx::QuickSortRecursive(int nSection, int iLow, int iHigh, BOOL bAscending)
{
	// Params renamed for easier comparison with literature
	int iLeft = iLow;
	int iRight = iHigh;

	// Important: Save Pivot-Element on Stack, instead of using GetAt(iPivot) in 
	// "while('compare')-Loop". Original implementation by Attila Hajdrik used 
	// GetAt(iPivot) in within the Loop
	// int iPivot = (iLow+iHigh) / 2;
	CString Pivot = m_Keys[nSection]->GetAt((iLow+iHigh) / 2); 

	do
	{
		if( bAscending )
		{
			while( CompareItems(m_Keys[nSection]->GetAt(iLeft), Pivot) < 0 ) iLeft++;
			while( CompareItems(Pivot, m_Keys[nSection]->GetAt(iRight)) < 0 ) iRight--;
		}
		else
		{
			while( CompareItems(m_Keys[nSection]->GetAt(iLeft), Pivot) > 0 ) iLeft++;
			while( CompareItems(Pivot, m_Keys[nSection]->GetAt(iRight)) > 0 ) iRight--;
		}

		if( iLeft <= iRight )
		{
			if( iLeft != iRight ) // Ignore unnecessary swaps
				Swap(nSection, iLeft, iRight);

			iLeft++;
			iRight--;
		}
	}
	while( iLeft <= iRight );

	if( iLow < iRight )
		QuickSortRecursive(nSection, iLow, iRight, bAscending);

	if( iLeft < iHigh )
		QuickSortRecursive(nSection, iLeft, iHigh, bAscending);
}
