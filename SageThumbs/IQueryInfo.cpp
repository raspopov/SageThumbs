/*
SageThumbs - Thumbnail image shell extension.

Copyright (C) Nikolay Raspopov, 2004-2014.

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

// IQueryInfo

STDMETHODIMP CThumb::GetInfoFlags(DWORD* pdwFlags)
{
	if ( pdwFlags )
		*pdwFlags = 0;

	ATLTRACE( "CThumb - IQueryInfo::GetInfoFlags() : S_OK\n" );
	return S_OK;
}

STDMETHODIMP CThumb::GetInfoTip(DWORD, LPWSTR* ppwszTip)
{
	if ( ! ppwszTip )
	{
		ATLTRACE( "CThumb - IQueryInfo::GetInfoTip() : E_POINTER\n" );
		return E_POINTER;
	}
	*ppwszTip = NULL;

	CComPtr<IMalloc> pIMalloc;
	if ( FAILED( SHGetMalloc ( &pIMalloc ) ) )
	{
		ATLTRACE( "CThumb - IQueryInfo::GetInfoTip() : E_OUTOFMEMORY\n" );
		return E_OUTOFMEMORY;
	}

	if ( ! m_sFilename.IsEmpty() )
	{
		m_Preview.LoadInfo( m_sFilename );
	}
#ifdef ISTREAM_ENABLED
	else if ( m_pStream )
	{
		m_Preview.LoadInfo( m_pStream );
	}
#endif // ISTREAM_ENABLED

	if ( ! m_Preview.IsInfoAvailable() )
	{
		ATLTRACE( "CThumb - IQueryInfo::GetInfoTip() : E_FAIL (Load failed)\n" );
		return E_FAIL;
	}

	CT2W info( m_Preview.GetInfoTipString() );
	const size_t len = wcslen( (LPCWSTR)info ) + 1;
	*ppwszTip = (LPWSTR) pIMalloc->Alloc( len * sizeof( WCHAR ) );
	if ( ! *ppwszTip )
	{
		ATLTRACE( "CThumb - IQueryInfo::GetInfoTip() : E_OUTOFMEMORY\n" );
		return E_OUTOFMEMORY;
	}
	wcscpy_s( *ppwszTip, len, (LPCWSTR)info );

	ATLTRACE( "CThumb - IQueryInfo::GetInfoTip() : S_OK (\"%s\")\n", (LPCSTR)CW2A( *ppwszTip ) );
	return S_OK;
}
