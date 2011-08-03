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

/*
Shortcut Menu Handlers:
	1. IClassFactory
	2. IShellExtInit::Initialize
	3. IObjectWithSite::SetSite(pExplorer)
	4. IContextMenu::QueryContextMenu	-> Load info and image
	5. Menu commands:
		IContextMenu::GetCommandString
		IContextMenu3::HandleMenuMsg2 : WM_MEASUREITEM (for ownerdraw items)
		IContextMenu3::HandleMenuMsg2 : WM_DRAWITEM (for ownerdraw items)
		IContextMenu3::HandleMenuMsg2 : WM_INITMENUPOPUP (for submenu)
		IContextMenu::InvokeCommand
	6. IObjectWithSite::SetSite(NULL)
	7. Release

Shell item load:
	1. IInitializeWithStream (Windows Vista)
	2. IPersistStream (Windows 7)
	3. IInitializeWithItem (Windows Vista)
	4. IInitializeWithFile (Windows Vista)
	5. IPersistFile (Windows 2000)

InfoTip Handler:
	1. IClassFactory
	2. Shell item load
	3. ICustomizeInfoTip::?				- ???
	4. IQueryInfo::GetInfoTip			-> Load info only
	5. IQueryInfo::GetInfoFlags			- ???
	4. Release

Thumbnail Image Handler WinXP:
	1. IClassFactory
	2. Shell item load
	3. IExtractImage::GetLocation		-> Load info and image
	4. IExtractImage2:GetDateStamp
	5. IExtractImage::Extract
	6. Release

Thumbnail Image Handler Vista:
	1. IClassFactory
	2. Shell item load
	3. IExtractImage::GetLocation		-> Load info and image
	4. IThumbnailProvider::GetThumbnail
	5. Release

Data Handler:
	1. IClassFactory
	2. IPersistFile::Load
	3. Release

Icon Handler:
	1. IClassFactory
	2. Shell item load
	3. IExtractIcon::GetIconLocation
	4. IExtractIcon::Extract			-> Load info and image
	5. Release
*/

#pragma once

#include "Entity.h"

class ATL_NO_VTABLE CThumb :
	public CComObjectRootEx< CComMultiThreadModel >,
	public CComCoClass< CThumb, &CLSID_Thumb >,
	public IObjectSafetyImpl< CThumb, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA >,
	public IShellExtInit,
	public IContextMenu3,
	public IPersistFile,
//	public IParentAndItem,
//	public IInitializeWithStream,
	public IInitializeWithItem,
	public IInitializeWithFile,
	public IThumbnailProvider,
//	public IPreviewHandler,
//	public IOleWindow,
	public IExtractImage2,
//	public IRunnableTask,
	public IQueryInfo,
//	public IDataObject,
	public IExtractIconA,
	public IExtractIconW,
//	public IImageDecodeFilter,
//	public IColumnProvider
	public IObjectWithSite,
	public IEmptyVolumeCache2
{
public:
	CThumb();

	DECLARE_REGISTRY_RESOURCEID(IDR_THUMB)

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CThumb)
		COM_INTERFACE_ENTRY(IObjectSafety)
		COM_INTERFACE_ENTRY(IShellExtInit)
		COM_INTERFACE_ENTRY(IContextMenu3)
		COM_INTERFACE_ENTRY(IContextMenu2)
		COM_INTERFACE_ENTRY(IContextMenu)
		COM_INTERFACE_ENTRY(IPersist)
		COM_INTERFACE_ENTRY(IPersistFile)
//		COM_INTERFACE_ENTRY(IParentAndItem)
//		COM_INTERFACE_ENTRY(IInitializeWithStream)
		COM_INTERFACE_ENTRY(IInitializeWithItem)
		COM_INTERFACE_ENTRY(IInitializeWithFile)
		COM_INTERFACE_ENTRY(IThumbnailProvider)
