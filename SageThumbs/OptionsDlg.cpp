/*
SageThumbs - Thumbnail image shell extension.

Copyright (C) Nikolay Raspopov, 2004-2010.

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
	return _AtlModule.GetLang();
}

LRESULT COptionsDialog::OnInitDialog(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& bHandled)
{
	bHandled = TRUE;

	// Загрузка списка доступных языков
	SendDlgItemMessage( IDC_LANG, CB_RESETCONTENT );
	AddLanguage( STANDARD_LANGID, _AtlModule.GetLang() );
	WIN32_FIND_DATA wfd = {};
	HANDLE ff = FindFirstFile( _ModuleFileName.Left(
		_ModuleFileName.ReverseFind( _T('\\') ) + 1 ) + _T("SageThumbs??.dll"), &wfd );
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
					hs2b( wfd.cFileName [11] ), _AtlModule.GetLang() );
			}
		}
		while ( FindNextFile( ff, &wfd ) );
		FindClose( ff );
	}

	// Загрузка информации о базе данных
	WIN32_FILE_ATTRIBUTE_DATA wfadDatabase = {};
	GetFileAttributesEx( _Database, GetFileExInfoStandard, &wfadDatabase );
	CString sDatabaseSize, sDatabaseSizeFmt;
	sDatabaseSizeFmt.LoadString( IDS_DATABASE_SIZE );
	sDatabaseSize.Format( sDatabaseSizeFmt, ( wfadDatabase.nFileSizeLow >> 10 ) );

	// Загрузка информации о версии
	DWORD handle = NULL;
	if ( DWORD size = GetFileVersionInfoSize( _ModuleFileName, &handle ) )
	{
		if ( char* ver = (char*)GlobalAlloc( GPTR, size ) )
		{
			if ( GetFileVersionInfo( _ModuleFileName, handle, size, ver ) )
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
							sAbout.Format( _T("%s %d.%d.%d.%d\r\n%s\r\n\r\n%s"),
								szProductName,
								HIWORD (fix->dwFileVersionMS),
								LOWORD (fix->dwFileVersionMS),
								HIWORD (fix->dwFileVersionLS),
								LOWORD (fix->dwFileVersionLS),
								szLegalCopyright, (LPCTSTR)sDatabaseSize );
							SetDlgItemText( IDC_COPYRIGHT, sAbout );
						}
					}
				}
			}
			GlobalFree (ver);
		}
	}		

	const DWORD max_size = GetRegValue( _T("MaxSize"), (DWORD)FILE_MAX_SIZE );
	SetDlgItemInt( IDC_FILE_SIZE, max_size, FALSE );

	const DWORD width = GetRegValue( _T("Width"), (DWORD)THUMB_STORE_SIZE );
	SetDlgItemInt( IDC_WIDTH, width, FALSE );

	const DWORD height = GetRegValue( _T("Height"), (DWORD)THUMB_STORE_SIZE );
	SetDlgItemInt( IDC_HEIGHT, height, FALSE );

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
	for ( POSITION pos = _ExtMap.GetStartPosition(); pos; )
	{
		const CAtlExtMap::CPair* p = _ExtMap.GetNext( pos );
		LVITEM lvi = {};
		lvi.mask = LVIF_TEXT;
		lvi.pszText = (LPTSTR)(LPCTSTR)p->m_key;
		int index = ListView_InsertItem( pList, &lvi );				
		lvi.iItem++;
		ListView_SetItemText( pList, index, 1, (LPTSTR)(LPCTSTR)p->m_value.info);
		ListView_SetCheckState( pList, index, p->m_value.enabled ? TRUE : FALSE );
	}

	const bool bEnableMenu = GetRegValue( _T("EnableMenu"), 1 ) != 0;
	CheckDlgButton( IDC_ENABLE_MENU, bEnableMenu ? BST_CHECKED : BST_UNCHECKED );

	const bool bEnableThumbs = GetRegValue( _T("EnableThumbs"), 1 ) != 0;
	CheckDlgButton( IDC_ENABLE_THUMBS, bEnableThumbs ? BST_CHECKED : BST_UNCHECKED );

	const bool bEnableIcons = GetRegValue( _T("EnableIcons"), 1 ) != 0;
	CheckDlgButton( IDC_ENABLE_ICONS, bEnableIcons ? BST_CHECKED : BST_UNCHECKED );

	const bool bUseEmbedded = GetRegValue( _T("UseEmbedded"), (DWORD)0 ) != 0;
	CheckDlgButton( IDC_EMBEDDED, bUseEmbedded ? BST_CHECKED : BST_UNCHECKED );

	const bool bUseFax = GetRegValue( _T(""), _T(""), ShellImagePreview,
		HKEY_CLASSES_ROOT ).CompareNoCase( FaxCLSID ) == 0;
	CheckDlgButton( IDC_USE_FAX, bUseFax ? BST_CHECKED : BST_UNCHECKED );

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

	const bool bEnableMenu = IsDlgButtonChecked( IDC_ENABLE_MENU ) == BST_CHECKED;
	SetRegValue( _T("EnableMenu"), bEnableMenu ? 1 : 0 );

	const bool bEnableThumbs = IsDlgButtonChecked( IDC_ENABLE_THUMBS ) == BST_CHECKED;
	SetRegValue( _T("EnableThumbs"), bEnableThumbs ? 1 : 0 );

	const bool bEnableIcons = IsDlgButtonChecked( IDC_ENABLE_ICONS ) == BST_CHECKED;
	SetRegValue( _T("EnableIcons"), bEnableIcons ? 1 : 0 );

	const bool bUseEmbedded = IsDlgButtonChecked( IDC_EMBEDDED ) == BST_CHECKED;
	SetRegValue( _T("UseEmbedded"), bUseEmbedded ? 1 : 0 );

	const bool bUseFax = IsDlgButtonChecked( IDC_USE_FAX ) == BST_CHECKED;
	if ( bUseFax )
		SetRegValue( _T(""), FaxCLSID, ShellImagePreview, HKEY_CLASSES_ROOT );
	else
	{
		SHDeleteKey( HKEY_CLASSES_ROOT, ShellImagePreview );
		SHDeleteKey( HKEY_CLASSES_ROOT, ShellImagePreview );
	}

	const CWindow& pList = GetDlgItem( IDC_TYPES_LIST );
	const int len = ListView_GetItemCount( pList );
	const CString key = CString( REG_SAGETHUMBS ) + _T("\\");
	for ( int index = 0; index < len; ++index )
	{
		CString ext;
		ListView_GetItemText( pList, index, 0, ext.GetBuffer( 65 ), 64 );
		ext.ReleaseBuffer ();
		_ExtMap[ ext ].enabled = ListView_GetCheckState( pList, index ) != 0;
		SetRegValue( _T("Enabled"), _ExtMap [ext].enabled ? 1 : 0, key + ext );
	}

	_AtlModule.LoadLang( GetLanguage () );
	_AtlModule.DllRegisterServer();

	CDatabase db( _Database );
	if ( db )
	{
		db.Exec( VACUUM_DATABASE );
	}

	EndDialog( IDOK );

	return TRUE;
}

LRESULT COptionsDialog::OnCancel(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& bHandled)
{
	bHandled = TRUE;

	EndDialog( IDCANCEL );

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
	CWaitCursor wc;

	bHandled = TRUE;

	if ( MsgBox( m_hWnd, IDS_CLEAR_PROMPT, MB_OKCANCEL | MB_ICONQUESTION ) == IDOK )
	{
		CDatabase db( _Database );
		if ( db )
		{
			// Удаление таблицы и сжатие базы
			db.Exec( RECREATE_DATABASE );
		}

		// Обновление
		OnInitDialog( 0, 0, 0, bHandled );
	}

	return TRUE;
}
