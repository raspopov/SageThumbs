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

// IExtractImage

// *pdwPriority:
// const DWORD ITSAT_MAX_PRIORITY		= 0x7fffffff;
// const DWORD ITSAT_MIN_PRIORITY		= 0x00000000;
// const DWORD ITSAT_DEFAULT_PRIORITY	= 0x10000000;
// const DWORD IEI_PRIORITY_MAX			= ITSAT_MAX_PRIORITY;
// const DWORD IEI_PRIORITY_MIN			= ITSAT_MIN_PRIORITY;
// const DWORD IEIT_PRIORITY_NORMAL		= ITSAT_DEFAULT_PRIORITY;
// *pdwFlags:
// const DWORD IEIFLAG_ASYNC      = 0x0001;	// ask the extractor if it supports ASYNC extract (free threaded)
// const DWORD IEIFLAG_CACHE      = 0x0002;	// returned from the extractor if it does NOT cache the thumbnail
// const DWORD IEIFLAG_ASPECT     = 0x0004;	// passed to the extractor to beg it to render to the aspect ratio of the supplied rect
// const DWORD IEIFLAG_OFFLINE    = 0x0008;	// if the extractor shouldn't hit the net to get any content neede for the rendering
// const DWORD IEIFLAG_GLEAM      = 0x0010;	// does the image have a gleam ? this will be returned if it does
// const DWORD IEIFLAG_SCREEN     = 0x0020;	// render as if for the screen  (this is exlusive with IEIFLAG_ASPECT )
// const DWORD IEIFLAG_ORIGSIZE   = 0x0040;	// render to the approx size passed, but crop if neccessary
// const DWORD IEIFLAG_NOSTAMP    = 0x0080;	// returned from the extractor if it does NOT want an icon stamp on the thumbnail
// const DWORD IEIFLAG_NOBORDER   = 0x0100;	// returned from the extractor if it does NOT want an a border around the thumbnail
// const DWORD IEIFLAG_QUALITY    = 0x0200;	// passed to the Extract method to indicate that a slower, higher quality image is desired, re-compute the thumbnail
// const DWORD IEIFLAG_REFRESH    = 0x0400;	// returned from the extractor if it would like to have Refresh Thumbnail available

STDMETHODIMP CThumb::GetLocation (
    /* [size_is][out] */ LPWSTR pszPathBuffer,
    /* [in] */ DWORD cch,
    /* [unique][out][in] */ DWORD* /* pdwPriority */,
    /* [in] */ const SIZE* prgSize,
    /* [in] */ DWORD /* dwRecClrDepth */,
    /* [in] */ DWORD* pdwFlags)
{
	if ( ! GetRegValue( _T("EnableThumbs"), 1ul ) )
	{
		ATLTRACE( "CThumb - IExtractImage::GetLocation(%dx%d) : E_FAIL (Disabled)\n", (prgSize ? prgSize->cx : 0), (prgSize ? prgSize->cy : 0) );
		return E_FAIL;
	}

	if ( pszPathBuffer )
	{
		*pszPathBuffer = 0;

		if ( ! m_sFilename.IsEmpty() )
		{
			wcsncpy_s( pszPathBuffer, cch, (LPCWSTR)m_sFilename, _TRUNCATE );
		}
#ifdef ISTREAM_ENABLED
		else if ( m_pStream )
		{
			STATSTG stat = {};
			if ( SUCCEEDED( m_pStream->Stat( &stat,  STATFLAG_DEFAULT ) ) && stat.pwcsName )
			{
				wcscpy_s( pszPathBuffer, cch, stat.pwcsName );
				if ( stat.pwcsName ) CoTaskMemFree( stat.pwcsName );
			}
		}
#endif // ISTREAM_ENABLED
	}

	// Получение размеров от системы
	if ( prgSize )
	{
		m_cx = prgSize->cx;
		m_cy = prgSize->cy;
	}

	if ( pdwFlags )
	{
		if ( GetRegValue( _T("WinCache"), 1ul ) != 0 )
			*pdwFlags = IEIFLAG_CACHE;
		else
			*pdwFlags = 0;
	}

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

	ATLTRACE( "CThumb - IExtractImage::GetLocation(%dx%d) : %s\n", (prgSize ? prgSize->cx : 0), (prgSize ? prgSize->cy : 0), ( m_Preview ? "S_OK" : "E_FAIL" ) );
	return m_Preview ? S_OK : E_FAIL;
}

STDMETHODIMP CThumb::Extract (
	/* [out] */ HBITMAP *phBmpThumbnail)
{
	if ( ! phBmpThumbnail )
	{
		ATLTRACE( "CThumb - IExtractImage::Extract() : E_POINTER\n" );
		return E_POINTER;
	}
	*phBmpThumbnail = NULL;

	if ( ! GetRegValue( _T("EnableThumbs"), 1ul ) )
	{
		ATLTRACE( "CThumb - IExtractImage::Extract() : E_FAIL (Disabled)\n" );
		return E_FAIL;
	}

	*phBmpThumbnail = m_Preview.GetImage( m_cx, m_cy );
	if ( ! *phBmpThumbnail )
	{
		ATLTRACE( "CThumb - IExtractImage::Extract() : E_FAIL (Load failed)\n" );
		return E_FAIL;
	}

	ATLTRACE( "CThumb - IExtractImage::Extract() : S_OK\n" );
	return S_OK;
}

// IExtractImage2

STDMETHODIMP CThumb::GetDateStamp (
	/* [out] */ FILETIME *pDateStamp)
{
	if ( ! GetRegValue( _T("EnableThumbs"), 1ul ) )
	{
		ATLTRACE( "CThumb - IExtractImage2:GetDateStamp() : E_FAIL (Disabled)\n" );
		return E_FAIL;
	}

	if ( ! pDateStamp )
	{
		ATLTRACE( "CThumb - IExtractImage2:GetDateStamp() : E_POINTER\n" );
		return E_POINTER;
	}

	m_Preview.GetLastWriteTime( pDateStamp );

	ATLTRACE( "CThumb - IExtractImage2:GetDateStamp() : S_OK\n" );
	return S_OK;
}