//		COM_INTERFACE_ENTRY(IPreviewHandler)
//		COM_INTERFACE_ENTRY(IOleWindow)
		COM_INTERFACE_ENTRY(IExtractImage)
		COM_INTERFACE_ENTRY(IExtractImage2)
//		COM_INTERFACE_ENTRY(IRunnableTask)
		COM_INTERFACE_ENTRY(IQueryInfo)
//		COM_INTERFACE_ENTRY(IDataObject)
		COM_INTERFACE_ENTRY(IExtractIconA)
		COM_INTERFACE_ENTRY(IExtractIconW)
//		COM_INTERFACE_ENTRY(IImageDecodeFilter)
//		COM_INTERFACE_ENTRY(IColumnProvider)
		COM_INTERFACE_ENTRY(IObjectWithSite)
		COM_INTERFACE_ENTRY(IEmptyVolumeCache)
		COM_INTERFACE_ENTRY(IEmptyVolumeCache2)
	END_COM_MAP()

	BEGIN_CATEGORY_MAP(CThumb)
		IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
		IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
	END_CATEGORY_MAP()

	HRESULT FinalConstruct();
	void FinalRelease();

public:
// IShellExtInit
	STDMETHOD(Initialize)(LPCITEMIDLIST, LPDATAOBJECT pDO, HKEY);

// IContextMenu
	STDMETHOD(QueryContextMenu)(HMENU, UINT, UINT, UINT, UINT);
	STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO);
	STDMETHOD(GetCommandString)(UINT_PTR, UINT, UINT*, LPSTR, UINT);

// IContextMenu2
	STDMETHOD(HandleMenuMsg)(UINT, WPARAM, LPARAM);

// IContextMenu3
	STDMETHOD(HandleMenuMsg2)(UINT, WPARAM, LPARAM, LRESULT*);

// IPersist & IPersistFile
	STDMETHOD(Load)(LPCOLESTR wszFile, DWORD dwMode);
	STDMETHOD(GetClassID)(CLSID* pclsid);
	STDMETHOD(IsDirty)();
	STDMETHOD(Save)(LPCOLESTR, BOOL);
	STDMETHOD(SaveCompleted)(LPCOLESTR);
	STDMETHOD(GetCurFile)(LPOLESTR*);

// IParentAndItem
	//STDMETHOD(SetParentAndItem)( 
	//	/* [unique][in] */ __RPC__in_opt PCIDLIST_ABSOLUTE pidlParent,
	//	/* [unique][in] */ __RPC__in_opt IShellFolder *psf,
	//	/* [in] */ __RPC__in PCUITEMID_CHILD pidlChild);
	//STDMETHOD(GetParentAndItem)( 
	//	/* [out] */ __RPC__deref_out_opt PIDLIST_ABSOLUTE *ppidlParent,
	//	/* [out] */ __RPC__deref_out_opt IShellFolder **ppsf,
	//	/* [out] */ __RPC__deref_out_opt PITEMID_CHILD *ppidlChild);

// IInitializeWithStream
	//STDMETHOD(Initialize)( 
	//	/* [in] */ IStream *pstream,
	//	/* [in] */ DWORD grfMode);

// IInitializeWithItem
	STDMETHOD(Initialize)( 
		/* [in] */ __RPC__in_opt IShellItem *psi,
		/* [in] */ DWORD grfMode);

// IInitializeWithFile
	STDMETHOD(Initialize)(LPCWSTR pszFilePath, DWORD grfMode);

// IThumbnailProvider
	STDMETHOD(GetThumbnail)(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha);

// IPreviewHandler
	//STDMETHOD(SetWindow)( 
	//	/* [in] */ __RPC__in HWND hwnd,
	//	/* [in] */ __RPC__in const RECT *prc);
	//STDMETHOD(SetRect)( 
	//	/* [in] */ __RPC__in const RECT *prc);
	//STDMETHOD(DoPreview)(void);
	//STDMETHOD(Unload)(void);
	//STDMETHOD(SetFocus)(void);
	//STDMETHOD(QueryFocus)( 
	//	/* [out] */ __RPC__deref_out_opt HWND *phwnd);
	//STDMETHOD(TranslateAccelerator)( 
	//	/* [in] */ __RPC__in MSG *pmsg);

