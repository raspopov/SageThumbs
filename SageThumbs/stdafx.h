/*
SageThumbs - Thumbnail image shell extension.

Copyright (C) Nikolay Raspopov, 2004-2016.

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

#define STRICT

#ifndef _SECURE_ATL
	#define _SECURE_ATL	1
#endif

#ifndef VC_EXTRALEAN
	#define VC_EXTRALEAN
#endif

#define WIN32_LEAN_AND_MEAN

#define _ATL_FREE_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_NO_CRT
#define _ATL_ALL_WARNINGS
#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW
#define NO_PRINT
#define STRICT_TYPED_ITEMIDS

#ifdef _DEBUG
	//#define _ATL_DEBUG_INTERFACES
	#define _ATL_DEBUG_QI
#endif

#define ISOLATION_AWARE_ENABLED 1

#include "targetver.h"

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
#include <shellapi.h>
#include <shlobj.h>
#include <sddl.h>
#include <time.h>
#include <thumbcache.h>
#include <uxtheme.h>

#include <list>

// Windows 2000 compatibility

#ifndef BCM_FIRST
	#define BCM_FIRST				0x1600
#endif

#ifndef BCM_SETSHIELD
	#define BCM_SETSHIELD			(BCM_FIRST + 0x000C)
#endif

#ifndef PROGDLG_MARQUEEPROGRESS
	#define PROGDLG_MARQUEEPROGRESS	0x00000020
#endif

#ifndef PROGDLG_NOCANCEL
	#define PROGDLG_NOCANCEL		0x00000040
#endif

#ifndef PERCEIVED_TYPE_IMAGE
	#define PERCEIVED_TYPE_IMAGE	2
#endif

#include "..\Localization\Localization.h"

#include "..\gfl\libgfl.h"
#include "..\gfl\libgfle.h"

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
// {6D748DE2-8D38-4CC3-AC60-F009B057C557}
DEFINE_GUID(FMTID_RecordedTVSummaryInformation, 0x6D748DE2, 0x8D38, 0x4CC3, 0xAC, 0x60, 0xF0, 0x09, 0xB0, 0x57, 0xC5, 0x57);

#define ShellImagePreview	_T("SystemFileAssociations\\image\\ShellEx\\ContextMenuHandlers\\ShellImagePreview")
#define FileExts			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts")
#define KindMap				_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\KindMap")
#define PropertyHandlers	_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\PropertySystem\\PropertyHandlers")
#define REG_XNVIEW_KEY		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\XnView_is1")
#define REG_XNVIEW_PATH1	_T("UninstallString")
#define REG_XNVIEW_PATH2	_T("Inno Setup: App Path")
#define CLSID_FAX			_T("{E84FDA7C-1D6A-45F6-B725-CB260C236066}")
#define CLSID_HTML			_T("{25336920-03F9-11cf-8FD0-00AA00686F13}")
#define MAX_LONG_PATH		(MAX_PATH * 2)

using namespace ATL;

typedef CComCritSecLock < CComAutoCriticalSection > CLock;

inline size_t lengthof(LPCTSTR szString)
{
	return ( _tcslen( szString ) + 1 ) * sizeof( TCHAR );
}

inline size_t lengthof(const CString& sString)
{
	return (size_t)( sString.GetLength() + 1 ) * sizeof( TCHAR );
}

inline DWORD Span( DWORD cur, DWORD prev )
{
	return ( cur >= prev ) ? ( cur - prev ) : ( ( (DWORD)-1 - prev ) + 1 + cur );
}

template <class Base, const IID* piid, class T, class Copy, class CollType>
class ATL_NO_VTABLE IEnumOnSTLImplExt : public Base
{
public:
	inline HRESULT Init(IUnknown *pUnkForRelease, CollType& collection)
	{
		m_spUnk = pUnkForRelease;
		m_collection = collection;
		m_iter = m_collection.begin();
		return S_OK;
	}
	inline STDMETHOD(Next)(ULONG celt, T* rgelt, ULONG* pceltFetched);
	inline STDMETHOD(Skip)(ULONG celt);
	inline STDMETHOD(Reset)(void)
	{
		m_iter = m_collection.begin();
		return S_OK;
	}
	inline STDMETHOD(Clone)(Base** ppEnum);
//Data
	CComPtr<IUnknown> m_spUnk;
	typename CollType m_collection;
	typename CollType::const_iterator m_iter;
};

template <class Base, const IID* piid, class T, class Copy, class CollType>
inline STDMETHODIMP IEnumOnSTLImplExt<Base, piid,
	T, Copy, CollType>::Next(ULONG celt, T* rgelt, ULONG* pceltFetched)
{
	if (rgelt == NULL || (celt != 1 && pceltFetched == NULL))
		return E_POINTER;

	ULONG nActual = 0;
	HRESULT hr = S_OK;
	if ( ! m_collection.empty() )
	{
		T* pelt = rgelt;
		while (SUCCEEDED(hr) && m_iter != m_collection.end() && nActual < celt)
		{
			hr = Copy::copy(pelt, &*m_iter);
			if (FAILED(hr))
			{
				while (rgelt < pelt)
					Copy::destroy(rgelt++);
				nActual = 0;
			}
			else
			{
				pelt++;
				m_iter++;
				nActual++;
			}
		}
	}
	if (pceltFetched)
		*pceltFetched = nActual;
	if (SUCCEEDED(hr) && (nActual < celt))
		hr = S_FALSE;
	return hr;
}

template <class Base, const IID* piid, class T, class Copy, class CollType>
inline STDMETHODIMP IEnumOnSTLImplExt<Base, piid, T, Copy, CollType>::Skip(ULONG celt)
{
	if ( m_collection.empty() )
		return celt ? S_FALSE : S_OK;

	HRESULT hr = S_OK;
	while (celt--)
	{
		if (m_iter != m_collection.end())
			m_iter++;
		else
		{
			hr = S_FALSE;
			break;
		}
	}
	return hr;
}

template <class Base, const IID* piid, class T, class Copy, class CollType>
inline STDMETHODIMP IEnumOnSTLImplExt<Base, piid, T, Copy, CollType>::Clone(Base** ppEnum)
{
	typedef CComObject<CComEnumOnSTL<Base, piid, T, Copy, CollType> > _class;
	HRESULT hRes = E_POINTER;
	if (ppEnum != NULL)
	{
		*ppEnum = NULL;
		_class* p;
		hRes = _class::CreateInstance(&p);
		if (SUCCEEDED(hRes))
		{
			hRes = p->Init(m_spUnk, m_collection);
			if (SUCCEEDED(hRes))
			{
				p->m_iter = m_iter;
				hRes = p->_InternalQueryInterface(*piid, (void**)ppEnum);
			}
			if (FAILED(hRes))
				delete p;
		}
	}
	return hRes;
}

template <class Base, const IID* piid, class T, class Copy, class CollType, class ThreadModel = CComObjectThreadModel>
class ATL_NO_VTABLE CComEnumOnSTLExt :
	public IEnumOnSTLImplExt<Base, piid, T, Copy, CollType>,
	public CComObjectRootEx< ThreadModel >
{
public:
	typedef CComEnumOnSTLExt<Base, piid, T, Copy, CollType, ThreadModel > _CComEnum;
	typedef IEnumOnSTLImplExt<Base, piid, T, Copy, CollType > _CComEnumBase;
	BEGIN_COM_MAP(_CComEnum)
		COM_INTERFACE_ENTRY_IID(*piid, _CComEnumBase)
	END_COM_MAP()
};

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
