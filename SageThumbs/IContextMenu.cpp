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
#define ID_THUMBNAIL_INFO_ITEM		13
#define ID_END_ITEM					14

static const LPCTSTR szVerbs[ ID_END_ITEM ] =
{
	_T("submenu"),
	_T("clipboard_image"),
	_T("thumbnail"),
	_T("options"),
	_T("wallpaper_stretch"),
	_T("wallpaper_tile"),
	_T("wallpaper_center"),
	_T("mail_image"),
	_T("mail_thumbnail"),
	_T("convert_jpg"),
	_T("convert_gif"),
	_T("convert_bmp"),
	_T("convert_png"),
	_T("info")
};

STDMETHODIMP CThumb::QueryContextMenu(HMENU hMenu, UINT uIndex, UINT uidCmdFirst, UINT uidCmdLast, UINT uFlags)
{
	// If the flags include CMF_DEFAULTONLY then we shouldn't do anything.
	if ( uFlags & CMF_DEFAULTONLY )
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : S_OK (Bypass)\n" );
		return MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_NULL, 0 );
	}

	bool bEnableMenu = GetRegValue( _T("EnableMenu"), 1ul ) != 0;
	if ( ! bEnableMenu )
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Menu disabled)\n" );
		// Меню выключено
		return E_FAIL;
	}

	// Проверка на нехватку идентификаторов
	if ( uidCmdFirst + ID_END_ITEM > uidCmdLast )
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (No free IDs)\n" );
		return E_FAIL;
	}

	bool bPreviewInSubMenu = GetRegValue( _T("SubMenu"), 1ul ) != 0;
	bool bSingleFile = ( m_Filenames.GetCount () == 1 );

	int nPos = 0;

	// Creating submenu items
	HMENU hSubMenu = CreateMenu ();

	if ( bSingleFile )
	{
		// Clipboard operation items
		if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING, uidCmdFirst + ID_CLIPBOARD_ITEM, LoadString( IDS_CLIPBOARD ) ) )
		{
			ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
			return E_FAIL;
		}
		if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_SEPARATOR, 0, 0))
		{
			ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
			return E_FAIL;
		}

		// Wallpaper operation items
		if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING, uidCmdFirst + ID_WALLPAPER_STRETCH_ITEM, LoadString( IDS_WALLPAPER_STRETCH ) ) )
		{
			ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
			return E_FAIL;
		}
		if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING, uidCmdFirst + ID_WALLPAPER_TILE_ITEM, LoadString( IDS_WALLPAPER_TILE ) ) )
		{
			ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
			return E_FAIL;
		}
		if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING, uidCmdFirst + ID_WALLPAPER_CENTER_ITEM, LoadString( IDS_WALLPAPER_CENTER ) ) )
		{
			ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
			return E_FAIL;
		}
		if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_SEPARATOR, 0, 0))
		{
			ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
			return E_FAIL;
		}
	}

	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING, uidCmdFirst + ID_MAIL_IMAGE_ITEM, LoadString( IDS_MAIL_IMAGE ) ) )
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
		return E_FAIL;
	}
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING, uidCmdFirst + ID_MAIL_THUMBNAIL_ITEM, LoadString( IDS_MAIL_THUMBNAIL ) ) )
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
		return E_FAIL;
	}
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_SEPARATOR, 0, 0))
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
		return E_FAIL;
	}
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING, uidCmdFirst + ID_CONVERT_JPG_ITEM, LoadString( IDS_CONVERT_JPG ) ) )
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
		return E_FAIL;
	}
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING,uidCmdFirst + ID_CONVERT_GIF_ITEM, LoadString( IDS_CONVERT_GIF ) ) )
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
		return E_FAIL;
	}
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING, uidCmdFirst + ID_CONVERT_BMP_ITEM, LoadString( IDS_CONVERT_BMP ) ) )
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
		return E_FAIL;
	}
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING, uidCmdFirst + ID_CONVERT_PNG_ITEM, LoadString( IDS_CONVERT_PNG ) ) )
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
		return E_FAIL;
	}
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_SEPARATOR, 0, 0))
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
		return E_FAIL;
	}
	if ( ! InsertMenu( hSubMenu, nPos++, MF_BYPOSITION | MF_STRING, uidCmdFirst + ID_OPTIONS_ITEM, LoadString( IDS_OPTIONS ) ) )
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu item %d)\n", nPos );
		return E_FAIL;
	}

	// Creating main menu items
	if ( ! InsertMenu( hMenu, uIndex++, MF_BYPOSITION | MF_SEPARATOR, 0, 0 ) )
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu separator)\n" );
		return E_FAIL;
	}

	// Preview menu item
	if ( bSingleFile )
	{
		// Загрузка файла
		DWORD width = GetRegValue( _T("Width"), THUMB_STORE_SIZE );
		DWORD height = GetRegValue( _T("Height"), THUMB_STORE_SIZE );
		m_Preview.LoadImage( m_Filenames.GetHead(), width, height );

		if ( m_Preview )
		{
			// Store the menu item's ID so we can check against it later when
			// WM_MEASUREITEM/WM_DRAWITEM are sent.
			m_uOurItemID = uidCmdFirst + ID_THUMBNAIL_ITEM;
			/*if ( IsThemeActive() )
			{
				HBITMAP hPreview = m_Preview.GetImage( width, height );
				if ( ! InsertMenu( ( bPreviewInSubMenu ? hSubMenu : hMenu ), ( bPreviewInSubMenu ? 0 : uIndex++ ), MF_BYPOSITION | MF_BITMAP, m_uOurItemID, (LPCTSTR)hPreview ) )
				{
					ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert image menu item)\n" );
					return E_FAIL;
				}
			}
			else*/
			{
				MENUITEMINFO mii = {};
				mii.cbSize = sizeof (MENUITEMINFO);
				mii.fMask  = MIIM_ID | MIIM_TYPE;
				mii.fType  = MFT_OWNERDRAW;
				mii.wID    = m_uOurItemID;
				if ( ! InsertMenuItem( ( bPreviewInSubMenu ? hSubMenu : hMenu ), ( bPreviewInSubMenu ? 0 : uIndex++ ), TRUE, &mii ) )
				{
					ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert image menu item)\n" );
					return E_FAIL;
				}
			}

			if ( ! InsertMenu( ( bPreviewInSubMenu ? hSubMenu : hMenu ), ( bPreviewInSubMenu ? 1:  uIndex++ ),
				MF_BYPOSITION | MF_STRING | MF_DISABLED, uidCmdFirst + ID_THUMBNAIL_INFO_ITEM, _T("( ") + m_Preview.GetTitleString() + _T(" )") ) )
			{
				ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert image menu item)\n" );
				return E_FAIL;
			}
		}
	}

	CString sAppName = LoadString( IDS_PROJNAME );
	MENUITEMINFO mii = { sizeof( MENUITEMINFO ) };
	mii.fMask  = MIIM_STRING | MIIM_SUBMENU | MIIM_ID | MIIM_CHECKMARKS;
	mii.wID = uidCmdFirst + ID_SUBMENU_ITEM;
	mii.hSubMenu = hSubMenu;
	mii.dwTypeData = (LPTSTR)(LPCTSTR)sAppName;
	mii.cch = (UINT)sAppName.GetLength();
	mii.hbmpChecked = mii.hbmpUnchecked = (HBITMAP)LoadImage( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDR_SAGETHUMBS ), IMAGE_BITMAP, 16, 16, LR_SHARED );
	if ( ! InsertMenuItem ( hMenu, uIndex++, TRUE, &mii ) )
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert main menu)\n" );
		return E_FAIL;
	}

	if ( ! InsertMenu ( hMenu, uIndex++, MF_BYPOSITION | MF_SEPARATOR, 0, 0 ) )
	{
		ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : E_FAIL (Failed to insert menu separator)\n" );
		return E_FAIL;
	}

	ATLTRACE( "CThumb - IContextMenu::QueryContextMenu() : S_OK\n" );
	return MAKE_HRESULT (SEVERITY_SUCCESS, FACILITY_NULL, ID_END_ITEM);
}

