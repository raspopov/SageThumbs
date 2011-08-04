/*
SageThumbs - Thumbnail image shell extension.

Copyright (C) Nikolay Raspopov, 2004-2011.

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

#include <SDKDDKVer.h>

#define STRICT

#ifndef _SECURE_ATL
	#define _SECURE_ATL 1
#endif

#define _ATL_FREE_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_NO_CRT
#define _ATL_ALL_WARNINGS

#ifdef _DEBUG
	//#define _ATL_DEBUG_INTERFACES
	#define _ATL_DEBUG_QI
#endif

#define ISOLATION_AWARE_ENABLED 1

#if UNICODE
	#define gflLoadBitmapT			gflLoadBitmapW
	#define gflSaveBitmapT			gflSaveBitmapW
	#define gflLoadThumbnailT		gflLoadThumbnailW
	#define gflSetPluginsPathnameT	gflSetPluginsPathnameW
	#define gflGetFileInformationT	gflGetFileInformationW
#else
	#pragma warning( disable: 4127 )
	#define gflLoadBitmapT			gflLoadBitmap
	#define gflSaveBitmapT			gflSaveBitmap
	#define gflLoadThumbnailT		gflLoadThumbnail
	#define gflSetPluginsPathnameT	gflSetPluginsPathname
	#define gflGetFileInformationT	gflGetFileInformation
#endif

#include "resource.h"

#include <atlbase.h>
#include <atlcoll.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlenc.h>
#include <atlstr.h>
#include <atlwin.h>
#include <accctrl.h>
#include <aclapi.h>
#include <comdef.h>
#include <commctrl.h>
#include <cpl.h>
#include <ddraw.h>
#include <delayimp.h>
#include <emptyvc.h>
#include <mapi.h>
#include <ocmm.h>
#include <richedit.h>
#include <shlobj.h>
#include <thumbcache.h>

#include "../gfl/libgfl.h"
#include "../gfl/libgfle.h"

#define LIB_GFL				"libgfl340.dll"	// Name of GFL library (case sensitive)
#define LIB_GFLE			"libgfle340.dll"// Name of GFLe library (case sensitive)
#define LIB_SQLITE			"sqlite3.dll"	// Name of SQLite library (case sensitive)

#ifndef QWORD
	typedef ULONGLONG QWORD;
#endif

#define MAKEQWORD(l,h) ((QWORD)(l)|((QWORD)(h)<<32))

// SDK Fix
struct __declspec(uuid("85788d00-6807-11d0-b810-00c04fd706ec")) IRunnableTask;
struct __declspec(uuid("000214E4-0000-0000-C000-000000000046")) IContextMenu;
struct __declspec(uuid("000214F4-0000-0000-C000-000000000046")) IContextMenu2;
struct __declspec(uuid("BCFCE0A0-EC17-11d0-8D10-00A0C90F2719")) IContextMenu3;
struct __declspec(uuid("00021500-0000-0000-c000-000000000046")) IQueryInfo;
struct __declspec(uuid("e8025004-1c42-11d2-be2c-00a0c9a83da1")) IColumnProvider;

// {889900c3-59f3-4c2f-ae21-a409ea01e605}
DEFINE_GUID(CLSID_WindowsThumbnailer,0x889900c3,0x59f3,0x4c2f,0xae,0x21,0xa4,0x09,0xea,0x01,0xe6,0x05);

#define REG_SAGETHUMBS		_T("Software\\SageThumbs")
#define ShellImagePreview	_T("SystemFileAssociations\\image\\ShellEx\\ContextMenuHandlers\\ShellImagePreview")
#define REG_XNVIEW_KEY		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\XnView_is1")
#define REG_XNVIEW_PATH1	_T("UninstallString")
#define REG_XNVIEW_PATH2	_T("Inno Setup: App Path")
#define FaxCLSID			_T("{e84fda7c-1d6a-45f6-b725-cb260c236066}")
#define THUMB_STORE_SIZE	256
#define THUMB_MIN_SIZE		32				// Минимальный размер просмотра в пикселях
#define THUMB_MAX_SIZE		512				// Максимальный размер просмотра в пикселях 
#define FILE_MAX_SIZE		10				// Максимальный размер файла в Мб
#define MAX_LONG_PATH		1024			// Длина строк с файловыми путями
#define STANDARD_LANGID		0x09			// Стандартный встроенный язык - English

// SQL для создания базы данных
const LPCTSTR RECREATE_DATABASE =
	_T("PRAGMA foreign_keys = ON;")
	_T("BEGIN TRANSACTION;")
		_T("CREATE TABLE IF NOT EXISTS Pathes ( ")
			_T("PathID INTEGER NOT NULL PRIMARY KEY, ")
			_T("Pathname TEXT NOT NULL UNIQUE );")
		_T("CREATE INDEX IF NOT EXISTS PathesIndex ON Pathes( Pathname );")
		_T("CREATE TABLE IF NOT EXISTS Entities ( ")
			_T("PathID INTEGER NOT NULL, ")
			_T("Filename TEXT NOT NULL, ")
			_T("LastWriteTime INTEGER, ")
			_T("CreationTime INTEGER, ")
			_T("FileSize INTEGER, ")
			_T("ImageInfo BLOB, ")
			_T("Image BLOB, ")
			_T("Width INTEGER DEFAULT 0, ")
			_T("Height INTEGER DEFAULT 0, ")
			_T("PRIMARY KEY ( PathID, Filename ),")
			_T("FOREIGN KEY ( PathID ) REFERENCES Pathes( PathID ) );")
		_T("CREATE INDEX IF NOT EXISTS EntitiesIndex ON Entities( PathID );")
	_T("COMMIT;")
	_T("VACUUM;");

const LPCTSTR DROP_DATABASE =
	_T("BEGIN TRANSACTION;")
		_T("DROP INDEX IF EXISTS EntitiesIndex;")
		_T("DROP TABLE IF EXISTS Entities;")
		_T("DROP INDEX IF EXISTS PathesIndex;")
		_T("DROP TABLE IF EXISTS Pathes;")
	_T("COMMIT;")
	_T("VACUUM;");

const LPCTSTR VACUUM_DATABASE =
	_T("VACUUM;");

// Без IDL
class DECLSPEC_UUID("4A34B3E3-F50E-4FF6-8979-7E4176466FF2") Thumb;
DEFINE_GUID(CLSID_Thumb,0x4A34B3E3,0xF50E,0x4FF6,0x89,0x79,0x7E,0x41,0x76,0x46,0x6F,0xF2);
#define MyCLSID _T("{4A34B3E3-F50E-4FF6-8979-7E4176466FF2}")

using namespace ATL;

typedef CComCritSecLock < CComAutoCriticalSection > CLock;

DWORD GetRegValue(LPCTSTR szName, DWORD dwDefault, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
CString GetRegValue(LPCTSTR szName, LPCTSTR szDefault = _T(""), LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
void SetRegValue(LPCTSTR szName, DWORD dwValue, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
void SetRegValue(LPCTSTR szName, LPCTSTR szValue, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
void SetRegValue(LPCTSTR szName, const CString& sValue, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);

BOOL IsProcessElevated();

DWORD CRC32(const char *buf, int len);

// Создание пути (со всеми директориями которых нет)
void MakeDirectory(LPCTSTR dir);

// Получение системного пути
CString GetSpecialFolderPath(int csidl);

inline BYTE hs2b(TCHAR s)
{
	return (BYTE) ( ( s >= '0' && s <= _T('9') ) ? ( s - _T('0') ) :
		( s >= 'a' && s <= _T('f') ) ? ( s - _T('a') + 10 ) :
		( s >= 'A' && s <= _T('F') ) ? ( s - _T('A') + 10 ) : 0 );
}

inline size_t lengthof(LPCTSTR szString)
{
	return ( _tcslen( szString ) + 1 ) * sizeof( TCHAR );
}

inline size_t lengthof(const CString& sString)
{
	return (size_t)( sString.GetLength() + 1 ) * sizeof( TCHAR );
}

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
