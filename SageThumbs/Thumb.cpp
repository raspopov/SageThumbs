/*
SageThumbs - Thumbnail image shell extension.

Copyright (C) Nikolay Raspopov, 2004-2018.

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
#include "SageThumbs.h"
#include "Thumb.h"
#include <InitGuid.h>

DEFINE_GUID(CLSID_Thumb,0x4A34B3E3,0xF50E,0x4FF6,0x89,0x79,0x7E,0x41,0x76,0x46,0x6F,0xF2);

CThumb::CThumb()
	: m_uOurItemID	( 0 )
	, m_cx			( THUMB_STORE_SIZE )
	, m_cy			( THUMB_STORE_SIZE )
	, m_bCleanup	( FALSE )
{
}

HRESULT CThumb::FinalConstruct()
{
#ifdef _DEBUG
	TCHAR user_name[ 256 ] = {};
	DWORD user_name_size = _countof( user_name );
	GetUserName ( user_name, &user_name_size );
	ATLTRACE( "CThumb - FinalConstruct() : Running under user: \"%s\"\n", (LPCSTR)CT2A( user_name ) );
#endif

	return CoCreateFreeThreadedMarshaler( GetControllingUnknown(), &m_pUnkMarshaler.p );
}

void CThumb::FinalRelease()
{
	m_Filenames.RemoveAll();

	m_sFilename.Empty();

#ifdef ISTREAM_ENABLED
	m_pStream.Release();
#endif // ISTREAM_ENABLED

	m_pSite.Release();

	m_pUnkMarshaler.Release();

	ATLTRACE( "CThumb - FinalRelease()\n" );
}

// IShellExtInit

STDMETHODIMP CThumb::Initialize( _In_opt_ PCIDLIST_ABSOLUTE /*pidlFolder*/, _In_opt_ IDataObject* pdtobj, _In_opt_ HKEY /*hkeyProgID*/ )
{
	if ( ! pdtobj )
	{
		ATLTRACE( "CThumb - IShellExtInit::Initialize() : E_INVALIDARG (No data)\n" );
		return E_INVALIDARG;
	}

	bool bEnableMenu = GetRegValue( _T("EnableMenu"), 1ul ) != 0;
	if ( ! bEnableMenu )
	{
		ATLTRACE( "CThumb - IShellExtInit::Initialize() : E_INVALIDARG (Menu disabled)\n" );
		return E_INVALIDARG;
	}

	// Получение данных о выделенных элементах
	FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM med = { TYMED_HGLOBAL, NULL, NULL };
	HRESULT hr = pdtobj->GetData( &fe, &med );
	if ( FAILED( hr ) )
	{
		ATLTRACE( "CThumb - IShellExtInit::Initialize() : E_INVALIDARG (No data)\n" );
		return E_INVALIDARG;
	}

    HDROP hDrop = (HDROP)GlobalLock( med.hGlobal );
    if ( ! hDrop )
	{
		ReleaseStgMedium (&med);
		ATLTRACE( "CThumb - IShellExtInit::Initialize() : E_INVALIDARG (No data)\n" );
		return E_INVALIDARG;
	}

	// Отбор интересных имен объектов
	UINT count = DragQueryFile( hDrop, 0xFFFFFFFF, 0, 0 );
	for ( UINT i = 0; i < count; i++ )
	{
		CString filename;
		LPTSTR buf = filename.GetBuffer( MAX_LONG_PATH );
		DWORD len = DragQueryFile( hDrop, i, buf, MAX_LONG_PATH - 1 );
		buf[ len ] = _T('\0');
		filename.ReleaseBuffer();

		if ( _Module.IsGoodFile( filename ) )
		{
			m_Filenames.AddTail( filename );
		}
		else
		{
			ATLTRACE( "CThumb - IShellExtInit::Initialize() : Disabled file \"%s\"\n", (LPCSTR)CT2A( filename ) );
		}
	}
    GlobalUnlock( med.hGlobal );
	ReleaseStgMedium( &med );

	if ( m_Filenames.IsEmpty() )
	{
		ATLTRACE( "CThumb - IShellExtInit::Initialize() : E_INVALIDARG (No files selected)\n" );
		return E_INVALIDARG;
	}

	ATLTRACE( "CThumb - IShellExtInit::Initialize() : S_OK (%d files, first: \"%s\")\n", m_Filenames.GetCount(), (LPCSTR)CT2A( m_Filenames.GetHead() ) );
	return S_OK;
}

// IPersistFile

