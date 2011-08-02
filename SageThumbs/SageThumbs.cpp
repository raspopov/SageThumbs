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

CString					_ModuleFileName;
CString					_Database;
CString					_PlugInsPathname;
CExtMap					_ExtMap;
CRITICAL_SECTION		_GflGuard;
//BitsDescriptionMap	_BitsMap;
CSageThumbsModule		_AtlModule;
DllLoader				_libgfl  (_T(LIB_GFL), false);
DllLoader				_libgfle (_T(LIB_GFLE), false);
DllLoader				_libsqlite (_T(LIB_SQLITE), false);

CSageThumbsModule::CSageThumbsModule()
	: m_CurLangID( STANDARD_LANGID )
	, m_LangDLL( NULL )
//	, m_hWatchThread (NULL)
{
}

static const LPCTSTR szHandlers[] =
{
	_T("IconHandler"),								// IExtractIcon
	_T("{BB2E617C-0920-11D1-9A0B-00C04FC2D6C1}"),	// IExtractImage
	_T("{00021500-0000-0000-C000-000000000046}"),	// IQueryInfo
	_T("{E357FCCD-A995-4576-B01F-234630154E96}"),	// IThumbnailProvider
//	_T("DataHandler"),								// IDataObject
//	_T("{8895B1C6-B41F-4C1C-A562-0D564250836F}"),	// IPreviewHandler
	NULL
};

HRESULT CSageThumbsModule::DllRegisterServer()
{
	CHECKPOINT_BEGIN(DllRegisterServer)

	HRESULT hr = CAtlDllModuleT< CSageThumbsModule >::DllRegisterServer (FALSE);

	for ( POSITION pos = _ExtMap.GetHeadPosition(); pos; )
	{
		if ( const CExtMap::CPair* p = _ExtMap.GetNext( pos ) )
		{
			if ( p->m_value.enabled )
				RegisterExt( p->m_key );
			else
				UnregisterExt( p->m_key );
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
	//			_T(""), MyCLSID, _T("SageThumbs.bak"));
	//	}
	//	else
	//	{
	//		RegisterValue( HKEY_CLASSES_ROOT, sRoot + szHandlers[ i ],
	//			_T(""), MyCLSID, _T("SageThumbs.bak") );
	//	}
	//}

	CleanWindowsCache();

	UpdateShell();

	ATLTRACE ("DllRegisterServer : ");
	CHECKPOINT(DllRegisterServer)
	ATLTRACE ("\n");

	return hr;
}

HRESULT CSageThumbsModule::DllUnregisterServer()
{
	CHECKPOINT_BEGIN(DllUnregisterServer)

	HRESULT hr = CAtlDllModuleT< CSageThumbsModule >::DllUnregisterServer (FALSE);

	for ( POSITION pos = _ExtMap.GetHeadPosition(); pos; )
	{
		if ( const CExtMap::CPair* p = _ExtMap.GetNext( pos ) )
		{
			UnregisterExt( p->m_key );
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
		UnregisterValue (HKEY_CLASSES_ROOT, sRoot + szHandlers[ i ],
			_T(""), MyCLSID, _T("SageThumbs.bak"));
	}

	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, _T("SystemFileAssociations\\image\\ShellEx"));	// HKCU
	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, _T("SystemFileAssociations\\image\\ShellEx"));	// HKLM
	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, _T("SystemFileAssociations\\image"));	// HKCU
	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, _T("SystemFileAssociations\\image"));	// HKLM

	CleanWindowsCache();

	UpdateShell();

	ATLTRACE ("DllUnregisterServer : ");
	CHECKPOINT(DllUnregisterServer)
	ATLTRACE ("\n");

	return hr;
}

