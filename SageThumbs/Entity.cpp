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
#include "Entity.h"
#include "SQLite.h"

CEntity::CEntity ()
	: m_FileData		()
	, m_ImageInfo		()
	, m_hGflBitmap		( NULL )
	, m_bInfoLoaded		( TRI_UNKNOWN )
{
}

CEntity::~CEntity()
{
	_Module.FreeBitmap( m_hGflBitmap );

	gflFreeFileInformation( &m_ImageInfo );
}

CString CEntity::GetTitleString() const
{
	CString tmp;
	tmp.Format( _T("%d x %d %d bit"),
		m_ImageInfo.Width,
		m_ImageInfo.Height,
		m_ImageInfo.ComponentsPerPixel * m_ImageInfo.BitsPerComponent );
	return CString( (LPCTSTR)CA2T( m_ImageInfo.FormatName ) ) + _T(" ") + tmp;
}

CString CEntity::GetInfoTipString() const
{
	CString sInfoTipString = GetMenuTipString();
	sInfoTipString.Replace (_T(','), _T('\n'));
	return sInfoTipString;
}

CString CEntity::GetMenuTipString() const
{
	CString sMenuTipString;

	const CString type = _Module.m_oLangs.LoadString( IDS_TYPE );
	const CString dim = _Module.m_oLangs.LoadString( IDS_DIM );
	const CString colors = _Module.m_oLangs.LoadString( IDS_COLORS );
	const CString date = _Module.m_oLangs.LoadString( IDS_DATE );
	const CString size = _Module.m_oLangs.LoadString( IDS_SIZE );
	const CString resolution = _Module.m_oLangs.LoadString( IDS_RESOLUTION );
	const CString compression = _Module.m_oLangs.LoadString( IDS_COMPRESSION );

	CString tmp = type;
	tmp += (LPCTSTR)CA2T( m_ImageInfo.Description[ 0 ] ? m_ImageInfo.Description : m_ImageInfo.FormatName );
	sMenuTipString += tmp;

	tmp.Format (_T(", %s%d x %d"), (LPCTSTR)dim, m_ImageInfo.Width, m_ImageInfo.Height);
	sMenuTipString += tmp;

	tmp.Format (_T(", %s%d"), (LPCTSTR)colors, m_ImageInfo.ComponentsPerPixel * m_ImageInfo.BitsPerComponent);
	sMenuTipString += tmp;

	if ( m_ImageInfo.Xdpi )
	{
		tmp.Format (_T(", %s%d dpi"), (LPCTSTR)resolution, m_ImageInfo.Xdpi);
		sMenuTipString += tmp;
	}

	if ( m_ImageInfo.Compression != GFL_NO_COMPRESSION )
	{
		tmp = _T(", ");
		tmp += compression;
		tmp += (LPCTSTR)CA2T (m_ImageInfo.CompressionDescription);
		sMenuTipString += tmp;
	}

	FILETIME ftLastWriteLocal;
	FileTimeToLocalFileTime( &m_FileData.ftLastWriteTime, &ftLastWriteLocal );
	SYSTEMTIME stLastWriteLocal;
	FileTimeToSystemTime( &ftLastWriteLocal, &stLastWriteLocal );
	TCHAR datetime [64];
	GetDateFormat( LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stLastWriteLocal, NULL,
		datetime, _countof( datetime ) );
	GetTimeFormat( LOCALE_USER_DEFAULT, 0, &stLastWriteLocal, NULL,
		datetime + _tcslen (datetime) + 1, _countof( datetime ) );
	datetime [ _tcslen( datetime ) ] = _T(' ');
	tmp = _T(", ");
	tmp += date;
	tmp += datetime;
	sMenuTipString += tmp;

	TCHAR file_size [ 33 ];
	_ui64tot_s( MAKEQWORD( m_FileData.nFileSizeLow,  m_FileData.nFileSizeHigh ), file_size, 33, 10 );
	tmp = _T(", ");
	tmp += size;
	tmp += file_size;
	sMenuTipString += tmp;

	return sMenuTipString;
}

