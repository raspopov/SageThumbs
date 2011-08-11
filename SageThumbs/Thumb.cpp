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
#include "Thumb.h"
#include "SQLite.h"
#include <InitGuid.h>

DEFINE_GUID(CLSID_Thumb,0x4A34B3E3,0xF50E,0x4FF6,0x89,0x79,0x7E,0x41,0x76,0x46,0x6F,0xF2);

CThumb::CThumb() :
	m_uOurItemID( 0 ),
	m_cx( THUMB_STORE_SIZE ),
	m_cy( THUMB_STORE_SIZE ),
	m_bCleanup( FALSE )
{
}

HRESULT CThumb::FinalConstruct()
{
	return S_OK;
}

void CThumb::FinalRelease()
{
	m_Filenames.RemoveAll();

	m_sFilename.Empty();

//	m_pStream.Release();

	m_pSite.Release();
}

// IShellExtInit

STDMETHODIMP CThumb::Initialize(LPCITEMIDLIST, IDataObject* pDO, HKEY)
{
	ATLTRACE( "0x%08x::IShellExtInit::Initialize() : ", this );

	bool bEnableMenu = GetRegValue( _T("EnableMenu"), 1ul ) != 0;
	if ( ! bEnableMenu )
	{
		ATLTRACE( "E_INVALIDARG (Menu disabled)\n" );
		return E_INVALIDARG;
	}

	if ( ! pDO )
	{
		ATLTRACE( "E_INVALIDARG (No data)\n" );
		return E_INVALIDARG;
	}

	// Получение данных о выделенных элементах
	FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM med = { TYMED_HGLOBAL, NULL, NULL };
	HRESULT hr = pDO->GetData( &fe, &med );
	if ( FAILED( hr ) )
	{
		ATLTRACE( "E_INVALIDARG (No data)\n" );
		return E_INVALIDARG;
	}

    HDROP hDrop = (HDROP)GlobalLock( med.hGlobal );
    if ( ! hDrop )
	{
		ReleaseStgMedium (&med);
		ATLTRACE( "E_INVALIDARG (No data)\n" );
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
	}
    GlobalUnlock( med.hGlobal );
	ReleaseStgMedium( &med );

	if ( m_Filenames.IsEmpty() )
	{
		ATLTRACE( "E_INVALIDARG (No files selected)\n" );
		return E_INVALIDARG;
	}

	ATLTRACE( "S_OK (%d, \"%s\")\n", m_Filenames.GetCount(), (LPCSTR)CT2A( m_Filenames.GetHead() ) );

	return S_OK;
}

// IContextMenu

#define ID_SUBMENU_ITEM				0
#define ID_CLIPBOARD_ITEM			1
#define ID_THUMBNAIL_ITEM			2
#define ID_OPTIONS_ITEM				3
#define ID_WALLPAPER_STRETCH_ITEM	4
#define ID_WALLPAPER_TILE_ITEM		5
#define ID_WALLPAPER_CENTER_ITEM	6
#define ID_MAIL_IMAGE_ITEM			7
#define ID_MAIL_THUMBNAIL_ITEM		8
#define ID_CONVERT_JPG_ITEM			9
#define ID_CONVERT_GIF_ITEM			10
#define ID_CONVERT_BMP_ITEM			11
#define ID_CONVERT_PNG_ITEM			12
#define ID_END_ITEM					13			

STDMETHODIMP CThumb::QueryContextMenu(HMENU hMenu, UINT uIndex, UINT uidCmdFirst, UINT uidCmdLast, UINT uFlags)
{
	ATLTRACE( "0x%08x::IContextMenu::QueryContextMenu (hmenu=0x%08x, index=%d, first=%d, last=%d, flags=0x%08x)\n", this, hMenu, uIndex, uidCmdFirst, uidCmdLast, uFlags);

	bool bEnableMenu = GetRegValue( _T("EnableMenu"), 1ul ) != 0;
	if ( ! bEnableMenu )
		// Меню выключено
		return E_FAIL;

	// If the flags include CMF_DEFAULTONLY then we shouldn't do anything.
	if ( uFlags & CMF_DEFAULTONLY )
		return MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_NULL, 0 );

	// Проверка на нехватку идентификаторов
	if ( uidCmdFirst + ID_END_ITEM > uidCmdLast )
		return E_FAIL;

	bool bSingleFile = ( m_Filenames.GetCount () == 1 );

	// Store the menu item's ID so we can check against it later when
	// WM_MEASUREITEM/WM_DRAWITEM are sent. 
	m_uOurItemID = uidCmdFirst;

	CString tmp;
	int nPos = 0;

	// Creating submenu items
	HMENU hSubMenu = CreateMenu ();
	
	// Clipboard operation items
	if ( bSingleFile )
	{
		tmp.LoadString (IDS_CLIPBOARD);
		if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING,
			uidCmdFirst + ID_CLIPBOARD_ITEM, tmp))
			return E_FAIL;
		if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_SEPARATOR, 0, 0))
			return E_FAIL;
	}

	// Wallpaper operation items
	if ( bSingleFile )
	{
		tmp.LoadString (IDS_WALLPAPER_STRETCH);
		if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING,
			uidCmdFirst + ID_WALLPAPER_STRETCH_ITEM, tmp))
			return E_FAIL;
		tmp.LoadString (IDS_WALLPAPER_TILE);
		if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING,
			uidCmdFirst + ID_WALLPAPER_TILE_ITEM, tmp))
			return E_FAIL;
		tmp.LoadString (IDS_WALLPAPER_CENTER);
		if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING,
			uidCmdFirst + ID_WALLPAPER_CENTER_ITEM, tmp))
			return E_FAIL;
		if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_SEPARATOR, 0, 0))
			return E_FAIL;
	}

	tmp.LoadString (IDS_MAIL_IMAGE);
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING,
		uidCmdFirst + ID_MAIL_IMAGE_ITEM, tmp))
		return E_FAIL;
	tmp.LoadString (IDS_MAIL_THUMBNAIL);
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING,
		uidCmdFirst + ID_MAIL_THUMBNAIL_ITEM, tmp))
		return E_FAIL;
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_SEPARATOR, 0, 0))
		return E_FAIL;

	tmp.LoadString (IDS_CONVERT_JPG);
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING,
		uidCmdFirst + ID_CONVERT_JPG_ITEM, tmp))
		return E_FAIL;
	tmp.LoadString (IDS_CONVERT_GIF);
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING,
		uidCmdFirst + ID_CONVERT_GIF_ITEM, tmp))
		return E_FAIL;
	tmp.LoadString (IDS_CONVERT_BMP);
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING,
		uidCmdFirst + ID_CONVERT_BMP_ITEM, tmp))
		return E_FAIL;
	tmp.LoadString (IDS_CONVERT_PNG);
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING,
		uidCmdFirst + ID_CONVERT_PNG_ITEM, tmp))
		return E_FAIL;
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_SEPARATOR, 0, 0))
		return E_FAIL;

	tmp.LoadString (IDS_OPTIONS);
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING,
		uidCmdFirst + ID_OPTIONS_ITEM, tmp))
		return E_FAIL;

	// Creating main menu items
	if ( ! InsertMenu( hMenu, uIndex++, MF_BYPOSITION | MF_SEPARATOR, 0, 0 ) )
		return E_FAIL;

	// Preview menu item
	if ( bSingleFile )
	{
		// Загрузка файла
		if ( SUCCEEDED( m_Preview.LoadImage( m_Filenames.GetHead(), m_cx, m_cy ) ) )
		{
			if ( ! InsertMenu( hSubMenu, 0, MF_BYPOSITION | MF_OWNERDRAW,
				uidCmdFirst + ID_THUMBNAIL_ITEM, 0 ) )
				return E_FAIL;
		}
	}

	tmp = _Module.GetAppName();

	MENUITEMINFO mii = { sizeof( MENUITEMINFO ) };
	mii.fMask  = MIIM_STRING | MIIM_SUBMENU | MIIM_ID | MIIM_BITMAP;
	mii.wID = uidCmdFirst + ID_SUBMENU_ITEM;
	mii.hSubMenu = hSubMenu;
	mii.dwTypeData = (LPTSTR)(LPCTSTR)tmp;
	mii.cch = (UINT)tmp.GetLength();
	mii.hbmpItem = (HBITMAP)LoadImage( _AtlBaseModule.GetResourceInstance(),
		MAKEINTRESOURCE( IDR_SAGETHUMBS ), IMAGE_BITMAP, 16, 16, LR_SHARED );

	if ( ! InsertMenuItem ( hMenu, uIndex++, TRUE, &mii ) )
		return E_FAIL;

	if ( ! InsertMenu (hMenu, uIndex++, MF_BYPOSITION | MF_SEPARATOR, 0, 0 ) )
		return E_FAIL;

	// Tell the shell we added top-level menu items
	return MAKE_HRESULT (SEVERITY_SUCCESS, FACILITY_NULL, ID_END_ITEM);
}

