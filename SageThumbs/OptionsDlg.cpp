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
#include "OptionsDlg.h"
#include "SQLite.h"

COptionsDialog::COptionsDialog()
{
}

LRESULT COptionsDialog::AddLanguage (LANGID lang, LANGID selected)
{
	LCID lcID =  MAKELCID (MAKELANGID (lang, SUBLANG_NEUTRAL), SORT_DEFAULT);
	TCHAR szNativeLangName [64] = {};
	GetLocaleInfo (lcID, LOCALE_SNATIVELANGNAME, szNativeLangName, 64);
	TCHAR szLangName [64] = {};
	GetLocaleInfo (lcID, LOCALE_SENGLANGUAGE, szLangName, 64);
	CString strName;
	strName.Format (_T("%s - %s"), szLangName, szNativeLangName);
	LRESULT nIndex = SendDlgItemMessage (IDC_LANG, CB_ADDSTRING, 0,
		(LPARAM) (LPCTSTR) strName);
	SendDlgItemMessage (IDC_LANG, CB_SETITEMDATA, nIndex, lang);
	if (lang == selected)
		SendDlgItemMessage (IDC_LANG, CB_SETCURSEL, nIndex);
	return nIndex;
}

LANGID COptionsDialog::GetLanguage()
{
	LRESULT nIndex = SendDlgItemMessage (IDC_LANG, CB_GETCURSEL);
	if (nIndex != CB_ERR)
	{
		LANGID lang = (LANGID) SendDlgItemMessage (IDC_LANG, CB_GETITEMDATA, nIndex);
		if ( lang != CB_ERR )
			return lang;
	}
	return _Module.GetLang();
}

void COptionsDialog::ShowAbout()
{
	// Загрузка информации о базе данных
	WIN32_FILE_ATTRIBUTE_DATA wfadDatabase = {};
	GetFileAttributesEx( _Module.m_sDatabase, GetFileExInfoStandard, &wfadDatabase );
	CString sDatabaseSize, sDatabaseSizeFmt;
	sDatabaseSizeFmt.LoadString( IDS_DATABASE_SIZE );
	sDatabaseSize.Format( sDatabaseSizeFmt, ( wfadDatabase.nFileSizeLow >> 10 ) );
	SetDlgItemText( IDC_CACHE_SIZE, sDatabaseSize );

	// Загрузка информации о версии
	DWORD handle = NULL;
	if ( DWORD size = GetFileVersionInfoSize( _Module.m_sModuleFileName, &handle ) )
	{
		if ( char* ver = (char*)GlobalAlloc( GPTR, size ) )
		{
			if ( GetFileVersionInfo( _Module.m_sModuleFileName, handle, size, ver ) )
			{
				UINT len;
				VS_FIXEDFILEINFO* fix;
				if ( VerQueryValue( ver, _T("\\"), (void**)&fix, &len) && len )
				{
					LPCTSTR szProductName;
					if ( VerQueryValue( ver, _T("\\StringFileInfo\\000004b0\\ProductName"),
						(void**)&szProductName, &len ) && len )
					{
						LPCTSTR szLegalCopyright;
						if ( VerQueryValue (ver, _T("\\StringFileInfo\\000004b0\\LegalCopyright"),
							(void**)&szLegalCopyright, &len ) && len )
						{
							CString sAbout;
							sAbout.Format( _T("%s %d.%d.%d.%d\r\n%s"),
								szProductName,
								HIWORD (fix->dwFileVersionMS),
								LOWORD (fix->dwFileVersionMS),
								HIWORD (fix->dwFileVersionLS),
								LOWORD (fix->dwFileVersionLS),
								szLegalCopyright );
							SetDlgItemText( IDC_COPYRIGHT, sAbout );
						}
					}
				}
			}
			GlobalFree (ver);
		}
	}
}

