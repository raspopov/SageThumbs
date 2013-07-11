/*
SageThumbs - Thumbnail image shell extension.

Copyright (C) Nikolay Raspopov, 2004-2013.

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
#include "OptionsDlg.h"

//static BitsDescription	_Bits [] = {
//	{_T("psd"),		2, "\xff\xff", "\x38\x42"},
//	{_T("xcf"),		8, "\xff\xff\xff\xff\xff\xff\xff\xff", "gimp xcf"},
//	{_T("tiff"),	2, "\xff\xff", "II"},
//	{_T("xpm"),		9, "\xff\xff\xff\xff\xff\xff\xff\xff\xff", "/* XPM */"},
//	{_T("ps"),		4, "\xff\xff\xff\xff", "%!PS"},
//	{NULL, 0, NULL}
//};
//#define XNVIEW_MIME "\x10\x00\x00\x00" \
//	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
//	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

struct
{
	bool	bDelete;		// Delete key if true
	bool	bUseProgID;		// Register to ProgID key (true) else to extension key (false)
	LPCTSTR szName;			// Key name
}
static const Handlers[] =
{
	{ false, true , _T("IconHandler") },							// IExtractIconA and IExtractIconW (Windows 2000)
	{ false, false, _T("{BB2E617C-0920-11D1-9A0B-00C04FC2D6C1}") },	// IExtractImage (Windows 2000)
	{ false, true , _T("{00021500-0000-0000-C000-000000000046}") },	// IQueryInfo (Windows 2000)
	{ false, false, _T("{E357FCCD-A995-4576-B01F-234630154E96}") },	// IThumbnailProvider (Windows Vista)
	{ false, false, _T("PropertyHandler") },						// IPropertyStore (Windows Vista)
	{ true , true , _T("DataHandler") },							// IDataObject (Windows 2000)
	{ true , false, _T("{8895B1C6-B41F-4C1C-A562-0D564250836F}") },	// IPreviewHandler (Windows Vista)
	{ false, false, NULL }
};

// Properties are displayed on the Details tab of the Properties dialog box. This is the complete list of properties that the file type supports.
LPCTSTR FullDetails	=		_T("prop:System.PropGroup.Image;")
							_T("System.FileDescription;")
							_T("System.Image.Dimensions;")
							_T("System.Image.HorizontalSize;")
							_T("System.Image.VerticalSize;")
							_T("System.Image.HorizontalResolution;")
							_T("System.Image.VerticalResolution;")
							_T("System.Image.BitDepth;")
							_T("System.Image.Compression;")
							_T("System.PropGroup.FileSystem;")
							_T("System.ItemNameDisplay;")
							_T("System.ItemType;")
							_T("System.ItemFolderPathDisplay;")
							_T("System.DateCreated;")
							_T("System.DateModified;")
							_T("System.Size;")
							_T("System.FileAttributes;")
							_T("System.OfflineAvailability;")
							_T("System.OfflineStatus;")
							_T("System.SharedWith;")
							_T("System.FileOwner;")
							_T("System.ComputerName");

// Properties are displayed in the Preview Pane.
LPCTSTR PreviewDetails =	_T("prop:*System.DateModified;")
							_T("*System.Image.Dimensions;")
							_T("*System.Image.BitDepth;")
							_T("*System.Size;")
							_T("*System.OfflineAvailability;")
							_T("*System.OfflineStatus;")
							_T("*System.DateCreated;")
							_T("*System.SharedWith");

// Properties are displayed in the title area of the Preview Pane next to the thumbnail for the item.
LPCTSTR PreviewTitle =		_T("prop:System.ItemNameDisplay;")
							_T("System.FileDescription");

// Properties are displayed for an item when the list view is in Extended Tile view mode.
LPCTSTR ExtendedTileInfo =	_T("prop:System.FileDescription;")
							_T("*System.DateModified;")
							_T("*System.Image.Dimensions;")
							_T("*System.Image.BitDepth");

// Properties are displayed when the list view is in Tiles view mode.
LPCTSTR TileInfo =			_T("prop:System.FileDescription;")
							_T("System.Size;")
							_T("System.DateModified");

// Properties are displayed in an infotip when a user hovers over an item.
LPCTSTR InfoTip =			_T("prop:System.FileDescription;")
							_T("System.DateModified;")
							_T("System.Image.Dimensions;")
							_T("System.Image.BitDepth;")
							_T("System.Image.Compression;")
							_T("System.Size");

LPCTSTR ContentViewModeForBrowse =
							_T("prop:~System.ItemNameDisplay;")
							_T("~System.FileDescription;")
							_T("~System.Image.Dimensions;")
							_T("System.Image.BitDepth;")
							_T("System.DateModified;")
							_T("System.Size");

LPCTSTR ContentViewModeForSearch =
							_T("prop:~System.ItemNameDisplay;")
							_T("~System.ItemFolderPathDisplay;")
							_T("~System.Image.Dimensions;")
							_T("~System.FileDescription;")
							_T("System.DateModified;")
							_T("System.Size");

//LPCTSTR ConflictPrompt =	_T("prop:System.FileDescription;")
//							_T("System.Size;")
//							_T("System.DateModified;")
//							_T("System.DateCreated;")
//							_T("System.Image.Dimensions");

//BitsDescriptionMap	_BitsMap;
CSageThumbsModule		_Module;

CSageThumbsModule::CSageThumbsModule()
	: m_OSVersion	()
	, m_hGFL		( NULL )
	, m_hGFLe		( NULL )
	, m_hSQLite		( NULL )
{
	m_OSVersion.dwOSVersionInfoSize = sizeof( m_OSVersion );
	GetVersionEx( &m_OSVersion );

	GetModuleFileName( _AtlBaseModule.GetModuleInstance(),
		m_sModuleFileName.GetBuffer( MAX_LONG_PATH ), MAX_LONG_PATH );
	m_sModuleFileName.ReleaseBuffer();
	ATLTRACE( "Module path: %s\n", (LPCSTR)CT2A( m_sModuleFileName ) );
	m_sHome = m_sModuleFileName.Left( m_sModuleFileName.ReverseFind( _T('\\') ) + 1 );

	// Get database filename
	m_sDatabase = GetRegValue( _T("Database"), CString() );
	if ( m_sDatabase.IsEmpty() )
	{
		m_sDatabase = GetSpecialFolderPath( CSIDL_LOCAL_APPDATA );
		if ( m_sDatabase.IsEmpty() )
		{
			GetWindowsDirectory( m_sDatabase.GetBuffer( MAX_LONG_PATH ), MAX_LONG_PATH );
			m_sDatabase.ReleaseBuffer();
		}
		m_sDatabase.TrimRight (_T("\\"));
		m_sDatabase += _T("\\SageThumbs.db3");
		SetRegValue( _T("Database"), m_sDatabase );
	}
	ATLTRACE( "Database path: %s\n", (LPCSTR)CT2A( m_sDatabase ) );
}

BOOL CSageThumbsModule::DllMain(DWORD dwReason, LPVOID lpReserved) throw()
{
	BOOL res = CAtlDllModuleT< CSageThumbsModule >::DllMain( dwReason, lpReserved );

	switch ( dwReason )
	{
	case DLL_PROCESS_ATTACH:
		ATLTRACE( "CThumb - DllMain::DLL_PROCESS_ATTACH\n" );
		DisableThreadLibraryCalls( _AtlBaseModule.GetModuleInstance() );
		__try
		{
			if ( ! res || ! Initialize() )
				res = FALSE;
		}
		__except ( EXCEPTION_EXECUTE_HANDLER )
		{
			ATLTRACE( "Exception in CSageThumbsModule::Initialize()\n" );
			res = FALSE;
		}
		break;

	case DLL_PROCESS_DETACH:
		ATLTRACE( "CThumb - DllMain::DLL_PROCESS_DETACH\n" );
		__try
		{
			UnInitialize();
		}
		__except ( EXCEPTION_EXECUTE_HANDLER )
		{
			ATLTRACE( "Exception in CSageThumbsModule::UnInitialize()\n" );
		}
		break;

	case DLL_THREAD_ATTACH:
		ATLTRACE( "CThumb - DllMain::DLL_THREAD_ATTACH\n" );
		break;

	case DLL_THREAD_DETACH:
		ATLTRACE( "CThumb - DllMain::DLL_THREAD_DETACH\n" );
		break;
	}

	return res;
}

HRESULT CSageThumbsModule::DllRegisterServer() throw()
{
	HRESULT hr = CAtlDllModuleT< CSageThumbsModule >::DllRegisterServer( FALSE );

	RegisterExtensions( GetDesktopWindow() );

	return hr;
}

HRESULT CSageThumbsModule::DllUnregisterServer() throw()
{
	HRESULT hr = CAtlDllModuleT< CSageThumbsModule >::DllUnregisterServer( FALSE );

	UnregisterExtensions();

	return hr;
}

BOOL CSageThumbsModule::RegisterExtensions(HWND hWnd)
{
	BOOL bOK = TRUE;

	CComPtr< IProgressDialog > pProgress;
	if ( hWnd )
	{
		HRESULT hr = pProgress.CoCreateInstance( CLSID_ProgressDialog );
		if ( SUCCEEDED( hr ) )
		{
			pProgress->SetTitle( _Module.GetAppName() );
			pProgress->SetLine( 1, m_oLangs.LoadString( IDS_APPLYING ), FALSE, NULL );
			pProgress->StartProgressDialog( hWnd, NULL, PROGDLG_NORMAL | PROGDLG_NOCANCEL | PROGDLG_AUTOTIME, NULL );
		}
	}

	const bool bEnableThumbs = GetRegValue( _T("EnableThumbs"), 1ul ) != 0;
	const bool bEnableIcons  = GetRegValue( _T("EnableIcons"),  1ul ) != 0;
	// Enabled by default on Windows 2000 and Windows XP only
	const bool bEnableInfo = GetRegValue( _T("EnableInfo"), ( m_OSVersion.dwMajorVersion < 6 ) ? 1ul : 0ul ) != 0;
	const bool bEnableOverlay  = GetRegValue( _T("EnableOverlay"),  0ul ) != 0;

	if ( bEnableOverlay )
		// Enable by removing empty key
		bOK = DeleteRegValue( _T("TypeOverlay"), _T("SystemFileAssociations\\image"), HKEY_CLASSES_ROOT ) && bOK;
	else
		// Disable by setting empty key
		bOK = SetRegValue( _T("TypeOverlay"), _T(""), _T("SystemFileAssociations\\image"), HKEY_CLASSES_ROOT ) && bOK;

	// Register common associations
	const DWORD total = (DWORD)m_oExtMap.GetCount();
	DWORD count = 0;
	for ( POSITION pos = m_oExtMap.GetHeadPosition(); pos; ++count )
	{
		if ( const CExtMap::CPair* p = m_oExtMap.GetNext( pos ) )
		{
			if ( pProgress )
			{
				pProgress->SetLine( 2, p->m_value.info, FALSE, NULL );
				pProgress->SetProgress( count, total );
				Sleep( 10 );
			}

			if ( p->m_value.enabled )
				bOK = RegisterExt( p->m_key, p->m_value.info, bEnableThumbs, bEnableIcons, bEnableInfo, bEnableOverlay ) && bOK;
			else
				bOK = UnregisterExt( p->m_key, p->m_value.custom ) && bOK;
		}
	}

	// TODO: Register SystemFileAssociations

	if ( pProgress )
	{
		pProgress->SetLine( 2, m_oLangs.LoadString( IDS_UPDATING ), FALSE, NULL );
		pProgress->SetProgress( total, total );
	}

	CleanWindowsCache();

	UpdateShell();

	if ( pProgress )
	{
		Sleep( 1000 );
		pProgress->StopProgressDialog();
	}

	return bOK;
}

