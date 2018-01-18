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

#pragma once

class CSageThumbsModule;
class CWaitCursor;

//#define GFL_THREAD_SAFE	// When enabled GFL calls guarded by critical section
//#define ISTREAM_ENABLED	// Enable support for IInitializeWithStream interface

//extern BitsDescriptionMap		_BitsMap;
extern CSageThumbsModule		_Module;		// Application

#define LIB_GFL					"libgfl340.dll"	// Name of GFL library (case sensitive)
#define LIB_GFLE				"libgfle340.dll"// Name of GFLe library (case sensitive)
#define LIB_SQLITE				"sqlite3.dll"	// Name of SQLite library (case sensitive)
#define CLSID_THUMB				_T("{4A34B3E3-F50E-4FF6-8979-7E4176466FF2}")
#define REG_SAGETHUMBS			_T("Software\\SageThumbs")
#define REG_SAGETHUMBS_BAK		_T("SageThumbs.bak")
#define REG_SAGETHUMBS_IMG		_T("SageThumbsImage")
#define CUSTOM_TYPE				_T("SageThumbs Custom Type")
#define JPEG_DEFAULT			90ul			// JPEG default quality (0-100)
#define PNG_DEFAULT				9ul				// PNG default compression (0-9)
#define THUMB_STORE_SIZE		256ul			// Minimum thumbnail size for database, pixels
#define THUMB_MIN_SIZE			32ul			// Thumbnail minimum size, pixels
#define THUMB_MAX_SIZE			512ul			// Thumbnail maximum size, pixels
#define THUMB_STORE_PNG_RATIO	9				// Compression ratio for PNG-thumbnail
#define THUMB_STORE_JPG_RATIO	80				// Compression ration for JPG-thumbnail
#define THUMB_EMBEDDED_MIN_SIZE	96ul			// Embedded thumbnail minimum size, pixels
#define FILE_MAX_SIZE			100ul			// Default maximum file size, MB
#define STANDARD_LANGID			0x09			// Default language ID - English

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
	bool	custom;
	CString	info;
	int		index;		// Format index (-1 - custom)
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

	CString					m_sModuleFileName;		// This module full filename
	CString					m_sHome;				// Installation folder
	CString					m_sDatabase;			// Database filename
	CExtMap					m_oExtMap;				// Supported image extensions
	CLocalization			m_oLangs;				// Translations

	BOOL DllMain(DWORD dwReason, LPVOID lpReserved) throw();
	HRESULT DllRegisterServer() throw();
	HRESULT DllUnregisterServer() throw();

	BOOL Initialize();
	void UnInitialize();

	BOOL RegisterExtensions(HWND hWnd = NULL);
	BOOL UnregisterExtensions();

	// Обновление настроек Explorer
	void UpdateShell();

	// Image extensions disabled by default
	bool IsDisabledByDefault(LPCTSTR szExt) const;

	HRESULT LoadGFLBitmap(LPCTSTR filename, GFL_BITMAP **bitmap) throw();
	HRESULT GetFileInformation(LPCTSTR filename, GFL_FILE_INFORMATION* info) throw();
	HRESULT LoadThumbnail(LPCTSTR filename, int width, int height, GFL_BITMAP **bitmap) throw();
#ifdef ISTREAM_ENABLED
	HRESULT GetFileInformation(IStream* pStream, GFL_FILE_INFORMATION* info) throw();
	HRESULT LoadThumbnail(IStream* pStream, int width, int height, GFL_BITMAP **bitmap) throw();
#endif // ISTREAM_ENABLED
	HRESULT LoadBitmapFromMemory(LPCVOID data, UINT data_length, GFL_BITMAP **bitmap) throw();
	HRESULT ConvertBitmap(const GFL_BITMAP* bitmap, HBITMAP* phBitmap) throw();
	HRESULT Resize(GFL_BITMAP* src, GFL_BITMAP** dst, int width, int height) throw();
	HRESULT ResizeCanvas(GFL_BITMAP* src, GFL_BITMAP** dst, int width, int height) throw();
	HRESULT FreeBitmap(GFL_BITMAP*& bitmap) throw();
	HRESULT SaveBitmapIntoMemory( GFL_UINT8** data, GFL_UINT32* data_length, const GFL_BITMAP* bitmap, bool bPng ) throw();

	// Проверка, что файл подходит для загрузки по всем параметрам
	bool IsGoodFile(LPCTSTR szFilename, Ext* pdata = NULL) const;

	// Add user-defined custom extensions from string
	void AddCustomTypes(const CString& sCustom);

	CString GetAppName() const;

	int MsgBox( HWND hWnd, UINT nText, UINT nType = MB_OK | MB_ICONEXCLAMATION );

protected:
	HMODULE					m_hGFL;
	HMODULE					m_hGFLe;
	HMODULE					m_hSQLite;

	BOOL RegisterExt(const CString& sExt, const CString& sInfo, bool bEnableThumbs, bool bEnableIcons, bool bEnableInfo, bool bEnableOverlay);
	BOOL UnregisterExt(const CString& sExt, bool bFull);

	// Restore file extension lost ProgID using several methods
	BOOL FindAndFixProgID(const CString& sExt, CAtlList< CString >& oGoodProgIDs);

	void FillExtMap();