LRESULT COptionsDialog::OnInitDialog(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& bHandled)
{
	bHandled = TRUE;

	// Загрузка списка доступных языков
	SendDlgItemMessage( IDC_LANG, CB_RESETCONTENT );
	AddLanguage( STANDARD_LANGID, _Module.GetLang() );
	WIN32_FIND_DATA wfd = {};
	HANDLE ff = FindFirstFile( _Module.m_sHome + _Module.m_sModule + _T("??.dll"), &wfd );
	if ( ff != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( ! ( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
				wfd.cFileName [10] != '.' &&
				wfd.cFileName [11] != '.' &&
				wfd.cFileName [12] == '.' )
			{
				AddLanguage( ( hs2b( wfd.cFileName [10] ) << 4 ) |
					hs2b( wfd.cFileName [11] ), _Module.GetLang() );
			}
		}
		while ( FindNextFile( ff, &wfd ) );
		FindClose( ff );
	}

	ShowAbout();

	const DWORD max_size = GetRegValue( _T("MaxSize"), FILE_MAX_SIZE );
	SetDlgItemInt( IDC_FILE_SIZE, max_size, FALSE );

	const DWORD width = GetRegValue( _T("Width"), THUMB_STORE_SIZE );
	SetDlgItemInt( IDC_WIDTH, width, FALSE );

	const DWORD height = GetRegValue( _T("Height"), THUMB_STORE_SIZE );
	SetDlgItemInt( IDC_HEIGHT, height, FALSE );

	const DWORD jpeg = GetRegValue( _T("JPEG"), JPEG_DEFAULT );
	SetDlgItemInt( IDC_JPEG, jpeg, FALSE );
	SendDlgItemMessage( IDC_JPEG_SPIN, UDM_SETRANGE, 0, MAKELONG( 100, 0 ) );
	SendDlgItemMessage( IDC_JPEG_SPIN, UDM_SETPOS, 0, MAKELONG( jpeg, 0 ) );

	const DWORD png = GetRegValue( _T("PNG"), PNG_DEFAULT );
	SetDlgItemInt( IDC_PNG, png, FALSE );
	SendDlgItemMessage( IDC_PNG_SPIN, UDM_SETRANGE, 0, MAKELONG( 9, 0 ) );
	SendDlgItemMessage( IDC_PNG_SPIN, UDM_SETPOS, 0, MAKELONG( png, 0 ) );

	const CWindow& pList = GetDlgItem (IDC_TYPES_LIST);
	ListView_SetExtendedListViewStyle (pList,
		LVS_EX_LABELTIP | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT |
		LVS_EX_GRIDLINES);

	RECT rc;
	pList.GetClientRect (&rc);

	if ( ListView_GetItemCount( pList ) == 0 )
	{
		LVCOLUMN lvc = {};
		lvc.mask = LVCF_WIDTH;
		lvc.cx = 80;
		ListView_InsertColumn ( pList, 0, &lvc );
		lvc.cx = rc.right - rc.left - GetSystemMetrics( SM_CXVSCROLL ) - lvc.cx - 2;
		ListView_InsertColumn ( pList, 1, &lvc );
	}

	ListView_DeleteAllItems( pList );
	for ( POSITION pos = _Module.m_oExtMap.GetHeadPosition(); pos; )
	{
		const CExtMap::CPair* p = _Module.m_oExtMap.GetNext( pos );

		LVITEM lvi = {};
		lvi.mask = LVIF_TEXT;
		lvi.pszText = (LPTSTR)(LPCTSTR)p->m_key;
		int index = ListView_InsertItem( pList, &lvi );				
		lvi.iItem++;
		ListView_SetItemText( pList, index, 1, (LPTSTR)(LPCTSTR)p->m_value.info);
		ListView_SetCheckState( pList, index, p->m_value.enabled ? TRUE : FALSE );
	}

	const bool bEnableMenu = GetRegValue( _T("EnableMenu"), 1ul ) != 0;
	CheckDlgButton( IDC_ENABLE_MENU, bEnableMenu ? BST_CHECKED : BST_UNCHECKED );

	const bool bEnableThumbs = GetRegValue( _T("EnableThumbs"), 1ul ) != 0;
	CheckDlgButton( IDC_ENABLE_THUMBS, bEnableThumbs ? BST_CHECKED : BST_UNCHECKED );

	const bool bEnableIcons = GetRegValue( _T("EnableIcons"), 1ul ) != 0;
	CheckDlgButton( IDC_ENABLE_ICONS, bEnableIcons ? BST_CHECKED : BST_UNCHECKED );

	const bool bEnableOverlay  = GetRegValue( _T("EnableOverlay"),  0ul ) != 0;
	CheckDlgButton( IDC_ENABLE_OVERLAY, bEnableOverlay ? BST_CHECKED : BST_UNCHECKED );

	const bool bUseEmbedded = GetRegValue( _T("UseEmbedded"), 0ul ) != 0;
	CheckDlgButton( IDC_EMBEDDED, bUseEmbedded ? BST_CHECKED : BST_UNCHECKED );

	const bool bWinCache = GetRegValue( _T("WinCache"), 1ul ) != 0;
	CheckDlgButton( IDC_ENABLE_WINCACHE, bWinCache ? BST_CHECKED : BST_UNCHECKED );

	const bool bUseFax = GetRegValue( _T(""), _T(""), ShellImagePreview, HKEY_CLASSES_ROOT ).CompareNoCase( CLSID_FAX ) == 0;
	CheckDlgButton( IDC_USE_FAX, bUseFax ? BST_CHECKED : BST_UNCHECKED );

	if ( _Module.m_OSVersion.dwMajorVersion >= 6 && ! IsProcessElevated() )
		SendDlgItemMessage( IDOK, BCM_SETSHIELD, 0, 1 );

	return TRUE;
}

