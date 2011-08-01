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
	DECLARE_REGISTRY_APPID_RESOURCEID (IDR_SAGETHUMBS,
		"{B04F3D73-C8D6-4473-B47C-B942CAE19B45}")

	CSageThumbsModule();

	HRESULT DllRegisterServer();
	HRESULT DllUnregisterServer();

	BOOL Initialize();
	void UnInitialize();

	LANGID GetLang();					// Текущий загруженный язык
	BOOL LoadLang(LANGID LangID = 0);	// Загрузка языка (LangID == 0 - из реестра)

	// Обновление настроек Explorer'a
	void UpdateShell();

protected:
	HINSTANCE	m_LangDLL;
	LANGID		m_CurLangID;
//	HANDLE		m_hWatchThread;

//	static DWORD WINAPI WatchThread (LPVOID param);
	void UnLoadLang ();
	BOOL LoadLangIDDLL (LANGID LangID);

	void RegisterExt(LPCTSTR szExt);
	void UnregisterExt(LPCTSTR szExt);
	void FillExtMap ();
};

extern CString				_ModuleFileName;
extern CString				_Database;
extern CString				_PlugInsPathname;
extern CExtMap			_ExtMap;
extern CRITICAL_SECTION		_GflGuard;
//extern BitsDescriptionMap	_BitsMap;
extern CSageThumbsModule	_AtlModule;

typedef ULONG (FAR PASCAL *tMAPISendMail)(LHANDLE, ULONG_PTR, lpMapiMessage, FLAGS, ULONG);

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

// Сервисные функции
void RegisterValue(HKEY root, LPCTSTR key, LPCTSTR name, LPCTSTR value, LPCTSTR backup);
void RegisterValue(HKEY root, LPCTSTR key, LPCTSTR name, const BYTE* value, DWORD value_size, LPCTSTR backup);
void UnregisterValue(HKEY root, LPCTSTR key, LPCTSTR name, LPCTSTR value, LPCTSTR backup);
void UnregisterValue(HKEY root, LPCTSTR key, LPCTSTR name, const BYTE* value, DWORD value_size, LPCTSTR backup);
void MakeDirectory(LPCTSTR dir);

HRESULT SAFEgflGetFileInformation(LPCTSTR filename, GFL_FILE_INFORMATION* info);
HRESULT SAFEgflLoadBitmap(LPCTSTR filename, GFL_BITMAP **bitmap);
HRESULT SAFEgflLoadThumbnail(LPCTSTR filename, GFL_INT32 width, GFL_INT32 height, GFL_BITMAP **bitmap);
HRESULT SAFEgflLoadThumbnailFromMemory(const GFL_UINT8 * data, GFL_UINT32 data_length, GFL_INT32 width, GFL_INT32 height, GFL_BITMAP **bitmap);
HRESULT SAFEgflConvertBitmapIntoDDB(const GFL_BITMAP *bitmap, HBITMAP *hBitmap);
HRESULT SAFEgflFreeBitmap(GFL_BITMAP*& bitmap);

// Проверка, что файл подходит для загрузки по всем параметрам
bool IsGoodFile(LPCTSTR szFilename, Ext* pdata = NULL, WIN32_FIND_DATA* pfd = NULL);

BOOL LoadIcon(LPCTSTR szFilename, HICON* phSmallIcon, HICON* phLargeIcon = NULL, HICON* phHugeIcon = NULL, int nIcon = 0);

void CleanWindowsCache();

// Экспортируемые функции
STDAPI DllCanUnloadNow (void);
STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID* ppv);
STDAPI DllRegisterServer (void);
STDAPI DllUnregisterServer (void);
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