STDMETHODIMP CThumb::Load(LPCOLESTR wszFile, DWORD /*dwMode*/)
{
	if ( ! wszFile )
	{
		ATLTRACE( "CThumb - IPersistFile::Load() : E_POINTER\n" );
		return E_POINTER;
	}

	//if ( !_Module.IsGoodFile( wszFile ) )
	//{
	//	ATLTRACE( "CThumb - IPersistFile::Load(\"%s\") : E_INVALIDARG (Disabled file)\n", (LPCSTR)CT2A( wszFile ) );
	//	return E_INVALIDARG;
	//}

	m_sFilename = wszFile;

	ATLTRACE( "CThumb - IPersistFile::Load(\"%s\") : S_OK\n", (LPCSTR)CW2A( wszFile ) );
	return S_OK;
}

STDMETHODIMP CThumb::GetClassID(LPCLSID pclsid)
{
	if ( ! pclsid )
	{
		ATLTRACE ("CThumb - IPersist::GetClassID() : E_POINTER\n");
		return E_POINTER;
	}

	*pclsid = CLSID_Thumb;

	ATLTRACE ("CThumb - IPersist::GetClassID() : S_OK\n");
	return S_OK;
}

STDMETHODIMP CThumb::IsDirty()
{
	ATLTRACENOTIMPL( _T("CThumb - IPersistFile::IsDirty") );
}

STDMETHODIMP CThumb::Save(LPCOLESTR, BOOL)
{
	ATLTRACENOTIMPL( _T("CThumb - IPersistFile::Save") );
}

STDMETHODIMP CThumb::SaveCompleted(LPCOLESTR)
{
	ATLTRACENOTIMPL( _T("CThumb - IPersistFile::SaveCompleted") );
}

STDMETHODIMP CThumb::GetCurFile(LPOLESTR*)
{
	ATLTRACENOTIMPL( _T("CThumb - IPersistFile::GetCurFile") );
}

// IInitializeWithStream

#ifdef ISTREAM_ENABLED

STDMETHODIMP CThumb::Initialize(
	/* [in] */ IStream * pstream,
	/* [in] */ DWORD /*grfMode*/)
{
	if ( m_pStream )
	{
		ATLTRACE( "CThumb - IInitializeWithStream::Initialize() : ERROR_ALREADY_INITIALIZED\n" );
		return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
	}

	if ( ! pstream )
	{
		ATLTRACE( "CThumb - IInitializeWithStream::Initialize() : E_POINTER\n" );
		return E_POINTER;
	}

	m_pStream = pstream;

	ATLTRACE( "CThumb - IInitializeWithStream::Initialize() : S_OK\n" );
	return S_OK;
}

#endif // ISTREAM_ENABLED

// IInitializeWithItem

STDMETHODIMP CThumb::Initialize(
  /* [in] */ __RPC__in_opt IShellItem* psi,
  /* [in] */ DWORD /* grfMode */)
{
	if ( ! m_sFilename.IsEmpty() )
	{
		ATLTRACE( "CThumb - IInitializeWithItem::Initialize() : ERROR_ALREADY_INITIALIZED\n" );
		return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
	}

	if ( psi == NULL  )
	{
		ATLTRACE( "CThumb - IInitializeWithItem::Initialize() : E_POINTER\n" );
		return E_POINTER;
	}

	LPWSTR wszFile = NULL;
	HRESULT hr = psi->GetDisplayName( SIGDN_FILESYSPATH, &wszFile );
	if ( FAILED( hr ) )
	{
		ATLTRACE( "CThumb - IInitializeWithItem::Initialize() : E_FAIL (Unknown path) : 0x%08x\n", hr );
		return hr;
	}

	//if ( !_Module.IsGoodFile( wszFile ) )
	//{
	//	ATLTRACE( "CThumb - IInitializeWithItem::Initialize(\"%s\") : E_INVALIDARG (Disabled file)\n", (LPCSTR)CT2A( wszFile ) );
	//	CoTaskMemFree( wszFile );
	//	return E_INVALIDARG;
	//}

	m_sFilename = wszFile;

	ATLTRACE( "CThumb - IInitializeWithItem::Initialize(\"%s\") : S_OK\n", (LPCSTR)CW2A( wszFile ) );
	CoTaskMemFree( wszFile );
	return S_OK;
}

// IInitializeWithFile