LRESULT COptionsDialog::OnOK(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& bHandled)
{
	CWaitCursor wc;

	bHandled = TRUE;

	BOOL result;
	DWORD max_size = GetDlgItemInt( IDC_FILE_SIZE, &result, FALSE );
	if ( ! result )
	{
		max_size = FILE_MAX_SIZE;
		SetDlgItemInt( IDC_FILE_SIZE, max_size, FALSE );
		::SetFocus( GetDlgItem ( IDC_FILE_SIZE ) );
		FlashWindow( TRUE );
		MessageBeep( MB_ICONEXCLAMATION );
		FlashWindow( FALSE );
		return TRUE;
	}

	DWORD width = GetDlgItemInt( IDC_WIDTH, &result, FALSE );
	if ( ! result || width < THUMB_MIN_SIZE || width > THUMB_MAX_SIZE )
	{
		if ( ! result )
			width = THUMB_STORE_SIZE;
		else if ( width < THUMB_MIN_SIZE )
			width = THUMB_MIN_SIZE;
		else if ( width > THUMB_MAX_SIZE )
			width = THUMB_MAX_SIZE;
		SetDlgItemInt( IDC_WIDTH, width, FALSE );
		::SetFocus( GetDlgItem ( IDC_WIDTH ) );
		FlashWindow( TRUE );
		MessageBeep( MB_ICONEXCLAMATION );
		FlashWindow( FALSE );
		return TRUE;
	}

	DWORD height = GetDlgItemInt( IDC_HEIGHT, &result, FALSE );
	if ( ! result || height < THUMB_MIN_SIZE || height > THUMB_MAX_SIZE )
	{
		if ( ! result )
			height = THUMB_STORE_SIZE;
		else if ( height < THUMB_MIN_SIZE )
			height = THUMB_MIN_SIZE;
		else if ( height > THUMB_MAX_SIZE )
			height = THUMB_MAX_SIZE;
		SetDlgItemInt( IDC_HEIGHT, height, FALSE );
		::SetFocus( GetDlgItem ( IDC_HEIGHT ) );
		FlashWindow( TRUE );
		MessageBeep( MB_ICONEXCLAMATION );
		FlashWindow( FALSE );
		return TRUE;
	}

	SetRegValue( _T("MaxSize"), max_size );
	SetRegValue( _T("Width"), width );
	SetRegValue( _T("Height"), height );

	DWORD jpeg = min( 100, GetDlgItemInt( IDC_JPEG, &result, FALSE ) );
	SetRegValue( _T("JPEG"), jpeg );

	DWORD png = min( 9, GetDlgItemInt( IDC_PNG, &result, FALSE ) );
	SetRegValue( _T("PNG"), png );

	const bool bEnableMenu = IsDlgButtonChecked( IDC_ENABLE_MENU ) == BST_CHECKED;
	SetRegValue( _T("EnableMenu"), bEnableMenu ? 1ul : 0ul );

	const bool bEnableThumbs = IsDlgButtonChecked( IDC_ENABLE_THUMBS ) == BST_CHECKED;
	SetRegValue( _T("EnableThumbs"), bEnableThumbs ? 1ul : 0ul );

	const bool bEnableIcons = IsDlgButtonChecked( IDC_ENABLE_ICONS ) == BST_CHECKED;
	SetRegValue( _T("EnableIcons"), bEnableIcons ? 1ul : 0ul );

	const bool bEnableOverlay = IsDlgButtonChecked( IDC_ENABLE_OVERLAY ) == BST_CHECKED;
	SetRegValue( _T("EnableOverlay"), bEnableOverlay ? 1ul : 0ul );

	const bool bUseEmbedded = IsDlgButtonChecked( IDC_EMBEDDED ) == BST_CHECKED;
	SetRegValue( _T("UseEmbedded"), bUseEmbedded ? 1ul : 0ul );

	const bool bWinCache = IsDlgButtonChecked( IDC_ENABLE_WINCACHE ) == BST_CHECKED;
	SetRegValue( _T("WinCache"), bWinCache ? 1ul : 0ul );

	const bool bUseFax = IsDlgButtonChecked( IDC_USE_FAX ) == BST_CHECKED;
	if ( bUseFax )
		SetRegValue( _T(""), CLSID_FAX, ShellImagePreview, HKEY_CLASSES_ROOT );
	else
		DeleteRegKey( HKEY_CLASSES_ROOT, ShellImagePreview );

	const CWindow& pList = GetDlgItem( IDC_TYPES_LIST );
	const int len = ListView_GetItemCount( pList );
	const CString key = CString( REG_SAGETHUMBS ) + _T("\\");
	for ( int index = 0; index < len; ++index )
	{
		CString ext;
		ListView_GetItemText( pList, index, 0, ext.GetBuffer( 65 ), 64 );
		ext.ReleaseBuffer ();

		bool bEnabled = ListView_GetCheckState( pList, index ) != 0;
		if ( CExtMap::CPair* p = _Module.m_oExtMap.Lookup( ext ) )
		{
			p->m_value.enabled = bEnabled;
		}
		SetRegValue( _T("Enabled"), bEnabled ? 1ul : 0u, key + ext );
	}

	_Module.LoadLang( GetLanguage () );

	if ( ! _Module.RegisterExtensions( m_hWnd ) )
	{
		if ( ( _Module.m_OSVersion.dwMajorVersion >= 6 && ! IsProcessElevated() ) ||
			_Module.MsgBox( m_hWnd, IDS_ACCESS_DENIED, MB_OKCANCEL | MB_ICONEXCLAMATION ) == IDOK )
		{
			// Run as admin
			CString sParams = _T("/s \"");
			sParams += _Module.m_sModuleFileName + _T("\"");
			SHELLEXECUTEINFO sei = { sizeof( SHELLEXECUTEINFO ) };
			sei.lpVerb = _T("runas");
			sei.lpFile = _T("regsvr32.exe");
			sei.lpParameters = sParams;
			sei.hwnd = m_hWnd;
			sei.nShow = SW_NORMAL;
			if ( ShellExecuteEx( &sei ) )
			{
				// Success
				EndDialog( IDOK );
			}
		}
	}
	else
	{
		// Success
		EndDialog( IDOK );
	}

	return TRUE;
}