STDMETHODIMP CThumb::GetCommandString (
	UINT_PTR uCmd, UINT uFlags, UINT* /*puReserved*/,
	LPSTR pszName, UINT cchMax )
{
	ATLTRACE( "0x%08x::IContextMenu::GetCommandString (%d, %d, 0x%08x \"%s\", %d) ", this, uCmd, uFlags, pszName, pszName, cchMax);

	CString tmp;
	switch ( uFlags )
	{
	case GCS_VERBA:
	case GCS_VERBW:
		if ( uCmd == ID_THUMBNAIL_ITEM )
		{
			tmp = _T("preview");
		}
		break;

	case GCS_HELPTEXTA:
	case GCS_HELPTEXTW:
		switch ( uCmd )
		{
		case ID_SUBMENU_ITEM:
			tmp = _Module.GetAppName();
			break;
		case ID_THUMBNAIL_ITEM:
			tmp = m_Preview.GetMenuTipString ();
			break;
		case ID_OPTIONS_ITEM:
			tmp.LoadString (IDS_OPTIONS_HELP);
			break;
		case ID_CLIPBOARD_ITEM:
			tmp.LoadString (IDS_CLIPBOARD);
			break;
		case ID_WALLPAPER_STRETCH_ITEM:
			tmp.LoadString (IDS_WALLPAPER_STRETCH);
			break;
		case ID_WALLPAPER_TILE_ITEM:
			tmp.LoadString (IDS_WALLPAPER_TILE);
			break;
		case ID_WALLPAPER_CENTER_ITEM:
			tmp.LoadString (IDS_WALLPAPER_CENTER);
			break;
		case ID_MAIL_IMAGE_ITEM:
			tmp.LoadString (IDS_MAIL_IMAGE);
			break;
		case ID_MAIL_THUMBNAIL_ITEM:
			tmp.LoadString (IDS_MAIL_THUMBNAIL);
			break;
		case ID_CONVERT_JPG_ITEM:
			tmp.LoadString (IDS_CONVERT_JPG);
			break;
		case ID_CONVERT_GIF_ITEM:
			tmp.LoadString (IDS_CONVERT_GIF);
			break;
		case ID_CONVERT_BMP_ITEM:
			tmp.LoadString (IDS_CONVERT_BMP);
			break;
		case ID_CONVERT_PNG_ITEM:
			tmp.LoadString (IDS_CONVERT_PNG);
			break;
		default:
			ATLTRACE( "E_INVALIDARG\n" );
			return E_INVALIDARG;
		}
		break;

	case GCS_VALIDATEA:
	case GCS_VALIDATEW:
		ATLTRACE( "S_OK\n" );
		return S_OK;

	default:
		ATLTRACE( "E_INVALIDARG\n" );
		return E_INVALIDARG;
	}

	if ( uFlags & GCS_UNICODE )
		wcsncpy_s( (LPWSTR)pszName, cchMax, (LPCWSTR)CT2W( tmp ), cchMax );
	else
		strncpy_s( (LPSTR)pszName, cchMax, (LPCSTR)CT2A( tmp ), cchMax );

	ATLTRACE( "S_OK\n" );
	return S_OK;
}

void CThumb::ConvertTo(HWND hWnd, int ext)
{
	LPCSTR szExt = NULL;
	switch ( ext )
	{
	case ID_CONVERT_JPG_ITEM:
		szExt = "jpeg";
		break;
	case ID_CONVERT_GIF_ITEM:
		szExt = "gif";
		break;
	case ID_CONVERT_BMP_ITEM:
		szExt = "bmp";
		break;
	case ID_CONVERT_PNG_ITEM:
		szExt = "png";
		break;
	default:
		ATLASSERT( FALSE );
	}
	int index = gflGetFormatIndexByName( szExt );

	for ( POSITION pos = m_Filenames.GetHeadPosition (); pos ; )
	{
		CString filename( m_Filenames.GetNext( pos ) );

		GFL_BITMAP* hBitmap = NULL;
		if ( SUCCEEDED( _Module.LoadBitmap( filename, &hBitmap ) ) )
		{
			GFL_SAVE_PARAMS params = {};
			gflGetDefaultSaveParams( &params );
			params.Flags = GFL_SAVE_REPLACE_EXTENSION | GFL_SAVE_ANYWAY;
			params.FormatIndex = index;
			if ( ext == ID_CONVERT_JPG_ITEM )
			{
				params.Quality = (GFL_INT16)GetRegValue( _T("JPEG"), JPEG_DEFAULT );
				params.Progressive = GFL_TRUE;
				params.OptimizeHuffmanTable = GFL_TRUE;
			}
			else if ( ext == ID_CONVERT_GIF_ITEM )
			{
				params.Interlaced = GFL_TRUE;
			}
			else if ( ext == ID_CONVERT_PNG_ITEM )
			{
				params.CompressionLevel = (GFL_INT16)GetRegValue( _T("PNG"), PNG_DEFAULT );
			}

			if ( gflSaveBitmapT( (LPTSTR)(LPCTSTR)filename, hBitmap, &params ) != GFL_NO_ERROR )
			{
				_Module.MsgBox( hWnd, IDS_ERR_SAVE );
				break;
			}
			DeleteObject( hBitmap );
		}
		else
		{
			_Module.MsgBox( hWnd, IDS_ERR_OPEN );
			break;
		}
	}
}

