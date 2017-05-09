/*
SageThumbs - Thumbnail image shell extension.

Copyright (C) Nikolay Raspopov, 2004-2017.

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
#include "SQLite.h"

// IEmptyVolumeCache

STDMETHODIMP CThumb::Initialize(
	/* [in] */ HKEY /*hkRegKey*/,
	/* [in] */ LPCWSTR pcwszVolume,
	/* [out] */ LPWSTR *ppwszDisplayName,
	/* [out] */ LPWSTR *ppwszDescription,
	/* [out] */ DWORD *pdwFlags)
{
	if ( ppwszDisplayName )
	{
		CString foo = LoadString( IDS_CACHE );
		size_t len = ( foo.GetLength() + 1 ) * sizeof( TCHAR );
		*ppwszDisplayName = (LPWSTR)CoTaskMemAlloc( len );
		if ( *ppwszDisplayName )
			CopyMemory( *ppwszDisplayName, (LPCTSTR)foo, len );
	}

	if ( ppwszDescription )
	{
		CString foo = LoadString( IDS_DESCRIPTION );
		size_t len = ( foo.GetLength() + 1 ) * sizeof( TCHAR );
		*ppwszDescription = (LPWSTR)CoTaskMemAlloc( len );
		if ( *ppwszDescription )
			CopyMemory( *ppwszDescription, (LPCTSTR)foo, len );
	}

	m_bCleanup = ( _Module.m_sDatabase.GetAt( 0 ) == *pcwszVolume );

	if ( m_bCleanup )
	{
		return S_OK;
	}

	if ( pdwFlags )
	{
		*pdwFlags |= EVCF_DONTSHOWIFZERO;
	}

	return S_FALSE;
}

STDMETHODIMP CThumb::GetSpaceUsed(
	/* [out] */ __RPC__out DWORDLONG *pdwlSpaceUsed,
	/* [in] */ __RPC__in_opt IEmptyVolumeCacheCallBack* /*picb*/)
{
	if ( ! m_bCleanup )
	{
		if ( pdwlSpaceUsed )
		{
			*pdwlSpaceUsed = 0;
		}
		return S_OK;
	}

	WIN32_FILE_ATTRIBUTE_DATA wfadDatabase = {};
	GetFileAttributesEx( _Module.m_sDatabase, GetFileExInfoStandard, &wfadDatabase );
	if ( pdwlSpaceUsed )
	{
		*pdwlSpaceUsed = MAKEQWORD( wfadDatabase.nFileSizeLow, wfadDatabase.nFileSizeHigh );
	}

	return S_OK;
}

STDMETHODIMP CThumb::Purge(
	/* [in] */ DWORDLONG /*dwlSpaceToFree*/,
	/* [in] */ __RPC__in_opt IEmptyVolumeCacheCallBack * /*picb*/)
{
	CDatabase db( _Module.m_sDatabase );
	if ( db )
	{
		db.Exec( DROP_DATABASE );
		db.Exec( RECREATE_DATABASE );
	}

	return S_OK;
}

STDMETHODIMP CThumb::ShowProperties(
	/* [in] */ __RPC__in HWND /*hwnd*/)
{
	ATLTRACENOTIMPL( _T("IEmptyVolumeCache::ShowProperties") );
}

STDMETHODIMP CThumb::Deactivate(
	/* [out] */ __RPC__out DWORD* pdwFlags)
{
	*pdwFlags = 0;
	return S_OK;
}

// IEmptyVolumeCache2

STDMETHODIMP CThumb::InitializeEx(
	/* [in] */ HKEY hkRegKey,
	/* [in] */ LPCWSTR pcwszVolume,
	/* [in] */ LPCWSTR /*pcwszKeyName*/,
	/* [out] */ LPWSTR *ppwszDisplayName,
	/* [out] */ LPWSTR *ppwszDescription,
	/* [out] */ LPWSTR *ppwszBtnText,
	/* [out] */ DWORD *pdwFlags)
{
	if ( ppwszBtnText )
	{
		*ppwszBtnText = NULL;
	}

	return Initialize( hkRegKey, pcwszVolume, ppwszDisplayName, ppwszDescription, pdwFlags );
}