STDMETHODIMP CThumb::Initialize(
	/* [in] */ LPCWSTR wszFile,
	/* [in] */ DWORD /* grfMode */)
{
	if ( ! m_sFilename.IsEmpty() )
	{
		ATLTRACE( "CThumb - IInitializeWithFile::Initialize() : ERROR_ALREADY_INITIALIZED\n" );
		return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
	}

	if ( wszFile == NULL  )
	{
		ATLTRACE( "CThumb - IInitializeWithFile::Initialize() : E_POINTER\n" );
		return E_POINTER;
	}

	//if ( !_Module.IsGoodFile( wszFile ) )
	//{
	//	ATLTRACE( "CThumb - IInitializeWithFile::Initialize(\"%s\") : E_INVALIDARG (Disabled file)\n", (LPCSTR)CT2A( wszFile ) );
	//	return E_INVALIDARG;
	//}

	m_sFilename = wszFile;

	ATLTRACE( "CThumb - IInitializeWithFile::Initialize(\"%s\") : S_OK\n", (LPCSTR)CW2A( wszFile ) );
	return S_OK;
}

// IOleWindow

//STDMETHODIMP CThumb::GetWindow(HWND *phwnd)
//{
//	ATLTRACENOTIMPL( _T("IOleWindow::GetWindow") );
//}
//
//STDMETHODIMP CThumb::ContextSensitiveHelp(BOOL /*fEnterMode*/)
//{
//	ATLTRACENOTIMPL( _T("IOleWindow::ContextSensitiveHelp") );
//}

// IRunnableTask
//
//STDMETHODIMP CThumb::Run ()
//{
//	ATLTRACENOTIMPL ("IRunnableTask::Run");
//}
//
//STDMETHODIMP CThumb::Kill (BOOL /*fWait*/)
//{
//	ATLTRACENOTIMPL ("IRunnableTask::Kill");
//}
//
//STDMETHODIMP CThumb::Suspend ()
//{
//	ATLTRACENOTIMPL ("IRunnableTask::Suspend");
//}
//
//STDMETHODIMP CThumb::Resume ()
//{
//	ATLTRACENOTIMPL("IRunnableTask::Resume");
//}
//
//STDMETHODIMP_(ULONG) CThumb::IsRunning ()
//{
//	ATLTRACE ("IRunnableTask::IsRunning\n");
//	return IRTIR_TASK_FINISHED;
//}

// IObjectWithSite

STDMETHODIMP CThumb::SetSite(IUnknown *pUnkSite)
{
	ATLTRACE( "CThumb - IObjectWithSite::SetSite(0x%08x)\n", pUnkSite );

	if ( pUnkSite )
		m_pSite = pUnkSite;
	else
		m_pSite.Release();

	return S_OK;
}

STDMETHODIMP CThumb::GetSite(REFIID riid, void **ppvSite)
{
	ATLTRACE ( "CThumb - IObjectWithSite::GetSite()\n" );

	if ( ! ppvSite )
		return E_POINTER;

	*ppvSite = NULL;

	if ( ! m_pSite )
		return E_FAIL;

	return m_pSite->QueryInterface( riid, ppvSite );
}

// IColumnProvider
//
//STDMETHODIMP CThumb::Initialize (LPCSHCOLUMNINIT /* psci */)
//{
//	ATLTRACENOTIMPL ("IColumnProvider::Initialize");
//}
//
//STDMETHODIMP CThumb::GetColumnInfo (DWORD /* dwIndex */, SHCOLUMNINFO* /* psci */)
//{
//	ATLTRACENOTIMPL ("IColumnProvider::GetColumnInfo");
//}
//
//STDMETHODIMP CThumb::GetItemData (LPCSHCOLUMNID /* pscid */, LPCSHCOLUMNDATA /* pscd */, VARIANT* /* pvarData */)
//{
//	ATLTRACENOTIMPL ("IColumnProvider::GetItemData");
//}

// IParentAndItem

//STDMETHODIMP CThumb::SetParentAndItem(
//	/* [unique][in] */ __RPC__in_opt PCIDLIST_ABSOLUTE /*pidlParent*/,
//	/* [unique][in] */ __RPC__in_opt IShellFolder * /*psf*/,
//	/* [in] */ __RPC__in PCUITEMID_CHILD /*pidlChild*/)
//{
//	ATLTRACENOTIMPL( _T("IParentAndItem::SetParentAndItem") );
//}
//
//STDMETHODIMP CThumb::GetParentAndItem(
//	/* [out] */ __RPC__deref_out_opt PIDLIST_ABSOLUTE * /*ppidlParent*/,
//	/* [out] */ __RPC__deref_out_opt IShellFolder ** /*ppsf*/,
//	/* [out] */ __RPC__deref_out_opt PITEMID_CHILD * /*ppidlChild*/)
//{
//	ATLTRACENOTIMPL( _T("IParentAndItem::GetParentAndItem") );
//}