HRESULT CEntity::LoadInfo(const CString& sFilename)
{
	if ( m_bInfoLoaded != TRI_UNKNOWN )
	{
		ATLTRACE( "CEntity - LoadInfo(\"%s\") : %s (already loaded)\n", (LPCSTR)CT2A( sFilename ), ( m_bInfoLoaded == TRI_TRUE ) ? "S_FALSE" : "E_FAIL" );
		return ( m_bInfoLoaded == TRI_TRUE ) ? S_FALSE : E_FAIL;
	}

	CLock oLock( m_pSection );

	if ( m_bInfoLoaded != TRI_UNKNOWN )
	{
		ATLTRACE( "CEntity - LoadInfo(\"%s\") : %s (already loaded)\n", (LPCSTR)CT2A( sFilename ), ( m_bInfoLoaded == TRI_TRUE ) ? "S_FALSE" : "E_FAIL" );
		return ( m_bInfoLoaded == TRI_TRUE ) ? S_FALSE : E_FAIL;
	}

	ZeroMemory( &m_ImageInfo, sizeof( m_ImageInfo ) );

	m_bInfoLoaded = TRI_FALSE;

	if ( ! _Module.IsGoodFile( sFilename ) )
	{
		ATLTRACE( "CEntity - LoadInfo(\"%s\") : E_FAIL (Disabled file)\n", (LPCSTR)CT2A( sFilename ) );
		return E_FAIL;
	}

	if ( ! GetFileAttributesEx( sFilename, GetFileExInfoStandard, &m_FileData ) ||
		( m_FileData.dwFileAttributes & ( FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_OFFLINE ) ) != 0 )
	{
		ATLTRACE( "CEntity - LoadInfo(\"%s\") : E_FAIL (File not found)\n", (LPCSTR)CT2A( sFilename ) );
		return E_FAIL;
	}

	__int64 nPathID = 0;
	CString sName = PathFindFileName( sFilename );
	CString sPath = sFilename.Left( sFilename.GetLength() - sName.GetLength() );
	sName.MakeLower();
	sPath.MakeLower();

	const QWORD nLastWriteTime = MAKEQWORD( m_FileData.ftLastWriteTime.dwLowDateTime, m_FileData.ftLastWriteTime.dwHighDateTime );
	const QWORD nCreationTime = MAKEQWORD( m_FileData.ftCreationTime.dwLowDateTime, m_FileData.ftCreationTime.dwHighDateTime );
	const QWORD nFileSize = MAKEQWORD( m_FileData.nFileSizeLow, m_FileData.nFileSizeHigh );

	// Load image information from database
	{
		CDatabase db( _Module.m_sDatabase );
		if ( db )
		{
			bool result = db.Prepare(
				_T("SELECT e.FileSize, e.LastWriteTime, e.CreationTime, e.ImageInfo, p.PathID ")
				_T("FROM Entities e, Pathes p ")
				_T("WHERE p.Pathname==? AND p.PathID == e.PathID AND e.Filename==?;") );
			if ( ! result )
			{
				db.Exec( RECREATE_DATABASE );
			}
			if ( result &&
				 db.Bind( 1, sPath ) &&
				 db.Bind( 2, sName ) &&
				 db.Step() )
			{
				if ( db.GetCount() == 5 )
				{
					nPathID = db.GetInt64( _T("PathID") );

					// Сличение файла по размеру и дате последней записи
					if ( (QWORD)db.GetInt64( _T("FileSize") ) == nFileSize &&
						 (QWORD)db.GetInt64( _T("LastWriteTime") ) == nLastWriteTime &&
						 (QWORD)db.GetInt64( _T("CreationTime") ) == nCreationTime )
					{
						// Считывание данных о изображении из базы данных
						int nImageInfoSize = 0;
						if ( LPCVOID pImageInfo = db.GetBlob( _T("ImageInfo"), &nImageInfoSize ) )
						{
							if ( nImageInfoSize == sizeof( m_ImageInfo ) )
							{
								CopyMemory( &m_ImageInfo, pImageInfo, sizeof( m_ImageInfo ) );

								if ( m_ImageInfo.FileSize == nFileSize && m_ImageInfo.Width && m_ImageInfo.Height && m_ImageInfo.BitsPerComponent )
								{
									m_bInfoLoaded = TRI_TRUE;
								}

								ATLTRACE( "CEntity - LoadInfo(\"%s\") : %s (from database)\n", (LPCSTR)CT2A( sFilename ), ( m_bInfoLoaded == TRI_TRUE ) ? "S_OK" : "E_FAIL" );
								return ( m_bInfoLoaded == TRI_TRUE ) ? S_OK : E_FAIL;
							}
						}
					}
					else
					{
						ATLTRACE( "CEntity - LoadInfo(\"%s\") : File changed!\n", (LPCSTR)CT2A( sFilename ) );
					}
				}
				else
				{
					ATLTRACE( "CEntity - LoadInfo(\"%s\") : Not in database!\n", (LPCSTR)CT2A( sFilename ) );
				}
			}
			else
			{
				ATLTRACE( "CEntity - LoadInfo(\"%s\") : \"SELECT\" SQL Error: \"%s\"\n", (LPCSTR)CT2A( sFilename ), (LPCSTR)CT2A( db.GetLastErrorMessage() ) );
			}
		}
	}

	// Считывание данных об изображении из файла
	if ( SUCCEEDED( _Module.GetFileInformation( sFilename, &m_ImageInfo ) ) )
	{
		m_bInfoLoaded = TRI_TRUE;
	}

	// Вставка новых данных о файле
	CDatabase db( _Module.m_sDatabase );
	if ( db )
	{
		bool result = db.Prepare( _T("SELECT PathID FROM Pathes WHERE Pathname==?;") ) &&
			 db.Bind( 1, sPath ) &&
			 db.Step() &&
			 db.GetCount() == 1 &&
			 ( nPathID = db.GetInt64( _T("PathID") ) ) != 0;
		if ( ! result )
		{
			db.Prepare( _T("INSERT OR IGNORE INTO Pathes ( Pathname ) VALUES ( ? );") ) &&
				db.Bind( 1, sPath ) &&
				db.Step();
			result = db.Prepare( _T("SELECT PathID FROM Pathes WHERE Pathname==?;") ) &&
				db.Bind( 1, sPath ) &&
				db.Step() &&
				db.GetCount() == 1 &&
				( nPathID = db.GetInt64( _T("PathID") ) ) != 0;
		}
		if ( result )
		{
			if ( db.Prepare( _T("INSERT OR REPLACE INTO Entities ( PathID, Filename, LastWriteTime, CreationTime, FileSize, ImageInfo, Image, Width, Height ) VALUES ( ?, ?, ?, ?, ?, ?, NULL, 0, 0 );") ) &&
				 db.Bind( 1, nPathID ) &&
				 db.Bind( 2, sName ) &&
				 db.Bind( 3, (__int64)nLastWriteTime ) &&
				 db.Bind( 4, (__int64)nCreationTime ) &&
				 db.Bind( 5, (__int64)nFileSize ) &&
				 db.Bind( 6, &m_ImageInfo, sizeof( m_ImageInfo ) ) &&
				 db.Step() )
			{
				// Ok
				ATLTRACE( "CEntity - LoadInfo(\"%s\") : Saved to database!\n", (LPCSTR)CT2A( sFilename ) );
			}
			else
			{
				ATLTRACE( "CEntity - LoadInfo(\"%s\") : \"INSERT\" SQL Error: \"%s\"\n", (LPCSTR)CT2A( sFilename ), (LPCSTR)CT2A( db.GetLastErrorMessage() ) );
			}
		}
		else
		{
			ATLTRACE( "CEntity - LoadInfo(\"%s\") : \"SELECT\" SQL Error: \"%s\"\n", (LPCSTR)CT2A( sFilename ), (LPCSTR)CT2A( db.GetLastErrorMessage() ) );
		}
	}

	ATLTRACE( "CEntity - LoadInfo(\"%s\") : %s (from disk)\n", (LPCSTR)CT2A( sFilename ), ( m_bInfoLoaded == TRI_TRUE ) ? "S_OK" : "E_FAIL" );
	return ( m_bInfoLoaded == TRI_TRUE ) ? S_OK : E_FAIL;
}