void CSageThumbsModule::UpdateShell()
{
	SHChangeNotify (SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	SendMessageTimeout (HWND_BROADCAST, WM_SETTINGCHANGE, 0, NULL, SMTO_ABORTIFHUNG, 1000, NULL);
	CoFreeUnusedLibraries ();
}

void CSageThumbsModule::RegisterExt(LPCTSTR szExt)
{
	CString key ( _T(".") );
	CString ext ( szExt );
	key += ext;

	CString DefaultKey = GetRegValue( _T(""), _T(""), key, HKEY_CLASSES_ROOT );
	if ( DefaultKey.IsEmpty() )
	{
		DefaultKey = _T("XnView.Image");
		RegisterValue( HKEY_CLASSES_ROOT, key,
			_T(""), DefaultKey, _T("SageThumbs.bak") );
	}

	// Perceived Type Fix
	SetRegValue( _T("PerceivedType"), _T("image"), key, HKEY_CLASSES_ROOT );

	// Content Type Fix
	CString sContentType( _T("image/") );
	if ( _tcsicmp( szExt, _T("jpg")  ) == 0 ||
		 _tcsicmp( szExt, _T("jpe")  ) == 0 ||
		 _tcsicmp( szExt, _T("jfif") ) == 0 )
		sContentType += _T("jpeg");
	else
		sContentType += szExt;
	SetRegValue( _T("Content Type"), sContentType, key, HKEY_CLASSES_ROOT );
	SetRegValue( _T("Extension"), key, _T("MIME\\DataBase\\Content Type\\") + sContentType, HKEY_CLASSES_ROOT );

	const bool bEnableThumbs = GetRegValue( _T("EnableThumbs"), 1 ) != 0;
	const bool bEnableIcons = GetRegValue( _T("EnableIcons"), 1 ) != 0;

	for ( int i = 0; szHandlers[ i ]; ++i )
	{
		switch ( i )
		{
		case 0:	// IExtractIcon
		case 2:	// IQueryInfo
			UnregisterValue( HKEY_CLASSES_ROOT,
				key + _T("\\ShellEx\\") + szHandlers[ i ],
				_T(""), MyCLSID, _T("SageThumbs.bak") );
			if ( ( i == 0 && bEnableIcons ) || ( i == 2 ) )
			{
				RegisterValue (HKEY_CLASSES_ROOT,
					DefaultKey + _T("\\ShellEx\\") + szHandlers[ i ],
					_T(""), MyCLSID, _T("SageThumbs.bak"));
			}
			else
			{
				UnregisterValue( HKEY_CLASSES_ROOT,
					DefaultKey + _T("\\ShellEx\\") + szHandlers[ i ],
					_T(""), MyCLSID, _T("SageThumbs.bak") );
			}
			break;
		
		default:
			UnregisterValue( HKEY_CLASSES_ROOT,
				DefaultKey + _T("\\ShellEx\\") + szHandlers[ i ],
				_T(""), MyCLSID, _T("SageThumbs.bak") );
			if ( ( i == 1 && bEnableThumbs ) ||	// IExtractImage
				 ( i == 3 && bEnableThumbs ) )	// IThumbnailProvider
			{
				RegisterValue (HKEY_CLASSES_ROOT,
					key + _T("\\ShellEx\\") + szHandlers[ i ],
					_T(""), MyCLSID, _T("SageThumbs.bak"));
			}
			else
			{
				UnregisterValue( HKEY_CLASSES_ROOT,
					key + _T("\\ShellEx\\") + szHandlers[ i ],
					_T(""), MyCLSID, _T("SageThumbs.bak") );
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
	//		_T(""), HTMLCLSID, _T("SageThumbs.bak"));
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

	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, key + _T("\\ShellEx"));		// HKCU
	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, key + _T("\\ShellEx"));		// HKLM
	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, key);			// HKCU
	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, key);			// HKLM

	if ( ! DefaultKey.IsEmpty() )
	{
		SHDeleteEmptyKey (HKEY_CLASSES_ROOT, DefaultKey + _T("\\ShellEx"));	// HKCU
		SHDeleteEmptyKey (HKEY_CLASSES_ROOT, DefaultKey + _T("\\ShellEx"));	// HKLM
		SHDeleteEmptyKey (HKEY_CLASSES_ROOT, DefaultKey);	// HKCU
		SHDeleteEmptyKey (HKEY_CLASSES_ROOT, DefaultKey);	// HKLM
	}
}

