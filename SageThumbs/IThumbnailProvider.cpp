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

// IThumbnailProvider

STDMETHODIMP CThumb::GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha)
{
	if ( ! phbmp )
	{
		ATLTRACE( "CThumb - IThumbnailProvider::GetThumbnail(%d) : E_POINTER\n", cx );
		return E_POINTER;
	}
	*phbmp = NULL;

	if ( pdwAlpha )
		*pdwAlpha = WTSAT_UNKNOWN;

	if ( ! GetRegValue( _T("EnableThumbs"), 1ul ) )
	{
		ATLTRACE( "CThumb - IThumbnailProvider::GetThumbnail(%d) : E_FAIL (Disabled)\n", cx );
		return E_FAIL;
	}

	if ( ! m_Preview )
	{
		if ( ! m_sFilename.IsEmpty() )
		{
			m_Preview.LoadImage( m_sFilename, cx, cx );
		}
#ifdef ISTREAM_ENABLED
		else if ( m_pStream )
		{
			m_Preview.LoadImage( m_pStream, cx, cx );
		}
#endif // ISTREAM_ENABLED
	}

	*phbmp = m_Preview.GetImage( cx, cx );
	if ( ! *phbmp )
	{
		ATLTRACE( "CThumb - IThumbnailProvider::GetThumbnail(%d) : E_FAIL (Load failed)\n", cx );
		return E_FAIL;
	}

	ATLTRACE( "CThumb - IThumbnailProvider::GetThumbnail(%d) : S_OK\n", cx );

	return S_OK;
}