HRESULT CEntity::LoadImage(const CString& sFilename, UINT cx, UINT cy)
{
	CLock oLock( m_pSection );

	if ( m_hGflBitmap &&
		 m_hGflBitmap->Width  >= (int)cx &&
		 m_hGflBitmap->Height >= (int)cy )
	{
		ATLTRACE( "CEntity - LoadImage(\"%s\",%u,%u) : S_FALSE (already loaded)\n", (LPCSTR)CT2A( sFilename ), cx, cy );
		return S_FALSE;
	}

	_Module.FreeBitmap( m_hGflBitmap );

	HRESULT hr = LoadInfo( sFilename );
	if ( FAILED( hr ) )
		return hr;

	const QWORD max_size = GetRegValue( _T("MaxSize"), FILE_MAX_SIZE );
	if ( MAKEQWORD( m_FileData.nFileSizeLow, m_FileData.nFileSizeHigh ) > max_size * 1024 * 1024 )
	{
		// Too big file
		ATLTRACE( "CEntity - LoadImage(\"%s\",%u,%u) : E_FAIL (Bitmap too big)\n", (LPCSTR)CT2A( sFilename ), cx, cy );
		return E_FAIL;
	}

	UINT dx, dy;
	CalcSize( dx, dy,
		max( cx, max( GetRegValue( _T("Width"),  THUMB_STORE_SIZE ), THUMB_STORE_SIZE ) ),
		max( cy, max( GetRegValue( _T("Height"), THUMB_STORE_SIZE ), THUMB_STORE_SIZE ) ) );

	__int64 nPathID = 0;
	CString sName = PathFindFileName( sFilename );
	CString sPath = sFilename.Left( sFilename.GetLength() - sName.GetLength() );
	sName.MakeLower();
	sPath.MakeLower();

	{
		// Выбор нужного изображения из базы по размерам
		CDatabase db( _Module.m_sDatabase );
		if ( db )
		{
			if ( db.Prepare( _T("SELECT e.Image, e.Width, e.Height, p.PathID ")
							 _T("FROM Entities e, Pathes p ")
							 _T("WHERE p.Pathname==? AND p.PathID == e.PathID AND e.Filename==?;") ) &&
				 db.Bind( 1, sPath ) &&
				 db.Bind( 2, sName ) &&
				 db.Step() )
			{
				if ( db.GetCount() == 4 && ( nPathID = db.GetInt64( _T("PathID") ) ) != 0 )
				{
					// Проверка размерности изображения
					int db_cx = db.GetInt32( _T("Width") );
					int db_cy = db.GetInt32( _T("Height") );
					if ( ( db_cx >= (int)dx ) ||
						 ( db_cy >= (int)dy ) ||
						 ( m_ImageInfo.Width == db_cx && m_ImageInfo.Height <= db_cy ) ||
						 ( m_ImageInfo.Width <= db_cx && m_ImageInfo.Height == db_cy ) )
					{
						// Загрузка изображения из базы данных
						int nImageSize = 0;
						if ( LPCVOID pBlob = db.GetBlob( _T("Image"), &nImageSize ) )
						{
							if ( SUCCEEDED( _Module.LoadBitmapFromMemory( pBlob, nImageSize, &m_hGflBitmap ) ) && m_hGflBitmap )
							{
								ATLTRACE( "CEntity - LoadImage(\"%s\",%u,%u) : S_OK (from database %dx%d)\n", (LPCSTR)CT2A( sFilename ), cx, cy, m_hGflBitmap->Width, m_hGflBitmap->Height );
								return S_OK;
							}
						}
					}
					else if ( db_cx || db_cy )
					{
						// Слишком маленькое изображение - перезагрузка из файла
						ATLTRACE( "CEntity - LoadImage(\"%s\",%u,%u) : Reload, needed %dx%d but %dx%d in database\n", (LPCSTR)CT2A( sFilename ), cx, cy, (int)dx, (int)dy, db_cx, db_cy );
					}
				}
				else
				{
					ATLTRACE( "CEntity - LoadImage(\"%s\",%u,%u) : Not in database!\n", (LPCSTR)CT2A( sFilename ), cx, cy );
				}
			}
			else
			{
				ATLTRACE( "CEntity - LoadImage(\"%s\",%u,%u) : \"SELECT\" SQL Error: \"%s\"\n", (LPCSTR)CT2A( sFilename ), cx, cy, (LPCSTR)CT2A( db.GetLastErrorMessage() ) );
			}
		}
	}

	// Загрузка из файла
	if ( FAILED( _Module.LoadThumbnail( sFilename, (int)dx, (int)dy, &m_hGflBitmap ) ) || ! m_hGflBitmap )
	{
		ATLTRACE( "CEntity - LoadImage(\"%s\",%u,%u) : E_FAIL\n", (LPCSTR)CT2A( sFilename ), cx, cy );
		return E_FAIL;
	}

	// Save thumbnail image to database using best format
	GFL_SAVE_PARAMS params = {};
	gflGetDefaultSaveParams( &params );
	params.Flags = GFL_SAVE_ANYWAY;
	params.CompressionLevel = THUMB_STORE_PNG_RATIO;
	params.Quality = THUMB_STORE_JPG_RATIO;
	params.OptimizeHuffmanTable = GFL_TRUE;
	if ( m_ImageInfo.ComponentsPerPixel > 3 )
	{
		// Using PNG for images with alpha
		params.FormatIndex = gflGetFormatIndexByName( "png" );
	}
	else
	{
		// Using JPEG for rest
		params.FormatIndex = gflGetFormatIndexByName( "jpeg" );
	}
	BYTE* data = NULL;
	UINT data_length = 0;
	GFL_ERROR err = gflSaveBitmapIntoMemory( &data, &data_length, m_hGflBitmap, &params );
	if ( err == GFL_NO_ERROR )
	{
		if ( nPathID )
		{
			CDatabase db( _Module.m_sDatabase );
			if ( db )
			{
				if ( db.Prepare( _T("UPDATE Entities SET Image=?, Width=?, Height=? WHERE Filename==? AND PathID==?;") ) &&
					 db.Bind( 1, data, data_length ) &&
					 db.Bind( 2, m_hGflBitmap->Width ) &&
					 db.Bind( 3, m_hGflBitmap->Height ) &&
					 db.Bind( 4, sName ) &&
					 db.Bind( 5, nPathID ) &&
					 db.Step() )
				{
					// Ok
					ATLTRACE( "CEntity - LoadImage(\"%s\") : Saved to database!\n", (LPCSTR)CT2A( sFilename ) );
				}
				else
				{
					ATLTRACE( "CEntity - LoadImage(\"%s\",%u,%u) : \"UPDATE\" SQL Error: \"%s\"\n", (LPCSTR)CT2A( sFilename ), cx, cy, (LPCSTR)CT2A( db.GetLastErrorMessage() ) );
				}
			}
		}
		else
		{
			ATLTRACE ( "CEntity - LoadImage(\"%s\",%u,%u) : Bad PathID\n", (LPCSTR)CT2A( sFilename ), cx, cy );
		}
		gflMemoryFree( data );
	}
	else
	{
		ATLTRACE( "CEntity - LoadImage(\"%s\",%u,%u) : gflSaveBitmapIntoMemory failed : %s\n", (LPCSTR)CT2A( sFilename ), cx, cy, gflGetErrorString( err ) );
	}

	ATLTRACE( "CEntity - LoadImage(\"%s\",%u,%u) : S_OK (from disk %dx%d)\n", (LPCSTR)CT2A( sFilename ), cx, cy, m_hGflBitmap->Width, m_hGflBitmap->Height );
	return S_OK;
}