void CSageThumbsModule::UnregisterExt(LPCTSTR szExt)
{
	CString key ( _T(".") );
	CString ext ( szExt );
	key += ext;

	CString DefaultKey = GetRegValue( _T(""), _T(""), key, HKEY_CLASSES_ROOT );

	for ( int i = 0; szHandlers[ i ]; ++i )
	{
		UnregisterValue( HKEY_CLASSES_ROOT,
			key + _T("\\ShellEx\\") + szHandlers[ i ],
			_T(""), MyCLSID, _T("SageThumbs.bak") );

		if ( ! DefaultKey.IsEmpty() )
		{
			UnregisterValue( HKEY_CLASSES_ROOT,
				DefaultKey + _T("\\ShellEx\\") + szHandlers[ i ],
				_T(""), MyCLSID, _T("SageThumbs.bak") );
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
	//			_T(""), HTMLCLSID, _T("SageThumbs.bak"));
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

	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, key + _T("\\ShellEx"));		// HKCU
	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, key + _T("\\ShellEx"));		// HKLM
	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, key);			// HKCU
	SHDeleteEmptyKey (HKEY_CLASSES_ROOT, key);			// HKLM

	if ( ! DefaultKey.IsEmpty() )
	{
		SHDeleteEmptyKey (HKEY_CLASSES_ROOT, DefaultKey + _T("\\ShellEx"));	// HKCU
		SHDeleteEmptyKey (HKEY_CLASSES_ROOT, DefaultKey + _T("\\ShellEx"));	// HKLM
		SHDeleteEmptyKey (HKEY_CLASSES_ROOT, DefaultKey);	// HKCU
		SHDeleteEmptyKey (HKEY_CLASSES_ROOT, DefaultKey);	// HKLM
	}
}

LANGID CSageThumbsModule::GetLang ()
{
	return m_CurLangID;
}

BOOL CSageThumbsModule::LoadLang(LANGID LangID)
{
	if ( LangID == 0 )
		// Использование предыдущей настройки языка
		LangID = (LANGID)(DWORD)GetRegValue( _T("Lang"), (DWORD)0 );

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
	if ( m_LangDLL )
	{
		FreeLibrary( m_LangDLL );
		m_LangDLL = NULL;
	}

	// Установка стандартного языка
	m_CurLangID = STANDARD_LANGID;
	_AtlBaseModule.SetResourceInstance( _AtlBaseModule.GetModuleInstance() );
}

BOOL CSageThumbsModule::LoadLangIDDLL (LANGID LangID)
{
	ATLASSERT( LangID != 0 );
	ATLASSERT( m_LangDLL == NULL );

	if ( LangID == STANDARD_LANGID )
	{
		// Загрузка встроенного языка
		m_CurLangID = STANDARD_LANGID;
		return TRUE;
	}

	CString strLangIDDLL;
	strLangIDDLL.Format( _T("%sSageThumbs%.2x.dll"),
		(LPCTSTR)_ModuleFileName.Left ( _ModuleFileName.ReverseFind (_T('\\')) + 1),
		LangID );
	HINSTANCE hInstance = LoadLibrary( strLangIDDLL );
	if ( hInstance )
	{
		_AtlBaseModule.SetResourceInstance (hInstance);
		m_CurLangID = LangID;
		m_LangDLL = hInstance;
		return TRUE;
	}
	ATLTRACE ( "LoadLangIDDLL(%.2x) failed: %d\n", LangID, GetLastError ());
	return FALSE;
}

