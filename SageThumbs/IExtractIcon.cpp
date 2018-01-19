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

// IExtractIconA

STDMETHODIMP CThumb::GetIconLocation(UINT uFlags, __out_ecount( cch ) LPSTR szIconFile, UINT cch, __out int* piIndex, __out UINT* pwFlags)
{
	WCHAR szIconFileW[ MAX_LONG_PATH ] = {};
	HRESULT hr = GetIconLocation( uFlags, szIconFileW, MAX_LONG_PATH, piIndex, pwFlags );
	strcpy_s( szIconFile, cch, (LPCSTR)CW2A( szIconFileW ) );
	return hr;
}

STDMETHODIMP CThumb::Extract(LPCSTR pszFile, UINT nIconIndex, __out_opt HICON* phiconLarge, __out_opt HICON* phiconSmall, UINT nIconSize)
{
	return Extract( (LPCWSTR)CA2W( pszFile ), nIconIndex, phiconLarge, phiconSmall,nIconSize );
}

// IExtractIconW

STDMETHODIMP CThumb::GetIconLocation(UINT uFlags, __out_ecount( cch ) LPWSTR szIconFile, UINT cch, __out int* piIndex, __out UINT* pwFlags)
{
	if ( ! pwFlags || ! piIndex )
	{
		ATLTRACE( "CThumb - IExtractIcon::GetIconLocation() : E_POINTER\n" );
		return E_POINTER;
	}

	//if ( ! _Module.IsGoodFile( m_sFilename ) )
	//{
	//	ATLTRACE( "CThumb - IExtractIcon::GetIconLocation(\"%s\") : S_FALSE (Use default)\n", (LPCSTR)CT2A( (LPCTSTR)m_sFilename ) );
	//	return S_FALSE;
	//}

	// Make it unique
	LARGE_INTEGER count;
	QueryPerformanceCounter( &count );
	*piIndex = (int)count.LowPart;

	if ( szIconFile )
	{
		*szIconFile = 0;

		if ( ! m_sFilename.IsEmpty() )
		{
			wcsncpy_s( szIconFile, cch, (LPCWSTR)m_sFilename, _TRUNCATE );
		}
#ifdef ISTREAM_ENABLED
		else if ( m_pStream )
		{
			STATSTG stat = {};
			if ( SUCCEEDED( m_pStream->Stat( &stat,  STATFLAG_DEFAULT ) ) && stat.pwcsName )
			{
				wcscpy_s( szIconFile, cch, stat.pwcsName );
				if ( stat.pwcsName ) CoTaskMemFree( stat.pwcsName );
			}
		}
#endif // ISTREAM_ENABLED

		if ( *szIconFile )
		{
			*piIndex = (int)CRC32( (const char*)szIconFile, (int)( wcslen( szIconFile ) * sizeof( WCHAR ) ) );
		}
	}

	*pwFlags = GIL_NOTFILENAME | GIL_PERINSTANCE;

	if ( uFlags & GIL_CHECKSHIELD )
		*pwFlags |= GIL_FORCENOSHIELD;

	ATLTRACE( "CThumb - IExtractIcon::GetIconLocation(0x%08x,\"%s\",0x%08x,0x%08x) : %s\n", uFlags, (LPCSTR)CW2A( szIconFile ), (DWORD)*piIndex, *pwFlags, ( ( uFlags & GIL_DEFAULTICON ) ? "S_FALSE" : "S_OK" ) );
	return ( uFlags & GIL_DEFAULTICON ) ? S_FALSE : S_OK;
}

