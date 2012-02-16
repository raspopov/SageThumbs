/*
SageThumbs - Thumbnail image shell extension.

Copyright (C) Nikolay Raspopov, 2004-2012.

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
	, m_bDatabaseUsed	( false )
	, m_bInfoLoaded		( false )
{
}

CEntity::~CEntity()
{
	_Module.FreeBitmap( m_hGflBitmap );

	gflFreeFileInformation( &m_ImageInfo );
}

HRESULT CEntity::LoadInfo(const CString& sFilename)
{
	if ( m_bInfoLoaded )
		return S_OK;

	CLock oLock( m_pSection );

	if ( m_bInfoLoaded )
		return S_OK;

	Ext data;
	if ( ! _Module.IsGoodFile( sFilename, &data, &m_FileData ) )
	{
		ATLTRACE( "CEntity::LoadInfo() : E_FAIL (Bad File)\n" );
		return E_FAIL;
	}

	ATLTRACE( "CEntity::LoadInfo(\"%s\") : It's \"%s\". Load info ", (LPCSTR)CT2A( sFilename ), (LPCSTR)CT2A( data.info ) );

	__int64 nPathID = 0;
	CString sName = PathFindFileName( sFilename );
	CString sPath = sFilename.Left( sFilename.GetLength() - sName.GetLength() );
	sName.MakeLower();
	sPath.MakeLower();

	QWORD nLastWriteTime = MAKEQWORD( m_FileData.ftLastWriteTime.dwLowDateTime, m_FileData.ftLastWriteTime.dwHighDateTime );
	QWORD nCreationTime = MAKEQWORD( m_FileData.ftCreationTime.dwLowDateTime, m_FileData.ftCreationTime.dwHighDateTime );
	QWORD nFileSize = MAKEQWORD( m_FileData.nFileSizeLow, m_FileData.nFileSizeHigh );

	// Load image information from database
	bool bFound = false, bNew = false;
	CDatabase db( _Module.m_sDatabase );
	if ( db )
	{
		bool result = db.Prepare( _T("SELECT PathID FROM Pathes WHERE Pathname==?;") );
		if ( ! result )
		{
			db.Exec( RECREATE_DATABASE );
		}

		// Получение папки
		if ( result &&
			 db.Bind( 1, sPath ) &&
			 db.Step() &&
			 db.GetCount() == 1 )
		{
			nPathID = db.GetInt64( _T("PathID") );

			if ( db.Prepare( _T("SELECT FileSize, LastWriteTime, CreationTime, ImageInfo FROM Entities WHERE Filename==? AND PathID==?;") ) &&
				 db.Bind( 1, sName ) &&
				 db.Bind( 2, nPathID ) &&
				 db.Step() &&
				 db.GetCount() == 4 )
			{
				// Сличение файла по размеру и дате последней записи
				if ( (QWORD)db.GetInt64( _T("FileSize") ) == nFileSize &&
					 (QWORD)db.GetInt64( _T("LastWriteTime") ) == nLastWriteTime &&
					 (QWORD)db.GetInt64( _T("CreationTime") ) == nCreationTime )
				{
					// Считывание данных о изображении из базы данных
					ATLTRACE( "from database " );
					int nImageInfoSize = 0;
					if ( LPCVOID pImageInfo = db.GetBlob( _T("ImageInfo"), &nImageInfoSize ) )
					{
						if ( nImageInfoSize == sizeof( m_ImageInfo ) )
						{
							CopyMemory( &m_ImageInfo, pImageInfo, sizeof( m_ImageInfo ) );

							// Validate image information
							bFound =
								m_ImageInfo.FileSize == nFileSize &&
								m_ImageInfo.Width &&
								m_ImageInfo.Height &&
								m_ImageInfo.ComponentsPerPixel &&
								m_ImageInfo.BitsPerComponent &&
								m_ImageInfo.FormatName[ 0 ];
						}
					}
				}
				else
				{
					ATLTRACE( "(file changed!) " );
				}
			}
			else
				bNew = true;
		}
		else
			bNew = true;
	}

	if ( bFound )
	{
		m_bDatabaseUsed = true;
	}
	else
	{
		// Удаление устаревших данных о файле
		if ( db && ! bNew )
		{
			db.Prepare( _T("DELETE FROM Entities WHERE Filename==? AND PathID==?;") ) &&
			db.Bind( 1, sName ) &&
			db.Bind( 2, nPathID ) &&
			db.Step();
		}

		// Считывание данных об изображении из файла
		ATLTRACE( "from disk " );
		if ( FAILED( _Module.GetFileInformation( sFilename, &m_ImageInfo ) ) )
		{
			ZeroMemory( &m_ImageInfo, sizeof( m_ImageInfo ) );
			return E_FAIL;
		}

		// Вставка новых данных о файле
		if ( db )
		{
			db.Prepare( _T("INSERT INTO Pathes ( Pathname ) VALUES ( ? );") ) &&
			db.Bind( 1, sPath ) &&
			db.Step();

			if ( db.Prepare( _T("SELECT PathID FROM Pathes WHERE Pathname==?;") ) &&
				 db.Bind( 1, sPath ) &&
				 db.Step() &&
				 db.GetCount() == 1 &&
				 ( nPathID = db.GetInt64( _T("PathID") ) ) != 0 )
			{
				db.Prepare( _T("INSERT INTO Entities ( PathID, Filename, LastWriteTime, CreationTime, FileSize, ImageInfo ) VALUES ( ?, ?, ?, ?, ?, ? );") ) &&
				db.Bind( 1, nPathID ) &&
				db.Bind( 2, sName ) &&
				db.Bind( 3, (__int64)nLastWriteTime ) &&
				db.Bind( 4, (__int64)nCreationTime ) &&
				db.Bind( 5, (__int64)nFileSize ) &&
				db.Bind( 6, &m_ImageInfo, sizeof( m_ImageInfo ) ) &&
				db.Step();
			}
		}
	}

	m_bInfoLoaded = true;

	ATLTRACE( "S_OK.\n" );
	return S_OK;
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

	CString type = _Module.m_oLangs.LoadString( IDS_TYPE );
	CString dim = _Module.m_oLangs.LoadString( IDS_DIM );
	CString colors = _Module.m_oLangs.LoadString( IDS_COLORS );
	CString date = _Module.m_oLangs.LoadString( IDS_DATE );
	CString size = _Module.m_oLangs.LoadString( IDS_SIZE );
	CString resolution = _Module.m_oLangs.LoadString( IDS_RESOLUTION );
	CString compression = _Module.m_oLangs.LoadString( IDS_COMPRESSION );

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

HRESULT CEntity::LoadImage(const CString& sFilename, UINT cx, UINT cy)
{
	CLock oLock( m_pSection );

	if ( m_hGflBitmap &&
		 m_hGflBitmap->Width  >= (int)cx &&
		 m_hGflBitmap->Height >= (int)cy )
	{
		ATLTRACE( "CEntity::LoadImage(\"%s\",%d,%d) : S_FALSE (Bitmap already loaded)\n", (LPCSTR)CT2A( sFilename ), cx, cy );
		return S_FALSE;
	}

	_Module.FreeBitmap( m_hGflBitmap );

	HRESULT hr = LoadInfo( sFilename );
	if ( FAILED( hr ) )
		return hr;

	QWORD max_size = GetRegValue( _T("MaxSize"), FILE_MAX_SIZE );
	if ( MAKEQWORD( m_FileData.nFileSizeLow, m_FileData.nFileSizeHigh ) > max_size * 1024 * 1024 )
	{
		// Too big file
		ATLTRACE( "CEntity::LoadImage(\"%s\",%d,%d) : E_FAIL (Bitmap too big)\n", (LPCSTR)CT2A( sFilename ), cx, cy );
		return E_FAIL;
	}

	ATLTRACE( "CEntity::LoadImage(\"%s\",%d,%d) : Load image ", (LPCSTR)CT2A( sFilename ), cx, cy );

	__int64 nPathID = 0;
	CString sName = PathFindFileName( sFilename );
	CString sPath = sFilename.Left( sFilename.GetLength() - sName.GetLength() );
	sName.MakeLower();
	sPath.MakeLower();

	// Выбор нужного изображения из базы по размерам
	CDatabase db( _Module.m_sDatabase );
	if ( db )
	{
		if ( db.Prepare( _T("SELECT PathID FROM Pathes WHERE Pathname==?;") ) &&
			 db.Bind( 1, sPath ) &&
			 db.Step() &&
			 db.GetCount() == 1 &&
			 ( nPathID = db.GetInt64( _T("PathID") ) ) != 0 )
		{
			if ( db.Prepare( _T("SELECT Image, Width, Height FROM Entities WHERE Filename==? AND PathID==?;") ) &&
				 db.Bind( 1, sName ) &&
				 db.Bind( 2, nPathID ) &&
				 db.Step() &&
				 db.GetCount() == 3 )
			{
				// Проверка размерности изображения
				int dx = db.GetInt32( _T("Width") );
				int dy = db.GetInt32( _T("Height") );
				if ( ( dx >= (int)cx ) ||
					 ( dy >= (int)cy ) ||
					 ( m_ImageInfo.Width == dx && m_ImageInfo.Height <= dy ) ||
					 ( m_ImageInfo.Width <= dx && m_ImageInfo.Height == dy ) )
				{
					// Загрузка изображения из базы данных
					int nImageSize;
					if ( LPCVOID pImage = db.GetBlob( _T("Image"), &nImageSize ) )
					{
						_Module.LoadBitmapFromMemory( pImage, nImageSize, &m_hGflBitmap );
						ATLTRACE( "from database (%d bytes %dx%d) ", nImageSize, m_hGflBitmap->Width, m_hGflBitmap->Height );
					}
				}
				else
				{
					// Слишком маленькое изображение - перезагрузка из файла
					ATLTRACE( "reload (needed %dx%d but %dx%d in database) ", (int)cx, (int)cy, dx, dy );
				}
			}
		}
	}

	if ( ! m_hGflBitmap )
	{
		cx = max( cx, max( GetRegValue( _T("Width"), THUMB_STORE_SIZE ), THUMB_STORE_SIZE ) );
		cy = max( cy, max( GetRegValue( _T("Height"), THUMB_STORE_SIZE ), THUMB_STORE_SIZE ) );

		// Загрузка из файла
		_Module.LoadThumbnail( sFilename, (int)cx, (int)cy, &m_hGflBitmap );
		if ( ! m_hGflBitmap )
		{
			ATLTRACE( "E_FAIL\n" );
			return E_FAIL;
		}
		ATLTRACE( "from disk (%dx%d) ", m_hGflBitmap->Width, m_hGflBitmap->Height );

		// Save thumbnail image to database using best format
		GFL_SAVE_PARAMS params;
		gflGetDefaultSaveParams( &params );
		params.Flags = GFL_SAVE_ANYWAY;
		if ( m_ImageInfo.ComponentsPerPixel > 3 )
		{
			// Using PNG for images with alpha
			params.CompressionLevel = 6;
			params.FormatIndex = gflGetFormatIndexByName( "png" );
			ATLTRACE( "as PNG " );
		}
		else
		{
			// Using JPEG for rest
			params.Quality = 80;
			params.OptimizeHuffmanTable = GFL_TRUE;
			params.FormatIndex = gflGetFormatIndexByName( "jpeg" );
			ATLTRACE( "as JPEG " );
		}
		BYTE* data = NULL;
		UINT data_length = 0;
		GFL_ERROR err = gflSaveBitmapIntoMemory( &data, &data_length, m_hGflBitmap, &params );
		if ( err == GFL_NO_ERROR )
		{
			ATLTRACE( "to database (%u bytes) ", data_length );

			if ( nPathID )
			{
				db.Prepare( _T("UPDATE Entities SET Image=?, Width=?, Height=? WHERE Filename==? AND PathID==?;") ) &&
				db.Bind( 1, data, data_length ) &&
				db.Bind( 2, m_hGflBitmap->Width ) &&
				db.Bind( 3, m_hGflBitmap->Height ) &&
				db.Bind( 4, sName ) &&
				db.Bind( 5, nPathID ) &&
				db.Step();
			}
			else
			{
				ATLTRACE ( "bad PathID " );
			}
			gflMemoryFree( data );
		}
		else
		{
			ATLTRACE( "(gflSaveBitmapIntoMemory failed : %s) ", gflGetErrorString( err ) );
		}
	}

	ATLTRACE( "S_OK.\n" );
	return S_OK;
}

#ifdef ISTREAM_ENABLED

HRESULT CEntity::LoadInfo(IStream* pStream)
{
	if ( m_bInfoLoaded )
	{
		ATLTRACE( "CEntity::LoadInfo() : S_FALSE (Already loaded)\n" );
		return S_OK;
	}

	CLock oLock( m_pSection );

	if ( m_bInfoLoaded )
	{
		ATLTRACE( "CEntity::LoadInfo() : S_FALSE (Already loaded)\n" );
		return S_OK;
	}

	// Load file information from stream
	STATSTG stat = {};
	if ( SUCCEEDED( pStream->Stat( &stat,  STATFLAG_DEFAULT ) ) && stat.pwcsName )
	{
		m_FileData.ftCreationTime = stat.ctime;
		m_FileData.ftLastAccessTime = stat.atime;
		m_FileData.ftLastWriteTime = stat.mtime;
		m_FileData.nFileSizeHigh = stat.cbSize.HighPart;
		m_FileData.nFileSizeLow = stat.cbSize.LowPart;
		wcscpy_s( m_FileData.cFileName, stat.pwcsName );
		if ( stat.pwcsName ) CoTaskMemFree( stat.pwcsName );

		CharLowerBuff( m_FileData.cFileName, (DWORD)wcslen( m_FileData.cFileName ) );
		QWORD nFileSize = MAKEQWORD( m_FileData.nFileSizeLow, m_FileData.nFileSizeHigh );

		// Load image information from database
		CDatabase db( _Module.m_sDatabase );
		if ( db &&
			 db.Prepare( _T("SELECT ImageInfo FROM Entities WHERE Filename==? AND FileSize==? AND LastWriteTime==? AND CreationTime==?;") ) &&
			 db.Bind( 1, m_FileData.cFileName ) &&
			 db.Bind( 2, (__int64)nFileSize ) &&
			 db.Bind( 3, (__int64)MAKEQWORD( m_FileData.ftLastWriteTime.dwLowDateTime, m_FileData.ftLastWriteTime.dwHighDateTime ) ) &&
			 db.Bind( 4, (__int64)MAKEQWORD( m_FileData.ftCreationTime.dwLowDateTime, m_FileData.ftCreationTime.dwHighDateTime ) ) &&
			 db.Step() &&
			 db.GetCount() == 1 )
		{
			int nImageInfoSize = 0;
			if ( LPCVOID pImageInfo = db.GetBlob( _T("ImageInfo"), &nImageInfoSize ) )
			{
				if ( nImageInfoSize == sizeof( m_ImageInfo ) )
				{
					CopyMemory( &m_ImageInfo, pImageInfo, sizeof( m_ImageInfo ) );

					// Validate image information
					if ( m_ImageInfo.FileSize == nFileSize &&
						 m_ImageInfo.Width &&
						 m_ImageInfo.Height &&
						 m_ImageInfo.ComponentsPerPixel &&
						 m_ImageInfo.BitsPerComponent &&
						 m_ImageInfo.FormatName[ 0 ] )
					{
						m_bDatabaseUsed = true;
						m_bInfoLoaded = true;

						ATLTRACE( "CEntity::LoadInfo(\"%s\") : S_OK (from database %dx%d)\n", (LPCSTR)CT2A( m_FileData.cFileName ), m_ImageInfo.Width, m_ImageInfo.Height );
						return S_OK;
					}
				}
			}
		}
	}

	GFL_LOAD_CALLBACKS calls = { IStreamRead, IStreamTell, IStreamSeek };
	const LARGE_INTEGER zero = {};
	pStream->Seek( zero, STREAM_SEEK_SET, NULL );
	GFL_ERROR res = gflGetFileInformationFromHandle( (GFL_HANDLE)pStream, -1, &calls, &m_ImageInfo );
	if ( res != GFL_NO_ERROR )
	{
		ATLTRACE( "CEntity::LoadInfo(\"%s\") : E_FAIL (GFL error %d)\n", (LPCSTR)CT2A( m_FileData.cFileName ), res);
		return E_FAIL;
	}

	m_bInfoLoaded = true;

	ATLTRACE( "CEntity::LoadInfo(\"%s\") : S_OK (from stream %dx%d)\n", (LPCSTR)CT2A( m_FileData.cFileName ), m_ImageInfo.Width, m_ImageInfo.Height );
	return S_OK;
}

HRESULT CEntity::LoadImage(IStream* pStream, UINT cx, UINT cy)
{
	if ( m_hGflBitmap )
	{
		ATLTRACE( "CEntity::LoadImage(%ux%u) : S_FALSE (Already loaded)\n", cx, cy );
		return S_FALSE;
	}

	HRESULT hr = LoadInfo( pStream );
	if ( FAILED( hr ) )
		return hr;

	if ( m_bDatabaseUsed )
	{
		QWORD nFileSize = MAKEQWORD( m_FileData.nFileSizeLow, m_FileData.nFileSizeHigh );

		// Load image from database
		CDatabase db( _Module.m_sDatabase );
		if ( db &&
			db.Prepare( _T("SELECT Image, Width, Height FROM Entities WHERE Filename==? AND FileSize==? AND LastWriteTime==? AND CreationTime==?;") ) &&
			db.Bind( 1, m_FileData.cFileName ) &&
			db.Bind( 2, (__int64)nFileSize ) &&
			db.Bind( 3, (__int64)MAKEQWORD( m_FileData.ftLastWriteTime.dwLowDateTime, m_FileData.ftLastWriteTime.dwHighDateTime ) ) &&
			db.Bind( 4, (__int64)MAKEQWORD( m_FileData.ftCreationTime.dwLowDateTime, m_FileData.ftCreationTime.dwHighDateTime ) ) &&
			db.Step() &&
			db.GetCount() == 3 )
		{
			// Test image dimensions
			int dx = db.GetInt32( _T("Width") );
			int dy = db.GetInt32( _T("Height") );
			if ( ( dx >= (int)cx ) ||
				 ( dy >= (int)cy ) ||
				 ( m_ImageInfo.Width == dx && m_ImageInfo.Height <= dy ) ||
				 ( m_ImageInfo.Width <= dx && m_ImageInfo.Height == dy ) )
			{
				// Good image
				int nImageSize = 0;
				if ( LPCVOID pImage = db.GetBlob( _T("Image"), &nImageSize ) )
				{
					_Module.LoadBitmapFromMemory( pImage, nImageSize, &m_hGflBitmap );
					if ( m_hGflBitmap )
					{
						ATLTRACE( "CEntity::LoadImage(%ux%u) : S_OK (from database %d bytes %dx%d)\n", cx, cy, nImageSize, m_hGflBitmap->Width, m_hGflBitmap->Height );
						return S_OK;
					}
				}
			}
		}
	}

	GFL_LOAD_PARAMS params;
	gflGetDefaultThumbnailParams( &params );
	if ( LPCTSTR szExt = PathFindExtension( m_FileData.cFileName ) )
	{
		if ( *szExt == _T('.') )
		{
			params.FormatIndex = gflGetFormatIndexByName( (LPCSTR)CT2A( &szExt[ 1 ] ) );
		}
	}
	params.Flags =
		GFL_LOAD_ONLY_FIRST_FRAME |
		GFL_LOAD_HIGH_QUALITY_THUMBNAIL |
		( ( ::GetRegValue( _T("UseEmbedded"), 0ul ) != 0 ) ? GFL_LOAD_EMBEDDED_THUMBNAIL : 0 ) |
		GFL_LOAD_PREVIEW_NO_CANVAS_RESIZE;
	params.ColorModel = GFL_RGBA;
	params.Callbacks.Read = IStreamRead;
	params.Callbacks.Tell = IStreamTell;
	params.Callbacks.Seek = IStreamSeek;
	const LARGE_INTEGER zero = {};
	pStream->Seek( zero, STREAM_SEEK_SET, NULL );
	GFL_ERROR err = gflLoadThumbnailFromHandle( (GFL_HANDLE)pStream, cx, cy, &m_hGflBitmap, &params, NULL );
	if ( err == GFL_ERROR_FILE_READ )
	{
		params.Flags |= GFL_LOAD_IGNORE_READ_ERROR;
		pStream->Seek( zero, STREAM_SEEK_SET, NULL );
		err = gflLoadThumbnailFromHandle( (GFL_HANDLE)pStream, cx, cy, &m_hGflBitmap, &params, NULL );
	}
	if ( err != GFL_NO_ERROR )
	{
		ATLTRACE( "CEntity::LoadImage(%ux%u) : E_FAIL (GFL error %d)\n", cx, cy, err );
		return E_FAIL;
	}

	if ( m_hGflBitmap->Type != GFL_RGBA )
	{
		Change_Color_Depth( m_hGflBitmap );
	}

	ATLTRACE( "CEntity::LoadImage(%ux%u) : S_OK (from stream)\n", cx, cy );
	return S_OK;
}

#endif // ISTREAM_ENABLED

HBITMAP CEntity::GetImage(UINT cx, UINT cy)
{
	CLock oLock( m_pSection );

	HBITMAP hBitmap = NULL;
	if ( m_hGflBitmap )
	{
		if ( ( m_hGflBitmap->Width == (int)cx && m_hGflBitmap->Height <= (int)cy ) ||
			 ( m_hGflBitmap->Width <= (int)cx && m_hGflBitmap->Height == (int)cy ) )
		{
			_Module.ConvertBitmap( m_hGflBitmap, &hBitmap );
		}
		else
		{
			UINT dx, dy;
			CalcSize( dx, dy, cx, cy );

			GFL_BITMAP* pResizedBitmap = NULL;
			if ( SUCCEEDED( _Module.Resize( m_hGflBitmap, &pResizedBitmap, dx, dy ) ) &&
				 pResizedBitmap )
			{
				_Module.ConvertBitmap( pResizedBitmap, &hBitmap );
				_Module.FreeBitmap( pResizedBitmap );
			}
		}
	}
	return hBitmap;
}

HICON CEntity::GetIcon(UINT cx)
{
	CLock oLock( m_pSection );

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

		const HWND hWnd = GetDesktopWindow();
		if ( const HDC hDC = GetDC( hWnd ) )
		{
			HBRUSH hWhite = (HBRUSH)GetStockObject( WHITE_BRUSH );
			HBRUSH hBlack = (HBRUSH)GetStockObject( BLACK_BRUSH );
			HDC hMemDC1 = CreateCompatibleDC( hDC );
			HDC hMemDC2 = CreateCompatibleDC( hDC );
			HBITMAP hOldMask2 = (HBITMAP)SelectObject( hMemDC2, hBitmap );

			// Создание маски
			HBITMAP hbmMask = CreateCompatibleBitmap( hDC, cx, cx );
			HBITMAP hOldMask1 = (HBITMAP)SelectObject( hMemDC1, hbmMask );
			FillRect( hMemDC1, &rcAll, hWhite );
			FillRect( hMemDC1, &rcIcon, hBlack );

			// Создание изображения (по центру)
			HBITMAP hbmColor = CreateCompatibleBitmap( hDC, cx, cx );
			SelectObject( hMemDC1, hbmColor );
			FillRect( hMemDC1, &rcAll, hBlack );
			BitBlt( hMemDC1, nx, ny, bm.bmWidth, bm.bmHeight, hMemDC2, 0, 0, SRCCOPY );

			SelectObject( hMemDC2, hOldMask2 );
			DeleteObject( hMemDC2 );
			SelectObject( hMemDC1, hOldMask1 );
			DeleteObject( hMemDC1 );

			ICONINFO ii =
			{
				TRUE,
				0,
				0,
				hbmMask,
				hbmColor
			};
			hIcon = CreateIconIndirect( &ii );

			DeleteObject( hbmMask );
			DeleteObject( hbmColor );

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