LRESULT COptionsDialog::OnCancel(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& bHandled)
{
	bHandled = TRUE;

	EndDialog( IDCANCEL );

	return TRUE;
}

LRESULT COptionsDialog::OnDefault(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& bHandled)
{
	bHandled = TRUE;

	const CWindow& pList = GetDlgItem (IDC_TYPES_LIST);
	const int len = ListView_GetItemCount (pList);
	for ( int index = 0; index < len; ++index )
	{
		CString ext;
		ListView_GetItemText( pList, index, 0, ext.GetBuffer( 65 ), 64 );
		ext.ReleaseBuffer();
		ListView_SetCheckState( pList, index, EXT_DEFAULT( ext ) ? FALSE : TRUE );
	}

	return TRUE;
}

LRESULT COptionsDialog::OnSelect(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& bHandled)
{
	bHandled = TRUE;

	const bool bState = IsDlgButtonChecked (IDC_SELECT) == BST_CHECKED;
	const CWindow& pList = GetDlgItem (IDC_TYPES_LIST);
	const int len = ListView_GetItemCount (pList);
	for ( int index = 0; index < len; ++index )
	{
		ListView_SetCheckState( pList, index, bState ? TRUE : FALSE );
	}

	return TRUE;
}

LRESULT COptionsDialog::OnClear(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& bHandled)
{
	bHandled = TRUE;

	if ( _Module.MsgBox( m_hWnd, IDS_CLEAR_PROMPT, MB_OKCANCEL | MB_ICONQUESTION ) == IDOK )
	{
		CWaitCursor wc;

		// Clean SageThumbs cache
		CDatabase db( _Module.m_sDatabase );
		if ( db )
		{
			db.Exec( DROP_DATABASE );
		}

		ShowAbout();
	}

	return TRUE;
}

LRESULT COptionsDialog::OnOptimize(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& bHandled)
{
	bHandled = TRUE;

	CWaitCursor wc;

	// Analyze and vacuum SageThumbs cache
	CDatabase db( _Module.m_sDatabase );
	if ( db )
	{
		db.Exec( OPTIMIZE_DATABASE );
	}

	ShowAbout();

	return TRUE;
}
