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
#include <delayimp.h>
#include <ddraw.h>
#include <emptyvc.h>
#include <mapi.h>
#include <ocmm.h>
#include <propkey.h>
#include <richedit.h>
#include <shlobj.h>
#include <sddl.h>
#include <thumbcache.h>

#include "../gfl/libgfl.h"
#include "../gfl/libgfle.h"

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

#ifndef QWORD
	typedef ULONGLONG QWORD;
#endif

typedef ULONG (FAR PASCAL *tMAPISendMail)(LHANDLE, ULONG_PTR, lpMapiMessage, FLAGS, ULONG);
typedef UINT (WINAPI *tPrivateExtractIconsT)(LPCTSTR, int, int, int, HICON*, UINT*, UINT, UINT);

#define MAKEQWORD(l,h) ((QWORD)(l)|((QWORD)(h)<<32))

// SDK Fix
struct __declspec(uuid("85788D00-6807-11D0-B810-00C04FD706EC")) IRunnableTask;
struct __declspec(uuid("000214E4-0000-0000-C000-000000000046")) IContextMenu;
struct __declspec(uuid("000214F4-0000-0000-C000-000000000046")) IContextMenu2;
struct __declspec(uuid("BCFCE0A0-EC17-11d0-8D10-00A0C90F2719")) IContextMenu3;
struct __declspec(uuid("00021500-0000-0000-c000-000000000046")) IQueryInfo;
struct __declspec(uuid("E8025004-1C42-11D2-BE2C-00A0C9A83DA1")) IColumnProvider;

// {889900c3-59f3-4c2f-ae21-a409ea01e605}
DEFINE_GUID(CLSID_WindowsThumbnailer,0x889900c3,0x59f3,0x4c2f,0xae,0x21,0xa4,0x09,0xea,0x01,0xe6,0x05);

#define ShellImagePreview	_T("SystemFileAssociations\\image\\ShellEx\\ContextMenuHandlers\\ShellImagePreview")
#define FileExts			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\")
#define PropertyHandlers	_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\PropertySystem\\PropertyHandlers\\")
#define REG_XNVIEW_KEY		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\XnView_is1")
#define REG_XNVIEW_PATH1	_T("UninstallString")
#define REG_XNVIEW_PATH2	_T("Inno Setup: App Path")
#define CLSID_FAX			_T("{E84FDA7C-1D6A-45F6-B725-CB260C236066}")
#define CLSID_HTML			_T("{25336920-03F9-11cf-8FD0-00AA00686F13}")
#define MAX_LONG_PATH		(MAX_PATH * 2)

using namespace ATL;

typedef CComCritSecLock < CComAutoCriticalSection > CLock;

BOOL IsProcessElevated();
void CleanWindowsCache();
BOOL LoadIcon(LPCTSTR szFilename, HICON* phSmallIcon, HICON* phLargeIcon = NULL, HICON* phHugeIcon = NULL, int nIcon = 0);
DWORD CRC32(const char *buf, int len);

// Recreate folder including all sub-folders
void MakeDirectory(LPCTSTR dir);

// Get system folder path
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