#ifdef ISTREAM_ENABLED

HRESULT CEntity::LoadInfo(IStream* pStream)
{
	if ( m_bInfoLoaded != TRI_UNKNOWN )
	{
		ATLTRACE( "CEntity - LoadInfo(stream) : %s (already loaded)\n", ( m_bInfoLoaded == TRI_TRUE ) ? "S_FALSE" : "E_FAIL" );
		return ( m_bInfoLoaded == TRI_TRUE ) ? S_FALSE : E_FAIL;
	}

	CLock oLock( m_pSection );

	if ( m_bInfoLoaded != TRI_UNKNOWN )
	{
		ATLTRACE( "CEntity - LoadInfo(stream) : %s (already loaded)\n", ( m_bInfoLoaded == TRI_TRUE ) ? "S_FALSE" : "E_FAIL" );
		return ( m_bInfoLoaded == TRI_TRUE ) ? S_FALSE : E_FAIL;
	}

	m_bInfoLoaded = TRI_FALSE;

	// Load file information from stream
	STATSTG stat = {};
	if ( FAILED( pStream->Stat( &stat,  STATFLAG_DEFAULT ) ) )
	{
		ATLTRACE( "CEntity - LoadInfo(stream) : E_FAIL (Stat failed)\n" );
		return E_FAIL;
	}
	
	m_FileData.ftCreationTime = stat.ctime;
	m_FileData.ftLastAccessTime = stat.atime;
	m_FileData.ftLastWriteTime = stat.mtime;
	m_FileData.nFileSizeHigh = stat.cbSize.HighPart;
	m_FileData.nFileSizeLow = stat.cbSize.LowPart;

	if ( stat.pwcsName )
	{
		wcscpy_s( m_FileData.cFileName, stat.pwcsName );
		CoTaskMemFree( stat.pwcsName );
		CharLowerBuff( m_FileData.cFileName, (DWORD)wcslen( m_FileData.cFileName ) );

		if ( ! _Module.IsGoodFile( m_FileData.cFileName ) )
		{
			ATLTRACE( "CEntity - LoadInfo(stream) : E_FAIL (Disabled file)\n" );
			return E_FAIL;
		}
	}

	if ( FAILED( _Module.GetFileInformation( pStream, &m_ImageInfo ) ) )
	{
		ZeroMemory( &m_ImageInfo, sizeof( m_ImageInfo ) );
		return E_FAIL;
	}

	m_bInfoLoaded = TRI_TRUE;

	ATLTRACE( "CEntity - LoadInfo(stream) : S_OK\n" );
	return S_OK;
}