BOOL CSageThumbsModule::UnregisterExtensions()
{
	BOOL bOK = TRUE;

	// Unregister common associations
	for ( POSITION pos = m_oExtMap.GetHeadPosition(); pos; )
	{
		if ( const CExtMap::CPair* p = m_oExtMap.GetNext( pos ) )
		{
			bOK = UnregisterExt( p->m_key, true ) && bOK;
		}
	}

	// Unregister SystemFileAssociations
	CString sRoot( _T("SystemFileAssociations\\image\\ShellEx\\") );
	for ( int i = 0; Handlers[ i ].szName; ++i )
	{
		bOK = UnregisterValue( HKEY_CLASSES_ROOT, sRoot + Handlers[ i ].szName ) && bOK;
	}

	bOK = SetRegValue( _T("TypeOverlay"), _T(""), _T("SystemFileAssociations\\image"), HKEY_CLASSES_ROOT ) && bOK;

	CleanWindowsCache();

	UpdateShell();

	return bOK;
}

void CSageThumbsModule::UpdateShell()
{
	SHChangeNotify( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL );
	SystemParametersInfo( SPI_SETICONS, 0, NULL, SPIF_SENDCHANGE );
	CoFreeUnusedLibraries();
}

bool CSageThumbsModule::IsDisabledByDefault(LPCTSTR szExt) const
{
	// Disable .png on Windows 8 due Metro bug
	if ( m_OSVersion.dwMajorVersion >= 6 && m_OSVersion.dwMinorVersion >= 2 )
	{
		if ( _tcsicmp( szExt, _T("png") ) == 0 )
			return true;
	}

	const LPCTSTR szDisabledExts [] = { _T("ico"), _T("icl"), _T("ani"), _T("cur"), _T("pdf"), _T("sys"), _T("vst"), _T("wmz") };

	for ( int i = 0; i < _countof( szDisabledExts ); ++i )
	{
		if ( _tcsicmp( szExt, szDisabledExts[ i ] ) == 0 )
			return true;
	}

	return false;
}

BOOL CSageThumbsModule::RegisterExt(LPCTSTR szExt, LPCTSTR szInfo, bool bEnableThumbs, bool bEnableIcons, bool bEnableInfo, bool bEnableOverlay)
{
	BOOL bOK = TRUE;

	CString sType( _T(".") );
	sType += szExt;
	CString sFileExt = FileExts + sType;
	CString sDefaultKey = REG_SAGETHUMBS_IMG + sType;
	CString sDefaultType = GetDefaultType( szExt );

	FixProgID( szExt );

	CString sUserChoice = GetRegValue( _T("Progid"), CString(), sFileExt + _T("\\UserChoice"), HKEY_CURRENT_USER );
	if ( sUserChoice.IsEmpty() )	
	{
		sUserChoice = sDefaultKey;
		SetRegValue( _T("Progid"), sUserChoice, sFileExt + _T("\\UserChoice"), HKEY_CURRENT_USER );
	}
	bool bOurUserChoice = ( sUserChoice.CompareNoCase( sDefaultKey ) == 0 );

	// Register extension
	CString sCurrentKey = GetRegValue( _T(""), CString(), sType, HKEY_CLASSES_ROOT );
	if ( sCurrentKey.IsEmpty() )
	{
		// Use default key
		bOK = RegisterValue( HKEY_CLASSES_ROOT, sType, _T(""), sDefaultKey ) && bOK;
	}
	else if ( sCurrentKey.CompareNoCase( sDefaultKey ) != 0 )
	{
		// Clean lost existing key
		DeleteRegKey( HKEY_CLASSES_ROOT, sDefaultKey );

		// Copy old key to our key
		HKEY hKey = NULL;
		LSTATUS res = RegCreateKeyEx( HKEY_CLASSES_ROOT, sDefaultKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL );
		if ( res == ERROR_SUCCESS )
		{
			res = SHCopyKey( HKEY_CLASSES_ROOT, sCurrentKey, hKey, 0 );
			RegCloseKey( hKey );
			if ( res == ERROR_SUCCESS )
			{
				bOK = RegisterValue( HKEY_CLASSES_ROOT, sType, _T(""), sDefaultKey ) && bOK;
			}
		}

		// Save old key as alternate ProgID
		bOK = SetRegValue( sCurrentKey, sType + _T("\\OpenWithProgids"), HKEY_CLASSES_ROOT ) && bOK;

		// Use this key
		sDefaultKey = GetRegValue( _T(""), CString(), sType, HKEY_CLASSES_ROOT );
	}

	// Save our key as alternate ProgID too
	bOK = SetRegValue( sDefaultKey, sType + _T("\\OpenWithProgids"), HKEY_CLASSES_ROOT ) && bOK;

	// Extension information
	bOK = SetRegValue( _T(""), szInfo, sDefaultKey, HKEY_CLASSES_ROOT ) && bOK;

	// Default icon (using Windows if absent) + restore .ico icon
	CString sDefaultIcon;
	if ( sType.CompareNoCase( _T(".ico") ) == 0 )
	{
		bOK = SetRegValue( _T(""), _T("%1"), sDefaultKey + _T("\\DefaultIcon"), HKEY_CLASSES_ROOT ) && bOK;
	}
	else
	{
		sDefaultIcon = GetRegValue( _T(""), CString(), sDefaultKey + _T("\\DefaultIcon"), HKEY_CLASSES_ROOT );
		if ( sDefaultIcon.IsEmpty() )
		{
			sDefaultIcon = GetRegValue( _T(""), CString(), _T("jpegfile\\DefaultIcon"), HKEY_CLASSES_ROOT );
			if ( sDefaultIcon.IsEmpty() )
				sDefaultIcon = GetRegValue( _T(""), CString(), _T("pngfile\\DefaultIcon"), HKEY_CLASSES_ROOT );
			if ( sDefaultIcon.IsEmpty() )
				sDefaultIcon = GetRegValue( _T(""), CString(), _T("giffile\\DefaultIcon"), HKEY_CLASSES_ROOT );
			if ( ! sDefaultIcon.IsEmpty() )
			{
				bOK = SetRegValue( _T(""), sDefaultIcon, sDefaultKey + _T("\\DefaultIcon"), HKEY_CLASSES_ROOT ) && bOK;
			}
		}

		if ( bEnableOverlay && ! sDefaultIcon.IsEmpty() )
		{
			bOK = SetRegValue( _T("TypeOverlay"), sDefaultIcon, sDefaultKey, HKEY_CLASSES_ROOT ) && bOK;
		}
		else
		{
			bOK = DeleteRegValue( _T("TypeOverlay"), sDefaultKey, HKEY_CLASSES_ROOT ) && bOK;
		}
	}

	// Edit flags (add "OpenIsSafe" flag)
	DWORD dwEditFlags = GetRegValue( _T("EditFlags"), 0ul, sDefaultKey, HKEY_CLASSES_ROOT );
	if ( ! ( dwEditFlags & 0x00010000 ) )
	{
		dwEditFlags |= 0x00010000;
		bOK = SetRegValue( _T("EditFlags"), dwEditFlags, sDefaultKey, HKEY_CLASSES_ROOT ) && bOK;
	}

	// Perceived Type Fix (optional)
	CString sPerceivedType = GetRegValue( _T("PerceivedType"), CString(), sType, HKEY_CLASSES_ROOT );
	if ( sPerceivedType.IsEmpty() )
	{
		SetRegValue( _T("PerceivedType"), GetPerceivedType( szExt ), sType, HKEY_CLASSES_ROOT );
	}

	// Content Type Fix (optional)
	CString sContentType = GetRegValue( _T("Content Type"), CString(), sType, HKEY_CLASSES_ROOT );
	if ( sContentType.IsEmpty() )
	{
		sContentType = GetContentType( szExt );
		CString sContentKey = _T("MIME\\DataBase\\Content Type\\") + sContentType;

		SetRegValue( _T("Content Type"), sContentType, sType, HKEY_CLASSES_ROOT );

		// MIME Fix (optional)
		SetRegValue( _T("AutoplayContentTypeHandler"), _T("PicturesContentHandler"), sContentKey, HKEY_CLASSES_ROOT );
		SetRegValue( _T("Extension"), GetContentExt( szExt ), sContentKey, HKEY_CLASSES_ROOT );

		// Set image filter only if it was free
		/*CString sImageFilterCLSID = GetRegValue( _T("Image Filter CLSID"), _T(""), sContentKey, HKEY_CLASSES_ROOT );
		if ( bEnableFilter && ( sImageFilterCLSID.IsEmpty() || sImageFilterCLSID.CompareNoCase( CLSID_THUMB ) == 0 ) )
		{
			bOK = SetRegValue( _T(""), CLSID_HTML, sDefaultKey + _T("\\CLSID"), HKEY_CLASSES_ROOT ) && bOK;
			bOK = SetRegValue( _T("Image Filter CLSID"), CLSID_THUMB, sContentKey, HKEY_CLASSES_ROOT ) && bOK;
			bOK = SetRegValue( _T("CLSID"), CLSID_HTML, sContentKey, HKEY_CLASSES_ROOT ) && bOK;

			const BYTE Bits[ 6 ] = { 01, 00, 00, 00, 00, 00 };
			SHSetValue( HKEY_CLASSES_ROOT, sContentKey + _T("\\Bits"), _T("0"), REG_BINARY, &Bits, sizeof( Bits ) );
		}
		else if ( ! bEnableFilter && sImageFilterCLSID.CompareNoCase( CLSID_THUMB ) == 0 )
		{
			bOK = DeleteRegValue( _T("Image Filter CLSID"), sContentKey, HKEY_CLASSES_ROOT ) && bOK;
			bOK = DeleteRegValue( _T("CLSID"), sContentKey, HKEY_CLASSES_ROOT ) && bOK;
			bOK = DeleteRegValue( _T("0"), sContentKey + _T("\\Bits"), HKEY_CLASSES_ROOT ) && bOK;
			DeleteRegKey( HKEY_CLASSES_ROOT, sContentKey + _T("\\Bits") );
			DeleteRegKey( HKEY_CLASSES_ROOT, sDefaultKey + _T("\\CLSID") );
		}*/
	}

	for ( int i = 0; Handlers[ i ].szName; ++i )
	{
		CString sHandlerTypeKey = sType + _T("\\ShellEx\\") + Handlers[ i ].szName;
		CString sHandlerProgIDKey = sDefaultKey + _T("\\ShellEx\\") + Handlers[ i ].szName;
		bool bDelete =	( i == 0 && ! bEnableIcons ) ||		// IExtractIcon
						( i == 1 && ! bEnableThumbs ) ||	// IExtractImage
						( i == 2 && ! bEnableInfo ) ||		// IQueryInfo
						( i == 3 && ! bEnableThumbs ) ||	// IThumbnailProvider
						Handlers[ i ].bDelete;
		if ( Handlers[ i ].bUseProgID )
		{
			if ( bDelete )
				bOK = UnregisterValue( HKEY_CLASSES_ROOT, sHandlerProgIDKey ) && bOK;
			else
			{
				// Check for existing CLSID
				CString buf = GetRegValue( _T(""), CString(), sHandlerProgIDKey, HKEY_CLASSES_ROOT );
				bool bValid = ! buf.IsEmpty() && ( buf.CompareNoCase( CLSID_THUMB ) != 0 ) && IsValidCLSID( buf );

				bOK = RegisterValue( HKEY_CLASSES_ROOT, sHandlerProgIDKey, _T(""), CLSID_THUMB, bValid ? NULL : REG_SAGETHUMBS_BAK ) && bOK;
			}

			// Clean wrong registration
			bOK = DeleteRegKey( HKEY_CLASSES_ROOT, sHandlerTypeKey ) && bOK;

			// User choice
			if ( ! bOurUserChoice )
			{
				CString sUserChoiceProgIDKey = sUserChoice + _T("\\ShellEx\\") + Handlers[ i ].szName;
				if ( bDelete )
					bOK = UnregisterValue( HKEY_CLASSES_ROOT, sUserChoiceProgIDKey ) && bOK;
				else
				{
					// Check for existing CLSID
					CString buf = GetRegValue( _T(""), CString(), sUserChoiceProgIDKey, HKEY_CLASSES_ROOT );
					bool bValid = ! buf.IsEmpty() && ( buf.CompareNoCase( CLSID_THUMB ) != 0 ) && IsValidCLSID( buf );

					bOK = RegisterValue( HKEY_CLASSES_ROOT, sUserChoiceProgIDKey, _T(""), CLSID_THUMB, bValid ? NULL : REG_SAGETHUMBS_BAK ) && bOK;
				}
			}
		}
		else
		{
			if ( bDelete )
				bOK = UnregisterValue( HKEY_CLASSES_ROOT, sHandlerTypeKey ) && bOK;
			else
			{
				// Check for existing CLSID
				CString buf = GetRegValue( _T(""), CString(), sHandlerTypeKey, HKEY_CLASSES_ROOT );
				bool bValid = ! buf.IsEmpty() && ( buf.CompareNoCase( CLSID_THUMB ) != 0 ) && IsValidCLSID( buf );

				bOK = RegisterValue( HKEY_CLASSES_ROOT, sHandlerTypeKey, _T(""), CLSID_THUMB, bValid ? NULL : REG_SAGETHUMBS_BAK ) && bOK;
			}

			// Clean wrong registration
			bOK = DeleteRegKey( HKEY_CLASSES_ROOT, sHandlerProgIDKey ) && bOK;
		}
	}

	// Register IPropertyStore handler
	{
		CString sPropKey = PropertyHandlers + sType;

		// Check for existing CLSID
		CString buf = GetRegValue( _T(""), CString(), sPropKey, HKEY_LOCAL_MACHINE );
		bool bValid = ! buf.IsEmpty() && ( buf.CompareNoCase( CLSID_THUMB ) != 0 ) && IsValidCLSID( buf );

		bOK = RegisterValue( HKEY_LOCAL_MACHINE, sPropKey, _T(""), CLSID_THUMB, bValid ? NULL : REG_SAGETHUMBS_BAK ) && bOK;
	}

	CString sSysKey = _T("SystemFileAssociations\\") + sType;
//	bOK = RegisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("ConflictPrompt"), ConflictPrompt, NULL ) && bOK;
	bOK = RegisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("ExtendedTileInfo"), ExtendedTileInfo, NULL ) && bOK;
	bOK = RegisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("TileInfo"), TileInfo, NULL ) && bOK;
	bOK = RegisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("FullDetails"), FullDetails, NULL ) && bOK;
	bOK = RegisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("InfoTip"), InfoTip, NULL ) && bOK;
	bOK = RegisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("PreviewDetails"), PreviewDetails, NULL ) && bOK;
	bOK = RegisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("PreviewTitle"), PreviewTitle, NULL ) && bOK;
	bOK = RegisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("ContentViewModeForBrowse"), ContentViewModeForBrowse, NULL ) && bOK;
	bOK = RegisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("ContentViewModeForSearch"), ContentViewModeForSearch, NULL ) && bOK;

	SetRegValue( sDefaultKey, sFileExt + _T("\\OpenWithProgids"), HKEY_CURRENT_USER );

	// Clean empty keys
	if ( ! sUserChoice.IsEmpty() )
		DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sUserChoice + _T("\\ShellEx") );
	DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sType + _T("\\ShellEx") );
	DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sType );
	if ( ! sDefaultKey.IsEmpty() )
	{
		DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sDefaultKey + _T("\\ShellEx") );
		DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sDefaultKey );
	}

	return bOK;
}

