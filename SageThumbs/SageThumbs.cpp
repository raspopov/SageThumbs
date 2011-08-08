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
#include "OptionsDlg.h"

//#define HTMLCLSID		_T("{25336920-03F9-11cf-8FD0-00AA00686F13}")
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

//BitsDescriptionMap	_BitsMap;
CSageThumbsModule		_Module;

CSageThumbsModule::CSageThumbsModule()
	: m_OSVersion	()
	, m_hGFL		( NULL )
	, m_hGFLe		( NULL )
	, m_hSQLite		( NULL )
	, m_hLangDLL	( NULL )
	, m_CurLangID	( STANDARD_LANGID )
//	, m_hWatchThread( NULL )
{
	m_OSVersion.dwOSVersionInfoSize = sizeof( m_OSVersion );
	GetVersionEx( &m_OSVersion );

	GetModuleFileName( _AtlBaseModule.GetModuleInstance(),
		m_sModuleFileName.GetBuffer( MAX_LONG_PATH ), MAX_LONG_PATH );
	m_sModuleFileName.ReleaseBuffer();
	ATLTRACE( "Module path: %s\n", (LPCSTR)CT2A( m_sModuleFileName ) );

	m_sModule = PathFindFileName( m_sModuleFileName );
	m_sHome = m_sModuleFileName.Left( m_sModuleFileName.GetLength() - m_sModule.GetLength() );
	m_sModule = m_sModule.Left( m_sModule.GetLength() - 4 ); // cut ".dll"

	// Get database filename
	m_sDatabase = GetRegValue( _T("Database") );
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

static const LPCTSTR szHandlers[] =
{
	_T("IconHandler"),								// IExtractIconA and IExtractIconW (Windows 2000)
	_T("{BB2E617C-0920-11D1-9A0B-00C04FC2D6C1}"),	// IExtractImage (Windows 2000)
	_T("{00021500-0000-0000-C000-000000000046}"),	// IQueryInfo (Windows 2000)
	_T("{E357FCCD-A995-4576-B01F-234630154E96}"),	// IThumbnailProvider (Windows Vista)
//	_T("{000214fa-0000-0000-c000-000000000046}"),	// IExtractIconW (Windows Vista)
//	_T("DataHandler"),								// IDataObject (Windows 2000)
//	_T("{8895B1C6-B41F-4C1C-A562-0D564250836F}"),	// IPreviewHandler (Windows Vista)
	NULL
};

HRESULT CSageThumbsModule::DllRegisterServer()
{
	CHECKPOINT_BEGIN(DllRegisterServer)

	HRESULT hr = CAtlDllModuleT< CSageThumbsModule >::DllRegisterServer( FALSE );

	ATLVERIFY ( RegisterExtensions() );

	ATLTRACE ("DllRegisterServer : ");
	CHECKPOINT(DllRegisterServer)
	ATLTRACE ("\n");

	return hr;
}

HRESULT CSageThumbsModule::DllUnregisterServer()
{
	CHECKPOINT_BEGIN(DllUnregisterServer)

	HRESULT hr = CAtlDllModuleT< CSageThumbsModule >::DllUnregisterServer( FALSE );

	ATLVERIFY ( UnregisterExtensions() );

	ATLTRACE ("DllUnregisterServer : ");
	CHECKPOINT(DllUnregisterServer)
	ATLTRACE ("\n");

	return hr;
}

BOOL CSageThumbsModule::RegisterExtensions()
{
	BOOL bOK = TRUE;

	const bool bEnableThumbs = GetRegValue( _T("EnableThumbs"), 1ul ) != 0;
	const bool bEnableIcons = GetRegValue( _T("EnableIcons"), 1ul ) != 0;

	for ( POSITION pos = m_oExtMap.GetHeadPosition(); pos; )
	{
		if ( const CExtMap::CPair* p = m_oExtMap.GetNext( pos ) )
		{
			if ( p->m_value.enabled )
				bOK = RegisterExt( p->m_key, bEnableThumbs, bEnableIcons ) && bOK;
			else
				bOK = UnregisterExt( p->m_key ) && bOK;
		}
	}

	//const bool bEnableThumbs = GetRegValue( _T("EnableThumbs"), 1 ) != 0;
	//const bool bEnableIcons = GetRegValue( _T("EnableIcons"), 1 ) != 0;
	//CString sRoot( _T("SystemFileAssociations\\image\\ShellEx\\") );
	//for ( int i = 0; szHandlers[ i ]; ++i )
	//{
	//	if ( ( i == 0 && ! bEnableIcons ) ||	// IExtractIcon
	//		 ( i == 1 && ! bEnableThumbs ) ||	// IExtractImage
	//		 ( i == 3 && ! bEnableThumbs ) )	// IThumbnailProvider
	//	{
	//		UnregisterValue (HKEY_CLASSES_ROOT, sRoot + szHandlers[ i ],
	//			_T(""), MyCLSID, REG_SAGETHUMBS_BAK);
	//	}
	//	else
	//	{
	//		RegisterValue( HKEY_CLASSES_ROOT, sRoot + szHandlers[ i ],
	//			_T(""), MyCLSID, REG_SAGETHUMBS_BAK );
	//	}
	//}

	CleanWindowsCache();

	UpdateShell();

	return bOK;
}

BOOL CSageThumbsModule::UnregisterExtensions()
{
	BOOL bOK = TRUE;

	for ( POSITION pos = m_oExtMap.GetHeadPosition(); pos; )
	{
		if ( const CExtMap::CPair* p = m_oExtMap.GetNext( pos ) )
		{
			bOK = UnregisterExt( p->m_key ) && bOK;
		}
	}

	// IImageDecodeFilter
	//UnregisterValue (HKEY_CLASSES_ROOT, _T("MIME\\Database\\Content Type\\image/xnview"),
	//	_T("Image Filter CLSID"), MyCLSID, _T("Image Filter CLSID SageThumbs.bak"));
	//UnregisterValue (HKEY_CLASSES_ROOT, _T("MIME\\Database\\Content Type\\image/xnview\\Bits"),
	//	_T("0"), (const BYTE*) XNVIEW_MIME, 36, _T("0 SageThumbs.bak"));
	//SHDeleteEmptyKey (HKEY_CLASSES_ROOT, _T("MIME\\Database\\Content Type\\image/xnview\\Bits"));
	//SHDeleteEmptyKey (HKEY_CLASSES_ROOT, _T("MIME\\Database\\Content Type\\image/xnview\\Bits"));
	//SHDeleteEmptyKey (HKEY_CLASSES_ROOT, _T("MIME\\Database\\Content Type\\image/xnview"));
	//SHDeleteEmptyKey (HKEY_CLASSES_ROOT, _T("MIME\\Database\\Content Type\\image/xnview"));

	CString sRoot( _T("SystemFileAssociations\\image\\ShellEx\\") );
	for ( int i = 0; szHandlers[ i ]; ++i )
	{
		bOK = UnregisterValue( HKEY_CLASSES_ROOT, sRoot + szHandlers[ i ], _T(""), CLSID_THUMB ) && bOK;
	}

	bOK = DeleteRegKey( HKEY_CLASSES_ROOT, _T("SystemFileAssociations\\image\\ShellEx") ) && bOK;
	bOK = DeleteRegKey( HKEY_CLASSES_ROOT, _T("SystemFileAssociations\\image") ) && bOK;

	CleanWindowsCache();

	UpdateShell();

	return bOK;
}

void CSageThumbsModule::UpdateShell()
{
	SHChangeNotify( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL );
	SystemParametersInfo( SPI_SETICONS, 0, NULL, SPIF_SENDCHANGE );
	CoFreeUnusedLibraries ();
}

BOOL CSageThumbsModule::RegisterExt(LPCTSTR szExt, bool bEnableThumbs, bool bEnableIcons)
{
	BOOL bOK = TRUE;

	CString key ( _T(".") );
	CString ext ( szExt );
	key += ext;

	CString DefaultKey = GetRegValue( _T(""), _T(""), key, HKEY_CLASSES_ROOT );
	if ( DefaultKey.IsEmpty() )
	{
		DefaultKey = _T("XnView.Image");
		RegisterValue( HKEY_CLASSES_ROOT, key, _T(""), DefaultKey );
	}

	// Perceived Type Fix
	SetRegValue( _T("PerceivedType"), _T("image"), key, HKEY_CLASSES_ROOT );

	// Content Type Fix
	CString sExtension;
	if ( _tcsicmp( szExt, _T("jpg")  ) == 0 ||
		 _tcsicmp( szExt, _T("jpe")  ) == 0 ||
		 _tcsicmp( szExt, _T("jfif") ) == 0 )
		sExtension = _T("jpeg");
	else
		sExtension = szExt;
	SetRegValue( _T("Content Type"), _T("image/") + sExtension, key, HKEY_CLASSES_ROOT );
	SetRegValue( _T("Extension"), sExtension, _T("MIME\\DataBase\\Content Type\\image/") + sExtension, HKEY_CLASSES_ROOT );

	for ( int i = 0; szHandlers[ i ]; ++i )
	{
		switch ( i )
		{
		case 0:	// IExtractIcon
		case 2:	// IQueryInfo
			bOK = UnregisterValue( HKEY_CLASSES_ROOT, key + _T("\\ShellEx\\") + szHandlers[ i ], _T(""), CLSID_THUMB ) && bOK;
			if ( ( i == 0 && bEnableIcons ) || ( i == 2 ) )
			{
				bOK = RegisterValue( HKEY_CLASSES_ROOT, DefaultKey + _T("\\ShellEx\\") + szHandlers[ i ], _T(""), CLSID_THUMB ) && bOK;
			}
			else
			{
				bOK = UnregisterValue( HKEY_CLASSES_ROOT, DefaultKey + _T("\\ShellEx\\") + szHandlers[ i ], _T(""), CLSID_THUMB ) && bOK;
			}
			break;
		
		default:
			bOK = UnregisterValue( HKEY_CLASSES_ROOT, DefaultKey + _T("\\ShellEx\\") + szHandlers[ i ], _T(""), CLSID_THUMB ) && bOK;
			if ( ( i == 1 && bEnableThumbs ) ||	// IExtractImage
				 ( i == 3 && bEnableThumbs ) )	// IThumbnailProvider
			{
				bOK = RegisterValue( HKEY_CLASSES_ROOT, key + _T("\\ShellEx\\") + szHandlers[ i ], _T(""), CLSID_THUMB ) && bOK;
			}
			else
			{
				bOK = UnregisterValue( HKEY_CLASSES_ROOT, key + _T("\\ShellEx\\") + szHandlers[ i ], _T(""), CLSID_THUMB ) && bOK;
			}
		}
	}

	// IImageDecodeFilter
	//const BitsDescription* bits = NULL;
	//if (_BitsMap.Lookup (ext, bits)) {
	//	CString mime (_T("image/"));
	//	mime += ext;
	//	CString mimekey (_T("MIME\\Database\\Content Type\\"));
	//	mimekey += mime;
	//	BYTE* data = new BYTE [bits->size * 2 + 4];
	//	*((DWORD*) data) = bits->size;
	//	CopyMemory (data + 4, bits->mask, bits->size);
	//	CopyMemory (data + 4 + bits->size, bits->data, bits->size);
	//	RegisterValue (HKEY_CLASSES_ROOT, DefaultKey + _T("\\CLSID"),
	//		_T(""), HTMLCLSID, REG_SAGETHUMBS_BAK);
	//	RegisterValue (HKEY_CLASSES_ROOT, key, _T("Content Type"),
	//		mime, _T("Content Type SageThumbs.bak"));
	//	RegisterValue (HKEY_CLASSES_ROOT, mimekey, _T("Image Filter CLSID"),
	//		MyCLSID, _T("Image Filter CLSID SageThumbs.bak"));
	//	RegisterValue (HKEY_CLASSES_ROOT, mimekey, _T("Extension"),
	//		key, _T("Extension SageThumbs.bak"));
	//	RegisterValue (HKEY_CLASSES_ROOT, mimekey + _T("\\Bits"), _T("0"),
	//		data, bits->size * 2 + 4, _T("0 SageThumbs.bak"));
	//	delete [] data;
	//} else {
	//	// Если нет значения, то установка универсального
	//	BYTE foo [256];
	//	DWORD size = sizeof (foo);
	//	LONG err = SHGetValue (HKEY_CLASSES_ROOT, key, _T("Content Type"), &type, foo, &size);
	//	if (err != ERROR_SUCCESS)
	//		RegisterValue (HKEY_CLASSES_ROOT, key, _T("Content Type"),
	//			_T("image/xnview"), _T("Content Type SageThumbs.bak"));
	//}

	bOK = DeleteRegKey( HKEY_CLASSES_ROOT, key + _T("\\ShellEx") ) && bOK;
	bOK = DeleteRegKey( HKEY_CLASSES_ROOT, key ) && bOK;

	if ( ! DefaultKey.IsEmpty() )
	{
		bOK = DeleteRegKey( HKEY_CLASSES_ROOT, DefaultKey + _T("\\ShellEx") ) && bOK;
		bOK = DeleteRegKey( HKEY_CLASSES_ROOT, DefaultKey ) && bOK;
	}

	return bOK;
}

BOOL CSageThumbsModule::UnregisterExt(LPCTSTR szExt)
{
	BOOL bOK = TRUE;

	CString key ( _T(".") );
	CString ext ( szExt );
	key += ext;

	CString DefaultKey = GetRegValue( _T(""), _T(""), key, HKEY_CLASSES_ROOT );

	for ( int i = 0; szHandlers[ i ]; ++i )
	{
		bOK = UnregisterValue( HKEY_CLASSES_ROOT, key + _T("\\ShellEx\\") + szHandlers[ i ], _T(""), CLSID_THUMB ) && bOK;

		if ( ! DefaultKey.IsEmpty() )
		{
			bOK = UnregisterValue( HKEY_CLASSES_ROOT, DefaultKey + _T("\\ShellEx\\") + szHandlers[ i ], _T(""), CLSID_THUMB ) && bOK;
		}
	}
	
	// IImageDecodeFilter
	//const BitsDescription* bits = NULL;
	//if (_BitsMap.Lookup (ext, bits)) {
	//	CString mime (_T("image/"));
	//	mime += ext;
	//	CString mimekey (_T("MIME\\Database\\Content Type\\"));
	//	mimekey += mime;
	//	BYTE* data = new BYTE [bits->size * 2 + 4];
	//	*((DWORD*) data) = bits->size;
	//	CopyMemory (data + 4, bits->mask, bits->size);
	//	CopyMemory (data + 4 + bits->size, bits->data, bits->size);
	//	if (!DefaultKey.IsEmpty ())
	//		UnregisterValue (HKEY_CLASSES_ROOT, DefaultKey + _T("\\CLSID"),
	//			_T(""), HTMLCLSID, REG_SAGETHUMBS_BAK);
	//	UnregisterValue (HKEY_CLASSES_ROOT, key, _T("Content Type"),
	//		mime, _T("Content Type SageThumbs.bak"));
	//	UnregisterValue (HKEY_CLASSES_ROOT, mimekey, _T("Image Filter CLSID"),
	//		MyCLSID, _T("Image Filter CLSID SageThumbs.bak"));
	//	UnregisterValue (HKEY_CLASSES_ROOT, mimekey, _T("Extension"),
	//		key, _T("Extension SageThumbs.bak"));
	//	UnregisterValue (HKEY_CLASSES_ROOT, mimekey + _T("\\Bits"), _T("0"),
	//		data, bits->size * 2 + 4, _T("0 SageThumbs.bak"));
	//	delete [] data;

	//	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, mimekey + _T("\\Bits"));
	//	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, mimekey + _T("\\Bits"));
	//	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, mimekey);
	//	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, mimekey);
	//} else {
	//	UnregisterValue (HKEY_CLASSES_ROOT, key, _T("Content Type"),
	//		_T("image/xnview"), _T("Content Type SageThumbs.bak"));
	//}

	bOK = DeleteRegKey( HKEY_CLASSES_ROOT, key + _T("\\ShellEx") ) && bOK;
	bOK = DeleteRegKey( HKEY_CLASSES_ROOT, key ) && bOK;

	if ( ! DefaultKey.IsEmpty() )
	{
		bOK = DeleteRegKey( HKEY_CLASSES_ROOT, DefaultKey + _T("\\ShellEx") ) && bOK;
		bOK = DeleteRegKey( HKEY_CLASSES_ROOT, DefaultKey ) && bOK;
	}

	return bOK;
}

LANGID CSageThumbsModule::GetLang()
{
	return m_CurLangID;
}

BOOL CSageThumbsModule::LoadLang(LANGID LangID)
{
	if ( LangID == 0 )
		// Использование предыдущей настройки языка
		LangID = (LANGID)(DWORD)GetRegValue( _T("Lang"), 0ul );

	if ( LangID == m_CurLangID )
		// Загрузка того же самого языка
		return TRUE;

	// Выгрузка старого языка
	UnLoadLang();

	// Попытка загрузки локализаций, вначале указанной затем системные
	if ( ( LangID ? LoadLangIDDLL( LangID ) : false ) ||
		LoadLangIDDLL( GetUserDefaultLangID() & 0xff ) ||
		LoadLangIDDLL( GetSystemDefaultLangID() & 0xff ) )
	{
		// Сохранение загруженного языка в реестре
		SetRegValue( _T("Lang"), m_CurLangID );
		return TRUE;
	}
	else
		return FALSE;
}

void CSageThumbsModule::UnLoadLang ()
{
	// Выгрузка языка
	if ( m_hLangDLL )
	{
		FreeLibrary( m_hLangDLL );
		m_hLangDLL = NULL;
	}

	// Установка стандартного языка
	m_CurLangID = STANDARD_LANGID;
	_AtlBaseModule.SetResourceInstance( _AtlBaseModule.GetModuleInstance() );
}

BOOL CSageThumbsModule::LoadLangIDDLL (LANGID LangID)
{
	ATLASSERT( LangID != 0 );
	ATLASSERT( m_hLangDLL == NULL );

	if ( LangID == STANDARD_LANGID )
	{
		// Загрузка встроенного языка
		m_CurLangID = STANDARD_LANGID;
		return TRUE;
	}

	CString strLangIDDLL;
	strLangIDDLL.Format( _T("%s%s%.2x.dll"), (LPCTSTR)m_sHome, (LPCTSTR)m_sModule, LangID );
	HMODULE hInstance = LoadLibrary( strLangIDDLL );
	if ( hInstance )
	{
		_AtlBaseModule.SetResourceInstance (hInstance);
		m_CurLangID = LangID;
		m_hLangDLL = hInstance;
		return TRUE;
	}
	ATLTRACE ( "LoadLangIDDLL(%.2x) failed: %d\n", LangID, GetLastError ());
	return FALSE;
}

void CSageThumbsModule::FillExtMap()
{
#ifdef GFL_THREAD_SAFE
	CLock oLock( m_pSection );
#endif // GFL_THREAD_SAFE

	CHECKPOINT_BEGIN( FillExtMap )

	m_oExtMap.RemoveAll();

	// Загрузка расширений через GFL
	GFL_INT32 count = gflGetNumberOfFormat();
	GFL_INT32 i = 0;
	for ( ; i < count; ++i )
	{
		GFL_FORMAT_INFORMATION info = {};
		GFL_ERROR err = gflGetFormatInformationByIndex (i, &info);
		if ( err == GFL_NO_ERROR && ( info.Status & GFL_READ ) )
		{
			Ext data = { true, (LPCTSTR)CA2T( info.Description ) };
			for ( GFL_UINT32 j = 0; j < info.NumberOfExtension; ++j )
			{
				CString sExt = (LPCTSTR)CA2T( info.Extension [ j ] );
				sExt.MakeLower();
				m_oExtMap.SetAt( sExt, data );
			}
		}
	}
    							
	// Загрузка данных о расширении
	i = 1;
	const CString key = CString( REG_SAGETHUMBS ) + _T("\\");
	for ( POSITION pos = m_oExtMap.GetHeadPosition(); pos; ++i )
	{
		CExtMap::CPair* p = m_oExtMap.GetNext (pos);

		// Exclude bad extensions
		DWORD dwEnabled = GetRegValue( _T("Enabled"), EXT_DEFAULT( p->m_key ) ? 0ul : 1ul, key + p->m_key );
		SetRegValue( _T(""), p->m_value.info, key + p->m_key );
		SetRegValue( _T("Enabled"), dwEnabled, key + p->m_key );
		p->m_value.enabled = ( dwEnabled != 0 );

		//ATLTRACE( "%4d. %c %8s \"%s\"\n", i, ( p->m_value.enabled ? '+' : '-' ), (LPCSTR)CT2A( p->m_key ), (LPCSTR)CT2A( p->m_value.info ) );	
	}

	ATLTRACE( "Loaded %d formats, %d extensions. ", count, m_oExtMap.GetCount() );
	CHECKPOINT( FillExtMap )
}

BOOL CSageThumbsModule::Initialize()
{
	ATLTRACE ( "CSageThumbsModule::Initialize ()\n" );

	//DWORD id;
	//m_hWatchThread = CreateThread( NULL, 0, WatchThread, (LPVOID) this, 0, &id );
	//Sleep( 0 );

	CHECKPOINT_BEGIN(LoadLibrarySQLite)
	m_hSQLite = ::LoadLibrary( m_sHome + LIB_SQLITE );
	if ( ! m_hSQLite )
		// Ошибка загрузки
		return FALSE;
	CHECKPOINT(LoadLibrarySQLite)

	// Загрузка библиотек
	CHECKPOINT_BEGIN(LoadLibraryGFL)
	m_hGFL = ::LoadLibrary( m_sHome + LIB_GFL );
	if ( ! m_hGFL )
		// Ошибка загрузки
		return FALSE;
	CHECKPOINT(LoadLibraryGFL)

	CHECKPOINT_BEGIN(LoadLibraryGFLE)
	m_hGFLe = ::LoadLibrary( m_sHome + LIB_GFLE );
	if ( ! m_hGFLe )
		// Ошибка загрузки
		return FALSE;
	CHECKPOINT(LoadLibraryGFLE)

	// Загрузка локализации
	CHECKPOINT_BEGIN(LoadLangs)
	LoadLang ();
	CHECKPOINT(LoadLangs)

	CHECKPOINT_BEGIN(GFLInit)

	// Получение папки с плагинами
	CString sPlugins;
	CString buf = GetRegValue( REG_XNVIEW_PATH1, _T(""),
		REG_XNVIEW_KEY, HKEY_LOCAL_MACHINE );
	if ( ! buf.IsEmpty() )
	{
		// 	UninstallString = "C:\Program Files\XnView\unins000.exe"
		buf.Trim (_T("\""));
		int n = buf.ReverseFind (_T('\\'));
		if (n > 0)
		{
			buf = buf.Left (n) + _T("\\PlugIns");
			sPlugins = buf;
		}
	}
	else
	{
		buf = GetRegValue( REG_XNVIEW_PATH2, _T(""),
			REG_XNVIEW_KEY, HKEY_LOCAL_MACHINE );
		if ( ! buf.IsEmpty() )
		{
			// Inno Setup: App Path = C:\Program Files\XnView
			buf.Trim (_T("\""));
			buf.TrimRight (_T("\\"));
			buf += _T("\\PlugIns");
			sPlugins = buf;
		}
		else
		{
			// %Program Files%\XnView\PlugIns
			buf = GetSpecialFolderPath( CSIDL_PROGRAM_FILES );
			if ( ! buf.IsEmpty() )
			{
				buf.TrimRight (_T("\\"));
				buf += _T("\\XnView\\PlugIns");
				sPlugins = buf;
			}
		}
	}
	if ( ! sPlugins.IsEmpty ())
		MakeDirectory( sPlugins );
	if ( !sPlugins.IsEmpty() )
		gflSetPluginsPathnameT( sPlugins );
	ATLTRACE( "gflSetPluginsPathnameW : \"%s\"\n", (LPCSTR)CT2A( sPlugins ) );

	// Инициализация GFL
	GFL_ERROR err = gflLibraryInit();
	if ( err != GFL_NO_ERROR )
		// Ошибка инициализациия GFL
		return FALSE;

	gflEnableLZW (GFL_TRUE);

	ATLTRACE( "gflLibraryInit : GFL Version %s\n", gflGetVersion() );

	FillExtMap ();

	CHECKPOINT(GFLInit)

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

	UnLoadLang ();

	//ATLTRACE (_T("CSageThumbsModule::UnInitialize () -> begin\n"));
	//if (m_hWatchThread)
	//{
	//	HANDLE hWatchEvent = CreateEvent (NULL, TRUE, TRUE, _T("SageThumbsWatch"));
	//	if (hWatchEvent)
	//	{
	//		for (int count = 0;
	//			(WaitForSingleObject (m_hWatchThread, 100) == WAIT_TIMEOUT) &&
	//			count < 20; count++)
	//			SetEvent (hWatchEvent);
	//		if (WaitForSingleObject (m_hWatchThread, 0) == WAIT_TIMEOUT)
	//		{
	//			ATLTRACE (_T("CSageThumbsModule::UnInitialize () -> TerminateThread\n"));
	//			TerminateThread (m_hWatchThread, 0);
	//		}
	//		CloseHandle (m_hWatchThread);
	//		m_hWatchThread = NULL;
	//		CloseHandle (hWatchEvent);
	//	}
	//}
	//ATLTRACE (_T("CSageThumbsModule::UnInitialize () -> end\n"));
}

/*DWORD WINAPI CSageThumbsModule::WatchThread(LPVOID)
{
	ATLTRACE( "WatchThread -> begin\n" );
	HANDLE hWatchEvent = CreateEvent( NULL, TRUE, FALSE, _T("SageThumbsWatch") );
	if ( hWatchEvent )
	{
		WaitForSingleObject( hWatchEvent, INFINITE );
		CloseHandle( hWatchEvent );
	}
	//CoFreeUnusedLibraries();
	ATLTRACE( "WatchThread -> end\n" );
	return 0;
}*/

extern "C" BOOL WINAPI DllMain(HINSTANCE /* hInstance */, DWORD dwReason, LPVOID lpReserved)
{
	if ( _Module.DllMain( dwReason, lpReserved ) )
	{
		switch ( dwReason )
		{
		case DLL_PROCESS_ATTACH:
			ATLTRACE( "DllMain::DLL_PROCESS_ATTACH\n" );
			__try
			{
				if ( ! _Module.Initialize() )
					return FALSE;
			}
			__except ( EXCEPTION_EXECUTE_HANDLER )
			{
				ATLTRACE( "Exception in CSageThumbsModule::Initialize()\n" );
				return FALSE;
			}
			break;

		case DLL_PROCESS_DETACH:
			ATLTRACE( "DllMain::DLL_PROCESS_DETACH\n" );
			__try
			{
				_Module.UnInitialize();
			}
			__except ( EXCEPTION_EXECUTE_HANDLER )
			{
				ATLTRACE( "Exception in CSageThumbsModule::UnInitialize()\n" );
			}
			break;
		}
		return TRUE;
	}
	else
	{
		ATLTRACE( "Failed CSageThumbsModule::DllMain() call\n" );
		return FALSE;
	}
}

STDAPI DllCanUnloadNow(void)
{
	ATLTRACE( "Calling ::DllCanUnloadNow()...\n" );

	return _Module.DllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	ATLTRACE( "Calling ::DllGetClassObject()...\n" );

	return _Module.DllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer(void)
{
	ATLTRACE( "Calling ::DllRegisterServer()...\n" );

	return _Module.DllRegisterServer();
}

STDAPI DllUnregisterServer(void)
{
	ATLTRACE( "Calling ::DllUnregisterServer()...\n" );

	return _Module.DllUnregisterServer();
}

void CALLBACK Options (HWND hwnd, HINSTANCE /* hinst */, LPSTR /* lpszCmdLine */, int /* nCmdShow */)
{
	OleInitialize( NULL );

	INITCOMMONCONTROLSEX init =
	{
		sizeof( INITCOMMONCONTROLSEX ),
		ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES
	};
	InitCommonControlsEx( &init );

	COptionsDialog dlg;
	dlg.DoModal( hwnd );

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

DWORD GetRegValue(LPCTSTR szName, DWORD dwDefault, LPCTSTR szKey, HKEY hRoot)
{
	DWORD dwValue, dwType = REG_DWORD, dwSize = sizeof( DWORD );
	LSTATUS res = SHGetValue( hRoot, szKey, szName, &dwType, &dwValue, &dwSize );
	return ( res == ERROR_SUCCESS &&
		dwType == REG_DWORD &&
		dwSize == sizeof( DWORD ) ) ? dwValue : dwDefault;
}

CString GetRegValue(LPCTSTR szName, LPCTSTR szDefault, LPCTSTR szKey, HKEY hRoot)
{
	CString sValue;
	DWORD dwType = REG_SZ;
	DWORD dwSize = ( MAX_LONG_PATH - 1 )* sizeof( TCHAR );
	LPTSTR buf = sValue.GetBuffer( MAX_LONG_PATH );
	bool ret = SHGetValue( hRoot, szKey, szName, &dwType, buf, &dwSize ) == ERROR_SUCCESS &&
		( dwType == REG_SZ || dwType == REG_EXPAND_SZ );
	buf [ dwSize / sizeof( TCHAR ) ] = _T('\0');
	if ( dwType == REG_EXPAND_SZ )
	{
		DoEnvironmentSubst( buf, MAX_LONG_PATH );
	}
	sValue.ReleaseBuffer();
	return ret ? sValue : szDefault;
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

	// Remove wrong type value if any
	SHDeleteValue( hRoot, szKey, szName );

	res = SHSetValue( hRoot, szKey, szName, REG_DWORD, &dwValue, sizeof( DWORD ) );
	if ( res != ERROR_SUCCESS )
	{
		ATLTRACE( "Got %s during value setting: %s : %s = %d\n", ( ( res == ERROR_ACCESS_DENIED ) ? "\"Access Denied\"" : "error" ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ), dwValue );
		return FALSE;
	}

	return TRUE;
}

BOOL SetRegValue(LPCTSTR szName, LPCTSTR szValue, LPCTSTR szKey, HKEY hRoot)
{
	LSTATUS res;

	CString sCurrent = GetRegValue( szName, _T(""), szKey, hRoot );
	if ( sCurrent.CompareNoCase( szValue ) == 0 )
	{
		// Already set
		return TRUE;
	}

	// Remove wrong type value if any
	SHDeleteValue( hRoot, szKey, szName );

	if ( _istalpha( szValue[ 0 ] ) && szValue[ 1 ] == _T(':') && szValue[ 2 ] == _T('\\') )
	{
		CString sCompact;
		BOOL bCompact = PathUnExpandEnvStrings( szValue, sCompact.GetBuffer( MAX_LONG_PATH ), MAX_LONG_PATH );
		sCompact.ReleaseBuffer();
		if ( bCompact )
		{
			res = SHSetValue( hRoot, szKey, szName, REG_EXPAND_SZ, sCompact, (DWORD)lengthof( sCompact ) );
			if ( res != ERROR_SUCCESS )
			{
				ATLTRACE( "Got %s during value setting: %s : %s = %d\n", ( ( res == ERROR_ACCESS_DENIED ) ? "\"Access Denied\"" : "error" ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ), (LPCSTR)CT2A( sCompact ) );
				return FALSE;
			}

			return TRUE;
		}
	}

	res = SHSetValue( hRoot, szKey, szName, REG_SZ, szValue, (DWORD)lengthof( szValue ) );
	if ( res != ERROR_SUCCESS )
	{
		ATLTRACE( "Got %s during value setting: %s : %s = %s\n", ( ( res == ERROR_ACCESS_DENIED ) ? "\"Access Denied\"" : "error" ), (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ), (LPCSTR)CT2A( szValue ) );
		return FALSE;
	}

	return TRUE;
}

BOOL RegisterValue(HKEY hRoot, LPCTSTR szKey, LPCTSTR szName, LPCTSTR szValue)
{
	// Check for new value
	CString buf = GetRegValue( szName, _T(""), szKey, hRoot );
	if ( buf.CompareNoCase( szValue ) != 0 )
	{
		// Set new value
		if ( ! SetRegValue( szName, szValue, szKey, hRoot ) )
			return FALSE;

		// Backup old one
		if ( ! buf.IsEmpty() )
		{
			if ( ! SetRegValue( REG_SAGETHUMBS_BAK, buf, szKey, hRoot ) )
				return FALSE;
		}
	}
	return TRUE;
}

/*void RegisterValue(HKEY hRoot, LPCTSTR key, LPCTSTR name, const BYTE* value, DWORD value_size, LPCTSTR backup)
{
	// Check for new value
	BYTE buf [256];
	DWORD type = REG_BINARY;
	DWORD size = sizeof (buf) - 1;
	DWORD res = SHGetValue( root, key, name, &type, buf, &size );
	if ( ( ERROR_SUCCESS != res ) ||
		 ( type != REG_BINARY ) ||
		 ( size != value_size ) ||
		 ( memcmp( buf, value, size ) != 0 ) )
	{
		// Set new value
		SHSetValue (root, key, name, REG_BINARY, value, value_size);				

		// Backup old one
		if ( res == ERROR_SUCCESS && size > 0 && type == REG_BINARY )
			SHSetValue( root, key, backup, REG_BINARY, buf, size );
	}
}*/

BOOL UnregisterValue(HKEY hRoot, LPCTSTR szKey, LPCTSTR szName, LPCTSTR szValue)
{
	BOOL bOK = TRUE;

	// Check backup
	CString buf = GetRegValue( szName, _T(""), szKey, hRoot );
	CString backup_buf = GetRegValue( REG_SAGETHUMBS_BAK, _T(""), szKey, hRoot );
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
			LSTATUS res = SHDeleteValue( hRoot, szKey, szName );
			if ( res == ERROR_SUCCESS )
				res = SHDeleteValue( hRoot, szKey, szName );
			if ( res == ERROR_ACCESS_DENIED )
			{
				bOK = FALSE;
				ATLTRACE( "Got \"Access Denied\" during value deletion: %s : %s\n", (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ) );
			}
		}
	}

	if ( ! backup_buf.IsEmpty() )
	{
		// Delete backup value
		LSTATUS res = SHDeleteValue( hRoot, szKey, REG_SAGETHUMBS_BAK );
		if ( res == ERROR_SUCCESS )
			res = SHDeleteValue( hRoot, szKey, REG_SAGETHUMBS_BAK );
		if ( res == ERROR_ACCESS_DENIED )
		{
			ATLTRACE( "Got \"Access Denied\" during value deletion: %s : %s\n", (LPCSTR)CT2A( szKey ), (LPCSTR)CT2A( szName ) );
			bOK = FALSE;
		}
	}

	// Clean-up empty key
	bOK = DeleteRegKey( hRoot, szKey ) && bOK;

	return bOK;
}

/*void UnregisterValue(HKEY hRoot, LPCTSTR key, LPCTSTR name, const BYTE* value, DWORD value_size, LPCTSTR backup)
{
	// Check backup
	BYTE buf [256];
	DWORD type = REG_BINARY;
	DWORD size = sizeof (buf) - 1;
	DWORD res = SHGetValue (root, key, name, &type, buf, &size );
	if ( ( ERROR_SUCCESS == res ) &&
		 ( type == REG_BINARY ) &&
		 ( size == value_size ) &&
		 ( memcmp( buf, value, size ) == 0 ) )
	{
		// Check backup value
		type = REG_BINARY;
		size = sizeof (buf) - 1;
		res = SHGetValue (root, key, backup, &type, buf, &size );
		if ( ( ERROR_SUCCESS == res ) &&
			 ( type == REG_BINARY ) &&
			 ( size > 0 ) )
		{
			// Restore value from backup
			SHSetValue (root, key, name, REG_BINARY, buf, size);

			// Delete backup value
			SHDeleteValue (root, key, backup);
			SHDeleteValue (root, key, backup);
		}
		else
		{
			// Delete original value
			SHDeleteValue (root, key, name);
			SHDeleteValue (root, key, name);
		}
	}

	// Clean-up empty key
	SHDeleteEmptyKey (root, key);
	SHDeleteEmptyKey (root, key);
}*/

BOOL DeleteRegKey(HKEY hRoot, LPCTSTR szSubKey)
{
	SHDeleteEmptyKey( hRoot, szSubKey );	// HKCU
	SHDeleteEmptyKey( hRoot, szSubKey );	// HKLM
	return TRUE;
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
		GFL_INT32 index = -1;
		err = gflGetFileInformationT( filename, index, info );
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

HRESULT CSageThumbsModule::LoadBitmap(LPCTSTR filename, GFL_BITMAP **bitmap)
{
#ifdef GFL_THREAD_SAFE
	CLock oLock( m_pSection );
	return LoadBitmapE( filename, bitmap );
}
HRESULT CSageThumbsModule::LoadBitmapE(LPCTSTR filename, GFL_BITMAP **bitmap)
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
				gflChangeColorDepth( *bitmap, NULL, GFL_MODE_TO_RGBA, GFL_MODE_ADAPTIVE );
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

HRESULT CSageThumbsModule::LoadThumbnail(LPCTSTR filename, GFL_INT32 width, GFL_INT32 height, GFL_BITMAP **bitmap)
{
#ifdef GFL_THREAD_SAFE
	CLock oLock( m_pSection );
	return LoadThumbnailE( filename, width, height, bitmap );
}
HRESULT CSageThumbsModule::LoadThumbnailE(LPCTSTR filename, GFL_INT32 width, GFL_INT32 height, GFL_BITMAP **bitmap)
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
			GFL_LOAD_ONLY_FIRST_FRAME |
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
				gflChangeColorDepth( *bitmap, NULL, GFL_MODE_TO_RGBA, GFL_MODE_ADAPTIVE );
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

HRESULT CSageThumbsModule::LoadBitmapFromMemory(LPCVOID data, GFL_UINT32 data_length, GFL_BITMAP **bitmap)
{
#ifdef GFL_THREAD_SAFE
	CLock oLock( m_pSection );
	return LoadBitmapFromMemoryE( data, data_length, bitmap );
}
HRESULT CSageThumbsModule::LoadBitmapFromMemoryE(LPCVOID data, GFL_UINT32 data_length, GFL_BITMAP **bitmap)
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
		err = gflLoadBitmapFromMemory( (const GFL_UINT8*)data, data_length, bitmap, &params, NULL );
		if ( err == GFL_ERROR_FILE_READ )
		{
			params.Flags |= GFL_LOAD_IGNORE_READ_ERROR;

			err = gflLoadBitmapFromMemory( (const GFL_UINT8*)data, data_length, bitmap, &params, NULL );
		}
		if ( err == GFL_NO_ERROR )
		{
			if ( (*bitmap)->Type != GFL_RGBA )
			{
				gflChangeColorDepth( *bitmap, NULL, GFL_MODE_TO_RGBA, GFL_MODE_ADAPTIVE );
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

		err = gflConvertBitmapIntoDIBSection( bitmap, phBitmap );
		if ( err == GFL_NO_ERROR )
		{
			hr = S_OK;
		}
		else
		{
			ATLTRACE ("E_FAIL (gflConvertBitmapIntoDDB) : %s\n", gflGetErrorString (err));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ATLTRACE ("E_FAIL (gflConvertBitmapIntoDDB exception)\n");
	}

	return hr;
}

HRESULT CSageThumbsModule::Resize(GFL_BITMAP* src, GFL_BITMAP** dst, GFL_INT32 width, GFL_INT32 height)
{
#ifdef GFL_THREAD_SAFE
	CLock oLock( m_pSection );
	return ResizeE( src, dst, width, height );
}
HRESULT CSageThumbsModule::ResizeE(GFL_BITMAP* src, GFL_BITMAP** dst, GFL_INT32 width, GFL_INT32 height)
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

	DWORD max_size = GetRegValue( _T("MaxSize"), FILE_MAX_SIZE );
	if ( max_size && ( pfd->nFileSizeHigh || pfd->nFileSizeLow / ( 1024 * 1024 ) > max_size ) )
		// Too big file
		return false;

	return true;
}