STDMETHODIMP CThumb::GetCommandString (
	UINT_PTR uCmd, UINT uFlags, UINT* /*puReserved*/,
	LPSTR pszName, UINT cchMax )
{
	CString tmp;
	switch ( uFlags )
	{
	case GCS_VERBA:
	case GCS_VERBW:
		if ( uCmd < ID_END_ITEM )
		{
			tmp = CString( _T("SageThumbs.") ) + szVerbs[ uCmd ];
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
		case ID_THUMBNAIL_INFO_ITEM:
			tmp = m_Preview.GetMenuTipString ();
			break;
		case ID_OPTIONS_ITEM:
			tmp = LoadString (IDS_OPTIONS_HELP);
			break;
		case ID_CLIPBOARD_ITEM:
			tmp = LoadString (IDS_CLIPBOARD);
			break;
		case ID_WALLPAPER_STRETCH_ITEM:
			tmp = LoadString (IDS_WALLPAPER_STRETCH);
			break;
		case ID_WALLPAPER_TILE_ITEM:
			tmp = LoadString (IDS_WALLPAPER_TILE);
			break;
		case ID_WALLPAPER_CENTER_ITEM:
			tmp = LoadString (IDS_WALLPAPER_CENTER);
			break;
		case ID_MAIL_IMAGE_ITEM:
			tmp = LoadString (IDS_MAIL_IMAGE);
			break;
		case ID_MAIL_THUMBNAIL_ITEM:
			tmp = LoadString (IDS_MAIL_THUMBNAIL);
			break;
		case ID_CONVERT_JPG_ITEM:
			tmp = LoadString (IDS_CONVERT_JPG);
			break;
		case ID_CONVERT_GIF_ITEM:
			tmp = LoadString (IDS_CONVERT_GIF);
			break;
		case ID_CONVERT_BMP_ITEM:
			tmp = LoadString (IDS_CONVERT_BMP);
			break;
		case ID_CONVERT_PNG_ITEM:
			tmp = LoadString (IDS_CONVERT_PNG);
			break;
		default:
			ATLTRACE( "CThumb - IContextMenu::GetCommandString(%d, %d, 0x%08x \"%s\", %d) E_INVALIDARG\n", uCmd, uFlags, pszName, pszName, cchMax);
			return E_INVALIDARG;
		}
		break;

	case GCS_VALIDATEA:
	case GCS_VALIDATEW:
		return S_OK;

	default:
		ATLTRACE( "CThumb - IContextMenu::GetCommandString(%d, %d, 0x%08x \"%s\", %d) E_INVALIDARG\n", uCmd, uFlags, pszName, pszName, cchMax);
		return E_INVALIDARG;
	}

	if ( uFlags & GCS_UNICODE )
		wcsncpy_s( (LPWSTR)pszName, cchMax, (LPCWSTR)CT2W( tmp ), cchMax );
	else
		strncpy_s( (LPSTR)pszName, cchMax, (LPCSTR)CT2A( tmp ), cchMax );

	return S_OK;
}