BOOL CSageThumbsModule::UnregisterExt(LPCTSTR szExt, bool bFull)
{
	BOOL bOK = TRUE;

	CString sType( _T(".") );
	sType += szExt;
	CString sFileExt = FileExts + sType;
	CString sDefaultKey = REG_SAGETHUMBS_IMG + sType;
	CString sCurrentKey = GetRegValue( _T(""), CString(), sType, HKEY_CLASSES_ROOT );

	CString sUserChoice = GetRegValue( _T("Progid"), CString(), sFileExt + _T("\\UserChoice"), HKEY_CURRENT_USER );

	bOK = DeleteRegValue( _T("TypeOverlay"), sCurrentKey, HKEY_CLASSES_ROOT ) && bOK;

	/*CString sContentExt = GetContentType( szExt );
	CString sContentKey = _T("MIME\\DataBase\\Content Type\\image/") + sContentExt;
	CString sImageFilterCLSID = GetRegValue( _T("Image Filter CLSID"), _T(""), sContentKey, HKEY_CLASSES_ROOT );
	if ( sImageFilterCLSID.CompareNoCase( CLSID_THUMB ) == 0 )
	{
		bOK = DeleteRegValue( _T("Image Filter CLSID"), sContentKey, HKEY_CLASSES_ROOT ) && bOK;
		bOK = DeleteRegValue( _T("CLSID"), sContentKey, HKEY_CLASSES_ROOT ) && bOK;
		bOK = DeleteRegValue( _T("0"), sContentKey + _T("\\Bits"), HKEY_CLASSES_ROOT ) && bOK;
		DeleteRegKey( HKEY_CLASSES_ROOT, sContentKey + _T("\\Bits") );
		DeleteRegKey( HKEY_CLASSES_ROOT, sCurrentKey + _T("\\CLSID") );
	}*/

	// Unregister all handlers
	for ( int i = 0; Handlers[ i ].szName; ++i )
	{
		bOK = UnregisterValue( HKEY_CLASSES_ROOT, sType + _T("\\ShellEx\\") + Handlers[ i ].szName ) && bOK;
		DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sType + _T("\\ShellEx\\") + Handlers[ i ].szName );

		if ( ! sCurrentKey.IsEmpty() )
		{
			bOK = UnregisterValue( HKEY_CLASSES_ROOT, sCurrentKey + _T("\\ShellEx\\") + Handlers[ i ].szName ) && bOK;
			DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sCurrentKey + _T("\\ShellEx\\") + Handlers[ i ].szName );
		}

		if ( bFull && ! sUserChoice.IsEmpty() )
		{
			bOK = UnregisterValue( HKEY_CLASSES_ROOT, sUserChoice + _T("\\ShellEx\\") + Handlers[ i ].szName ) && bOK;
			DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sUserChoice + _T("\\ShellEx\\") + Handlers[ i ].szName );
		}
	}

	// Unregister IPropertyStore handler
	const CString sPropKey = PropertyHandlers + sType;
	bOK = UnregisterValue( HKEY_LOCAL_MACHINE, sPropKey ) && bOK;
	DeleteEmptyRegKey( HKEY_LOCAL_MACHINE, sPropKey );

	const CString sSysKey = _T("SystemFileAssociations\\") + sType;
//	bOK = UnregisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("ConflictPrompt"), ConflictPrompt, _T("ConflictPrompt.") REG_SAGETHUMBS_BAK ) && bOK;
	bOK = UnregisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("ExtendedTileInfo"), ExtendedTileInfo, _T("ExtendedTileInfo.") REG_SAGETHUMBS_BAK ) && bOK;
	bOK = UnregisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("TileInfo"), TileInfo, _T("TileInfo.") REG_SAGETHUMBS_BAK ) && bOK;
	bOK = UnregisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("FullDetails"), FullDetails, _T("FullDetails.") REG_SAGETHUMBS_BAK ) && bOK;
	bOK = UnregisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("InfoTip"), InfoTip, _T("InfoTip.") REG_SAGETHUMBS_BAK ) && bOK;
	bOK = UnregisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("PreviewDetails"), PreviewDetails, _T("PreviewDetails.") REG_SAGETHUMBS_BAK ) && bOK;
	bOK = UnregisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("PreviewTitle"), PreviewTitle, _T("PreviewTitle.") REG_SAGETHUMBS_BAK ) && bOK;
	bOK = UnregisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("ContentViewModeForBrowse"), ContentViewModeForBrowse, _T("ContentViewModeForBrowse.") REG_SAGETHUMBS_BAK ) && bOK;
	bOK = UnregisterValue( HKEY_CLASSES_ROOT, sSysKey, _T("ContentViewModeForSearch"), ContentViewModeForSearch, _T("ContentViewModeForSearch.") REG_SAGETHUMBS_BAK ) && bOK;
	DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sSysKey );

	DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sType + _T("\\ShellEx") );
	DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sType );
	if ( ! sCurrentKey.IsEmpty() )
	{
		DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sCurrentKey + _T("\\ShellEx") );
		DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sCurrentKey );
	}

	if ( bFull )
	{
		// Restore old association
		bOK = UnregisterValue( HKEY_CLASSES_ROOT, sType, _T(""), sDefaultKey ) && bOK;

		// Test for unused key and delete it
		sCurrentKey = GetRegValue( _T(""), CString(), sType, HKEY_CLASSES_ROOT );
		if ( sCurrentKey.CompareNoCase( sDefaultKey ) != 0 &&
			IsKeyExists( HKEY_CLASSES_ROOT, sDefaultKey ) )
		{
			DeleteRegKey( HKEY_CLASSES_ROOT, sDefaultKey );

			DeleteRegValue( sDefaultKey, sType + _T("\\OpenWithProgids"), HKEY_CLASSES_ROOT );
			DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sType + _T("\\OpenWithProgids") );

			DeleteRegValue( sDefaultKey, sFileExt + _T("\\OpenWithProgids"), HKEY_CURRENT_USER );
			if ( sCurrentKey.IsEmpty() )
				DeleteRegValue( _T("Progid"), sFileExt + _T("\\UserChoice"), HKEY_CURRENT_USER );
			else
				SetRegValue( _T("Progid"), sCurrentKey, sFileExt + _T("\\UserChoice"), HKEY_CURRENT_USER );

			// Extra clean-up
			UnregisterExt( szExt, false );
		}

		// Clean empty keys
		if ( ! sUserChoice.IsEmpty() )
			DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sUserChoice + _T("\\ShellEx") );
		DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sFileExt + _T("\\OpenWithList") );
		DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sFileExt + _T("\\OpenWithProgids") );
		DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sFileExt + _T("\\UserChoice") );
		DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sFileExt );
	}

	FixProgID( szExt );

	return bOK;
}