void CSageThumbsModule::FillExtMap()
{
	CHECKPOINT_BEGIN( FillExtMap )

	if (_ExtMap.IsEmpty ())
	{
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
					_ExtMap.SetAt( sExt, data );
				}
			}
		}
        							
		// Загрузка данных о расширении
		i = 1;
		const CString key = CString( REG_SAGETHUMBS ) + _T("\\");
		for ( POSITION pos = _ExtMap.GetHeadPosition(); pos; ++i )
		{
			CExtMap::CPair* p = _ExtMap.GetNext (pos);

			// Разрешено или нет
			DWORD dwEnabled = GetRegValue( _T("Enabled"),
				( ( p->m_key == _T("ico") || p->m_key == _T("cur") ) ? 0 : 1 ),
				key + p->m_key );
			SetRegValue( _T(""), p->m_value.info, key + p->m_key );
			SetRegValue( _T("Enabled"), dwEnabled, key + p->m_key );
			p->m_value.enabled = ( dwEnabled != 0 );

			ATLTRACE( "%4d. %c %8s \"%s\"\n", i, ( p->m_value.enabled ? '+' : '-' ), (LPCSTR)CT2A( p->m_key ), (LPCSTR)CT2A( p->m_value.info ) );	
		}

		ATLTRACE( "Loaded %d formats, %d extensions. ", count, _ExtMap.GetCount() );
		CHECKPOINT( FillExtMap )
	}
}

BOOL CSageThumbsModule::Initialize()
{
	ATLTRACE ( "CSageThumbsModule::Initialize ()\n" );

	InitializeCriticalSection (&_GflGuard);

	// Получение полного пути модуля
	GetModuleFileName(_AtlBaseModule.GetModuleInstance (),
		_ModuleFileName.GetBuffer( MAX_LONG_PATH ), MAX_LONG_PATH );
	_ModuleFileName.ReleaseBuffer();
	ATLTRACE( "Module path: %s\n", (LPCSTR)CT2A( _ModuleFileName ) );

	//DWORD id;
	//m_hWatchThread = CreateThread( NULL, 0, WatchThread, (LPVOID) this, 0, &id );
	//Sleep( 0 );

	CHECKPOINT_BEGIN(LoadLibrarySQLite)
	if ( ! _libsqlite.LoadLibrary ( _AtlBaseModule.GetModuleInstance() ) )
		// Ошибка загрузки
		return FALSE;
	CHECKPOINT(LoadLibrarySQLite)

	// Загрузка библиотек
	CHECKPOINT_BEGIN(LoadLibraryGFL)
	if ( ! _libgfl.LoadLibrary  ( _AtlBaseModule.GetModuleInstance() ) )
		// Ошибка загрузки
		return FALSE;
	CHECKPOINT(LoadLibraryGFL)

	CHECKPOINT_BEGIN(LoadLibraryGFLE)
	if ( ! _libgfle.LoadLibrary ( _AtlBaseModule.GetModuleInstance() ) )
		// Ошибка загрузки
		return FALSE;
	CHECKPOINT(LoadLibraryGFLE)

	// Получение пути до базы данных
	_Database = GetRegValue( _T("Database") );
	if ( _Database.IsEmpty() )
	{
		_Database = GetSpecialFolderPath( CSIDL_LOCAL_APPDATA );
		if ( _Database.IsEmpty() )
		{
			GetWindowsDirectory( _Database.GetBuffer( MAX_LONG_PATH ), MAX_LONG_PATH );
			_Database.ReleaseBuffer();
		}
		_Database.TrimRight (_T("\\"));
		_Database += _T("\\SageThumbs.db3");
		SetRegValue( _T("Database"), _Database );
	}
	ATLTRACE( "Database path: %s\n", (LPCSTR)CT2A( _Database ) );

	// Загрузка локализации
	CHECKPOINT_BEGIN(LoadLangs)
	LoadLang ();
	CHECKPOINT(LoadLangs)

	CHECKPOINT_BEGIN(GFLInit)

	// Получение папки с плагинами
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
			_PlugInsPathname = buf;
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
			_PlugInsPathname = buf;
		}
		else
		{
			// %Program Files%\XnView\PlugIns
			buf = GetSpecialFolderPath( CSIDL_PROGRAM_FILES );
			if ( ! buf.IsEmpty() )
			{
				buf.TrimRight (_T("\\"));
				buf += _T("\\XnView\\PlugIns");
				_PlugInsPathname = buf;
			}
		}
	}
	if ( ! _PlugInsPathname.IsEmpty ())
		MakeDirectory( _PlugInsPathname );
	if ( !_PlugInsPathname.IsEmpty() )
		gflSetPluginsPathnameT( _PlugInsPathname );
	ATLTRACE( "gflSetPluginsPathnameW : \"%s\"\n", (LPCSTR)CT2A( _PlugInsPathname ) );

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
//	for (int i = 0; _Bits [i].ext; ++i)
//		_BitsMap.SetAt (_Bits [i].ext, &_Bits [i]);

	return TRUE;
}

