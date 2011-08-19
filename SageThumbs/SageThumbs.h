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

#pragma once

// #define GFL_THREAD_SAFE	// When enabled GFL calls guarded by critical section

#define LIB_GFL				"libgfl340.dll"	// Name of GFL library (case sensitive)
#define LIB_GFLE			"libgfle340.dll"// Name of GFLe library (case sensitive)
#define LIB_SQLITE			"sqlite3.dll"	// Name of SQLite library (case sensitive)
#define CLSID_THUMB			_T("{4A34B3E3-F50E-4FF6-8979-7E4176466FF2}")
#define REG_SAGETHUMBS		_T("Software\\SageThumbs")
#define REG_SAGETHUMBS_BAK	_T("SageThumbs.bak")
#define REG_SAGETHUMBS_IMG	_T("SageThumbsImage")
#define JPEG_DEFAULT		85ul			// JPEG default quality
#define PNG_DEFAULT			6ul				// PNG default compression
#define THUMB_STORE_SIZE	256ul			// Minimum thumbnail size for database
#define THUMB_MIN_SIZE		32ul			// Минимальный размер просмотра в пикселях
#define THUMB_MAX_SIZE		512ul			// Максимальный размер просмотра в пикселях 
#define FILE_MAX_SIZE		10ul			// Максимальный размер файла в Мб
#define STANDARD_LANGID		0x09			// Стандартный встроенный язык - English

// Disabled by default
#define EXT_DEFAULT(ext) \
	((ext)==_T("ico")|| \
	 (ext)==_T("ani")|| \
	 (ext)==_T("cur")|| \
	 (ext)==_T("sys")|| \
	 (ext)==_T("wmz"))

// SQL для создания базы данных
const LPCTSTR RECREATE_DATABASE =
	_T("PRAGMA foreign_keys = ON;")
	_T("BEGIN TRANSACTION;")
		_T("CREATE TABLE IF NOT EXISTS Pathes ( ")
			_T("PathID INTEGER NOT NULL PRIMARY KEY, ")
			_T("Pathname TEXT NOT NULL UNIQUE );")
		_T("CREATE INDEX IF NOT EXISTS PathesIndex ON Pathes( Pathname );")
		_T("CREATE TABLE IF NOT EXISTS Entities ( ")
			_T("PathID INTEGER NOT NULL, ")
			_T("Filename TEXT NOT NULL, ")
			_T("LastWriteTime INTEGER, ")
			_T("CreationTime INTEGER, ")
			_T("FileSize INTEGER, ")
			_T("ImageInfo BLOB, ")
			_T("Image BLOB, ")
			_T("Width INTEGER DEFAULT 0, ")
			_T("Height INTEGER DEFAULT 0, ")
			_T("PRIMARY KEY ( PathID, Filename ),")
			_T("FOREIGN KEY ( PathID ) REFERENCES Pathes( PathID ) );")
		_T("CREATE INDEX IF NOT EXISTS EntitiesIndex ON Entities( PathID );")
	_T("COMMIT;")
	_T("VACUUM;");

const LPCTSTR DROP_DATABASE =
	_T("BEGIN TRANSACTION;")
		_T("DROP INDEX IF EXISTS EntitiesIndex;")
		_T("DROP TABLE IF EXISTS Entities;")
		_T("DROP INDEX IF EXISTS PathesIndex;")
		_T("DROP TABLE IF EXISTS Pathes;")
	_T("COMMIT;")
	_T("VACUUM;");

const LPCTSTR OPTIMIZE_DATABASE =
	_T("ANALYZE;")
	_T("VACUUM;");

typedef struct
{
	bool	enabled;
	CString	info;
} Ext;

typedef CRBMap < CString, Ext > CExtMap;

/*typedef struct
{
	LPCTSTR		ext;
	DWORD		size;
	const char*	mask;
	const char*	data;
} BitsDescription;

typedef CAtlMap < CString, const BitsDescription* > BitsDescriptionMap;*/