void CSageThumbsModule::FixProgID(LPCTSTR szExt)
{
	CString sType( _T(".") );
	sType += szExt;
	CString sFileExt = FileExts + sType;

	// Test for orphan extension key
	CString sCurrentKey = GetRegValue( _T(""), CString(), sType, HKEY_CLASSES_ROOT );
	if ( ! sCurrentKey.IsEmpty() && IsKeyExists( HKEY_CLASSES_ROOT, sCurrentKey ) )
		return;

	// Test for standard types
	CString sDefaultType = GetDefaultType( szExt );
	if ( ! sDefaultType.IsEmpty() )
	{
		// Restore standard image type
		SetRegValue( _T(""), sDefaultType, sType, HKEY_CLASSES_ROOT );
		return;
	}

	// Trying to restore ProgID from .ext\OpenWithProgids
	CString sGoodProgID;
	HKEY hKey = NULL;
	LSTATUS res = RegOpenKeyEx( HKEY_CLASSES_ROOT, sType + _T("\\OpenWithProgids"), 0, KEY_READ, &hKey );
	if ( res == ERROR_SUCCESS )
	{
		CAtlList< CString > oOrphanProgIDs;
		for ( int i = 0;; ++i )
		{
			CString sProgID;
			DWORD dwSize = MAX_PATH;
			res = RegEnumValue( hKey, i, sProgID.GetBuffer( MAX_PATH ), &dwSize, 0, NULL, NULL, NULL );
			sProgID.ReleaseBuffer();
			if ( res != ERROR_SUCCESS )
				break;
			if ( IsKeyExists( HKEY_CLASSES_ROOT, sProgID ) )
				sGoodProgID = sProgID;
			else
				oOrphanProgIDs.AddTail( sProgID );
		}
		RegCloseKey( hKey );

		// Clean unused ProgID
		for ( POSITION pos = oOrphanProgIDs.GetHeadPosition(); pos; )
		{
			CString sProgID = oOrphanProgIDs.GetNext( pos );
			DeleteRegValue( sProgID, sType + _T("\\OpenWithProgids"), HKEY_CLASSES_ROOT );
		}
		DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sType + _T("\\OpenWithProgids") );
	}
	// Trying to restore ProgID from FileExts\.ext\OpenWithProgids
	res = RegOpenKeyEx( HKEY_CLASSES_ROOT, sFileExt + _T("\\OpenWithProgids"), 0, KEY_READ, &hKey );
	if ( res == ERROR_SUCCESS )
	{
		CAtlList< CString > oOrphanProgIDs;
		for ( int i = 0;; ++i )
		{
			CString sProgID;
			DWORD dwSize = MAX_PATH;
			res = RegEnumValue( hKey, i, sProgID.GetBuffer( MAX_PATH ), &dwSize, 0, NULL, NULL, NULL );
			sProgID.ReleaseBuffer();
			if ( res != ERROR_SUCCESS )
				break;
			if ( IsKeyExists( HKEY_CLASSES_ROOT, sProgID ) )
				sGoodProgID = sProgID;
			else
				oOrphanProgIDs.AddTail( sProgID );
		}
		RegCloseKey( hKey );

		// Clean unused ProgID
		for ( POSITION pos = oOrphanProgIDs.GetHeadPosition(); pos; )
		{
			CString sProgID = oOrphanProgIDs.GetNext( pos );
			DeleteRegValue( sProgID, sFileExt + _T("\\OpenWithProgids"), HKEY_CLASSES_ROOT );
		}
		DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sFileExt + _T("\\OpenWithProgids") );
		DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sFileExt );
	}
	if ( ! sGoodProgID.IsEmpty() )
	{
		// Use this
		SetRegValue( _T(""), sGoodProgID, sType, HKEY_CLASSES_ROOT );
		return;
	}

	// Clean orphan extension key
	DeleteRegValue( _T(""), sType, HKEY_CLASSES_ROOT );

	CString sPerceivedType = GetRegValue( _T("PerceivedType"), CString(), sType, HKEY_CLASSES_ROOT );
	if ( sPerceivedType.CompareNoCase( GetPerceivedType( szExt ) ) == 0 )
	{
		DeleteRegValue( _T("PerceivedType"), sType, HKEY_CLASSES_ROOT );
	}

	CString sContentType = GetRegValue( _T("Content Type"), CString(), sType, HKEY_CLASSES_ROOT );
	if ( sContentType.CompareNoCase( GetContentType( szExt ) ) == 0 )
	{
		DeleteRegValue( _T("Content Type"), sType, HKEY_CLASSES_ROOT );
	}

	DeleteEmptyRegKey( HKEY_CLASSES_ROOT, sType );
}

void CSageThumbsModule::FillExtMap()
{
#ifdef GFL_THREAD_SAFE
	CLock oLock( m_pSection );
#endif // GFL_THREAD_SAFE

	const CString key = CString( REG_SAGETHUMBS ) + _T("\\");

	m_oExtMap.RemoveAll();

	// Загрузка расширений через GFL
	const int count = gflGetNumberOfFormat();
	for ( int i = 0; i < count; ++i )
	{
		GFL_FORMAT_INFORMATION info = {};
		GFL_ERROR err = gflGetFormatInformationByIndex (i, &info);
		if ( err == GFL_NO_ERROR && ( info.Status & GFL_READ ) )
		{
			Ext data = { true, false, (LPCTSTR)CA2T( info.Description ) };
			for ( UINT j = 0; j < info.NumberOfExtension; ++j )
			{
				CString sExt = (LPCTSTR)CA2T( info.Extension [ j ] );
				sExt.MakeLower();

				// GFL bug fix for short extensions
				if ( sExt == _T("pspbrus") )		// PaintShopPro Brush
					sExt = _T("pspbrush");
				else if ( sExt == _T("pspfram") )	// PaintShopPro Frame
					sExt = _T("pspframe");
				else if ( sExt == _T("pspimag") )	// PaintShopPro Image
					sExt = _T("pspimage");
				
				m_oExtMap.SetAt( sExt, data );
			}
		}
	}

	// Load user-defined custom extensions
	HKEY hKey;
	LRESULT res = RegOpenKeyEx( HKEY_CURRENT_USER, key, 0, KEY_READ, &hKey );
	if ( res == ERROR_SUCCESS )
	{
		for ( int i = 0;; ++i )
		{
			CString sExt;
			res = RegEnumKey( hKey, i, sExt.GetBuffer( MAX_PATH ), MAX_PATH );
			sExt.ReleaseBuffer();
			if ( res != ERROR_SUCCESS )
				break;
			Ext foo;
			if ( ! m_oExtMap.Lookup( sExt, foo ) )
			{
				const Ext data = { true, true, CUSTOM_TYPE };
				m_oExtMap.SetAt( sExt, data );
			}
		}
		RegCloseKey( hKey );
	}

	// Load extensions "enable/disable" state
	for ( POSITION pos = m_oExtMap.GetHeadPosition(); pos; )
	{
		CExtMap::CPair* p = m_oExtMap.GetNext (pos);

		DWORD dwEnabled = GetRegValue( _T("Enabled"), 2ul, key + p->m_key );
		if ( dwEnabled == 2 )
		{
			// No extension
			dwEnabled = ( IsDisabledByDefault( p->m_key ) ? 0ul : 1ul );						// Use default
			SetRegValue( _T("Enabled"), ( p->m_value.enabled ? 1ul : 0ul ), key + p->m_key );	// Explicit create key
		}
		p->m_value.enabled = ( dwEnabled != 0 );
	}

	ATLTRACE( "Loaded %d formats, %d extensions. ", count, m_oExtMap.GetCount() );
}

void CSageThumbsModule::AddCustomTypes(const CString& sCustom)
{
	const CString key = CString( REG_SAGETHUMBS ) + _T("\\");

	// Disable all old custom extensions
	for ( POSITION pos = m_oExtMap.GetHeadPosition(); pos; )
	{
		CExtMap::CPair* p = m_oExtMap.GetNext (pos);
		if ( p->m_value.custom )
		{
			p->m_value.enabled = false;
		}
	}

	// Parse custom extensions string
	for ( int i = 0; ; )
	{
		CString sExt = sCustom.Tokenize( _T(";,*?<>:\r\n/\\\"\'| \t"), i );
		if ( sExt.IsEmpty() )
			// No more
			break;
		sExt.Trim( _T(".") );
		if ( sExt.IsEmpty() )
			// Skip empty
			continue;
		sExt.MakeLower();

		if ( CExtMap::CPair* p = _Module.m_oExtMap.Lookup( sExt ) )
		{
			// Re-enable custom extensions
			p->m_value.enabled = true;
		}
		else
		{
			// New custom extension
			const Ext data = { true, true, CUSTOM_TYPE };
			_Module.m_oExtMap.SetAt( sExt, data );
		}
		SetRegValue( _T("Enabled"), 1ul, key + sExt );
	}

	// Commit changes to registry
	for ( POSITION pos = m_oExtMap.GetHeadPosition(); pos; )
	{
		CExtMap::CPair* p = m_oExtMap.GetNext (pos);
		if ( p->m_value.custom && ! p->m_value.enabled )
		{
			SetRegValue( _T("Enabled"), 0ul, key + p->m_key );
		}
	}
}