void CThumb::ConvertTo(HWND hWnd, int ext)
{
	CComPtr< IProgressDialog > pProgress;
	HRESULT hr = pProgress.CoCreateInstance( CLSID_ProgressDialog );
	if ( SUCCEEDED( hr ) )
	{
		pProgress->SetTitle( _Module.GetAppName() );
		pProgress->SetLine( 1, LoadString( IDS_CONVERTING ), FALSE, NULL );
		pProgress->StartProgressDialog( hWnd, NULL, PROGDLG_NORMAL | PROGDLG_AUTOTIME, NULL );
	}
	DWORD total = (DWORD)m_Filenames.GetCount(), counter = 0;

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

	for ( POSITION pos = m_Filenames.GetHeadPosition (); pos ; ++counter )
	{
		CString filename( m_Filenames.GetNext( pos ) );

		if ( pProgress )
		{
			pProgress->SetLine( 2, filename, TRUE, NULL );
			pProgress->SetProgress( counter, total );
			Sleep( 10 );
			if ( pProgress->HasUserCancelled() )
			{
				pProgress->StopProgressDialog();
				return;
			}
		}

		GFL_BITMAP* hBitmap = NULL;
		if ( SUCCEEDED( _Module.LoadGFLBitmap( filename, &hBitmap ) ) )
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
			_Module.FreeBitmap( hBitmap );
		}
		else
		{
			_Module.MsgBox( hWnd, IDS_ERR_OPEN );
			break;
		}
	}

	if ( pProgress )
	{
		pProgress->SetLine( 2, _T(""), TRUE, NULL );
		pProgress->SetProgress( total, total );
		Sleep( 1000 );
		pProgress->StopProgressDialog();
	}
}

