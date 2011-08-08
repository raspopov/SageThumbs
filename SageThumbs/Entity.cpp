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
#include "SageThumbs.h"
#include "Entity.h"
#include "SQLite.h"

CEntity::CEntity ()
	: m_FileData	()
	, m_ImageInfo	()
	, m_hGflBitmap	( NULL )
{
}

CEntity::~CEntity()
{
	_Module.FreeBitmap( m_hGflBitmap );

	gflFreeFileInformation( &m_ImageInfo );
}

/*
STATSTG stat = {};
hr = pstream->Stat( &stat, STATFLAG_DEFAULT );
if ( FAILED( hr ) )
{
	ATLTRACE( "E_FAIL (Not a real file)\n" );
	return E_FAIL;
}

if ( stat.pwcsName )
CoTaskMemFree( stat.pwcsName );
*/

HRESULT CEntity::LoadInfo(const CString& sFilename)
{
	CLock oLock( m_pSection );

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

	// Поиск файла в базе данных
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

			if ( db.Prepare( _T("SELECT FileSize, LastWriteTime, CreationTime, ImageInfo, Width, Height FROM Entities WHERE Filename==? AND PathID==?;") ) &&
				 db.Bind( 1, sName ) &&
				 db.Bind( 2, nPathID ) &&
				 db.Step() &&
				 db.GetCount() == 6 )
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

							// Проверка информации
							bFound =
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

	if ( ! bFound )
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

	CString type;
	type.LoadString (IDS_TYPE);
	CString dim;
	dim.LoadString (IDS_DIM);
	CString colors;
	colors.LoadString (IDS_COLORS);
	CString date;
	date.LoadString (IDS_DATE);
	CString size;
	size.LoadString (IDS_SIZE);
	CString resolution;
	resolution.LoadString (IDS_RESOLUTION);
	CString compression;
	compression.LoadString (IDS_COMPRESSION);

	CString tmp;
	tmp = type;
	tmp += m_ImageInfo.Description[ 0 ] ? (LPCTSTR)CA2T( m_ImageInfo.Description ) : data.info;
	m_MenuTipString += tmp;
	tmp.Format (_T(", %s%d x %d"), (LPCTSTR)dim, m_ImageInfo.Width, m_ImageInfo.Height);
	m_MenuTipString += tmp;
	tmp.Format (_T(", %s%d"), (LPCTSTR)colors, m_ImageInfo.ComponentsPerPixel * m_ImageInfo.BitsPerComponent);
	m_MenuTipString += tmp;
	if ( m_ImageInfo.Xdpi )
	{
		tmp.Format (_T(", %s%d dpi"), (LPCTSTR)resolution, m_ImageInfo.Xdpi);
		m_MenuTipString += tmp;
	}
	if ( m_ImageInfo.Compression != GFL_NO_COMPRESSION )
	{
		tmp = _T(", ");
		tmp += compression;
		tmp += (LPCTSTR)CA2T (m_ImageInfo.CompressionDescription);
		m_MenuTipString += tmp;
	}
	tmp = _T(", ");
	tmp += date;
	tmp += datetime;
	m_MenuTipString += tmp;
	TCHAR file_size [ 33 ] = {};
	_ui64tot_s( MAKEQWORD( m_FileData.nFileSizeLow,  m_FileData.nFileSizeHigh ), file_size, 33, 10 );
	tmp = _T(", ");
	tmp += size;
	tmp += file_size;
	m_MenuTipString += tmp;

	m_InfoTipString = m_MenuTipString;
	m_InfoTipString.Replace (_T(','), _T('\n'));

	tmp.Format (_T("%dx%d %d bit"),
		m_ImageInfo.Width, m_ImageInfo.Height,
		m_ImageInfo.ComponentsPerPixel * m_ImageInfo.BitsPerComponent);
	m_TitleString += (LPCTSTR)CA2T (m_ImageInfo.FormatName);
	m_TitleString += _T(" ");
	m_TitleString += tmp;

	ATLTRACE( "S_OK.\n" );
	return S_OK;
}