HRESULT CEntity::LoadImage(IStream* pStream, UINT cx, UINT cy)
{
	CLock oLock( m_pSection );

	if ( m_hGflBitmap &&
		 m_hGflBitmap->Width  >= (int)cx &&
		 m_hGflBitmap->Height >= (int)cy )
	{
		ATLTRACE( "CEntity - LoadImage(stream,%u,%u) : S_FALSE (already loaded)\n", cx, cy );
		return S_FALSE;
	}

	_Module.FreeBitmap( m_hGflBitmap );

	HRESULT hr = LoadInfo( pStream );
	if ( FAILED( hr ) )
		return hr;
	
	QWORD max_size = GetRegValue( _T("MaxSize"), FILE_MAX_SIZE );
	if ( MAKEQWORD( m_FileData.nFileSizeLow, m_FileData.nFileSizeHigh ) > max_size * 1024 * 1024 )
	{
		// Too big file
		ATLTRACE( "CEntity - LoadImage(stream,%u,%u) : E_FAIL (Bitmap too big)\n", cx, cy );
		return E_FAIL;
	}

	UINT dx, dy;
	CalcSize( dx, dy,
		max( cx, max( GetRegValue( _T("Width"),  THUMB_STORE_SIZE ), THUMB_STORE_SIZE ) ),
		max( cy, max( GetRegValue( _T("Height"), THUMB_STORE_SIZE ), THUMB_STORE_SIZE ) ) );

	// Загрузка из файла
	if ( FAILED( _Module.LoadThumbnail( pStream, (int)dx, (int)dy, &m_hGflBitmap ) ) || ! m_hGflBitmap )
	{
		ATLTRACE( "CEntity - LoadImage(stream,%u,%u) : E_FAIL\n", cx, cy );
		return E_FAIL;
	}

	ATLTRACE( "CEntity - LoadImage(stream,%u,%u) : S_OK (%dx%d)\n", cx, cy, m_hGflBitmap->Width, m_hGflBitmap->Height );
	return S_OK;
}