void CThumb::SetWallpaper(HWND hWnd, WORD reason)
{
	CString filename( m_Filenames.GetHead() );

	GFL_BITMAP* hBitmap = NULL;
	if ( SUCCEEDED( _Module.LoadBitmap( filename, &hBitmap ) ) )
	{
		GFL_SAVE_PARAMS params = {};
		gflGetDefaultSaveParams( &params );
		params.Flags = GFL_SAVE_ANYWAY;
		params.Compression = GFL_NO_COMPRESSION;
		params.FormatIndex = gflGetFormatIndexByName( "bmp" );
		CString save_path = GetSpecialFolderPath( CSIDL_APPDATA ).TrimRight( _T("\\") ) +
			_T("\\SageThumbs wallpaper.bmp");
		if ( gflSaveBitmapT( (LPTSTR)(LPCTSTR)save_path, hBitmap, &params) == GFL_NO_ERROR)
		{
			SetRegValue( _T("TileWallpaper"),
				((reason == ID_WALLPAPER_TILE_ITEM) ? _T("1") : _T("0")),
				_T("Control Panel\\Desktop"), HKEY_CURRENT_USER );
			SetRegValue( _T("WallpaperStyle"),
				((reason == ID_WALLPAPER_STRETCH_ITEM) ? _T("2") : _T("0")),
				_T("Control Panel\\Desktop"), HKEY_CURRENT_USER );
			SystemParametersInfo (SPI_SETDESKWALLPAPER, 0,
				(LPVOID) (LPCTSTR) save_path, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
		}
		else
			_Module.MsgBox( hWnd, IDS_ERR_SAVE );
		DeleteObject( hBitmap );
	}
	else
		_Module.MsgBox( hWnd, IDS_ERR_OPEN );
}

void CThumb::SendByMail(HWND hwnd, WORD reason)
{
	HRESULT hr;

	// Загрузка размеров из реестра
	DWORD width = GetRegValue( _T("Width"), THUMB_STORE_SIZE );
	DWORD height = GetRegValue( _T("Height"), THUMB_STORE_SIZE );

	// Инициализация MAPI
	if ( HMODULE hLibrary = LoadLibrary( _T("MAPI32.DLL") ) )
	{
		tMAPISendMail pMAPISendMail = (tMAPISendMail)GetProcAddress( hLibrary, "MAPISendMail" );
		if ( pMAPISendMail )
		{
			// Подготовка изображений к отсылке
			CAtlArray< CStringA > save_names;
			CAtlArray< CStringA > save_filenames;
			for ( POSITION pos = m_Filenames.GetHeadPosition () ; pos ; )
			{
				CString filename( m_Filenames.GetNext( pos ) );
				if ( reason == ID_MAIL_IMAGE_ITEM )
				{
					save_names.Add( CT2CA( PathFindFileName( filename ) ) );
					save_filenames.Add( CT2CA( filename ) );
				}
				else
				{
					GFL_BITMAP* hGflBitmap = NULL;
					hr = _Module.LoadThumbnail( filename, width, height, &hGflBitmap );
					if ( SUCCEEDED( hr ) )
					{
						GFL_SAVE_PARAMS params = {};
						gflGetDefaultSaveParams (&params);
						params.Flags = GFL_SAVE_ANYWAY;
						params.FormatIndex = gflGetFormatIndexByName( "jpeg" );
						TCHAR tmp [ MAX_PATH ] = {};
						GetTempPath( MAX_PATH, tmp );
						GetTempFileName( tmp, _T("tmb"), 0, tmp );
						if ( gflSaveBitmapT( tmp, hGflBitmap, &params ) == GFL_NO_ERROR )
						{
							save_names.Add( CT2CA( PathFindFileName( filename ) ) );
							save_filenames.Add( CT2CA( tmp ) );
						}
					}
				}
			}
			if ( size_t count = save_names.GetCount() )
			{
				// Отсылка письма
				MapiFileDesc* mfd = new MapiFileDesc[ count ];
				MapiMessage mm = {};
				mm.nFileCount = (ULONG)count;
				mm.lpFiles = mfd;
				if ( mfd )
				{
					ZeroMemory( mfd, sizeof( MapiFileDesc ) * count );

					for ( size_t i = 0; i < count; ++i )
					{
						mfd [i].nPosition = (ULONG)-1;
						mfd [i].lpszPathName = const_cast< LPSTR >(
							(LPCSTR)save_filenames[ i ] );
						mfd [i].lpszFileName = const_cast< LPSTR >(
							(LPCSTR)save_names[ i ] );
						mfd [i].lpFileType = NULL;
					}
					ULONG err = pMAPISendMail (0, (ULONG_PTR)hwnd, &mm,
						MAPI_DIALOG | MAPI_LOGON_UI | MAPI_NEW_SESSION, 0);
					if (MAPI_E_USER_ABORT != err && SUCCESS_SUCCESS != err)
						_Module.MsgBox( hwnd, IDS_ERR_MAIL );
					delete [] mfd;
				}
				else
					_Module.MsgBox( hwnd, IDS_ERR_MAIL );

				// Удаление временных изображений
				if ( reason != ID_MAIL_IMAGE_ITEM )
				{
					for ( size_t i = 0; i < count; ++i )
					{
						ATLVERIFY( DeleteFileA( save_filenames[ i ] ) );
					}
				}
			}
			else
				_Module.MsgBox( hwnd, IDS_ERR_NOTHING );
		}
		else
			_Module.MsgBox( hwnd, IDS_ERR_MAIL );
		FreeLibrary (hLibrary);
	}
	else
		_Module.MsgBox( hwnd, IDS_ERR_MAIL );
}

void CThumb::CopyToClipboard(HWND hwnd)
{
	CString filename( m_Filenames.GetHead() );

	GFL_BITMAP* pBitmap = NULL;
	HBITMAP hBitmap = NULL;
	if ( SUCCEEDED( _Module.LoadBitmap( filename, &pBitmap ) ) &&
		 SUCCEEDED( _Module.ConvertBitmap( pBitmap, &hBitmap ) ) )
	{
		if ( OpenClipboard ( hwnd ) )
		{
			EmptyClipboard();
			SetClipboardData( CF_BITMAP, hBitmap );
			CloseClipboard();
		}
		else
			_Module.MsgBox( hwnd, IDS_ERR_CLIPBOARD );

		DeleteObject( hBitmap );
	}
	else
		_Module.MsgBox(hwnd, IDS_ERR_OPEN );
}

STDMETHODIMP CThumb::InvokeCommand(LPCMINVOKECOMMANDINFO pInfo)
{
	ATLTRACE ( "0x%08x::IContextMenu::InvokeCommand\n", this);

	// If lpVerb really points to a string, ignore this function call and bail out.
	if (0 != HIWORD (pInfo->lpVerb))
		return E_INVALIDARG;

	// The command ID must be 0 since we only have one menu item.
	switch (LOWORD (pInfo->lpVerb))
	{
	case ID_THUMBNAIL_ITEM:
		// Open the bitmap in the default paint program.
		SHAddToRecentDocs (SHARD_PATH, m_Filenames.GetHead ());
		ShellExecute (pInfo->hwnd, _T("open"), m_Filenames.GetHead (),
			NULL, NULL, SW_SHOWNORMAL);
		break;

	case ID_OPTIONS_ITEM:
		// Options
		Options (pInfo->hwnd);
		break;

	case ID_CONVERT_JPG_ITEM:
		ConvertTo( pInfo->hwnd, ID_CONVERT_JPG_ITEM );
		break;
	case ID_CONVERT_GIF_ITEM:
		ConvertTo( pInfo->hwnd, ID_CONVERT_GIF_ITEM );
		break;
	case ID_CONVERT_BMP_ITEM:
		ConvertTo( pInfo->hwnd, ID_CONVERT_BMP_ITEM );
		break;
	case ID_CONVERT_PNG_ITEM:
		ConvertTo( pInfo->hwnd, ID_CONVERT_PNG_ITEM );
		break;

	case ID_WALLPAPER_STRETCH_ITEM:
	case ID_WALLPAPER_TILE_ITEM:
	case ID_WALLPAPER_CENTER_ITEM:
		SetWallpaper( pInfo->hwnd, LOWORD( pInfo->lpVerb ) );
		break;

	case ID_MAIL_IMAGE_ITEM:
	case ID_MAIL_THUMBNAIL_ITEM:
		SendByMail( pInfo->hwnd, LOWORD( pInfo->lpVerb ) );
		break;

	case ID_CLIPBOARD_ITEM:
		CopyToClipboard( pInfo->hwnd );
		break;

	default:
		return E_INVALIDARG;
	}

	return S_OK;
}

// IContextMenu2

STDMETHODIMP CThumb::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ATLTRACE( "0x%08x::IContextMenu2::HandleMenuMsg\n", this);
	LRESULT res = 0;
	return MenuMessageHandler (uMsg, wParam, lParam, &res);
}

// IContextMenu3

STDMETHODIMP CThumb::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	ATLTRACE( "0x%08x::IContextMenu3::HandleMenuMsg2\n", this);
	LRESULT res = 0;
	return MenuMessageHandler (uMsg, wParam, lParam, (pResult ? pResult : &res));
}

STDMETHODIMP CThumb::MenuMessageHandler(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, LRESULT* pResult)
{
	switch (uMsg)
	{
	case WM_INITMENUPOPUP:
		ATLTRACE ( "0x%08x::WM_INITMENUPOPUP\n", this);
		break;

	case WM_MEASUREITEM:
		ATLTRACE ( "0x%08x::WM_MEASUREITEM\n", this);
		return OnMeasureItem ((MEASUREITEMSTRUCT*) lParam, pResult);

	case WM_DRAWITEM:
		ATLTRACE ( "0x%08x::WM_DRAWITEM\n", this);
		return OnDrawItem ((DRAWITEMSTRUCT*) lParam, pResult);

	case WM_MENUCHAR:
		ATLTRACE ( "0x%08x::WM_MENUCHAR\n", this);
		break;

	default:
		ATLTRACE ( "0x%08x::Unknown message\n", this);
	}
	return S_OK;
}

STDMETHODIMP CThumb::OnMeasureItem(MEASUREITEMSTRUCT* pmis, LRESULT* pResult)
{
	// Check that we're getting called for our own menu item.
	if ( ID_THUMBNAIL_ITEM != pmis->itemID - m_uOurItemID )
		return S_OK;

	// Загрузка размеров из реестра
	DWORD width = GetRegValue( _T("Width"), THUMB_STORE_SIZE );
	DWORD height = GetRegValue( _T("Height"), THUMB_STORE_SIZE );

	// Расчет всех размеров
	m_Preview.CalcSize( pmis->itemWidth, pmis->itemHeight, width, height );
	pmis->itemWidth += 4;
	pmis->itemHeight += 24;

	*pResult = TRUE;
	return S_OK;
}