HRESULT CEntity::LoadImage(const CString& sFilename, UINT cx, UINT cy)
{
	CLock oLock( m_pSection );

	if ( m_hGflBitmap &&
		 m_hGflBitmap->Width  >= (GFL_INT32)cx && 
		 m_hGflBitmap->Height >= (GFL_INT32)cy )
	{
		ATLTRACE( "CEntity::LoadImage(\"%s\",%d,%d) : S_FALSE (Bitmap already loaded)\n", (LPCSTR)CT2A( sFilename ), cx, cy );
		return S_FALSE;
	}

	_Module.FreeBitmap( m_hGflBitmap );

	HRESULT hr = LoadInfo( sFilename );
	if ( FAILED( hr ) )
		return hr;

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
				UINT dx = (UINT)db.GetInt32( _T("Width") );
				UINT dy = (UINT)db.GetInt32( _T("Height") );
				if ( dx >= cx || dy >= cy )
				{
					// Загрузка изображения из базы данных
					int nImageSize;
					if ( LPCVOID pImage = db.GetBlob( _T("Image"), &nImageSize ) )
					{
						_Module.LoadBitmapFromMemory( pImage, nImageSize, &m_hGflBitmap );
						ATLTRACE ( "from database " );
					}
				}
				else
				{
					// Слишком маленькое изображение - перезагрузка из файла
					ATLTRACE ( "reload (needed %ux%u but %ux%u in database) ", cx, cy, dx, dy );
				}
			}
		}
	}

	if ( ! m_hGflBitmap )
	{
		cx = max( cx, max( GetRegValue( _T("Width"), THUMB_STORE_SIZE ), THUMB_STORE_SIZE ) );
		cy = max( cy, max( GetRegValue( _T("Height"), THUMB_STORE_SIZE ), THUMB_STORE_SIZE ) );

		// Загрузка из файла
		_Module.LoadThumbnail( sFilename, cx, cy, &m_hGflBitmap );
		ATLTRACE ( "from disk " );
		if ( ! m_hGflBitmap )
		{
			ATLTRACE( "E_FAIL\n" );
			return E_FAIL;
		}

		// Сохранение изображения в базе если нужно
		GFL_SAVE_PARAMS params;
		gflGetDefaultSaveParams( &params );
		params.Flags = GFL_SAVE_ANYWAY;
		params.CompressionLevel = 9;
		//params.Quality = 70;
		//params.Progressive = GFL_TRUE;
		//params.OptimizeHuffmanTable = GFL_TRUE;
		params.FormatIndex = gflGetFormatIndexByName( "png" /* "jpeg" */ );

		GFL_UINT8* data = NULL;
		GFL_UINT32 data_length = 0;
		GFL_ERROR err = gflSaveBitmapIntoMemory( &data, &data_length, m_hGflBitmap, &params );
		if ( err == GFL_NO_ERROR )
		{
			if ( nPathID )
			{
				db.Prepare( _T("UPDATE Entities SET Image=?, Width=?, Height=? WHERE Filename==? AND PathID==?;") ) &&
				db.Bind( 1, data, data_length ) &&
				db.Bind( 2, (__int32)cx ) &&
				db.Bind( 3, (__int32)cy ) &&
				db.Bind( 4, sName ) &&
				db.Bind( 5, nPathID ) &&
				db.Step();
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

HBITMAP CEntity::GetImage(UINT cx, UINT cy)
{
	CLock oLock( m_pSection );

	HBITMAP hBitmap = NULL;
	if ( m_hGflBitmap )
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
	CLock oLock( m_pSection );

	if ( (UINT)m_ImageInfo.Width < width && (UINT)m_ImageInfo.Height < height )
	{
		tx = (UINT)m_ImageInfo.Width;
		ty = (UINT)m_ImageInfo.Height;
	}
	else
	{
		tx = width;
		ty = height;
		if ( m_ImageInfo.Width && m_ImageInfo.Height && width && height )
		{
			double aspect = ( (double)m_ImageInfo.Width / (double)m_ImageInfo.Height ) /
				( (double)width / (double)height );
			if ( aspect < 1 )
				tx = (UINT)( width * aspect );
			else
				ty = (UINT)( height / aspect );
		}
	}
	if ( ! tx ) tx = 1;
	if ( ! ty ) ty = 1;
}