STDMETHODIMP CThumb::Extract(LPCWSTR pszFile, UINT nIconIndex, __out_opt HICON* phiconLarge, __out_opt HICON* phiconSmall, UINT nIconSize)
{
	pszFile;
	nIconIndex;

	//if ( !_Module.IsGoodFile( m_sFilename ) )
	//{
	//	ATLTRACE( "CThumb - IExtractIcon::Extract(\"%s\") : S_FALSE (Use default)\n", (LPCSTR)CT2A( (LPCTSTR)m_sFilename ) );
	//	return S_FALSE;
	//}

	if ( phiconLarge ) *phiconLarge = NULL;
	if ( phiconSmall ) *phiconSmall = NULL;

	UINT cxLarge = ( phiconLarge ? LOWORD( nIconSize ) : 0 );
	UINT cxSmall = ( phiconSmall ? HIWORD( nIconSize ) : 0 );
	m_cx = m_cy = max( cxLarge, cxSmall );
	if ( ! m_cx )
	{
		ATLTRACE( "CThumb - IExtractIcon::Extract(\"%s\") : E_FAIL (No size)\n", (LPCSTR)CW2A( pszFile ) );
		return E_FAIL;
	}

	if ( ! m_Preview )
	{
		if ( ! m_sFilename.IsEmpty() )
		{
			m_Preview.LoadImage( m_sFilename, m_cx, m_cy );
		}
#ifdef ISTREAM_ENABLED
		else if ( m_pStream )
		{
			m_Preview.LoadImage( m_pStream, m_cx, m_cy );
		}
#endif // ISTREAM_ENABLED
	}

	if ( ! m_Preview && ! m_sFilename.IsEmpty() )
	{
		// Attempt to load default icon
		CString sExt = PathFindExtension( m_sFilename );
		if ( sExt.IsEmpty() )
		{
			// No extension
			ATLTRACE( "CThumb - IExtractIcon::Extract(\"%s\") : E_FAIL (No extension)\n", (LPCSTR)CW2A( pszFile ) );
			return E_FAIL;
		}

		CString sDefaultIcon;
		CString sDefaultKey = GetRegValue( _T(""), CString(), sExt, HKEY_CLASSES_ROOT );
		if ( sDefaultKey.IsEmpty() )
		{
			sDefaultIcon = GetRegValue( _T(""), CString(), sExt + _T("\\DefaultIcon"), HKEY_CLASSES_ROOT );
		}
		else
		{
			sDefaultIcon = GetRegValue( _T(""), CString(), sDefaultKey + _T("\\DefaultIcon"), HKEY_CLASSES_ROOT );
			if ( sDefaultIcon.IsEmpty() )
			{
				sDefaultIcon = GetRegValue( _T(""), CString(), sExt + _T("\\DefaultIcon"), HKEY_CLASSES_ROOT );
			}
		}
		if ( sDefaultIcon.IsEmpty() )
		{
			// No icon
			ATLTRACE( "CThumb - IExtractIcon::Extract(\"%s\") : E_FAIL (No icon)\n", (LPCSTR)CW2A( pszFile ) );
			return E_FAIL;
		}

		sDefaultIcon.Replace( _T("%1"), m_sFilename );
		sDefaultIcon.Trim( _T('\"') );

		if ( ! LoadIcon( sDefaultIcon,
			( cxSmall == 16 ) ? phiconSmall : ( ( cxLarge == 16 ) ? phiconLarge : NULL ),
			( cxSmall == 32 ) ? phiconSmall : ( ( cxLarge == 32 ) ? phiconLarge : NULL ),
			( cxSmall == 48 ) ? phiconSmall : ( ( cxLarge == 48 ) ? phiconLarge : NULL ) ) )
		{
			// Found no icon
			ATLTRACE( "CThumb - IExtractIcon::Extract(\"%s\") : E_FAIL (Found no icon \"%s\")\n", (LPCSTR)CW2A( pszFile ), (LPCSTR)CW2A( sDefaultIcon ) );
			return E_FAIL;
		}

		ATLTRACE( "CThumb - IExtractIcon::Extract(\"%s\") : S_OK (Default \"%s\")\n", (LPCSTR)CW2A( pszFile ), (LPCSTR)CW2A( sDefaultIcon ) );
		return S_OK;
	}

	if ( cxLarge )
	{
		*phiconLarge = m_Preview.GetIcon( cxLarge );
	}

	if ( cxSmall )
	{
		*phiconSmall = m_Preview.GetIcon( cxSmall );
	}

	ATLTRACE( "CThumb - IExtractIcon::Extract(\"%s\",0x%08x,%d,%d) : S_OK\n", (LPCSTR)CW2A( pszFile ), nIconIndex, cxLarge, cxSmall );
	return S_OK;
}