STDMETHODIMP CThumb::OnDrawItem(DRAWITEMSTRUCT* pdis, LRESULT* pResult)
{
	// Check that we're getting called for our own menu item.
	if ( ID_THUMBNAIL_ITEM != pdis->itemID - m_uOurItemID )
		return S_OK;

	// Загрузка размеров из реестра
	DWORD width = GetRegValue( _T("Width"), THUMB_STORE_SIZE );
	DWORD height = GetRegValue( _T("Height"), THUMB_STORE_SIZE );

	UINT cx, cy;
	m_Preview.CalcSize( cx, cy, width, height );

	RECT rcItem =
	{
		pdis->rcItem.left + 1, pdis->rcItem.top + 1,
		pdis->rcItem.right - 1, pdis->rcItem.bottom - 21
	};
	RECT rcText =
	{
		rcItem.left, rcItem.bottom + 2, rcItem.right, pdis->rcItem.bottom - 2
	};

	RECT rcDraw;
	rcDraw.left = ( rcItem.right + rcItem.left - cx ) / 2;
	rcDraw.top = ( rcItem.top + rcItem.bottom - cy ) / 2;
	rcDraw.right = rcDraw.left + cx;
	rcDraw.bottom = rcDraw.top + cy;

	// Отрисовка

	// Windows XP: Enables or disables flat menu appearance for native User menus.
	// When enabled, the menu bar uses COLOR_MENUBAR for the menubar background,
	// COLOR_MENU for the menu-popup background,
	// COLOR_MENUHILIGHT for the fill of the current menu selection, and
	// COLOR_HILIGHT for the outline of the current menu selection.
	// If disabled, menus are drawn using the same metrics and colors as
	// in Windows 2000 and earlier.
	BOOL bFlatMenu = FALSE;
	SystemParametersInfo (SPI_GETFLATMENU, 0, &bFlatMenu, 0);
	HBRUSH FillBrush = GetSysColorBrush (
		(pdis->itemState & ODS_SELECTED) ? 
		(bFlatMenu ? COLOR_MENUHILIGHT : COLOR_HIGHLIGHT) : COLOR_MENU);
	HBRUSH old_brush = (HBRUSH) SelectObject (pdis->hDC, FillBrush);
	HPEN FillPen = CreatePen (PS_SOLID, 1, GetSysColor (
		(pdis->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_MENU));
	HPEN old_pen = (HPEN) SelectObject (pdis->hDC, FillPen);

	Rectangle (pdis->hDC, pdis->rcItem.left, pdis->rcItem.top,
		pdis->rcItem.right, pdis->rcItem.bottom);

	if ( HBITMAP hThumbnail = m_Preview.GetImage( cx, cy ) )
	{
		if ( HDC hMemDC = CreateCompatibleDC( pdis->hDC ) )
		{
			HBITMAP hOldBitmap = (HBITMAP)SelectObject( hMemDC, hThumbnail );

			//BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
			//if ( ! AlphaBlend( pdis->hDC, rcDraw.left, rcDraw.top, cx, cy, hMemDC, 0, 0, cx, cy, bf ) )
			//{
				BitBlt( pdis->hDC, rcDraw.left, rcDraw.top, cx, cy, hMemDC, 0, 0, SRCCOPY );
			//}

			SelectObject( hMemDC, hOldBitmap );
			DeleteDC( hMemDC );
		}
		DeleteObject( hThumbnail );
	}

	int old_mode = SetBkMode (pdis->hDC, TRANSPARENT);
	COLORREF old_color = SetTextColor (pdis->hDC, GetSysColor (
		(pdis->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));
	
	DrawText (pdis->hDC, m_Preview.GetTitleString (), -1, &rcText,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
	
	SetTextColor (pdis->hDC, old_color);
	SetBkMode (pdis->hDC, old_mode);

	SelectObject (pdis->hDC, old_pen);
	DeleteObject (FillPen);
	SelectObject (pdis->hDC, old_brush);

	*pResult = TRUE;
	return S_OK;
}

// IPersistFile

STDMETHODIMP CThumb::Load(LPCOLESTR wszFile, DWORD /*dwMode*/)
{
	if ( ! wszFile )
	{
		ATLTRACE( "0x%08x::IPersistFile::Load() : E_POINTER\n" );
		return E_POINTER;
	}

	if ( ! _Module.IsGoodFile( (LPCTSTR)CW2CT( wszFile ) ) )
	{
		ATLTRACE( "0x%08x::IPersistFile::Load(\"%s\") : E_FAIL (Bad File)\n", this, (LPCSTR)CW2A( wszFile ) );
		return E_FAIL;
	}

	m_sFilename = wszFile;

	ATLTRACE( "0x%08x::IPersistFile::Load(\"%s\") : S_OK\n", this, (LPCSTR)CW2A( wszFile ) );
	return S_OK;
}

STDMETHODIMP CThumb::GetClassID(LPCLSID pclsid)
{
	ATLTRACE( "0x%08x::IPersist::GetClassID : ", this );

	if ( ! pclsid )
	{
		ATLTRACE ("E_POINTER\n");
		return E_POINTER;
	}

	*pclsid = CLSID_Thumb;
		
	ATLTRACE ("S_OK\n");
	return S_OK;
}

STDMETHODIMP CThumb::IsDirty()
{
	ATLTRACENOTIMPL( _T("IPersistFile::IsDirty") );
}

STDMETHODIMP CThumb::Save(LPCOLESTR, BOOL)
{
	ATLTRACENOTIMPL( _T("IPersistFile::Save") );
}

STDMETHODIMP CThumb::SaveCompleted(LPCOLESTR)
{
	ATLTRACENOTIMPL( _T("IPersistFile::SaveCompleted") );
}

STDMETHODIMP CThumb::GetCurFile(LPOLESTR*)
{
	ATLTRACENOTIMPL( _T("IPersistFile::GetCurFile") );
}

// IInitializeWithStream

//STDMETHODIMP CThumb::Initialize( 
//	/* [in] */ IStream * pstream,
//	/* [in] */ DWORD /*grfMode*/)
//{
//	ATLTRACE( "0x%08x::IInitializeWithStream::Initialize(0x%08x) : ", this, pstream );
//
//	if ( ! pstream )
//	{
//		ATLTRACE( "E_POINTER\n" );
//		return E_POINTER;
//	}
//
//	m_pStream = pstream;
//
//	ATLTRACE( "E_NOTIMPL\n" );
//
//	return E_NOTIMPL;
//}

// IInitializeWithItem

STDMETHODIMP CThumb::Initialize( 
  /* [in] */ __RPC__in_opt IShellItem * psi,
  /* [in] */ DWORD /*grfMode*/)
{
	if ( ! psi  )
	{
		ATLTRACE( "CThumb - IInitializeWithItem::Initialize() : E_POINTER\n" );
		return E_POINTER;
	}

	LPWSTR pszFilePath = NULL;
	HRESULT hr = psi->GetDisplayName( SIGDN_FILESYSPATH, &pszFilePath );
	if ( FAILED( hr ) )
	{
		ATLTRACE( "CThumb - IInitializeWithItem::Initialize() : E_FAIL (Unknown path)\n" );
		return E_FAIL;
	}

	m_sFilename = pszFilePath;

	ATLTRACE( "CThumb - IInitializeWithItem::Initialize(\"%s\") : S_OK\n", (LPCSTR)CW2A( pszFilePath ) );
	CoTaskMemFree( pszFilePath );
	return S_OK;
}

// IInitializeWithFile

STDMETHODIMP CThumb::Initialize(
	/* [in] */ LPCWSTR pszFilePath,
	/* [in] */ DWORD /*grfMode*/)
{
	if ( ! pszFilePath  )
	{
		ATLTRACE( "0x%08x::IInitializeWithFile::Initialize() : E_POINTER\n", this );
		return E_POINTER;
	}

	m_sFilename = pszFilePath;

	ATLTRACE( "0x%08x::IInitializeWithFile::Initialize(\"%s\") : S_OK\n", this, (LPCSTR)CW2A( pszFilePath ) );
	return S_OK;
}

// IThumbnailProvider

STDMETHODIMP CThumb::GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha)
{
	if ( ! phbmp )
	{
		ATLTRACE( "CThumb - IThumbnailProvider::GetThumbnail(%d) : E_POINTER\n", cx );
		return E_POINTER;
	}
	*phbmp = NULL;

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
		//else if ( ! m_pStream )
		//{
		//	hr = m_Preview.LoadImage( m_pStream );
		//}
	}

	*phbmp = m_Preview.GetImage( cx, cx );
	if ( ! *phbmp )
	{
		ATLTRACE( "CThumb - IThumbnailProvider::GetThumbnail(%d) : E_FAIL (Load failed)\n", cx );
		return E_FAIL;
	}

	if ( pdwAlpha )
		*pdwAlpha = WTSAT_UNKNOWN;

#ifdef _DEBUG
	BITMAP bm = {};
	GetObject( *phbmp, sizeof( BITMAP ), &bm );
	ATLTRACE( "CThumb - IThumbnailProvider::GetThumbnail(%d) : S_OK (%d planes, %d bits)\n", cx, bm.bmPlanes, bm.bmBitsPixel );
#endif
	return S_OK;
}

// IPropertyStoreCapabilities

STDMETHODIMP CThumb::IsPropertyWritable( 
	/* [in] */ __RPC__in REFPROPERTYKEY key)
{
#ifdef _DEBUG
	CStringA sPropName;
	CComPtr< IPropertyDescription > pDesc;
	if ( SUCCEEDED( PSGetPropertyDescription( key, IID_PPV_ARGS( &pDesc ) ) ) )
	{
		LPWSTR szPropName = NULL;
		if ( SUCCEEDED( pDesc->GetCanonicalName( &szPropName ) ) )
		{
			sPropName = szPropName;
			CoTaskMemFree( szPropName );
		}
		pDesc.Release();
	}
#endif

	ATLTRACE( "CThumb - IPropertyStoreCapabilities::IsPropertyWritable(\"%s\") : S_FALSE\n", (LPCSTR)sPropName );
	return S_FALSE;
}

// IPropertyStore

STDMETHODIMP CThumb::GetCount( 
	/* [out] */ __RPC__out DWORD* cProps)
{
	if ( ! cProps )
	{
		ATLTRACE( "CThumb - IPropertyStore::GetCount() : E_POINTER\n" );
		return E_POINTER;
	}
	*cProps = 0;

	ATLTRACE( "CThumb - IPropertyStore::GetCount() : S_OK (%u)\n", *cProps );
	return S_OK;
}

STDMETHODIMP CThumb::GetAt( 
	/* [in] */ DWORD /* iProp */,
	/* [out] */ __RPC__out PROPERTYKEY* pkey)
{
	if ( ! pkey )
	{
		ATLTRACE( "CThumb - IPropertyStore::GetAt() : E_POINTER\n" );
		return E_POINTER;
	}

	ATLTRACENOTIMPL( "IPropertyStore::GetAt" );
}

STDMETHODIMP CThumb::GetValue( 
	/* [in] */ __RPC__in REFPROPERTYKEY key,
	/* [out] */ __RPC__out PROPVARIANT* pv)
{
#ifdef _DEBUG
	CStringA sPropName;
	CComPtr< IPropertyDescription > pDesc;
	if ( SUCCEEDED( PSGetPropertyDescription( key, IID_PPV_ARGS( &pDesc ) ) ) )
	{
		LPWSTR szPropName = NULL;
		if ( SUCCEEDED( pDesc->GetCanonicalName( &szPropName ) ) )
		{
			sPropName = szPropName;
			CoTaskMemFree( szPropName );
		}
		pDesc.Release();
	}
#endif

	if ( ! pv )
	{
		ATLTRACE( "CThumb - IPropertyStore::GetValue(\"%s\") : E_POINTER\n", (LPCSTR)sPropName );
		return E_POINTER;
	}
	PropVariantInit( pv );

	if ( SUCCEEDED( m_Preview.LoadInfo( m_sFilename ) ) )
	{
		if ( IsEqualPropertyKey( key, PKEY_DRM_IsProtected ) )
		{
			pv->vt = VT_BOOL;
			pv->boolVal = VARIANT_FALSE;
		}
		else if ( IsEqualPropertyKey( key, PKEY_Image_Dimensions ) )
		{
			CString sDimensions;
			sDimensions.Format( _T("%d x %d"), m_Preview.m_ImageInfo.Width, m_Preview.m_ImageInfo.Height );
			pv->vt = VT_BSTR;
			pv->bstrVal = sDimensions.AllocSysString();
		}
		else if ( IsEqualPropertyKey( key, PKEY_Image_HorizontalSize ) )
		{
			pv->vt = VT_UI4;
			pv->ulVal = m_Preview.m_ImageInfo.Width;
		}
		else if ( IsEqualPropertyKey( key, PKEY_Image_VerticalSize ) )
		{
			pv->vt = VT_UI4;
			pv->ulVal = m_Preview.m_ImageInfo.Height;
		}
		else if ( IsEqualPropertyKey( key, PKEY_Image_BitDepth ) )
		{
			pv->vt = VT_UI4;
			pv->ulVal = m_Preview.m_ImageInfo.ComponentsPerPixel * m_Preview.m_ImageInfo.BitsPerComponent;
		}
		else if ( IsEqualPropertyKey( key, PKEY_PerceivedType ) )
		{
			pv->vt = VT_I4;
			pv->lVal = PERCEIVED_TYPE_IMAGE;
		}
	}

	ATLTRACE( "CThumb - IPropertyStore::GetValue(\"%s\") : S_OK\n", (LPCSTR)sPropName );
	return S_OK;
}

STDMETHODIMP CThumb::SetValue( 
	/* [in] */ __RPC__in REFPROPERTYKEY /* key */,
	/* [in] */ __RPC__in REFPROPVARIANT /* propvar */)
{
	ATLTRACE( "CThumb - IPropertyStore::SetValue() : STG_E_ACCESSDENIED\n" );
	return STG_E_ACCESSDENIED;
}

STDMETHODIMP CThumb::Commit()
{
	ATLTRACE( "CThumb - IPropertyStore::Commit() : STG_E_ACCESSDENIED\n" );
	return STG_E_ACCESSDENIED;
}

// IPreviewHandler

//STDMETHODIMP CThumb::SetWindow(HWND hwnd, const RECT *prc)
//{
//	ATLTRACENOTIMPL( _T("IPreviewHandler::SetWindow") );
//}
//
//STDMETHODIMP CThumb::SetRect(const RECT *prc)
//{
//	ATLTRACENOTIMPL( _T("IPreviewHandler::SetRect") );
//}
//
//STDMETHODIMP CThumb::DoPreview(void)
//{
//	ATLTRACENOTIMPL( _T("IPreviewHandler::DoPreview") );
//}
//
//STDMETHODIMP CThumb::Unload(void)
//{
//	ATLTRACENOTIMPL( _T("IPreviewHandler::Unload") );
//}
//
//STDMETHODIMP CThumb::SetFocus(void)
//{
//	ATLTRACENOTIMPL( _T("IPreviewHandler::SetFocus") );
//}
//
//STDMETHODIMP CThumb::QueryFocus(HWND *phwnd)
//{
//	ATLTRACENOTIMPL( _T("IPreviewHandler::QueryFocus") );
//}
//
//STDMETHODIMP CThumb::TranslateAccelerator(MSG *pmsg)
//{
//	ATLTRACENOTIMPL( _T("IPreviewHandler::TranslateAccelerator") );
//}

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
	ATLTRACE( "CThumb - IExtractImage::GetLocation(%dx%d) : ", (prgSize ? prgSize->cx : 0), (prgSize ? prgSize->cy : 0) );

	if ( ! GetRegValue( _T("EnableThumbs"), 1ul ) )
	{
		ATLTRACE( "E_FAIL (Disabled)\n" );
		return E_FAIL;
	}

	if ( pszPathBuffer )
	{
		*pszPathBuffer = _T('\0');

		if ( ! m_sFilename.IsEmpty() )
		{
			CT2CW szFilenameW( m_sFilename );
			DWORD len = min( (DWORD)( m_sFilename.GetLength() + 1 ), cch );
			wcsncpy_s( pszPathBuffer, cch, (LPCWSTR)szFilenameW, len );
		}
		//else if ( ! m_pStream )
		//{
		//}
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

	HRESULT hr = E_FAIL;
	if ( ! m_sFilename.IsEmpty() )
	{
		hr = m_Preview.LoadImage( m_sFilename, m_cx, m_cy );
	}
	//else if ( ! m_pStream )
	//{
	//}

	return SUCCEEDED( hr ) ? S_OK : E_FAIL;
}

STDMETHODIMP CThumb::Extract ( 
	/* [out] */ HBITMAP *phBmpThumbnail)
{
	if ( ! GetRegValue( _T("EnableThumbs"), 1ul ) )
	{
		ATLTRACE( "CThumb - IExtractImage::Extract() : E_FAIL (Disabled)\n" );
		return E_FAIL;
	}

	if ( ! phBmpThumbnail )
	{
		ATLTRACE( "CThumb - IExtractImage::Extract() : E_POINTER\n" );
		return E_POINTER;
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

// IQueryInfo

STDMETHODIMP CThumb::GetInfoFlags(DWORD* pdwFlags)
{
	if ( pdwFlags )
		*pdwFlags = 0;

	ATLTRACE( "0x%08x::IQueryInfo::GetInfoFlags() : S_OK\n", this );
	return S_OK;
}

STDMETHODIMP CThumb::GetInfoTip(DWORD, LPWSTR* ppwszTip)
{
	if ( ! ppwszTip )
	{
		ATLTRACE( "0x%08x::IQueryInfo::GetInfoTip() : E_POINTER\n", this );
		return E_POINTER;
	}
	*ppwszTip = NULL;

	CComPtr<IMalloc> pIMalloc;
	if ( FAILED( SHGetMalloc ( &pIMalloc ) ) )
	{
		ATLTRACE( "0x%08x::IQueryInfo::GetInfoTip() : E_OUTOFMEMORY\n", this );
		return E_OUTOFMEMORY;
	}

	HRESULT hr = E_FAIL;
	if ( ! m_sFilename.IsEmpty() )
	{
		hr = m_Preview.LoadInfo( m_sFilename );
	}
	//else if ( ! m_pStream )
	//{
	//}

	if ( FAILED( hr ) )
	{
		ATLTRACE( "0x%08x::IQueryInfo::GetInfoTip() : E_FAIL (Load failed)\n", this );
		return E_FAIL;
	}

	CT2W info( m_Preview.GetInfoTipString() );
	size_t len = wcslen( (LPCWSTR)info ) + 1;
	*ppwszTip = (LPWSTR) pIMalloc->Alloc( len * sizeof( WCHAR ) );
	if ( ! *ppwszTip )
	{
		ATLTRACE( "0x%08x::IQueryInfo::GetInfoTip() : E_OUTOFMEMORY\n", this );
		return E_OUTOFMEMORY;
	}
	wcscpy_s( *ppwszTip, len, (LPCWSTR)info );

	ATLTRACE( "0x%08x::IQueryInfo::GetInfoTip() : S_OK\n", this );
	return S_OK;
}

// IExtractIconA

STDMETHODIMP CThumb::GetIconLocation(UINT uFlags, LPSTR szIconFile, UINT cch, int* piIndex, UINT* pwFlags)
{
	WCHAR szIconFileW[ MAX_LONG_PATH ] = {};
	HRESULT hr = GetIconLocation( uFlags, szIconFileW, MAX_LONG_PATH, piIndex, pwFlags );
	strcpy_s( szIconFile, cch, (LPCSTR)CW2A( szIconFileW ) );
	return hr;
}

STDMETHODIMP CThumb::Extract(LPCSTR pszFile, UINT nIconIndex, HICON* phiconLarge, HICON* phiconSmall, UINT nIconSize)
{
	return Extract( (LPCWSTR)CA2W( pszFile ), nIconIndex, phiconLarge, phiconSmall,nIconSize );
}

// IExtractIconW

STDMETHODIMP CThumb::GetIconLocation(UINT /* uFlags */, LPWSTR szIconFile, UINT cch, int* piIndex, UINT* pwFlags)
{
	if ( ! pwFlags || ! piIndex )
	{
		ATLTRACE( "CThumb - IExtractIcon::GetIconLocation() : E_POINTER\n" );
		return E_POINTER;
	}

	// Этот индекс Explorer использует для идентификации и кешировании иконок:
	// Делаем его уникальным
	//LARGE_INTEGER count;
	//QueryPerformanceCounter( &count );
	//*piIndex = (int)count.LowPart;

	if ( szIconFile )
	{
		CT2CW szFilenameW( m_sFilename );
		DWORD len = min( (DWORD)( m_sFilename.GetLength() + 1 ), cch );
		wcsncpy_s( szIconFile, cch, (LPCWSTR)szFilenameW, len );
	}

	*piIndex = (int)CRC32( (LPCSTR)(LPCTSTR)m_sFilename, m_sFilename.GetLength() * sizeof( TCHAR ) );

	*pwFlags = GIL_NOTFILENAME | GIL_PERINSTANCE;

	ATLTRACE( "CThumb - IExtractIcon::GetIconLocation(\"%s\",0x%08x,0x%08x) : S_OK\n", (LPCSTR)CT2A( m_sFilename ), (DWORD)*piIndex, *pwFlags );
	return S_OK;
}

STDMETHODIMP CThumb::Extract(LPCWSTR /*pszFile*/, UINT nIconIndex, HICON* phiconLarge, HICON* phiconSmall, UINT nIconSize)
{
	nIconIndex;

	if ( phiconLarge ) *phiconLarge = NULL;
	if ( phiconSmall ) *phiconSmall = NULL;

	UINT cxLarge = ( phiconLarge ? LOWORD( nIconSize ) : 0 );
	UINT cxSmall = ( phiconSmall ? HIWORD( nIconSize ) : 0 );
	m_cx = m_cy = max( cxLarge, cxSmall );
	if ( ! m_cx )
	{
		ATLTRACE( "CThumb - IExtractIcon::Extract() : E_FAIL (No size)\n" );
		return E_FAIL;
	}

	ATLTRACE( "CThumb - IExtractIcon::Extract(0x%08x,%d,%d) : ", nIconIndex, cxLarge, cxSmall );

	HRESULT hr = E_FAIL;
	if ( ! m_Preview )
	{
		if ( ! m_sFilename.IsEmpty() )
		{
			hr = m_Preview.LoadImage( m_sFilename, m_cx, m_cy );
		}
	}
	if ( FAILED( hr ) )
	{
		// Attempt to load default icon
		CString sExt = PathFindExtension( m_sFilename );
		if ( sExt.IsEmpty() )
		{
			// No extension
			ATLTRACE( "CThumb - IExtractIcon::Extract() : E_FAIL (No extension)\n" );
			return E_FAIL;
		}

		CString sDefaultIcon;
		CString sDefaultKey = GetRegValue( _T(""), _T(""), sExt, HKEY_CLASSES_ROOT );
		if ( sDefaultKey.IsEmpty() )
		{
			sDefaultIcon = GetRegValue( _T(""), _T(""), sExt + _T("\\DefaultIcon"), HKEY_CLASSES_ROOT );
		}
		else
		{
			sDefaultIcon = GetRegValue( _T(""), _T(""), sDefaultKey + _T("\\DefaultIcon"), HKEY_CLASSES_ROOT );
		}
		if ( sDefaultIcon.IsEmpty() )
		{
			// No icon
			ATLTRACE( "CThumb - IExtractIcon::Extract() : E_FAIL (No icon)\n" );
			return E_FAIL;
		}

		if ( ! LoadIcon( sDefaultIcon,
			( cxSmall == 16 ) ? phiconSmall : ( ( cxLarge == 16 ) ? phiconLarge : NULL ),
			( cxSmall == 32 ) ? phiconSmall : ( ( cxLarge == 32 ) ? phiconLarge : NULL ),
			( cxSmall == 48 ) ? phiconSmall : ( ( cxLarge == 48 ) ? phiconLarge : NULL ) ) )
		{
			// Found no icon
			ATLTRACE( "CThumb - IExtractIcon::Extract() : E_FAIL (Found no icon)\n" );
			return E_FAIL;
		}

		ATLTRACE( "CThumb - IExtractIcon::Extract() : S_OK (Default)\n" );
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

	ATLTRACE( "CThumb - IExtractIcon::Extract() : S_OK\n" );
	return S_OK;
}

// IDataObject

//#define PRINT_FORMAT1(fmt) if(pformatetcIn->cfFormat==RegisterClipboardFormat(fmt)) \
//	{ ATLTRACE ("%s\n", #fmt); } else
//#define PRINT_FORMAT2(fmt) if(pformatetcIn->cfFormat==(fmt)) \
//	{ ATLTRACE ("%s\n", #fmt); } else
//#define PRINT_FORMAT_END \
//	{ ATLTRACE ("no CF_\n"); }
//#define PRINT_FORMAT_ALL \
//	{ \
//		TCHAR fm [128]; fm [0] = _T('\0');\
//		GetClipboardFormatName (pformatetcIn->cfFormat, fm, sizeof (fm)); \
//		ATLTRACE ("0x%08x \"%s\" ", pformatetcIn->cfFormat, fm);\
//	} \
//	PRINT_FORMAT1(CFSTR_SHELLIDLIST) \
//	PRINT_FORMAT1(CFSTR_SHELLIDLISTOFFSET) \
//	PRINT_FORMAT1(CFSTR_NETRESOURCES) \
//	PRINT_FORMAT1(CFSTR_FILEDESCRIPTORA) \
//	PRINT_FORMAT1(CFSTR_FILEDESCRIPTORW) \
//	PRINT_FORMAT1(CFSTR_FILECONTENTS) \
//	PRINT_FORMAT1(CFSTR_FILENAMEA) \
//	PRINT_FORMAT1(CFSTR_FILENAMEW) \
//	PRINT_FORMAT1(CFSTR_PRINTERGROUP) \
//	PRINT_FORMAT1(CFSTR_FILENAMEMAPA) \
//	PRINT_FORMAT1(CFSTR_FILENAMEMAPW) \
//	PRINT_FORMAT1(CFSTR_SHELLURL) \
//	PRINT_FORMAT1(CFSTR_INETURLA) \
//	PRINT_FORMAT1(CFSTR_INETURLW) \
//	PRINT_FORMAT1(CFSTR_PREFERREDDROPEFFECT) \
//	PRINT_FORMAT1(CFSTR_PERFORMEDDROPEFFECT) \
//	PRINT_FORMAT1(CFSTR_PASTESUCCEEDED) \
//	PRINT_FORMAT1(CFSTR_INDRAGLOOP) \
//	PRINT_FORMAT1(CFSTR_DRAGCONTEXT) \
//	PRINT_FORMAT1(CFSTR_MOUNTEDVOLUME) \
//	PRINT_FORMAT1(CFSTR_PERSISTEDDATAOBJECT) \
//	PRINT_FORMAT1(CFSTR_TARGETCLSID) \
//	PRINT_FORMAT1(CFSTR_LOGICALPERFORMEDDROPEFFECT) \
//	PRINT_FORMAT1(CFSTR_AUTOPLAY_SHELLIDLISTS) \
//	PRINT_FORMAT1(CF_RTF) \
//	PRINT_FORMAT1(CF_RTFNOOBJS) \
//	PRINT_FORMAT1(CF_RETEXTOBJ) \
//	PRINT_FORMAT2(CF_TEXT) \
//	PRINT_FORMAT2(CF_BITMAP) \
//	PRINT_FORMAT2(CF_METAFILEPICT) \
//	PRINT_FORMAT2(CF_SYLK) \
//	PRINT_FORMAT2(CF_DIF) \
//	PRINT_FORMAT2(CF_TIFF) \
//	PRINT_FORMAT2(CF_OEMTEXT) \
//	PRINT_FORMAT2(CF_DIB) \
//	PRINT_FORMAT2(CF_PALETTE) \
//	PRINT_FORMAT2(CF_PENDATA) \
//	PRINT_FORMAT2(CF_RIFF) \
//	PRINT_FORMAT2(CF_WAVE) \
//	PRINT_FORMAT2(CF_UNICODETEXT) \
//	PRINT_FORMAT2(CF_ENHMETAFILE) \
//	PRINT_FORMAT2(CF_HDROP) \
//	PRINT_FORMAT2(CF_LOCALE) \
//	PRINT_FORMAT2(CF_DIBV5) \
//	PRINT_FORMAT2(CF_MAX) \
//	PRINT_FORMAT2(CF_OWNERDISPLAY) \
//	PRINT_FORMAT2(CF_DSPTEXT) \
//	PRINT_FORMAT2(CF_DSPBITMAP) \
//	PRINT_FORMAT2(CF_DSPMETAFILEPICT) \
//	PRINT_FORMAT2(CF_DSPENHMETAFILE) \
//	PRINT_FORMAT2(CF_PRIVATEFIRST) \
//	PRINT_FORMAT2(CF_PRIVATELAST) \
//	PRINT_FORMAT2(CF_GDIOBJFIRST) \
//	PRINT_FORMAT2(CF_GDIOBJLAST) \
//	PRINT_FORMAT_END
//
//STDMETHODIMP CThumb::GetData ( 
//	/* [unique][in] */ FORMATETC* pformatetcIn,
//	/* [out] */ STGMEDIUM* /*pmedium*/)
//{
//	ATLTRACE ("IDataObject::GetData() : ");
//	PRINT_FORMAT_ALL
//	return E_INVALIDARG;
//	/*startActualLoad ();
//	stopActualLoad ();
//	if (m_Status == LS_LOADED) {
//	pmedium->tymed = TYMED_HGLOBAL;
//	pmedium->hGlobal = GlobalAlloc (GHND, sizeof (CLSID));
//	pmedium->pUnkForRelease = NULL;
//	char* dst = (char*) GlobalLock (pmedium->hGlobal);
//	CLSID clsid = { 0x4A34B3E3,0xF50E,0x4FF6,0x89,0x79,0x7E,0x41,0x76,0x46,0x6F,0xF2 };
//	CopyMemory (dst, &clsid, sizeof (CLSID));
//	GlobalUnlock (pmedium->hGlobal);
//	return S_OK;
//	}
//	return E_FAIL;*/
//}
//
//STDMETHODIMP CThumb::GetDataHere ( 
//	/* [unique][in] */ FORMATETC* /* pformatetc */,
//	/* [out][in] */ STGMEDIUM* /* pmedium */)
//{
//	ATLTRACENOTIMPL ("IDataObject::GetDataHere");
//}
//
//STDMETHODIMP CThumb::QueryGetData ( 
//	/* [unique][in] */ FORMATETC* pformatetcIn)
//{
//	ATLTRACE ("IDataObject::QueryGetData() : ");
//	PRINT_FORMAT_ALL
//	return E_INVALIDARG;
//}
//
//STDMETHODIMP CThumb::GetCanonicalFormatEtc ( 
//	/* [unique][in] */ FORMATETC* /* pformatectIn */,
//	/* [out] */ FORMATETC* /* pformatetcOut */)
//{
//	ATLTRACENOTIMPL ("IDataObject::GetCanonicalFormatEtc");
//}
//
//STDMETHODIMP CThumb::SetData ( 
//	/* [unique][in] */ FORMATETC* pformatetcIn,
//	/* [unique][in] */ STGMEDIUM* /*pmedium*/,
//	/* [in] */ BOOL fRelease)
//{
//	ATLTRACE ("IDataObject::SetData(fRelease=%d) : ", fRelease);
//	PRINT_FORMAT_ALL
//	/*FILEDESCRIPTOR* src = (FILEDESCRIPTOR*) GlobalLock (pmedium->hGlobal);	
//	SIZE_T len = GlobalSize (pmedium->hGlobal);
//	GlobalUnlock (pmedium->hGlobal);
//	if (fRelease)
//		GlobalFree (pmedium->hGlobal);*/
//	return E_NOTIMPL;
//}
//
//static FORMATETC fmt [1] = {
//	{ CF_BITMAP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL }
//};
//
//class FORMATETCCopy
//{
//public:
//	static void init (FORMATETC* /*p*/)
//	{
//	}
//	static HRESULT copy (FORMATETC* pTo, const FORMATETC* pFrom)
//	{
//		CopyMemory (pTo, pFrom, sizeof (FORMATETC));
//		return S_OK;
//	}
//	static void destroy (FORMATETC* /*p*/)
//	{
//	}
//};
//
//STDMETHODIMP CThumb::EnumFormatEtc ( 
//	/* [in] */ DWORD dwDirection,
//	/* [out] */ IEnumFORMATETC** ppenumFormatEtc)
//{
//	ATLTRACE ("IDataObject::EnumFormatEtc(dwDirection=%d) : ", dwDirection);
//	if (!ppenumFormatEtc) {
//		ATLTRACE("E_POINTER\n");
//		return E_POINTER;
//	}
//	// Создание объекта-перечислителя
//	typedef CComEnum < IEnumFORMATETC, &IID_IEnumFORMATETC,
//		FORMATETC, FORMATETCCopy > EnumFORMATETCType;
//	typedef CComObject < EnumFORMATETCType > EnumFORMATETC;
//	EnumFORMATETC* pEnum = NULL;
//	EnumFORMATETC::CreateInstance (&pEnum);
//	pEnum->Init ((FORMATETC*) (&fmt[0]), (FORMATETC*) (&fmt[1]), NULL);
//	ATLTRACE("S_OK\n");
//	return pEnum->QueryInterface (IID_IEnumFORMATETC, (void**) ppenumFormatEtc);
//}
//
//STDMETHODIMP CThumb::DAdvise ( 
//	/* [in] */ FORMATETC* /* pformatetc */,
//	/* [in] */ DWORD /* advf */,
//	/* [unique][in] */ IAdviseSink* /* pAdvSink */,
//	/* [out] */ DWORD* /* pdwConnection */)
//{
//	ATLTRACENOTIMPL ("IDataObject::DAdvise");
//}
//
//STDMETHODIMP CThumb::DUnadvise ( 
//	/* [in] */ DWORD /* dwConnection */)
//{
//	ATLTRACENOTIMPL ("IDataObject::DUnadvise");
//}
//
//STDMETHODIMP CThumb::EnumDAdvise ( 
//	/* [out] */ IEnumSTATDATA** /* ppenumAdvise */)
//{
//	ATLTRACENOTIMPL ("IDataObject::EnumDAdvise");
//}

// IImageDecodeFilter

//STDMETHODIMP CThumb::Initialize(IImageDecodeEventSink* pEventSink)    
//{
//	if ( ! pEventSink )
//	{
//		ATLTRACE( "CThumb - IImageDecodeFilter::Initialize() : E_POINTER\n" );
//		return E_POINTER;
//	}
//
//	m_pEventSink = pEventSink;
//
//	DWORD dwEvents = 0;
//	ULONG nFormats = 0;
//    BFID *pFormats = NULL;
//	HRESULT hr = m_pEventSink->OnBeginDecode( &dwEvents, &nFormats, &pFormats );
//	if (FAILED (hr))
//	{
//		ATLTRACE( "CThumb - IImageDecodeFilter : OnBeginDecode error 0x%08x\n", hr);
//		m_pEventSink.Release();
//		return hr;
//	}
//	ATLTRACE( "CThumb - IImageDecodeFilter : OnBeginDecode returns: events=0x%08x, formats=%d\n", dwEvents, nFormats);
//	ULONG i = 0;
//	bool bOk = false;
//	for ( ; i < nFormats; ++i )
//	{
//		if ( IsEqualGUID( BFID_MONOCHROME, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_MONOCHROME\n" );
//		}
//		else if ( IsEqualGUID( BFID_RGB_4, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGB_4\n" );
//		}
//		else if ( IsEqualGUID( BFID_RGB_8, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGB_8\n" );
//		}
//		else if ( IsEqualGUID( BFID_RGB_555, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGB_555\n" );
//		}
//		else if ( IsEqualGUID( BFID_RGB_565, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGB_565\n" );
//		}
//		else if ( IsEqualGUID( BFID_RGB_24, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGB_24\n" );
//			bOk = true;
//		}
//		else if ( IsEqualGUID( BFID_RGB_32, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGB_32\n" );
//		}
//		else if ( IsEqualGUID( BFID_RGBA_32, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGBA_32\n" );
//		}
//		else if ( IsEqualGUID( BFID_GRAY_8, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_GRAY_8\n" );
//		}
//		else if ( IsEqualGUID( BFID_GRAY_16, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_GRAY_16\n" );
//		}
//		else
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found unknown format\n" );
//		}
//	}
//	CoTaskMemFree( pFormats );
//
//	if ( ! bOk )
//	{
//		ATLTRACE( "CThumb - IImageDecodeFilter : OnBeginDecode cannot find RGB_24 format\n");
//		return E_FAIL;
//	}
//
//	return S_OK;
//}
//
//STDMETHODIMP CThumb::Process(IStream* pStream)    
//{
//	HRESULT hr;
//
//	const ULONG chunk = 1024;
//	ULONG total = 0;
//	CAtlArray< unsigned char > data;
//	for (;;)
//	{
//		if ( ! data.SetCount( total + chunk ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter::Process() : Out of memory\n" );
//			return E_OUTOFMEMORY;
//		}
//
//		ULONG readed = 0;
//		hr = pStream->Read( data.GetData() + total, chunk, &readed );
//		total += readed;
//		if ( FAILED( hr ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter::Process() : Read error 0x%08x\n", hr );
//			return hr;
//		}
//		if ( hr == S_FALSE )
//			break;
//	}
//	
//	hr = m_pEventSink->OnBitsComplete();
//	ATLTRACE( "CThumb - IImageDecodeFilter::Process() : Readed %u bytes\n", total );
//
//	GFL_BITMAP* hGflBitmap = NULL;
//	hr = _Module.LoadBitmapFromMemory( data.GetData (), total, &hGflBitmap );
//	if ( FAILED( hr ) )
//	{
//		ATLTRACE( "CThumb - IImageDecodeFilter::Process() : Load error 0x%08x\n", hr );
//		return hr;
//	}
//	ATLTRACE( "CThumb - IImageDecodeFilter::Process() : Loaded as %dx%d bitmap (%d bpl)\n", hGflBitmap->Width, hGflBitmap->Height, hGflBitmap->BytesPerLine );
//
//	CComPtr< IDirectDrawSurface > pIDirectDrawSurface;
//	hr = m_pEventSink->GetSurface( hGflBitmap->Width, hGflBitmap->Height,
//		BFID_RGB_24, 1, IMGDECODE_HINT_TOPDOWN | IMGDECODE_HINT_FULLWIDTH,
//		(IUnknown**) &pIDirectDrawSurface );
//	if (FAILED (hr))
//	{
//		ATLTRACE ("CThumb - IImageDecodeFilter::Process() : m_spEventSink->GetSurface error 0x%08x\n", hr );
//		_Module.FreeBitmap( hGflBitmap );
//		return hr;
//	}
//
//	DDSURFACEDESC desc = { sizeof( DDSURFACEDESC ) };
//	RECT rc = { 0, 0, hGflBitmap->Width, hGflBitmap->Height };
//	hr = pIDirectDrawSurface->Lock( &rc, &desc, DDLOCK_WAIT, NULL );
//	if (FAILED (hr))
//	{
//		ATLTRACE ("CThumb - IImageDecodeFilter::Process() : pIDirectDrawSurface->Lock error 0x%08x\n", hr);
//		_Module.FreeBitmap( hGflBitmap );
//		return hr;
//	}
//
//	for ( int line = 0; line < hGflBitmap->Height; ++line )
//	{
//		char* dst = (char*)desc.lpSurface + line * desc.lPitch;
//		char* src = (char*)hGflBitmap->Data + line * hGflBitmap->BytesPerLine;
//		for ( int p = 0; p < hGflBitmap->Width; ++p, dst += 3, src += hGflBitmap->BytesPerPixel )
//		{
//			// RGB -> BGR
//			dst[0] = src[2];
//			dst[1] = src[1];
//			dst[2] = src[0];
//		}
//	}
//
//	hr = pIDirectDrawSurface->Unlock( &desc );
//	if (FAILED (hr))
//	{
//		ATLTRACE ("CThumb - IImageDecodeFilter::Process() : pIDirectDrawSurface->Unlock error 0x%08x\n", hr);
//		_Module.FreeBitmap( hGflBitmap );
//		return hr;
//	}
//
//	m_pEventSink->OnDecodeComplete( S_OK );
//
//	ATLTRACE( "CThumb - IImageDecodeFilter::Process() : OK\n" );
//	_Module.FreeBitmap( hGflBitmap );
//	return hr;
//}
//
//STDMETHODIMP CThumb::Terminate(HRESULT hrStatus)
//{
//	if ( m_pEventSink )
//	{
//		m_pEventSink->OnDecodeComplete( hrStatus );
//		m_pEventSink.Release();
//	}
//
//	return S_OK;
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
		CString foo;
		foo.LoadString( IDS_CACHE );
		size_t len = ( foo.GetLength() + 1 ) * sizeof( TCHAR );
		*ppwszDisplayName = (LPWSTR)CoTaskMemAlloc( len );
		CopyMemory( *ppwszDisplayName, (LPCTSTR)foo, len );
	}

	if ( ppwszDescription )
	{
		CString foo;
		foo.LoadString( IDS_DESCRIPTION );
		size_t len = ( foo.GetLength() + 1 ) * sizeof( TCHAR );
		*ppwszDescription = (LPWSTR)CoTaskMemAlloc( len );
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
	/* [out] */ __RPC__out DWORD* /*pdwFlags*/)
{
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