void CThumb::SetWallpaper(HWND hWnd, WORD reason)
{
	CString filename( m_Filenames.GetHead() );

	GFL_BITMAP* hBitmap = NULL;
	if ( SUCCEEDED( _Module.LoadGFLBitmap( filename, &hBitmap ) ) )
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
		_Module.FreeBitmap( hBitmap );
	}
	else
		_Module.MsgBox( hWnd, IDS_ERR_OPEN );
}

void CThumb::SendByMail(HWND hWnd, WORD reason)
{
	CComPtr< IProgressDialog > pProgress;
	HRESULT hr = pProgress.CoCreateInstance( CLSID_ProgressDialog );
	if ( SUCCEEDED( hr ) )
	{
		pProgress->SetTitle( _Module.GetAppName() );
		pProgress->SetLine( 1, LoadString( IDS_SENDING ), FALSE, NULL );
		pProgress->StartProgressDialog( hWnd, NULL, PROGDLG_NORMAL | PROGDLG_AUTOTIME, NULL );
	}
	DWORD total = (DWORD)m_Filenames.GetCount(), counter = 0;

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
			for ( POSITION pos = m_Filenames.GetHeadPosition () ; pos ; ++counter )
			{
				CString filename( m_Filenames.GetNext( pos ) );

				if ( pProgress )
				{
					pProgress->SetLine( 2, filename, TRUE, NULL );
					pProgress->SetProgress( counter, total );
					Sleep( 10 );
					if ( pProgress->HasUserCancelled() )
					{
						pProgress->StopProgressDialog();
						FreeLibrary (hLibrary);
						return;
					}
				}

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
					ULONG err = pMAPISendMail (0, (ULONG_PTR)hWnd, &mm,
						MAPI_DIALOG | MAPI_LOGON_UI | MAPI_NEW_SESSION, 0);
					if (MAPI_E_USER_ABORT != err && SUCCESS_SUCCESS != err)
						_Module.MsgBox( hWnd, IDS_ERR_MAIL );
					delete [] mfd;
				}
				else
					_Module.MsgBox( hWnd, IDS_ERR_MAIL );

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
				_Module.MsgBox( hWnd, IDS_ERR_NOTHING );
		}
		else
			_Module.MsgBox( hWnd, IDS_ERR_MAIL );

		FreeLibrary (hLibrary);
	}
	else
		_Module.MsgBox( hWnd, IDS_ERR_MAIL );

	if ( pProgress )
	{
		pProgress->SetLine( 2, _T(""), TRUE, NULL );
		pProgress->SetProgress( total, total );
		Sleep( 1000 );
		pProgress->StopProgressDialog();
	}
}

void CThumb::CopyToClipboard(HWND hwnd)
{
	if ( HBITMAP hBitmap = m_Preview.GetImage( m_cx, m_cy ) )
	{
		if ( OpenClipboard ( hwnd ) )
		{
			EmptyClipboard();
			
			if ( SetClipboardData( CF_BITMAP, hBitmap ) )
			{
				// OK
			}
			else
				_Module.MsgBox( hwnd, IDS_ERR_CLIPBOARD );

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
	// If lpVerb really points to a string, ignore this function call and bail out.
	if (0 != HIWORD (pInfo->lpVerb))
	{
		ATLTRACE( "CThumb - IContextMenu::InvokeCommand() : E_INVALIDARG\n" );
		return E_INVALIDARG;
	}

	// The command ID must be 0 since we only have one menu item.
	switch (LOWORD (pInfo->lpVerb))
	{
	case ID_THUMBNAIL_ITEM:
		// Open the bitmap in the default paint program.
		SHAddToRecentDocs( SHARD_PATH, m_Filenames.GetHead() );
		ShellExecute( pInfo->hwnd, _T("open"), m_Filenames.GetHead(), NULL, NULL, SW_SHOWNORMAL );
		break;

	case ID_THUMBNAIL_INFO_ITEM:
		// Disabled
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
		ATLTRACE ( "CThumb - IContextMenu::InvokeCommand() : E_INVALIDARG\n" );
		return E_INVALIDARG;
	}

	ATLTRACE ( "CThumb - IContextMenu::InvokeCommand() : S_OK\n" );
	return S_OK;
}

// IContextMenu2

STDMETHODIMP CThumb::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ATLTRACE( "CThumb - IContextMenu2::HandleMenuMsg()\n" );
	LRESULT res = 0;
	return MenuMessageHandler (uMsg, wParam, lParam, &res);
}

// IContextMenu3

STDMETHODIMP CThumb::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	ATLTRACE( "CThumb - IContextMenu3::HandleMenuMsg2()\n");
	LRESULT res = 0;
	return MenuMessageHandler (uMsg, wParam, lParam, (pResult ? pResult : &res));
}