class CSageThumbsModule : public CAtlDllModuleT< CSageThumbsModule >
{
public:
	DECLARE_REGISTRY_APPID_RESOURCEID( IDR_SAGETHUMBS, "{B04F3D73-C8D6-4473-B47C-B942CAE19B45}" )

	CSageThumbsModule();

	OSVERSIONINFO			m_OSVersion;			// OS version
	CString					m_sModuleFileName;		// This module full filename
	CString					m_sModule;				// This module name without extension
	CString					m_sHome;				// Installation folder
	CString					m_sDatabase;			// Database filename
	CExtMap					m_oExtMap;				// Supported image extensions

	HRESULT DllRegisterServer();
	HRESULT DllUnregisterServer();

	BOOL Initialize();
	void UnInitialize();

	BOOL RegisterExtensions(HWND hWnd = NULL);
	BOOL UnregisterExtensions();

	LANGID GetLang();					// Текущий загруженный язык
	BOOL LoadLang(LANGID LangID = 0);	// Загрузка языка (LangID == 0 - из реестра)

	// Обновление настроек Explorer'a
	void UpdateShell();

	HRESULT GetFileInformation(LPCTSTR filename, GFL_FILE_INFORMATION* info);
	HRESULT LoadBitmap(LPCTSTR filename, GFL_BITMAP **bitmap);
	HRESULT LoadThumbnail(LPCTSTR filename, int width, int height, GFL_BITMAP **bitmap);
	HRESULT LoadBitmapFromMemory(LPCVOID data, UINT data_length, GFL_BITMAP **bitmap);
	HRESULT ConvertBitmap(const GFL_BITMAP* bitmap, HBITMAP* phBitmap);
	HRESULT Resize(GFL_BITMAP* src, GFL_BITMAP** dst, int width, int height);
	HRESULT FreeBitmap(GFL_BITMAP*& bitmap);

	// Проверка, что файл подходит для загрузки по всем параметрам
	bool IsGoodFile(LPCTSTR szFilename, Ext* pdata = NULL, WIN32_FIND_DATA* pfd = NULL) const;

	inline CString GetAppName() const
	{
		CString sTitle;
		sTitle.LoadString( IDS_PROJNAME );
#ifdef WIN64
		return sTitle + _T(" 64-bit");
#else
		return sTitle + _T(" 32-bit");
#endif
	}

	inline int MsgBox(HWND hWnd, UINT nText, UINT nType = MB_OK | MB_ICONEXCLAMATION)
	{
		CString sText;
		sText.LoadString( nText );
		return MessageBox( hWnd, sText, GetAppName(), nType );
	}

protected:
	HMODULE					m_hGFL;
	HMODULE					m_hGFLe;
	HMODULE					m_hSQLite;
	HINSTANCE				m_hLangDLL;
	LANGID					m_CurLangID;
//	HANDLE					m_hWatchThread;

//	static DWORD WINAPI WatchThread (LPVOID param);
	void UnLoadLang ();
	BOOL LoadLangIDDLL(LANGID LangID);

	BOOL RegisterExt(LPCTSTR szExt, LPCTSTR szInfo, bool bEnableThumbs, bool bEnableIcons, bool bEnableInfo, bool bEnableOverlay);
	BOOL UnregisterExt(LPCTSTR szExt, bool bFull);

	// Restore file extension lost ProgID using several methods
	void FixProgID(LPCTSTR szExt);