BOOL CSageThumbsModule::Initialize()
{
	ATLTRACE ( "CSageThumbsModule::Initialize ()\n" );

#ifdef _DEBUG
	TCHAR user_name [256] = { _T("[unknown]") };
	DWORD user_name_size = _countof( user_name );
	GetUserName ( user_name, &user_name_size );
	ATLTRACE( "Running under user: %s%s\n", (LPCSTR)CT2A( (LPCTSTR)user_name ), IsProcessElevated() ? " (Elevated)" : "" );
#endif

	m_oLangs.Load( m_sModuleFileName );
	m_oLangs.Select( (LANGID)(DWORD)GetRegValue( _T("Lang"), (DWORD)LANG_NEUTRAL ) );
	SetRegValue( _T("Lang"), m_oLangs.GetLang() );

	m_hSQLite = LoadLibrary( m_sHome + LIB_SQLITE );
	if ( ! m_hSQLite )
		// Ошибка загрузки
		return FALSE;

	// Загрузка библиотек
	m_hGFL = LoadLibrary( m_sHome + LIB_GFL );
	if ( ! m_hGFL )
		// Ошибка загрузки
		return FALSE;

	m_hGFLe = LoadLibrary( m_sHome + LIB_GFLE );
	if ( ! m_hGFLe )
		// Ошибка загрузки
		return FALSE;

	// Get XnView folder
#ifdef WIN64
	LPCTSTR szPluginsKey = _T("PlugIns64");
#else
	LPCTSTR szPluginsKey = _T("PlugIns32");
#endif
	CString sPlugins = GetRegValue( szPluginsKey, CString() );
	if ( ! sPlugins.IsEmpty() && GetFileAttributes( sPlugins ) == INVALID_FILE_ATTRIBUTES )
		sPlugins.Empty();
	if ( sPlugins.IsEmpty() )
	{
		// 	UninstallString = "C:\Program Files\XnView\unins000.exe"
		CString buf = GetRegValue( REG_XNVIEW_PATH1, CString(), REG_XNVIEW_KEY, HKEY_LOCAL_MACHINE );
		if ( ! buf.IsEmpty() )
		{
			buf.Trim (_T("\""));
			int n = buf.ReverseFind (_T('\\'));
			if ( n > 0 )
			{
				buf = buf.Left (n) + _T("\\PlugIns");
				sPlugins = buf;
			}
			if ( ! sPlugins.IsEmpty() && GetFileAttributes( sPlugins ) == INVALID_FILE_ATTRIBUTES )
				sPlugins.Empty();
		}
	}
	if ( sPlugins.IsEmpty() )
	{
		// Inno Setup: App Path = C:\Program Files\XnView
		CString buf = GetRegValue( REG_XNVIEW_PATH2, CString(), REG_XNVIEW_KEY, HKEY_LOCAL_MACHINE );
		if ( ! buf.IsEmpty() )
		{
			buf.Trim (_T("\""));
			buf.TrimRight (_T("\\"));
			buf += _T("\\PlugIns");
			sPlugins = buf;
		}
		if ( ! sPlugins.IsEmpty() && GetFileAttributes( sPlugins ) == INVALID_FILE_ATTRIBUTES )
			sPlugins.Empty();
	}
	if ( sPlugins.IsEmpty() )
	{
		// %Program Files%\XnView\PlugIns
		CString buf = GetSpecialFolderPath(
#ifdef WIN64
			CSIDL_PROGRAM_FILES
#else
			CSIDL_PROGRAM_FILESX86
#endif
			);
		if ( ! buf.IsEmpty() )
		{
			buf.TrimRight (_T("\\"));
			buf += _T("\\XnView\\PlugIns");
			sPlugins = buf;
		}
		if ( ! sPlugins.IsEmpty() && GetFileAttributes( sPlugins ) == INVALID_FILE_ATTRIBUTES )
			sPlugins.Empty();
	}
	if ( ! sPlugins.IsEmpty() )
	{
		SetRegValue( szPluginsKey, sPlugins );
		gflSetPluginsPathnameT( sPlugins );
		ATLTRACE( "gflSetPluginsPathnameW : %s=\"%s\"\n", (LPCSTR)CT2A( szPluginsKey ), (LPCSTR)CT2A( sPlugins ) );
	}

	// Инициализация GFL
	GFL_ERROR err = gflLibraryInit();
	if ( err != GFL_NO_ERROR )
		// Ошибка инициализациия GFL
		return FALSE;

	gflEnableLZW (GFL_TRUE);

	ATLTRACE( "gflLibraryInit : GFL Version %s\n", gflGetVersion() );

	FillExtMap ();

	// Инициализация карты битовых масок форматов файлов
	//for (int i = 0; _Bits [i].ext; ++i)
	//	_BitsMap.SetAt (_Bits [i].ext, &_Bits [i]);

	return TRUE;
}

void CSageThumbsModule::UnInitialize ()
{
	ATLTRACE ( "CSageThumbsModule::UnInitialize ()\n" );

	if ( m_hGFLe )
	{
		FreeLibrary( m_hGFLe );
		m_hGFLe = NULL;
		__FUnloadDelayLoadedDLL2( LIB_GFLE );
	}

	if ( m_hGFL )
	{
		gflLibraryExit();

		FreeLibrary( m_hGFL );
		m_hGFL = NULL;
		__FUnloadDelayLoadedDLL2( LIB_GFL );
	}

	if ( m_hSQLite )
	{
		FreeLibrary( m_hSQLite );
		m_hSQLite = NULL;
		__FUnloadDelayLoadedDLL2( LIB_SQLITE );
	}

	m_oLangs.Empty();
}

extern "C" BOOL WINAPI DllMain(HINSTANCE /* hInstance */, DWORD dwReason, LPVOID lpReserved)
{
	return _Module.DllMain( dwReason, lpReserved );
}

STDAPI DllCanUnloadNow(void)
{
	HRESULT hr = _Module.DllCanUnloadNow();
	ATLTRACE( "CThumb - DllCanUnloadNow() : 0x%08x\n", hr );
	return hr;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.DllGetClassObject( rclsid, riid, ppv );
}

STDAPI DllRegisterServer(void)
{
	HRESULT hr = _Module.DllRegisterServer();
	ATLTRACE( "CThumb - DllRegisterServer() : 0x%08x\n", hr );
	return hr;
}

STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _Module.DllUnregisterServer();
	ATLTRACE( "CThumb - DllUnregisterServer() : 0x%08x\n", hr );
	return hr;
}

// DllInstall - Adds/Removes entries to the system registry per user per machine.
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	HRESULT hr = E_FAIL;
	static const wchar_t szUserSwitch[] = L"user";

	if ( pszCmdLine )
	{
		if ( _wcsnicmp( pszCmdLine, szUserSwitch, _countof( szUserSwitch ) ) == 0 )
		{
			ATL::AtlSetPerUserRegistration(true);
		}
	}

	if ( bInstall )
	{
		hr = DllRegisterServer();
		if ( FAILED( hr ) )
		{
			DllUnregisterServer();
		}
	}
	else
	{
		hr = DllUnregisterServer();
	}

	return hr;
}

void CALLBACK Options (HWND hwnd, HINSTANCE /* hinst */, LPSTR /* lpszCmdLine */, int /* nCmdShow */)
{
	OleInitialize( NULL );

	InitCommonControls();

	COptionsDialog dlg;
	while ( dlg.DoModal( hwnd ) == IDRETRY );

	OleUninitialize();
}

LONG APIENTRY CPlApplet(HWND hwnd, UINT uMsg, LPARAM /* lParam1 */, LPARAM lParam2)
{
	ATLTRACE( "Calling ::CPlApplet()...\n" );

	switch ( uMsg )
	{
	case CPL_DBLCLK:
	case CPL_STARTWPARMS:
		Options (hwnd);
	case CPL_INIT:
	case CPL_STOP:
	case CPL_EXIT:
		return TRUE;

	case CPL_GETCOUNT:
		return 1;

	case CPL_INQUIRE:
		{
			CPLINFO* info = reinterpret_cast <CPLINFO*> (lParam2);
			info->idIcon = IDR_SAGETHUMBS;
			info->idName = IDS_PROJNAME;
			info->idInfo = IDS_OPTIONS;
			info->lData  = 0;
		}
		break;
	}
	return FALSE;
}

bool IsValidCLSID(const CString& sCLSID)
{
	static CRBMap < CString, bool > oCLSIDCache;
	bool bIsValid = false;
	if ( oCLSIDCache.Lookup( sCLSID, bIsValid ) )
		return bIsValid;

	CString sPath = GetRegValue( _T(""), CString(), _T("CLSID\\") + sCLSID + _T("\\InprocServer32"), HKEY_CLASSES_ROOT );
	sPath.Trim();
	if ( sPath.IsEmpty() )
	{
		sPath = GetRegValue( _T(""), CString(), _T("CLSID\\") + sCLSID + _T("\\LocalServer32"), HKEY_CLASSES_ROOT );
		sPath.Trim();
	}
	if ( ! sPath.IsEmpty() )
	{
		// Test for quoted path
		if ( sPath.GetAt( 0 ) == _T('\"') )
		{
			int nSlash = sPath.Find( _T('\"'), 1 );
			if ( nSlash != -1 )
			{
				sPath = sPath.Mid( 1, nSlash - 1 );
				sPath.Trim();
			}
		}

		if ( HMODULE hModule = LoadLibraryEx( sPath, NULL, LOAD_LIBRARY_AS_DATAFILE ) )
		{
			FreeLibrary( hModule );
			bIsValid = true;
		}
		else
		{
			// Test for path with arguments
			PathRemoveArgs( sPath.GetBuffer( MAX_PATH ) );
			sPath.ReleaseBuffer();
			sPath.Trim();
			if ( HMODULE hModule = LoadLibraryEx( sPath, NULL, LOAD_LIBRARY_AS_DATAFILE ) )
			{
				FreeLibrary( hModule );
				bIsValid = true;
			}
		}
	}

	ATLTRACE( "IsValidCSID( %s ) detected %s CLSID path: \"%s\"\n", (LPCSTR)CT2A( sCLSID ), ( bIsValid ? "good" : "bad" ), (LPCSTR)CT2A( sPath ) );
	oCLSIDCache.SetAt( sCLSID, bIsValid );
	return bIsValid;
}

BOOL GetRegValue(LPCTSTR szName, LPCTSTR szKey, HKEY hRoot)
{
	DWORD dwType = REG_NONE, dwSize = 0;
	LSTATUS res = SHGetValue( hRoot, szKey, szName, &dwType, NULL, &dwSize );
	return ( res == ERROR_SUCCESS && dwType == REG_NONE && dwSize == 0 );
}

DWORD GetRegValue(LPCTSTR szName, DWORD dwDefault, LPCTSTR szKey, HKEY hRoot)
{
	DWORD dwValue, dwType = REG_DWORD, dwSize = sizeof( DWORD );
	LSTATUS res = SHGetValue( hRoot, szKey, szName, &dwType, &dwValue, &dwSize );
	return ( res == ERROR_SUCCESS && dwType == REG_DWORD &&
		dwSize == sizeof( DWORD ) ) ? dwValue : dwDefault;
}

CString GetRegValue(LPCTSTR szName, const CString& sDefault, LPCTSTR szKey, HKEY hRoot)
{
	HKEY hKey;
	LSTATUS res = RegOpenKeyEx( hRoot, szKey, NULL, KEY_READ, &hKey );
	if ( res == ERROR_SUCCESS )
	{
		DWORD dwType, dwSize = 0;
		res = RegQueryValueEx( hKey, szName, NULL, &dwType, (BYTE*)1, &dwSize );
		if ( res == ERROR_MORE_DATA && ( dwType == REG_SZ || dwType == REG_EXPAND_SZ ) )
		{
			dwSize += MAX_PATH;
			const DWORD dwValueSize = dwSize / sizeof( TCHAR );
			CAutoVectorPtr< TCHAR > pValue( new TCHAR[ dwValueSize ] );
			if ( pValue )
			{
				res = RegQueryValueEx( hKey, szName, NULL, &dwType, (BYTE*)(TCHAR*)pValue, &dwSize );
				if ( res == ERROR_SUCCESS )
				{
					RegCloseKey( hKey );
					pValue[ dwSize / sizeof( TCHAR ) ] = _T('\0');
					if ( dwType == REG_EXPAND_SZ )
					{
						DoEnvironmentSubst( pValue, dwValueSize );
					}
					return (LPCTSTR)pValue;
				}
			}
		}
		RegCloseKey( hKey );
	}
	return sDefault;
}

BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege) 
{
	TOKEN_PRIVILEGES tp = {};
	LUID luid = {};
	if ( ! LookupPrivilegeValue( NULL, lpszPrivilege, &luid ) )
	{
		ATLTRACE( "Failed to Lookup Privilege Value for \"%s\"\n", (LPCSTR)CT2A( lpszPrivilege ) );
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[ 0 ].Luid = luid;
	if ( bEnablePrivilege )
		tp.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[ 0 ].Attributes = 0;

	if ( ! AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof( TOKEN_PRIVILEGES ), NULL, NULL) || GetLastError() == ERROR_NOT_ALL_ASSIGNED )
	{ 
		ATLTRACE( "Failed to Adjust Token Privileges for \"%s\"\n", (LPCSTR)CT2A( lpszPrivilege ) );
		return FALSE;
	}

	ATLTRACE( "Set privilege \"%s\" : %s\n", (LPCSTR)CT2A( lpszPrivilege ), bEnablePrivilege ? "Enabled" : "Disabled" );

	return TRUE;
}

BOOL FixKeyRights(HKEY hRoot, LPCTSTR szKey)
{
	BOOL bOK = FALSE;
	HKEY hKey = NULL;
	LRESULT res = RegOpenKeyEx( hRoot, szKey, 0, READ_CONTROL | WRITE_DAC | ACCESS_SYSTEM_SECURITY, &hKey );
	if ( res == ERROR_SUCCESS )
	{
		const SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
		LPCTSTR szGoodRights = _T("D:AI(A;ID;KR;;;BU)(A;CIIOID;GR;;;BU)(A;ID;KA;;;BA)(A;CIIOID;GA;;;BA)(A;ID;KA;;;SY)(A;CIIOID;GA;;;SY)(A;CIIOID;GA;;;CO)");
		
		CString sSSD;
		DWORD dwSize = 0;
		res = RegGetKeySecurity( hKey, si, NULL, &dwSize );
		if ( res == ERROR_INSUFFICIENT_BUFFER )
		{
			CAutoVectorPtr< BYTE > pBuf( new BYTE[ dwSize ] );
			PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)(BYTE*)pBuf;
			res = RegGetKeySecurity( hKey, si, pSD, &dwSize );
			if ( res == ERROR_SUCCESS )
			{
				LPTSTR pSSD = NULL;
				if ( ConvertSecurityDescriptorToStringSecurityDescriptor( pSD, SDDL_REVISION_1, si, &pSSD, NULL ) )
				{
					sSSD = pSSD;
					LocalFree( pSSD );

					/*bool bDenied = false;
					for ( ;; )
					{
						int nFrom = sSSD.Find( _T("(D;") );
						if ( nFrom == -1 )
							break;
						int nTo = sSSD.Find( _T(')'), nFrom );
						if ( nTo == -1 )
							break;
						sSSD = sSSD.Left( nFrom ) + sSSD.Mid( nTo + 1 );
						bDenied = true;
					}*/

					// If denied rights present or administrative access rights absent
					if ( sSSD.Find( _T("(D;") ) != -1 || sSSD.Find( _T("(A;ID;KA;;;BA)(A;CIIOID;GA;;;BA)") ) == -1 )
					{
						ATLTRACE( "Found bad rights of key: %s\\%s : %s\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( (LPCTSTR)sSSD ) );

						PSECURITY_DESCRIPTOR pNewSD = NULL;
						if ( ConvertStringSecurityDescriptorToSecurityDescriptor( szGoodRights /*sSSD*/, SDDL_REVISION_1, &pNewSD, NULL ) )
						{
							res = RegSetKeySecurity( hKey, si, pNewSD );
							if ( res == ERROR_SUCCESS )
							{
								ATLTRACE( "Cleared bad rights of key: %s\\%s\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ) );
								bOK = TRUE;
							}							
							else
								ATLTRACE( "Failed to set security of key: %s\\%s\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ) );
							LocalFree( pNewSD );
						}
						else
							ATLTRACE( "Failed to convert security string of key: %s\\%s\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ) );
					}
					else
					{
						ATLTRACE( "Checked good rights of key: %s\\%s : %s\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( (LPCTSTR)sSSD ) );
						bOK = TRUE;
					}
				}
				else
					ATLTRACE( "Failed to convert security string of key: %s\\%s\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ) );
			}
			else
				ATLTRACE( "Failed to get security of key: %s\\%s : %d\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), res );
		}
		else
			ATLTRACE( "Failed to get security of key: %s\\%s : %d\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), res );

		RegCloseKey( hKey );
	}
	else if ( res == ERROR_PATH_NOT_FOUND || res == ERROR_FILE_NOT_FOUND )
	{
		// OK
		bOK = TRUE;
	}
	else
	{
		ATLTRACE( "Failed to open key: %s\\%s\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ) );
	}

	return bOK;
}

BOOL FixKey(__in HKEY hkey, __in_opt LPCTSTR pszSubKey)
{
	if ( hkey != HKEY_CLASSES_ROOT )
		// Skip
		return FALSE;

	BOOL bOK = FALSE;
	HANDLE hToken = NULL;
	if ( OpenProcessToken( GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken ) ) 
	{
		if ( SetPrivilege( hToken, SE_SECURITY_NAME, TRUE ) )
		{
			if ( SetPrivilege( hToken, SE_TAKE_OWNERSHIP_NAME, TRUE ) )
			{
				// Create a SID for the BUILTIN\Administrators group.
				PSID pSIDAdmin = NULL;
				SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
				if ( AllocateAndInitializeSid( &SIDAuthNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pSIDAdmin ) ) 
				{
					bOK = TRUE;

					CString strKey = pszSubKey, strPartialKey;
					for ( int i = 0; ; )
					{
						CString strSubKey = strKey.Tokenize( _T("\\"), i );
						if ( strSubKey.IsEmpty() )
							break;

						strSubKey.MakeLower();
						if ( ! strPartialKey.IsEmpty() )
							strPartialKey += _T("\\");
						strPartialKey += strSubKey;
					
						// First try
						if ( ! FixKeyRights( hkey, strPartialKey ) )
						{
							// Fix key owner
							LSTATUS resSecurity = SetNamedSecurityInfo( (LPTSTR)(LPCTSTR)( CString( GetShortKeyName( hkey ) ) + _T("\\") + strPartialKey ),
								SE_REGISTRY_KEY, OWNER_SECURITY_INFORMATION, pSIDAdmin, NULL, NULL, NULL );
							if ( resSecurity == ERROR_SUCCESS )
							{
								ATLTRACE( "Setting new owner of key: %s\\%s\n", (LPCSTR)CT2A( GetKeyName( hkey ) ), (LPCSTR)CT2A( (LPCTSTR)strPartialKey ) );

								// Second try
								if ( ! FixKeyRights( hkey, strPartialKey ) )
								{
									bOK = FALSE;
								}
							}
							else if ( resSecurity == ERROR_PATH_NOT_FOUND || resSecurity == ERROR_FILE_NOT_FOUND )
							{
								// OK
							}
							else
							{
								ATLTRACE( "Failed to set owner of key: %s\\%s\n", (LPCSTR)CT2A( GetKeyName( hkey ) ), (LPCSTR)CT2A( (LPCTSTR)strPartialKey ) );
								bOK = FALSE;
							}
						}
					}

					FreeSid( pSIDAdmin );
				}
				else
					ATLTRACE( "Failed to Allocate And Initialize Sid\n" );

				SetPrivilege( hToken, SE_TAKE_OWNERSHIP_NAME, FALSE );
			}
			SetPrivilege( hToken, SE_SECURITY_NAME, FALSE );
		}
		CloseHandle( hToken );
	}
	else
		ATLTRACE( "Failed to Open Process Token\n" );

	return bOK;
}

LSTATUS SHSetValueForced(__in HKEY hkey, __in_opt LPCTSTR pszSubKey, __in_opt LPCTSTR pszValue, __in DWORD dwType, __in_bcount_opt(cbData) LPCVOID pvData, __in DWORD cbData)
{
	LSTATUS res;

	// Remove wrong type value if any
	if ( *pszValue )
		DeleteRegValue( pszValue, pszSubKey, hkey );

	for ( int attempt = 0; ; ++attempt )
	{
		res = SHSetValue( hkey, pszSubKey, pszValue, dwType, pvData, cbData );
		if ( res == ERROR_SUCCESS )
			break;

		// Trying to fix access error
		if ( res == ERROR_ACCESS_DENIED && attempt < 1 )
		{
			 FixKey( hkey, pszSubKey );
		}
		else
			break;
	}

	return res;
}

BOOL SetRegValue(LPCTSTR szName, LPCTSTR szKey, HKEY hRoot)
{
	if ( GetRegValue( szName, szKey, hRoot) )
		// Already set
		return TRUE;

	LSTATUS res = SHSetValueForced( hRoot, szKey, szName, REG_NONE, NULL, 0 );
	if ( res != ERROR_SUCCESS )
	{
		ATLTRACE( "Got %s during value setting: %s\\%s \"%s\"\n", ( ( res == ERROR_ACCESS_DENIED ) ? "\"Access Denied\"" : "error" ), (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ) );
		return FALSE;
	}

	ATLTRACE( "Set value: %s\\%s \"%s\"\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ) );
	return TRUE;
}

BOOL SetRegValue(LPCTSTR szName, DWORD dwValue, LPCTSTR szKey, HKEY hRoot)
{
	DWORD dwCurrent, dwType = REG_DWORD, dwSize = sizeof( DWORD );
	LSTATUS res = SHGetValue( hRoot, szKey, szName, &dwType, &dwCurrent, &dwSize );
	if ( res == ERROR_SUCCESS &&
		 dwType == REG_DWORD &&
		 dwSize == sizeof( DWORD ) &&
		 dwCurrent == dwValue )
	{
		// Already set
		return TRUE;
	}

	res = SHSetValueForced( hRoot, szKey, szName, REG_DWORD, &dwValue, sizeof( DWORD ) );
	if ( res != ERROR_SUCCESS )
	{
		ATLTRACE( "Got %s during value setting: %s\\%s \"%s\" = %d\n", ( ( res == ERROR_ACCESS_DENIED ) ? "\"Access Denied\"" : "error" ), (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ), dwValue );
		return FALSE;
	}

	ATLTRACE( "Set value: %s\\%s \"%s\" = %u\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ), dwValue );
	return TRUE;
}

BOOL SetRegValue(LPCTSTR szName, const CString& sValue, LPCTSTR szKey, HKEY hRoot)
{
	LSTATUS res;

	CString sCurrent = GetRegValue( szName, _T("#VALUE_NOT_FOUND#"), szKey, hRoot );
	if ( sCurrent.CompareNoCase( sValue ) == 0 )
	{
		// Already set
		return TRUE;
	}

	if ( _istalpha( sValue.GetAt( 0 ) ) &&
		sValue.GetAt( 1 ) == _T(':') &&
		sValue.GetAt( 2 ) == _T('\\') )
	{
		CString sCompact;
		BOOL bCompact = PathUnExpandEnvStrings( sValue, sCompact.GetBuffer( MAX_LONG_PATH ), MAX_LONG_PATH );
		sCompact.ReleaseBuffer();
		if ( bCompact )
		{
			res = SHSetValueForced( hRoot, szKey, szName, REG_EXPAND_SZ, sCompact, (DWORD)lengthof( sCompact ) );
			if ( res != ERROR_SUCCESS )
			{
				ATLTRACE( "Got %s during value setting: %s\\%s \"%s\" = \"%s\"\n", ( ( res == ERROR_ACCESS_DENIED ) ? "\"Access Denied\"" : "error" ), (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ), (LPCSTR)CT2A( sCompact ) );
				return FALSE;
			}

			ATLTRACE( "Set value: %s\\%s \"%s\" = \"%s\"\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ), (LPCSTR)CT2A( sCompact ) );
			return TRUE;
		}
	}

	res = SHSetValueForced( hRoot, szKey, szName, REG_SZ, sValue, (DWORD)lengthof( sValue ) );
	if ( res != ERROR_SUCCESS )
	{
		ATLTRACE( "Got %s during value setting: %s\\%s \"%s\" = \"%s\"\n", ( ( res == ERROR_ACCESS_DENIED ) ? "\"Access Denied\"" : "error" ), (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ), (LPCSTR)CT2A( sValue ) );
		return FALSE;
	}

	ATLTRACE( "Set value: %s\\%s \"%s\" = \"%s\"\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ), (LPCSTR)CT2A( sValue ) );
	return TRUE;
}

BOOL DeleteRegValue(LPCTSTR szName, LPCTSTR szKey, HKEY hRoot)
{
	LSTATUS res = SHGetValue( hRoot, szKey, szName, NULL, NULL, NULL );
	if ( res == ERROR_FILE_NOT_FOUND || res == ERROR_PATH_NOT_FOUND )
		// Already deleted
		return TRUE;

	// First attempt
	res = SHDeleteValue( hRoot, szKey, szName );
	if ( res == ERROR_SUCCESS )
	{
		ATLTRACE( "Deleted value: %s\\%s \"%s\"\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ) );
		res = SHDeleteValue( hRoot, szKey, szName );
	}
	if ( res != ERROR_ACCESS_DENIED )
		return TRUE;

	if ( FixKey( hRoot, szKey ) )
	{
		// Second attempt
		res = SHDeleteValue( hRoot, szKey, szName );
		if ( res == ERROR_SUCCESS )
		{
			ATLTRACE( "Deleted value: %s\\%s \"%s\"\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ) );
			res = SHDeleteValue( hRoot, szKey, szName );
		}
		if ( res != ERROR_ACCESS_DENIED )
			return TRUE;
	}

	ATLTRACE( "Got \"Access Denied\" during value deletion: %s\\%s \"%s\"\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ) );
	return FALSE;
}

BOOL DeleteRegKey(HKEY hRoot, LPCTSTR szSubKey)
{
	LSTATUS res = SHDeleteKey( hRoot, szSubKey );		// HKCU
	if ( res == ERROR_SUCCESS )
	{
		ATLTRACE( "Deleted key  : %s\\%s\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szSubKey ) );
		res = SHDeleteKey( hRoot, szSubKey );			// HKLM
	}
	return ( res != ERROR_ACCESS_DENIED );
}

BOOL DeleteEmptyRegKey(HKEY hRoot, LPCTSTR szSubKey)
{
	LSTATUS res = SHDeleteEmptyKey( hRoot, szSubKey );	// HKCU
	if ( res == ERROR_SUCCESS )
	{
		ATLTRACE( "Deleted key  : %s\\%s\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szSubKey ) );
		res = SHDeleteEmptyKey( hRoot, szSubKey );		// HKLM
	}
	return ( res != ERROR_ACCESS_DENIED );
}

LPCTSTR GetKeyName(HKEY hRoot)
{
	switch ( (DWORD_PTR)hRoot )
	{
	case HKEY_CLASSES_ROOT:
		return _T("HKEY_CLASSES_ROOT");
	case HKEY_CURRENT_USER:
		return _T("HKEY_CURRENT_USER");
	case HKEY_LOCAL_MACHINE:
		return _T("HKEY_LOCAL_MACHINE");
	case HKEY_USERS:
		return _T("HKEY_USERS");
	case HKEY_CURRENT_CONFIG:
		return _T("HKEY_CURRENT_CONFIG");
	default:
		return _T("{custom}");
	}
}

LPCTSTR GetShortKeyName(HKEY hRoot)
{
	switch ( (DWORD_PTR)hRoot )
	{
	case HKEY_CLASSES_ROOT:
		return _T("CLASSES_ROOT");
	case HKEY_CURRENT_USER:
		return _T("CURRENT_USER");
	case HKEY_LOCAL_MACHINE:
		return _T("MACHINE");
	case HKEY_USERS:
		return _T("USERS");
	default:
		return _T("");
	}
}

BOOL RegisterValue(HKEY hRoot, LPCTSTR szKey, LPCTSTR szName, LPCTSTR szValue, LPCTSTR szBackupName)
{
	// Check for new value
	CString buf = GetRegValue( szName, CString(), szKey, hRoot );
	if ( buf.CompareNoCase( szValue ) != 0 )
	{
		if ( ! szBackupName && ! buf.IsEmpty() )
		{
			// Don't replace existing value
			ATLTRACE( "Registration skipped due existing key : %s\\%s=\"%s\"\n", (LPCSTR)CT2A( GetKeyName( hRoot ) ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( buf ) );
			return TRUE;
		}

		// Set new value
		if ( ! SetRegValue( szName, szValue, szKey, hRoot ) )
			return FALSE;

		// Backup old one
		if ( ! buf.IsEmpty() )
		{
			if ( ! SetRegValue( szBackupName, buf, szKey, hRoot ) )
				return FALSE;
		}
	}
	return TRUE;
}

BOOL UnregisterValue(HKEY hRoot, LPCTSTR szKey, LPCTSTR szName, LPCTSTR szValue, LPCTSTR szBackupName)
{
	BOOL bOK = TRUE;

	// Check backup
	CString buf = GetRegValue( szName, CString(), szKey, hRoot );
	CString backup_buf = szBackupName ? GetRegValue( szBackupName, CString(), szKey, hRoot ) : CString();
	if ( buf.CompareNoCase( szValue ) == 0 )
	{
		// Check backup value
		if ( ! backup_buf.IsEmpty() )
		{
			// Restore value from backup
			bOK = SetRegValue( szName, backup_buf, szKey, hRoot ) && bOK;
		}
		else
		{
			// Delete original value
			bOK = DeleteRegValue( szName, szKey, hRoot ) && bOK;
		}
	}

	if ( ! backup_buf.IsEmpty() )
	{
		// Delete backup value
		bOK = DeleteRegValue( szBackupName, szKey, hRoot ) && bOK;
	}

	// Clean-up empty key
	DeleteEmptyRegKey( hRoot, szKey );

	return bOK;
}

HRESULT CSageThumbsModule::GetFileInformation(LPCTSTR filename, GFL_FILE_INFORMATION* info)
{
#ifdef GFL_THREAD_SAFE
	CLock oLock( m_pSection );
	return GetFileInformationE( filename, info );
}
HRESULT CSageThumbsModule::GetFileInformationE(LPCTSTR filename, GFL_FILE_INFORMATION* info)
{
#endif // GFL_THREAD_SAFE
	GFL_ERROR err;
	HRESULT hr = E_FAIL;

	__try
	{
		err = gflGetFileInformationT( filename, -1, info );
		if ( err == GFL_NO_ERROR )
			hr = S_OK;
		else
		{
			ATLTRACE ("E_FAIL (gflGetFileInformationW) : %s\n", gflGetErrorString (err));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ATLTRACE ("E_FAIL (gflGetFileInformationW exception)\n");
	}

	return hr;
}

HRESULT CSageThumbsModule::LoadGFLBitmap(LPCTSTR filename, GFL_BITMAP **bitmap)
{
#ifdef GFL_THREAD_SAFE
	CLock oLock( m_pSection );
	return LoadGFLBitmapE( filename, bitmap );
}
HRESULT CSageThumbsModule::LoadGFLBitmapE(LPCTSTR filename, GFL_BITMAP **bitmap)
{
#endif // GFL_THREAD_SAFE
	GFL_ERROR err;
	HRESULT hr = E_FAIL;
	__try
	{
		*bitmap = NULL;

		GFL_LOAD_PARAMS params = {};
		gflGetDefaultLoadParams( &params );
		params.ColorModel = GFL_RGBA;
		err = gflLoadBitmapT( filename, bitmap, &params, NULL);
		if ( err == GFL_ERROR_FILE_READ )
		{
			params.Flags |= GFL_LOAD_IGNORE_READ_ERROR;

			err = gflLoadBitmapT( filename, bitmap, &params, NULL );
		}
		if ( err == GFL_NO_ERROR )
		{
			if ( (*bitmap)->Type != GFL_RGBA )
			{
				Change_Color_Depth( *bitmap );
			}
			hr = S_OK;
		}
		else
		{
			ATLTRACE ("E_FAIL (gflLoadBitmapW) : %s\n", gflGetErrorString (err));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ATLTRACE ("E_FAIL (gflLoadBitmapW exception)\n");
	}

	return hr;
}

HRESULT CSageThumbsModule::LoadThumbnail(LPCTSTR filename, int width, int height, GFL_BITMAP **bitmap)
{
#ifdef GFL_THREAD_SAFE
	CLock oLock( m_pSection );
	return LoadThumbnailE( filename, width, height, bitmap );
}
HRESULT CSageThumbsModule::LoadThumbnailE(LPCTSTR filename, int width, int height, GFL_BITMAP **bitmap)
{
#endif // GFL_THREAD_SAFE
	GFL_ERROR err;
	HRESULT hr = E_FAIL;
	__try
	{
		*bitmap = NULL;

		GFL_LOAD_PARAMS params = {};
		gflGetDefaultThumbnailParams( &params );
		params.Flags =
			GFL_LOAD_HIGH_QUALITY_THUMBNAIL |
			( ( ::GetRegValue( _T("UseEmbedded"), 0ul ) != 0 ) ? GFL_LOAD_EMBEDDED_THUMBNAIL : 0 ) |
			GFL_LOAD_PREVIEW_NO_CANVAS_RESIZE;
		params.ColorModel = GFL_RGBA;
		err = gflLoadThumbnailT( filename, width, height, bitmap, &params, NULL );
		if ( err == GFL_ERROR_FILE_READ )
		{
			params.Flags |= GFL_LOAD_IGNORE_READ_ERROR;

			err = gflLoadThumbnailT( filename, width, height, bitmap, &params, NULL );
		}
		if ( err == GFL_NO_ERROR )
		{
			if ( (*bitmap)->Type != GFL_RGBA )
			{
				Change_Color_Depth( *bitmap );
			}
			hr = S_OK;
		}
		else
		{
			ATLTRACE ("E_FAIL (gflLoadThumbnailW) : %s\n", gflGetErrorString (err));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ATLTRACE ("E_FAIL (gflLoadThumbnailW exception)\n");
	}

	return hr;
}

HRESULT CSageThumbsModule::LoadBitmapFromMemory(LPCVOID data, UINT data_length, GFL_BITMAP **bitmap)
{
#ifdef GFL_THREAD_SAFE
	CLock oLock( m_pSection );
	return LoadBitmapFromMemoryE( data, data_length, bitmap );
}
HRESULT CSageThumbsModule::LoadBitmapFromMemoryE(LPCVOID data, UINT data_length, GFL_BITMAP **bitmap)
{
#endif // GFL_THREAD_SAFE
	GFL_ERROR err;
	HRESULT hr = E_FAIL;
	__try
	{
		*bitmap = NULL;

		GFL_LOAD_PARAMS params = {};
		gflGetDefaultLoadParams( &params );
		params.ColorModel = GFL_RGBA;
		err = gflLoadBitmapFromMemory( (const BYTE*)data, data_length, bitmap, &params, NULL );
		if ( err == GFL_ERROR_FILE_READ )
		{
			params.Flags |= GFL_LOAD_IGNORE_READ_ERROR;

			err = gflLoadBitmapFromMemory( (const BYTE*)data, data_length, bitmap, &params, NULL );
		}
		if ( err == GFL_NO_ERROR )
		{
			if ( (*bitmap)->Type != GFL_RGBA )
			{
				Change_Color_Depth( *bitmap );
			}
			hr = S_OK;
		}
		else
		{
			ATLTRACE ("E_FAIL (gflLoadBitmapFromMemory) : %s\n", gflGetErrorString (err));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ATLTRACE ("E_FAIL (gflLoadBitmapFromMemory exception)\n");
	}

	return hr;
}

HRESULT CSageThumbsModule::ConvertBitmap(const GFL_BITMAP *bitmap, HBITMAP *phBitmap)
{
#ifdef GFL_THREAD_SAFE
	CLock oLock( m_pSection );
	return ConvertBitmapE( bitmap, phBitmap );
}
HRESULT CSageThumbsModule::ConvertBitmapE(const GFL_BITMAP *bitmap, HBITMAP *phBitmap)
{
#endif // GFL_THREAD_SAFE
	GFL_ERROR err;
	HRESULT hr = E_FAIL;
	__try
	{
		*phBitmap = NULL;

		if ( ( err = gflConvertBitmapIntoDDB( bitmap, phBitmap ) ) == GFL_NO_ERROR )
		{
			hr = S_OK;
		}
		else if ( (err = gflConvertBitmapIntoDIBSection( bitmap, phBitmap ) ) == GFL_NO_ERROR )
		{
			hr = S_OK;
		}
		else
			ATLTRACE ("E_FAIL (gflConvertBitmapIntoDDB) : %s\n", gflGetErrorString (err));
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ATLTRACE ("E_FAIL (gflConvertBitmapIntoDDB exception)\n");
	}

	return hr;
}

HRESULT CSageThumbsModule::Resize(GFL_BITMAP* src, GFL_BITMAP** dst, int width, int height)
{
#ifdef GFL_THREAD_SAFE
	CLock oLock( m_pSection );
	return ResizeE( src, dst, width, height );
}
HRESULT CSageThumbsModule::ResizeE(GFL_BITMAP* src, GFL_BITMAP** dst, int width, int height)
{
#endif // GFL_THREAD_SAFE
	GFL_ERROR err;
	HRESULT hr = E_FAIL;
	__try
	{
		*dst = NULL;

		err = gflResize( src, dst, width, height, GFL_RESIZE_HERMITE, 0 );
		if ( err == GFL_NO_ERROR )
		{
			hr = S_OK;
		}
		else
		{
			ATLTRACE ("E_FAIL (gflResize) : %s\n", gflGetErrorString (err));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ATLTRACE ("E_FAIL (gflResize exception)\n");
	}

	return hr;
}

HRESULT CSageThumbsModule::FreeBitmap(GFL_BITMAP*& bitmap)
{
#ifdef GFL_THREAD_SAFE
	CLock oLock( m_pSection );
	return FreeBitmapE( bitmap );
}
HRESULT CSageThumbsModule::FreeBitmapE(GFL_BITMAP*& bitmap)
{
#endif // GFL_THREAD_SAFE
	if ( ! bitmap )
		return S_FALSE;

	HRESULT hr = E_FAIL;
	__try
	{
		gflFreeBitmap( bitmap );
		hr = S_OK;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ATLTRACE ("E_FAIL (gflFreeBitmap exception)\n");
	}

	bitmap = NULL;

	return hr;
}

bool CSageThumbsModule::IsGoodFile(LPCTSTR szFilename, Ext* pdata, WIN32_FIND_DATA* pfd) const
{
	CString sExt = PathFindExtension( szFilename );
	if ( sExt.GetLength() < 2 )
		// No extension
		return false;
	sExt.MakeLower();

	Ext foo_data;
	if ( ! pdata )
		pdata = &foo_data;
	if ( ! m_oExtMap.Lookup( (LPCTSTR)sExt + 1, *pdata ) )
		// Unsupported extension
		return false;

	if ( ! pdata->enabled )
		// Disabled extension
		return false;

	WIN32_FIND_DATA foo_fd;
	if ( ! pfd )
		pfd = &foo_fd;
	if ( ! GetFileAttributesEx( szFilename, GetFileExInfoStandard, pfd ) )
		// File error
		return false;

	if ( ( pfd->dwFileAttributes & ( FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE |
		FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_OFFLINE ) ) != 0 )
		// Bad attributes
		return false;

	return true;
}

CString GetDefaultType(LPCTSTR szExt)
{
	struct
	{
		LPCTSTR szType;
		LPCTSTR szProgID;
	}
	static const DefaultTypes[] =
	{
		{ _T("bmp"),	_T("Paint.Picture") },
		{ _T("dib"),	_T("Paint.Picture") },
		{ _T("emf"),	_T("emffile") },
		{ _T("ico"),	_T("icofile") },
		{ _T("jfif"),	_T("pjpegfile") },
		{ _T("jpe"),	_T("jpegfile") },
		{ _T("jpeg"),	_T("jpegfile") },
		{ _T("jpg"),	_T("jpegfile") },
		{ _T("gif"),	_T("giffile") },
		{ _T("mdi"),	_T("MSPaper.Document") },
		{ _T("png"),	_T("pngfile") },
		{ _T("rle"),	_T("rlefile") },
		{ _T("tif"),	_T("TIFImage.Document") },
		{ _T("tiff"),	_T("TIFImage.Document") },
		{ _T("wdp"),	_T("wdpfile") },
		{ _T("wmf"),	_T("wmffile") },
		{ NULL,			NULL }
	};
	for ( int i = 0; DefaultTypes[ i ].szType; ++i )
	{
		if ( _tcsicmp( szExt, DefaultTypes[ i ].szType ) == 0 )
		{
			return DefaultTypes[ i ].szProgID;
		}
	}
	return CString();
}

CString GetPerceivedType(LPCTSTR /* szExt */)
{
	return _T("image");
}

CString GetContentExt(LPCTSTR szExt)
{
	if ( _tcsicmp( szExt, _T("jfif") ) == 0 ||
		 _tcsicmp( szExt, _T("jpe") ) == 0 ||
		 _tcsicmp( szExt, _T("jpeg") ) == 0 ||
		 _tcsicmp( szExt, _T("jpg") ) == 0 )
		return _T("jpeg");
	else if ( _tcsicmp( szExt, _T("bmp") ) == 0 ||
			  _tcsicmp( szExt, _T("dib") ) == 0 )
		return _T("bmp");
	else if ( _tcsicmp( szExt, _T("tif") ) == 0 ||
			  _tcsicmp( szExt, _T("tiff") ) == 0 )
		return _T("tif");
	else
		return szExt;
}

CString GetContentType(LPCTSTR szExt)
{
	if ( _tcsicmp( szExt, _T("jfif") ) == 0 ||
		 _tcsicmp( szExt, _T("jpe") ) == 0 ||
		 _tcsicmp( szExt, _T("jpeg") ) == 0 ||
		 _tcsicmp( szExt, _T("jpg") ) == 0 )
		return _T("image/jpeg");
	else if ( _tcsicmp( szExt, _T("bmp") ) == 0 ||
			  _tcsicmp( szExt, _T("dib") ) == 0 )
		return _T("image/bmp");
	else if ( _tcsicmp( szExt, _T("ico") ) == 0 )
		return _T("image/x-icon");
	else if ( _tcsicmp( szExt, _T("mdi") ) == 0 )
		return _T("image/vnd.ms-modi");
	else if ( _tcsicmp( szExt, _T("tif") ) == 0 ||
			  _tcsicmp( szExt, _T("tiff") ) == 0 )
		return _T("image/tiff");
	else if ( _tcsicmp( szExt, _T("wdp") ) == 0 )
		return _T("image/vnd.ms-photo");
	else
		return CString( _T("image/") ) + szExt;
}

BOOL IsKeyExists(HKEY hRoot, LPCTSTR szKey)
{
	HKEY hKey = NULL;
	LSTATUS res = RegOpenKeyEx( hRoot, szKey, 0, KEY_READ, &hKey );
	if ( res == ERROR_FILE_NOT_FOUND || res == ERROR_PATH_NOT_FOUND )
		return FALSE;
	else if ( res == ERROR_SUCCESS )
	{
		RegCloseKey( hKey );
	}
	return TRUE;
}

#ifdef ISTREAM_ENABLED

GFL_UINT32 GFLAPI IStreamRead(GFL_HANDLE handle, void* buffer, GFL_UINT32 size) throw()
{
	IStream* pStream = (IStream*)handle;

	ULONG read = 0;
	pStream->Read( buffer, size, &read );

	return read;
}

GFL_UINT32 GFLAPI IStreamTell(GFL_HANDLE handle) throw()
{
	IStream* pStream = (IStream*)handle;

	const LARGE_INTEGER zero = {};
	ULARGE_INTEGER pos = {};
	pStream->Seek( zero, STREAM_SEEK_CUR, &pos );

	return pos.LowPart;
}

GFL_UINT32 GFLAPI IStreamSeek(GFL_HANDLE handle, GFL_INT32 offset, GFL_INT32 origin) throw()
{
	IStream* pStream = (IStream*)handle;

	//STREAM_SEEK_SET  0
	//STREAM_SEEK_CUR  1
	//STREAM_SEEK_END  2

	//SEEK_SET    0
	//SEEK_CUR    1
	//SEEK_END    2

	LARGE_INTEGER to = { offset };
	ULARGE_INTEGER moved = {};
	pStream->Seek( to, origin, &moved );

	return moved.LowPart;
}

#endif // ISTREAM_ENABLED