// IOleWindow
	//STDMETHOD(GetWindow)( 
	//	/* [out] */ __RPC__deref_out_opt HWND *phwnd);
	//STDMETHOD(ContextSensitiveHelp)( 
	//	/* [in] */ BOOL fEnterMode);

// IExtractImage
	STDMETHOD (GetLocation) (LPWSTR pszPathBuffer, DWORD cch, DWORD *pdwPriority,
		const SIZE *prgSize, DWORD dwRecClrDepth, DWORD *pdwFlags);        
	STDMETHOD (Extract) (HBITMAP *phBmpThumbnail);
 
// IExtractImage2
	STDMETHOD (GetDateStamp) (FILETIME *pDateStamp);

// IRunnableTask
    //STDMETHOD (Run)();
    //STDMETHOD (Kill)(BOOL fWait);
    //STDMETHOD (Suspend)();
    //STDMETHOD (Resume)();
    //STDMETHOD_(ULONG, IsRunning)();

// IQueryInfo
	STDMETHOD(GetInfoFlags)(DWORD*);
	STDMETHOD(GetInfoTip)(DWORD, LPWSTR*);

// IExtractIconA
	STDMETHOD(GetIconLocation)(
		UINT uFlags,
		__out_ecount(cch) LPSTR szIconFile,
		UINT cch,
		__out int* piIndex,
		__out UINT* pwFlags);
	STDMETHOD(Extract)(
		LPCSTR pszFile,
		UINT nIconIndex,
		__out_opt HICON* phiconLarge,
		__out_opt HICON* phiconSmall,
		UINT nIconSize);

// IExtractIconW
	STDMETHOD(GetIconLocation)(
		UINT uFlags,
		__out_ecount(cch) LPWSTR szIconFile,
		UINT cch,
		__out int* piIndex,
		__out UINT* pwFlags);
	STDMETHOD(Extract)(
		LPCWSTR pszFile,
		UINT nIconIndex,
		__out_opt HICON* phiconLarge,
		__out_opt HICON* phiconSmall,
		UINT nIconSize);

// IDataObject
	//STDMETHOD(GetData)( 
	//	/* [unique][in] */ FORMATETC *pformatetcIn,
	//	/* [out] */ STGMEDIUM *pmedium);
	//STDMETHOD(GetDataHere)( 
	//	/* [unique][in] */ FORMATETC *pformatetc,
	//	/* [out][in] */ STGMEDIUM *pmedium);
	//STDMETHOD(QueryGetData)( 
	//	/* [unique][in] */ FORMATETC *pformatetc);
	//STDMETHOD(GetCanonicalFormatEtc)( 
	//	/* [unique][in] */ FORMATETC *pformatectIn,
	//	/* [out] */ FORMATETC *pformatetcOut);
	//STDMETHOD(SetData)( 
	//	/* [unique][in] */ FORMATETC *pformatetc,
	//	/* [unique][in] */ STGMEDIUM *pmedium,
	//	/* [in] */ BOOL fRelease);
	//STDMETHOD(EnumFormatEtc)( 
	//	/* [in] */ DWORD dwDirection,
	//	/* [out] */ IEnumFORMATETC **ppenumFormatEtc);
	//STDMETHOD(DAdvise)( 
	//	/* [in] */ FORMATETC *pformatetc,
	//	/* [in] */ DWORD advf,
	//	/* [unique][in] */ IAdviseSink *pAdvSink,
	//	/* [out] */ DWORD *pdwConnection);
	//STDMETHOD(DUnadvise)( 
	//	/* [in] */ DWORD dwConnection);
	//STDMETHOD(EnumDAdvise)( 
	//	/* [out] */ IEnumSTATDATA **ppenumAdvise);