	void FillExtMap();

#ifdef GFL_THREAD_SAFE
	CComAutoCriticalSection m_pSection;
	HRESULT GetFileInformationE(LPCTSTR filename, GFL_FILE_INFORMATION* info);
	HRESULT LoadBitmapE(LPCTSTR filename, GFL_BITMAP **bitmap);
	HRESULT LoadThumbnailE(LPCTSTR filename, int width, int height, GFL_BITMAP **bitmap);
	HRESULT LoadBitmapFromMemoryE(LPCVOID data, UINT data_length, GFL_BITMAP **bitmap);
	HRESULT ConvertBitmapE(const GFL_BITMAP* bitmap, HBITMAP* phBitmap);
	HRESULT ResizeE(GFL_BITMAP* src, GFL_BITMAP** dst, int width, int height);
	HRESULT FreeBitmapE(GFL_BITMAP*& bitmap);
#endif // GFL_THREAD_SAFE
};

//extern BitsDescriptionMap	_BitsMap;
extern CSageThumbsModule	_Module;

// Макросы для измерения времени
#ifdef _DEBUG
	#define CHECKPOINT_BEGIN(liLast) \
		LARGE_INTEGER liLast; \
		QueryPerformanceCounter (&liLast);
	#define CHECKPOINT(liLast) { \
		LARGE_INTEGER liCurrent; \
		QueryPerformanceCounter (&liCurrent); \
		LARGE_INTEGER liFrequency; \
		QueryPerformanceFrequency (&liFrequency); \
		ATLTRACE ("%s %ld ms\n", #liLast, (DWORD) ((liCurrent.QuadPart - liLast.QuadPart) / ( liFrequency.QuadPart / 1000 ) )); \
	}
#else // _DEBUG
	#define CHECKPOINT_BEGIN(liLast)
	#define CHECKPOINT(liLast)
#endif // _DEBUG

BOOL GetRegValue(LPCTSTR szName, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
DWORD GetRegValue(LPCTSTR szName, DWORD dwDefault, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
CString GetRegValue(LPCTSTR szName, const CString& sDefault, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
BOOL SetRegValue(LPCTSTR szName, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
BOOL SetRegValue(LPCTSTR szName, DWORD dwValue, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
BOOL SetRegValue(LPCTSTR szName, const CString& sValue, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);

// Cleaning DENIED rights from key
BOOL CleanRegKey(HKEY hRoot, LPCTSTR szKey);

BOOL DeleteRegValue(LPCTSTR szName, LPCTSTR szKey, HKEY hRoot);
BOOL DeleteRegKey(HKEY hRoot, LPCTSTR szSubKey);
BOOL DeleteEmptyRegKey(HKEY hRoot, LPCTSTR szSubKey);

LPCTSTR GetKeyName(HKEY hRoot);

BOOL RegisterValue(HKEY hRoot, LPCTSTR szKey, LPCTSTR szName = _T(""), LPCTSTR szValue = CLSID_THUMB, LPCTSTR szBackupName = REG_SAGETHUMBS_BAK);
BOOL UnregisterValue(HKEY hRoot, LPCTSTR szKey, LPCTSTR szName = _T(""), LPCTSTR szValue = CLSID_THUMB, LPCTSTR szBackupName = REG_SAGETHUMBS_BAK);

CString GetDefaultType(LPCTSTR szExt);
CString GetPerceivedType(LPCTSTR szExt);
CString GetContentExt(LPCTSTR szExt);
CString GetContentType(LPCTSTR szExt);

BOOL IsKeyExists(HKEY hRoot, LPCTSTR szKey);

// Экспортируемые функции
STDAPI DllCanUnloadNow (void);
STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID* ppv);
STDAPI DllRegisterServer (void);
STDAPI DllUnregisterServer (void);
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine);
void CALLBACK Options (HWND hwnd, HINSTANCE hinst = NULL, LPSTR lpszCmdLine = NULL, int nCmdShow = 0);
LONG APIENTRY CPlApplet (HWND hwnd, UINT uMsg, LPARAM lParam1, LPARAM lParam2);

class CWaitCursor
{
public:
	CWaitCursor() : m_hCursor( SetCursor( LoadCursor( NULL, IDC_WAIT ) ) ) { }
	~CWaitCursor() { SetCursor( m_hCursor ); }

protected:
	HCURSOR m_hCursor;
};