STDMETHODIMP CThumb::MenuMessageHandler(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, LRESULT* pResult)
{
	switch (uMsg)
	{
	case WM_INITMENUPOPUP:
		ATLTRACE( "CThumb - IContextMenu3::MenuMessageHandler(WM_INITMENUPOPUP) : S_OK\n" );
		break;

	case WM_MEASUREITEM:
		ATLTRACE( "CThumb - IContextMenu3::MenuMessageHandler(WM_MEASUREITEM) : S_OK\n" );
		return OnMeasureItem( (MEASUREITEMSTRUCT*)lParam, pResult );

	case WM_DRAWITEM:
		ATLTRACE( "CThumb - IContextMenu3::MenuMessageHandler(WM_DRAWITEM) : S_OK\n" );
		return OnDrawItem( (DRAWITEMSTRUCT*)lParam, pResult );

	case WM_MENUCHAR:
		ATLTRACE( "CThumb - IContextMenu3::MenuMessageHandler(WM_MENUCHAR) : S_OK\n" );
		break;

	default:
		ATLTRACE( "CThumb - IContextMenu3::MenuMessageHandler(%u) : S_OK\n", uMsg );
	}
	return S_OK;
}

STDMETHODIMP CThumb::OnMeasureItem(MEASUREITEMSTRUCT* pmis, LRESULT* pResult)
{
	// Check that we're getting called for our own menu item.
	if ( m_uOurItemID != pmis->itemID )
		return S_OK;

	if ( ! m_Preview )
		return S_OK;

	pmis->itemWidth = m_Preview.Width();
	pmis->itemHeight = m_Preview.Height();

	*pResult = TRUE;

	return S_OK;
}

STDMETHODIMP CThumb::OnDrawItem(DRAWITEMSTRUCT* pdis, LRESULT* pResult)
{
	// Check that we're getting called for our own menu item.
	if ( m_uOurItemID != pdis->itemID )
		return S_OK;

	const int width  = pdis->rcItem.right  - pdis->rcItem.left;
	const int height = pdis->rcItem.bottom - pdis->rcItem.top;

	if ( ( pdis->itemState & ODS_SELECTED ) )
		FillRect( pdis->hDC, &pdis->rcItem, GetSysColorBrush( COLOR_HIGHLIGHT ) );
	else
		FillRect( pdis->hDC, &pdis->rcItem, GetSysColorBrush( COLOR_MENU ) );

	if ( HBITMAP hBitmap = m_Preview.GetImage( width, height, ( pdis->itemState & ODS_SELECTED ) ? GetSysColor( COLOR_HIGHLIGHT ) : RGB( 241, 241, 241 ) ) )
	{
		BITMAP bm = {};
		GetObject( hBitmap, sizeof( BITMAP ), &bm );
	
		const int x = pdis->rcItem.left + ( width  - bm.bmWidth  ) / 2;
		const int y = pdis->rcItem.top  + ( height - bm.bmHeight ) / 2;

		if ( HDC hBitmapDC = CreateCompatibleDC( pdis->hDC ) )
		{
			HBITMAP hOldBitmap = (HBITMAP)SelectObject( hBitmapDC, hBitmap );

			BitBlt( pdis->hDC, x, y, bm.bmWidth, bm.bmHeight, hBitmapDC, 0, 0, SRCCOPY );

			SelectObject( hBitmapDC, hOldBitmap );
			DeleteDC( hBitmapDC );
		}

		DeleteObject( hBitmap );
	}

	*pResult = TRUE;
	return S_OK;
}