#endif // ISTREAM_ENABLED

HBITMAP CEntity::GetImage(UINT cx, UINT cy, COLORREF nColor1, COLORREF nColor2)
{
	CLock oLock( m_pSection );

	if ( ! m_hGflBitmap )
		return NULL;

	GFL_BITMAP* pResizedBitmap = m_hGflBitmap;

	// Reduce too big image
	UINT dx, dy;
	CalcSize( dx, dy, cx, cy );
	if ( FAILED( _Module.Resize( m_hGflBitmap, &pResizedBitmap, dx, dy ) ) || ! pResizedBitmap )
	{
		// Use original
		pResizedBitmap = m_hGflBitmap;
	}

	HBITMAP hBitmap = NULL;
	if ( FAILED( _Module.ConvertBitmap( pResizedBitmap, &hBitmap ) ) )
		return NULL;

	if ( pResizedBitmap != m_hGflBitmap )
		_Module.FreeBitmap( pResizedBitmap );

	BITMAP bm = {};
	GetObject( hBitmap, sizeof( BITMAP ), &bm );
	ATLTRACE( "CEntity - GetImage(%ux%u) : loaded %dx%d, %d bits/pixel, %d planes\n", cx, cy, bm.bmWidth, bm.bmHeight, bm.bmBitsPixel, bm.bmPlanes );

	HBITMAP hResult = NULL;
	HWND hWnd = GetDesktopWindow();
	if ( HDC hDC = GetDC( hWnd ) )
	{
		if ( HDC hBitmapDC = CreateCompatibleDC( hDC ) )
		{
			HBITMAP hOldBitmap = (HBITMAP)SelectObject( hBitmapDC, hBitmap );
			if ( HDC hResultDC = CreateCompatibleDC( hDC ) )
			{
				hResult = CreateCompatibleBitmap( hDC, dx, dy );
				if ( hResult )
				{
					HBITMAP hOldResult = (HBITMAP)SelectObject( hResultDC, hResult );

					UINT step = 2;
					const UINT size = max( dx, dy );
					while ( size > step * 16 )
					{
						step *= 2;
					}

					HBRUSH hBrush1 = CreateSolidBrush( nColor1 );
					HBRUSH hBrush2 = CreateSolidBrush( nColor2 );
					for ( UINT y = 0; y < dy; y += step )
					{
						for ( UINT x = 0; x < dx; x += step )
						{
							const RECT rc = { x, y, min( x + step, dx ), min( y + step, dy ) };
							FillRect( hResultDC, &rc, ( ( x / step + ( y / step ) & 1 ) & 1 ) ? hBrush1 : hBrush2 );
						}
					}
					DeleteObject( hBrush2 );
					DeleteObject( hBrush1 );

					// pre-multiply rgb channels with alpha channel
					if ( bm.bmBits && bm.bmPlanes == 1 && bm.bmBitsPixel == 32 )
					{
						for ( int y = 0; y < bm.bmHeight; ++y )
						{
							BYTE* pPixel = (BYTE*)bm.bmBits + bm.bmWidth * 4 * y;
							for ( int x = 0; x < bm.bmWidth; ++x )
							{
								pPixel[0] = pPixel[0] * pPixel[3] / 255;
								pPixel[1] = pPixel[1] * pPixel[3] / 255;
								pPixel[2] = pPixel[2] * pPixel[3] / 255;
								pPixel += 4;
							}
						}
					}

					BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
					ATLVERIFY( AlphaBlend( hResultDC, ( dx - bm.bmWidth ) / 2, ( dy - bm.bmHeight ) / 2, bm.bmWidth, bm.bmHeight,
						hBitmapDC, 0, 0, bm.bmWidth, bm.bmHeight, bf ) );

					SelectObject( hResultDC, hOldResult );
				}		
				DeleteDC( hResultDC );
			}
			SelectObject( hBitmapDC, hOldBitmap );
			DeleteDC( hBitmapDC );
		}
		ReleaseDC( hWnd, hDC );
	}

	if ( hResult )
	{
		DeleteObject( hBitmap );
		hBitmap = hResult;
	}

	return hBitmap;
}