#ifdef GFL_THREAD_SAFE
	CComAutoCriticalSection m_pSection;
	HRESULT LoadGFLBitmapE( LPCTSTR filename, GFL_BITMAP **bitmap ) throw();
	HRESULT GetFileInformationE(LPCTSTR filename, GFL_FILE_INFORMATION* info) throw();
	HRESULT LoadBitmapE(LPCTSTR filename, GFL_BITMAP **bitmap) throw();
	HRESULT LoadThumbnailE(LPCTSTR filename, int width, int height, GFL_BITMAP **bitmap) throw();
	HRESULT LoadBitmapFromMemoryE(LPCVOID data, UINT data_length, GFL_BITMAP **bitmap) throw();
	HRESULT ConvertBitmapE(const GFL_BITMAP* bitmap, HBITMAP* phBitmap) throw();
	HRESULT ResizeE(GFL_BITMAP* src, GFL_BITMAP** dst, int width, int height) throw();
	HRESULT ResizeCanvasE( GFL_BITMAP* src, GFL_BITMAP** dst, int width, int height ) throw();
	HRESULT FreeBitmapE(GFL_BITMAP*& bitmap) throw();
	HRESULT SaveBitmapIntoMemoryE( GFL_UINT8** data, GFL_UINT32* data_length, const GFL_BITMAP* bitmap, bool bPng ) throw();
#endif // GFL_THREAD_SAFE
};

BOOL IsWindowsXP();
BOOL IsWindows8OrNewer();

bool IsValidCLSID(const CString& sCLSID);
BOOL GetRegValue(LPCTSTR szName, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
DWORD GetRegValue(LPCTSTR szName, DWORD dwDefault, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
CString GetRegValue(LPCTSTR szName, const CString& sDefault, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
BOOL SetRegValue(LPCTSTR szName, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
BOOL SetRegValue(LPCTSTR szName, DWORD dwValue, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);
BOOL SetRegValue(LPCTSTR szName, const CString& sValue, LPCTSTR szKey = REG_SAGETHUMBS, HKEY hRoot = HKEY_CURRENT_USER);

// Cleaning DENIED rights from key
BOOL FixKeyRights(HKEY hRoot, LPCTSTR szKey);
BOOL FixKey(__in HKEY hkey, __in_opt LPCTSTR pszSubKey);

LSTATUS SHSetValueForced(__in HKEY hkey, __in_opt LPCTSTR pszSubKey, __in_opt LPCTSTR pszValue, __in DWORD dwType, __in_bcount_opt(cbData) LPCVOID pvData, __in DWORD cbData);
BOOL DeleteRegValue(LPCTSTR szName, LPCTSTR szKey, HKEY hRoot);
BOOL DeleteRegKey(HKEY hRoot, LPCTSTR szSubKey);
BOOL DeleteEmptyRegKey(HKEY hRoot, LPCTSTR szSubKey);

LPCTSTR GetKeyName(HKEY hRoot);
LPCTSTR GetShortKeyName(HKEY hRoot);

BOOL RegisterValue(HKEY hRoot, LPCTSTR szKey, LPCTSTR szName = _T(""), LPCTSTR szValue = CLSID_THUMB, LPCTSTR szBackupName = REG_SAGETHUMBS_BAK);
BOOL UnregisterValue(HKEY hRoot, LPCTSTR szKey, LPCTSTR szName = _T(""), LPCTSTR szValue = CLSID_THUMB, LPCTSTR szBackupName = REG_SAGETHUMBS_BAK);

CString GetDefaultType(LPCTSTR szExt);
CString GetPerceivedType(LPCTSTR szExt);
CString GetContentExt(LPCTSTR szExt);
CString GetContentType(LPCTSTR szExt);

BOOL IsKeyExists(HKEY hRoot, LPCTSTR szKey);

BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
BOOL IsProcessElevated();

void CleanWindowsCache();

BOOL LoadIcon( LPCTSTR szFilename, HICON* phSmallIcon, HICON* phLargeIcon = NULL, HICON* phHugeIcon = NULL, int nIcon = 0 );

DWORD CRC32( const char *buf, int len );

// Get system folder path
CString GetSpecialFolderPath( int csidl );

// GFL callback functions for IStream handle
#ifdef ISTREAM_ENABLED
GFL_UINT32 GFLAPI IStreamRead(GFL_HANDLE handle, void* buffer, GFL_UINT32 size) throw();
GFL_UINT32 GFLAPI IStreamTell(GFL_HANDLE handle) throw();
GFL_UINT32 GFLAPI IStreamSeek(GFL_HANDLE handle, GFL_INT32 offset, GFL_INT32 origin) throw();
#endif // ISTREAM_ENABLED

#define Change_Color_Depth(x) gflChangeColorDepth( (x), NULL, GFL_MODE_TO_RGBA, GFL_MODE_ADAPTIVE )

void CALLBACK Options(HWND hwnd, HINSTANCE hinst = NULL, LPSTR lpszCmdLine = NULL, int nCmdShow = 0);

// Wait cursor wrapper
class ATL_NO_VTABLE CWaitCursor
{
public:
	inline CWaitCursor() : m_hCursor( SetCursor( LoadCursor( NULL, IDC_WAIT ) ) ) { }
	inline ~CWaitCursor() { SetCursor( m_hCursor ); }

protected:
	HCURSOR m_hCursor;
};

// Load localized string from resource
inline CString LoadString( UINT nID )
{
	return _Module.m_oLangs.LoadString( nID );
}