// IImageDecodeFilter
	//STDMETHOD(Initialize)( 
	//	/* [in] */ IImageDecodeEventSink *pEventSink);	    
	//STDMETHOD(Process)( 
	//	/* [in] */ IStream *pStream);	    
	//STDMETHOD(Terminate)( 
	//	/* [in] */ HRESULT hrStatus);

// IObjectWithSite
	STDMETHOD(SetSite)(IUnknown *pUnkSite);
	STDMETHOD(GetSite)(REFIID riid, void **ppvSite);

// IColumnProvider
	//STDMETHOD(Initialize)(LPCSHCOLUMNINIT psci);
	//STDMETHOD(GetColumnInfo)(DWORD dwIndex, SHCOLUMNINFO *psci);
	//STDMETHOD(GetItemData)(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData);

// IEmptyVolumeCache
	STDMETHOD(Initialize)( 
		/* [in] */ HKEY hkRegKey,
		/* [in] */ LPCWSTR pcwszVolume,
		/* [out] */ LPWSTR *ppwszDisplayName,
		/* [out] */ LPWSTR *ppwszDescription,
		/* [out] */ DWORD *pdwFlags);

	STDMETHOD(GetSpaceUsed)( 
		/* [out] */ __RPC__out DWORDLONG *pdwlSpaceUsed,
		/* [in] */ __RPC__in_opt IEmptyVolumeCacheCallBack *picb);

	STDMETHOD(Purge)( 
		/* [in] */ DWORDLONG dwlSpaceToFree,
		/* [in] */ __RPC__in_opt IEmptyVolumeCacheCallBack *picb);

	STDMETHOD(ShowProperties)( 
		/* [in] */ __RPC__in HWND hwnd);

	STDMETHOD(Deactivate)( 
		/* [out] */ __RPC__out DWORD *pdwFlags);

// IEmptyVolumeCache2
	STDMETHOD(InitializeEx)( 
		/* [in] */ HKEY hkRegKey,
		/* [in] */ LPCWSTR pcwszVolume,
		/* [in] */ LPCWSTR pcwszKeyName,
		/* [out] */ LPWSTR *ppwszDisplayName,
		/* [out] */ LPWSTR *ppwszDescription,
		/* [out] */ LPWSTR *ppwszBtnText,
		/* [out] */ DWORD *pdwFlags);

protected:
	CAtlList< CString >				m_Filenames;		// Имена файлов для меню
	UINT							m_uOurItemID;		// Идентификатор пункта меню
	CEntity							m_Preview;			// Эскиз
	UINT							m_cx, m_cy;			// Нужные размеры эскиза
	CComPtr< IUnknown >				m_pSite;			// Хэндлер хоста IObjectWithSite
//	CComPtr<IImageDecodeEventSink>	m_spEventSink;		// Хэндлер событий IImageDecodeFilter

//	CComPtr< IStream >				m_pStream;			// Стрим файла
	CString							m_sFilename;		// Имя файла
	BOOL							m_bCleanup;		// Флаг пропуска очистки

	void ConvertTo(HWND hWnd, LPCSTR ext);		// Конвертирование в нужное расширение
	void SetWallpaper(HWND hwnd, WORD reason);	// Установка обоев
	void SendByMail(HWND hwnd, WORD reason);	// Посылка по почте
	void CopyToClipboard(HWND hwnd);			// Копирование в буфер обмена

	STDMETHOD(MenuMessageHandler)(UINT, WPARAM, LPARAM, LRESULT*);
	STDMETHOD(OnMeasureItem)(MEASUREITEMSTRUCT*, LRESULT*);
	STDMETHOD(OnDrawItem)(DRAWITEMSTRUCT*, LRESULT*);
};

OBJECT_ENTRY_AUTO(__uuidof(Thumb), CThumb)