HICON CEntity::GetIcon(UINT cx)
{
	HICON hIcon = NULL;
	if ( HBITMAP hBitmap = GetImage( cx, cx ) )
	{
		BITMAP bm = {};
		GetObject( hBitmap, sizeof( BITMAP ), &bm );

		const RECT rcAll =
		{
			0,
			0,
			cx,
			cx
		};
		const LONG nx = ( cx - bm.bmWidth ) / 2;
		const LONG ny = ( cx - bm.bmHeight ) / 2;
		const RECT rcIcon =
		{
			nx,
			ny,
			nx + bm.bmWidth,
			ny + bm.bmHeight
		};

		HWND hWnd = GetDesktopWindow();
		if ( HDC hDC = GetDC( hWnd ) )
		{
			// Create mask
			if ( HDC hMaskDC = CreateCompatibleDC( hDC ) )
			{
				if ( HBITMAP hbmMask = CreateCompatibleBitmap( hDC, cx, cx ) )
				{
					HBITMAP hOldMask = (HBITMAP)SelectObject( hMaskDC, hbmMask );

					FillRect( hMaskDC, &rcAll, (HBRUSH)GetStockObject( WHITE_BRUSH ) );
					FillRect( hMaskDC, &rcIcon, (HBRUSH)GetStockObject( BLACK_BRUSH ) );

					ICONINFO ii =
					{
						TRUE,
						0,
						0,
						hbmMask,
						hBitmap
					};
					hIcon = CreateIconIndirect( &ii );

					SelectObject( hMaskDC, hOldMask );
					DeleteObject( hbmMask );
				}
				DeleteObject( hMaskDC );			
			}
			ReleaseDC( hWnd, hDC );
		}
		DeleteObject( hBitmap );
	}
	return hIcon;
}

void CEntity::CalcSize(UINT& tx, UINT& ty, UINT width, UINT height)
{
	UINT w = (UINT)m_ImageInfo.Width;
	UINT h = (UINT)m_ImageInfo.Height;
	if ( w < width && h < height )
	{
		tx = w;
		ty = h;
	}
	else
	{
		tx = width;
		ty = height;
		if ( w && h && width && height )
		{
			UINT a = w * height;
			UINT b = h * width;
			if ( a < b )
				tx = a / h;
			else
				ty = b / w;
		}
	}
	if ( ! tx ) tx = 1;
	if ( ! ty ) ty = 1;
}