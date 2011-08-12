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

#include "stdafx.h"
#include <InitGuid.h>

DEFINE_GUID(CLSID_WindowsThumbnailer,0x889900c3,0x59f3,0x4c2f,0xae,0x21,0xa4,0x09,0xea,0x01,0xe6,0x05);

CString GetSpecialFolderPath(int csidl)
{
	CString buf;

	// Avoid SHGetSpecialFolderPath()
	PIDLIST_ABSOLUTE pidl = NULL;
	if ( SUCCEEDED( SHGetSpecialFolderLocation( GetDesktopWindow (), csidl, &pidl ) ) )
	{
		SHGetPathFromIDList( pidl, buf.GetBuffer( MAX_LONG_PATH ) );
		buf.ReleaseBuffer();

		CoTaskMemFree( pidl );
	}
	
	return buf;
}

BOOL IsProcessElevated()
{
	HANDLE hToken = NULL;
	if ( ! OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ) )
	{
		ATLTRACE( "OpenProcessToken error: %d\n", GetLastError() );
		return FALSE;
	}

	TOKEN_ELEVATION elevation = {};
	DWORD dwSize = 0;
	if ( ! GetTokenInformation( hToken, TokenElevation, &elevation, 
		sizeof( elevation ), &dwSize ) )
	{
		ATLTRACE( "GetTokenInformation error: %d\n", GetLastError() );
		CloseHandle( hToken );
		return FALSE;
	}

	CloseHandle( hToken );
	return ( elevation.TokenIsElevated != FALSE );
}

void CleanWindowsCache()
{
	CComPtr< IEmptyVolumeCache > pWindowsThumbnailer;
	HRESULT hr = pWindowsThumbnailer.CoCreateInstance( CLSID_WindowsThumbnailer );
	if ( SUCCEEDED( hr ) )
	{
		pWindowsThumbnailer->Purge( 0, NULL );
	}	
}

BOOL LoadIcon(LPCTSTR szFilename, HICON* phSmallIcon, HICON* phLargeIcon, HICON* phHugeIcon, int nIcon)
{
	CString strIcon( szFilename );

	if ( phSmallIcon )
		*phSmallIcon = NULL;
	if ( phLargeIcon )
		*phLargeIcon = NULL;
	if ( phHugeIcon )
		*phHugeIcon = NULL;

	int nIndex = strIcon.ReverseFind( _T(',') );
	if ( nIndex != -1 )
	{
		if ( _stscanf_s( strIcon.Mid( nIndex + 1 ), _T("%i"), &nIcon ) == 1 )
		{
			strIcon = strIcon.Left( nIndex );
		}
	}
	else
		nIndex = 0;

	if ( strIcon.GetLength() < 3 )
		return FALSE;

	if ( strIcon.GetAt( 0 ) == _T('\"') &&
		strIcon.GetAt( strIcon.GetLength() - 1 ) == _T('\"') )
		strIcon = strIcon.Mid( 1, strIcon.GetLength() - 2 );

	if ( phLargeIcon || phSmallIcon )
	{
		ExtractIconEx( strIcon, nIcon, phLargeIcon, phSmallIcon, 1 );
	}

	if ( phHugeIcon )
	{
		if ( HMODULE hUser32 = LoadLibrary( _T("user32.dll") ) )
		{
			tPrivateExtractIconsT pPrivateExtractIconsT = (tPrivateExtractIconsT)GetProcAddress( hUser32,
#if UNICODE
				"PrivateExtractIconsW"
#else
				"PrivateExtractIconsA"
#endif
				);
			if ( pPrivateExtractIconsT )
			{
				UINT nLoadedID;
				pPrivateExtractIconsT( strIcon, nIcon, 48, 48, phHugeIcon, &nLoadedID, 1, 0 );
			}
			FreeLibrary( hUser32 );
		}
	}

	return ( phLargeIcon && *phLargeIcon ) || ( phSmallIcon && *phSmallIcon ) ||
		( phHugeIcon && *phHugeIcon );
}

// CRC 32

static DWORD crc32_table[ 256 ];

class CCRC32
{
public:
	inline CCRC32()
	{
		const DWORD CRC32_POLY = 0x04c11db7;	// AUTODIN II, Ethernet, & FDDI
		for (int i = 0; i < 256; ++i) {
			DWORD c = i << 24;
			for (int j = 8; j > 0; --j)
				c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
			crc32_table[i] = c;
		}
	}

};

static CCRC32 _CRC32;

DWORD CRC32(const char *buf, int len)
{
	DWORD crc = 0xffffffff;
	for (const BYTE *p = (const BYTE *) buf; len > 0; ++p, --len)
		crc = (crc << 8) ^ crc32_table[(crc >> 24) ^ *p];
	return ~crc;
}