void CSageThumbsModule::UnInitialize ()
{
	ATLTRACE ( "CSageThumbsModule::UnInitialize ()\n" );

	if ( _libgfl )
	{
		ATLTRACE( "gflLibraryExit\n" );
		gflLibraryExit ();
	}

	UnLoadLang ();

	/*ATLTRACE (_T("CSageThumbsModule::UnInitialize () -> begin\n"));
	if (m_hWatchThread)
	{
		HANDLE hWatchEvent = CreateEvent (NULL, TRUE, TRUE, _T("SageThumbsWatch"));
		if (hWatchEvent)
		{
			for (int count = 0;
				(WaitForSingleObject (m_hWatchThread, 100) == WAIT_TIMEOUT) &&
				count < 20; count++)
				SetEvent (hWatchEvent);
			if (WaitForSingleObject (m_hWatchThread, 0) == WAIT_TIMEOUT)
			{
				ATLTRACE (_T("CSageThumbsModule::UnInitialize () -> TerminateThread\n"));
				TerminateThread (m_hWatchThread, 0);
			}
			CloseHandle (m_hWatchThread);
			m_hWatchThread = NULL;
			CloseHandle (hWatchEvent);
		}
	}
	ATLTRACE (_T("CSageThumbsModule::UnInitialize () -> end\n"));*/

	DeleteCriticalSection (&_GflGuard);
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
	ATLTRACE( "Calling ::DllMain()...\n" );

	if ( _AtlModule.DllMain( dwReason, lpReserved ) )
	{
		switch ( dwReason )
		{
		case DLL_PROCESS_ATTACH:
			ATLTRACE( "DllMain::DLL_PROCESS_ATTACH\n" );
			__try
			{
				if ( ! _AtlModule.Initialize() )
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
				_AtlModule.UnInitialize();
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

	return _AtlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	ATLTRACE( "Calling ::DllGetClassObject()...\n" );

	return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer(void)
{
	ATLTRACE( "Calling ::DllRegisterServer()...\n" );

	return _AtlModule.DllRegisterServer();
}

STDAPI DllUnregisterServer(void)
{
	ATLTRACE( "Calling ::DllUnregisterServer()...\n" );

	return _AtlModule.DllUnregisterServer();
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

void RegisterValue (HKEY root, LPCTSTR key, LPCTSTR name, LPCTSTR value, LPCTSTR backup)
{
	// Check for new value
	CString buf = GetRegValue( name, _T(""), key, root );
	if ( buf.IsEmpty() || buf.CompareNoCase( value ) != 0 )
	{
		// Set new value
		SetRegValue( name, value, key, root );

		// Backup old one
		if ( ! buf.IsEmpty() )
			SetRegValue( backup, buf, key, root );
	}
}

void RegisterValue(HKEY root, LPCTSTR key, LPCTSTR name, const BYTE* value, DWORD value_size, LPCTSTR backup)
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
}

void UnregisterValue(HKEY root, LPCTSTR key, LPCTSTR name, LPCTSTR value, LPCTSTR backup)
{
	// Check backup
	CString buf = GetRegValue( name, _T(""), key, root );
	CString backup_buf = GetRegValue( backup, _T(""), key, root );
	if ( ! buf.IsEmpty() && buf.CompareNoCase( value ) == 0 )
	{			
		// Check backup value
		if ( ! backup_buf.IsEmpty() )
		{
			// Restore value from backup
			SetRegValue( name, backup_buf, key, root );

			// Delete backup value
			SHDeleteValue( root, key, backup );
			SHDeleteValue( root, key, backup );
		}
		else
		{
			// Delete original value
			SHDeleteValue( root, key, name );
			SHDeleteValue( root, key, name );
		}
	}
	else if ( ! backup_buf.IsEmpty() )
	{
		// Delete backup value
		SHDeleteValue( root, key, backup );
		SHDeleteValue( root, key, backup );
	}

	// Clean-up empty key
	SHDeleteEmptyKey( root, key );
	SHDeleteEmptyKey( root, key );
}

void UnregisterValue(HKEY root, LPCTSTR key, LPCTSTR name, const BYTE* value, DWORD value_size, LPCTSTR backup)
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
}

HRESULT SAFEgflGetFileInformation(LPCTSTR filename, GFL_FILE_INFORMATION* info)
{
	GFL_ERROR err;
	HRESULT hr = E_FAIL;
	EnterCriticalSection (&_GflGuard);
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
	LeaveCriticalSection (&_GflGuard);
	return hr;
}

HRESULT SAFEgflLoadBitmap(LPCTSTR filename, GFL_BITMAP **bitmap)
{
	GFL_ERROR err;
	HRESULT hr = E_FAIL;
	EnterCriticalSection (&_GflGuard);
	__try
	{
		GFL_LOAD_PARAMS params = {};
		gflGetDefaultLoadParams( &params );

		*bitmap = NULL;
		err = gflLoadBitmapT( filename, bitmap, &params, NULL);
		if ( err == GFL_ERROR_FILE_READ )
		{
			params.Flags |= GFL_LOAD_IGNORE_READ_ERROR;

			err = gflLoadBitmapT( filename, bitmap, &params, NULL );
		}
		if ( err == GFL_NO_ERROR )
			hr = S_OK;
		else
		{
			ATLTRACE ("E_FAIL (gflLoadBitmapW) : %s\n", gflGetErrorString (err));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ATLTRACE ("E_FAIL (gflLoadBitmapW exception)\n");
	}
	LeaveCriticalSection (&_GflGuard);
	return hr;
}

HRESULT SAFEgflLoadThumbnail(LPCTSTR filename, GFL_INT32 width, GFL_INT32 height, GFL_BITMAP **bitmap)
{
	GFL_ERROR err;
	HRESULT hr = E_FAIL;
	EnterCriticalSection (&_GflGuard);
	__try
	{
		*bitmap = NULL;

		GFL_LOAD_PARAMS params = {};
		gflGetDefaultThumbnailParams( &params );
		params.Flags =
			GFL_LOAD_ONLY_FIRST_FRAME |
			GFL_LOAD_HIGH_QUALITY_THUMBNAIL |
			( ( ::GetRegValue( _T("UseEmbedded"), (DWORD)0 ) != 0 ) ? GFL_LOAD_EMBEDDED_THUMBNAIL : 0 ) |
			GFL_LOAD_PREVIEW_NO_CANVAS_RESIZE;

		err = gflLoadThumbnailT( filename, width, height, bitmap, &params, NULL );
		if ( err == GFL_ERROR_FILE_READ )
		{
			params.Flags |= GFL_LOAD_IGNORE_READ_ERROR;

			err = gflLoadThumbnailT( filename, width, height, bitmap, &params, NULL );
		}
		if ( err == GFL_NO_ERROR )
			hr = S_OK;
		else
		{
			ATLTRACE ("E_FAIL (gflLoadThumbnailW) : %s\n", gflGetErrorString (err));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ATLTRACE ("E_FAIL (gflLoadThumbnailW exception)\n");
	}
	LeaveCriticalSection (&_GflGuard);
	return hr;
}

HRESULT SAFEgflLoadThumbnailFromMemory(const GFL_UINT8* data, GFL_UINT32 data_length, GFL_INT32 width, GFL_INT32 height, GFL_BITMAP **bitmap)
{
	GFL_ERROR err;
	HRESULT hr = E_FAIL;
	EnterCriticalSection (&_GflGuard);
	__try
	{
		*bitmap = NULL;

		GFL_LOAD_PARAMS params = {};
		gflGetDefaultThumbnailParams (&params);
		params.Flags =
			GFL_LOAD_HIGH_QUALITY_THUMBNAIL |
			GFL_LOAD_PREVIEW_NO_CANVAS_RESIZE;

		err = gflLoadThumbnailFromMemory( data, data_length, width, height, bitmap, &params, NULL );
		if ( err == GFL_NO_ERROR )
			hr = S_OK;
		else
		{
			ATLTRACE ("E_FAIL (gflLoadBitmapFromMemory) : %s\n", gflGetErrorString (err));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ATLTRACE ("E_FAIL (gflLoadBitmapFromMemory exception)\n");
	}
	LeaveCriticalSection (&_GflGuard);
	return hr;
}

HRESULT SAFEgflConvertBitmapIntoDDB(const GFL_BITMAP *bitmap, HBITMAP *hBitmap)
{
	GFL_ERROR err;
	HRESULT hr = E_FAIL;
	EnterCriticalSection (&_GflGuard);
	__try
	{
		*hBitmap = NULL;

		err = gflConvertBitmapIntoDDB( bitmap, hBitmap );
		if ( err == GFL_NO_ERROR )
			hr = S_OK;
		else
		{
			ATLTRACE ("E_FAIL (gflConvertBitmapIntoDDB) : %s\n", gflGetErrorString (err));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ATLTRACE ("E_FAIL (gflConvertBitmapIntoDDB exception)\n");
	}
	LeaveCriticalSection (&_GflGuard);
	return hr;
}

HRESULT SAFEgflFreeBitmap(GFL_BITMAP*& bitmap)
{
	if ( ! bitmap )
		return S_FALSE;

	HRESULT hr = E_FAIL;
	EnterCriticalSection (&_GflGuard);
	__try
	{
		gflFreeBitmap( bitmap );
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ATLTRACE ("E_FAIL (gflFreeBitmap exception)\n");
	}
	LeaveCriticalSection (&_GflGuard);
	bitmap = NULL;
	return hr;
}

bool IsGoodFile(LPCTSTR szFilename, Ext* pdata, WIN32_FIND_DATA* pfd)
{
	CString sExt = PathFindExtension( szFilename );
	if ( sExt.GetLength() < 2 )
		// No extension
		return false;
	sExt.MakeLower();

	Ext foo_data;
	if ( ! pdata )
		pdata = &foo_data;
	if ( ! _ExtMap.Lookup( (LPCTSTR)sExt + 1, *pdata ) )
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

	DWORD max_size = GetRegValue( _T("MaxSize"), (DWORD)FILE_MAX_SIZE );
	if ( max_size && ( pfd->nFileSizeHigh || pfd->nFileSizeLow / ( 1024 * 1024 ) > max_size ) )
		// Too big file
		return false;

	return true;
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

void CleanWindowsCache()
{
	CComPtr< IEmptyVolumeCache > pWindowsThumbnailer;
	HRESULT hr = pWindowsThumbnailer.CoCreateInstance( CLSID_WindowsThumbnailer );
	if ( SUCCEEDED( hr ) )
	{
		pWindowsThumbnailer->Purge( 0, NULL );
	}	
}
