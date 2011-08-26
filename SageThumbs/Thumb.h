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
Shell item load:
	1. Query for IInitializeWithStream (Windows Vista):
		IInitializeWithStream::Initialize
	2. Query for IPersistStream (Windows 7):
		IPersistStream::Load
	3. Query for IInitializeWithItem (Windows Vista):
		IInitializeWithItem::Initialize
	4. Query for IInitializeWithFile (Windows Vista):
		IInitializeWithFile::Initialize
	5. Query for IPersistFile (Windows 2000):
		IPersistFile:Load

Shortcut Menu Handlers:
	1. Query for IClassFactory
	2. Query for IContextMenu
	3. Query for IShellExtInit
	4. IShellExtInit::Initialize
	5. Query for IObjectWithSite
	6. IObjectWithSite::SetSite(pExplorer)
	7. IContextMenu::QueryContextMenu	-> Load info and image
	8. Menu commands:
		IContextMenu::GetCommandString
		Query for IContextMenu3
		IContextMenu3::HandleMenuMsg2 : WM_MEASUREITEM (for ownerdraw items)
		IContextMenu3::HandleMenuMsg2 : WM_DRAWITEM (for ownerdraw items)
		IContextMenu3::HandleMenuMsg2 : WM_INITMENUPOPUP (for submenu)
		IContextMenu::InvokeCommand
	9. Query for IObjectWithSite
	10. IObjectWithSite::SetSite(NULL)
	11. Release

InfoTip Handler:
	1. Query for IClassFactory
	2. Shell item load
	3. Query for ICustomizeInfoTip
	4. Query for IQueryInfo
	5. IQueryInfo::GetInfoTip			-> Load info only
	6. IQueryInfo::GetInfoFlags
	7. Release

Thumbnail Image Handler WinXP:
	1. Query for IClassFactory
	2. Shell item load
	3. Query for IExtractImage
	4. Query for IExtractImage2
	5. IExtractImage2:GetDateStamp
	6. IExtractImage::GetLocation		-> Load info and image
	7. IExtractImage::Extract
	8. Release

Thumbnail Image Handler Vista:
	1. Query for IClassFactory
	2. Shell item load
	3. Query for IExtractImage
	4. IExtractImage::GetLocation		-> Load info and image
	5. Query for IThumbnailProvider
	6. IThumbnailProvider::GetThumbnail
	7. Release

Data Handler:
	1. Query for IClassFactory
	2. Query for IPersistFile
	3. IPersistFile::Load
	4. Release

Icon Handler:
	1. Query for IClassFactory
	2. Shell item load
	3. Query for IExtractIconW
	4. IExtractIconW::GetIconLocation
	5. IExtractIconW::Extract			-> Load info and image
	6. Release

Property Handler (Windows Vista):
	1. Query for IClassFactory
	2. Shell item load
	3. Query for IPropertyStore
	4. Query for INamedPropertyStore
	5. Query for IPropertyStoreCapabilities
	6. IPropertyStore::GetValue			-> Load info
	7. Release
*/

#pragma once

#include "Entity.h"

// Without IDL-file
class DECLSPEC_UUID    ("4A34B3E3-F50E-4FF6-8979-7E4176466FF2") Thumb;
DEFINE_GUID(CLSID_Thumb,0x4A34B3E3,0xF50E,0x4FF6,0x89,0x79,0x7E,0x41,0x76,0x46,0x6F,0xF2);

class ATL_NO_VTABLE CThumb :
	public CComObjectRootEx< CComMultiThreadModel >,
	public CComCoClass< CThumb, &CLSID_Thumb >,
	public IObjectSafetyImpl< CThumb, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA >,
	public IExternalConnectionImpl< CThumb >,
	public IShellExtInit,
	public IContextMenu3,
	public IPersistFile,
//	public IParentAndItem,
#ifdef ISTREAM_ENABLED
	public IInitializeWithStream,
#endif // ISTREAM_ENABLED
	public IInitializeWithItem,
	public IInitializeWithFile,
	public IThumbnailProvider,
	public IPropertyStoreCapabilities,
	public IPropertyStore,
//	public IPropertySetStorage,
//	public IPropertyStorage,
//	public INamedPropertyStore,
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
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CThumb)
		COM_INTERFACE_ENTRY(IObjectSafety)
		COM_INTERFACE_ENTRY(IExternalConnection)
		COM_INTERFACE_ENTRY(IShellExtInit)
		COM_INTERFACE_ENTRY(IContextMenu3)
		COM_INTERFACE_ENTRY(IContextMenu2)
		COM_INTERFACE_ENTRY(IContextMenu)
		COM_INTERFACE_ENTRY(IPersist)
		COM_INTERFACE_ENTRY(IPersistFile)
//		COM_INTERFACE_ENTRY(IParentAndItem)
#ifdef ISTREAM_ENABLED
		COM_INTERFACE_ENTRY(IInitializeWithStream)
#endif // ISTREAM_ENABLED
		COM_INTERFACE_ENTRY(IInitializeWithItem)
		COM_INTERFACE_ENTRY(IInitializeWithFile)
		COM_INTERFACE_ENTRY(IThumbnailProvider)
		COM_INTERFACE_ENTRY(IPropertyStoreCapabilities)
		COM_INTERFACE_ENTRY(IPropertyStore)
//		COM_INTERFACE_ENTRY(IPropertySetStorage)
//		COM_INTERFACE_ENTRY(IPropertyStorage)
//		COM_INTERFACE_ENTRY(INamedPropertyStore)
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
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	END_COM_MAP()

	BEGIN_CATEGORY_MAP(CThumb)
		IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
		IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
	END_CATEGORY_MAP()

	HRESULT FinalConstruct();
	void FinalRelease();
	
	void OnAddConnection(bool /*bThisIsFirstLock*/)
	{
		ATLTRACE( "CTumbs - IExternalConnection::AddConnection()\n" );
	}

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
#ifdef ISTREAM_ENABLED
	STDMETHOD(Initialize)( 
		/* [in] */ IStream *pstream,
		/* [in] */ DWORD grfMode);
#endif // ISTREAM_ENABLED

// IInitializeWithItem
	STDMETHOD(Initialize)( 
		/* [in] */ __RPC__in_opt IShellItem *psi,
		/* [in] */ DWORD grfMode);

// IInitializeWithFile
	STDMETHOD(Initialize)(LPCWSTR pszFilePath, DWORD grfMode);

// IThumbnailProvider
	STDMETHOD(GetThumbnail)(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha);

// IPropertyStoreCapabilities
	STDMETHOD(IsPropertyWritable)( 
		/* [in] */ __RPC__in REFPROPERTYKEY key);

// IPropertyStore
	STDMETHOD(GetCount)( 
		/* [out] */ __RPC__out DWORD *cProps);
	STDMETHOD(GetAt)( 
		/* [in] */ DWORD iProp,
		/* [out] */ __RPC__out PROPERTYKEY *pkey);
	STDMETHOD(GetValue)( 
		/* [in] */ __RPC__in REFPROPERTYKEY key,
		/* [out] */ __RPC__out PROPVARIANT *pv);
	STDMETHOD(SetValue)( 
		/* [in] */ __RPC__in REFPROPERTYKEY key,
		/* [in] */ __RPC__in REFPROPVARIANT propvar);
	STDMETHOD(Commit)(void);

// IPropertySetStorage
	//STDMETHOD(Create)( 
	//	/* [in] */ __RPC__in REFFMTID rfmtid,
	//	/* [unique][in] */ __RPC__in_opt const CLSID *pclsid,
	//	/* [in] */ DWORD grfFlags,
	//	/* [in] */ DWORD grfMode,
	//	/* [out] */ __RPC__deref_out_opt IPropertyStorage **ppprstg);
	//STDMETHOD(Open)( 
	//	/* [in] */ __RPC__in REFFMTID rfmtid,
	//	/* [in] */ DWORD grfMode,
	//	/* [out] */ __RPC__deref_out_opt IPropertyStorage **ppprstg);
	//STDMETHOD(Delete)( 
	//	/* [in] */ __RPC__in REFFMTID rfmtid);
	//STDMETHOD(Enum)( 
	//	/* [out] */ __RPC__deref_out_opt IEnumSTATPROPSETSTG **ppenum);

// IPropertyStorage
	//STDMETHOD(ReadMultiple)( 
	//	/* [in] */ ULONG cpspec,
	//	/* [size_is][in] */ __RPC__in_ecount_full(cpspec) const PROPSPEC rgpspec[  ],
	//	/* [size_is][out] */ __RPC__out_ecount_full(cpspec) PROPVARIANT rgpropvar[  ]);
	//STDMETHOD(WriteMultiple)( 
	//	/* [in] */ ULONG cpspec,
	//	/* [size_is][in] */ __RPC__in_ecount_full(cpspec) const PROPSPEC rgpspec[  ],
	//	/* [size_is][in] */ __RPC__in_ecount_full(cpspec) const PROPVARIANT rgpropvar[  ],
	//	/* [in] */ PROPID propidNameFirst);
	//STDMETHOD(DeleteMultiple)( 
	//	/* [in] */ ULONG cpspec,
	//	/* [size_is][in] */ __RPC__in_ecount_full(cpspec) const PROPSPEC rgpspec[  ]);
	//STDMETHOD(ReadPropertyNames)( 
	//	/* [in] */ ULONG cpropid,
	//	/* [size_is][in] */ __RPC__in_ecount_full(cpropid) const PROPID rgpropid[  ],
	//	/* [size_is][out] */ __RPC__out_ecount_full(cpropid) LPOLESTR rglpwstrName[  ]);
	//STDMETHOD(WritePropertyNames)( 
	//	/* [in] */ ULONG cpropid,
	//	/* [size_is][in] */ __RPC__in_ecount_full(cpropid) const PROPID rgpropid[  ],
	//	/* [size_is][in] */ __RPC__in_ecount_full(cpropid) const LPOLESTR rglpwstrName[  ]);
	//STDMETHOD(DeletePropertyNames)( 
	//	/* [in] */ ULONG cpropid,
	//	/* [size_is][in] */ __RPC__in_ecount_full(cpropid) const PROPID rgpropid[  ]);
	//STDMETHOD(Commit)( 
	//	/* [in] */ DWORD grfCommitFlags);
	//STDMETHOD(Revert)();
	//STDMETHOD(Enum)( 
	//	/* [out] */ __RPC__deref_out_opt IEnumSTATPROPSTG **ppenum);
	//STDMETHOD(SetTimes)( 
	//	/* [in] */ __RPC__in const FILETIME *pctime,
	//	/* [in] */ __RPC__in const FILETIME *patime,
	//	/* [in] */ __RPC__in const FILETIME *pmtime);
	//STDMETHOD(SetClass)( 
	//	/* [in] */ __RPC__in REFCLSID clsid);
	//STDMETHOD(Stat)( 
	//	/* [out] */ __RPC__out STATPROPSETSTG *pstatpsstg);

// INamedPropertyStore
	//STDMETHOD(GetNamedValue)( 
	//	/* [string][in] */ __RPC__in_string LPCWSTR pszName,
	//	/* [out] */ __RPC__out PROPVARIANT *ppropvar);
	//STDMETHOD(SetNamedValue)( 
	//	/* [string][in] */ __RPC__in_string LPCWSTR pszName,
	//	/* [in] */ __RPC__in REFPROPVARIANT propvar);
	//STDMETHOD(GetNameCount)( 
	//	/* [out] */ __RPC__out DWORD *pdwCount);
	//STDMETHOD(GetNameAt)( 
	//	/* [in] */ DWORD iProp,
	//	/* [out] */ __RPC__deref_out_opt BSTR *pbstrName);

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
	CComPtr< IUnknown >				m_pUnkMarshaler;
	CAtlList< CString >				m_Filenames;		// Имена файлов для меню
	UINT							m_uOurItemID;		// Идентификатор пункта меню
	CEntity							m_Preview;			// Эскиз
	UINT							m_cx, m_cy;			// Нужные размеры эскиза
	CComPtr< IUnknown >				m_pSite;			// Хэндлер хоста IObjectWithSite
//	CComPtr< IImageDecodeEventSink >m_pEventSink;		// Хэндлер событий IImageDecodeFilter

#ifdef ISTREAM_ENABLED
	CComPtr< IStream >				m_pStream;			// Стрим файла
#endif ISTREAM_ENABLED

	CString							m_sFilename;		// Full file name 
	BOOL							m_bCleanup;		// Флаг пропуска очистки

	void ConvertTo(HWND hWnd, int ext);			// Конвертирование в нужное расширение
	void SetWallpaper(HWND hwnd, WORD reason);	// Установка обоев
	void SendByMail(HWND hwnd, WORD reason);	// Посылка по почте
	void CopyToClipboard(HWND hwnd);			// Копирование в буфер обмена

	STDMETHOD(MenuMessageHandler)(UINT, WPARAM, LPARAM, LRESULT*);
	STDMETHOD(OnMeasureItem)(MEASUREITEMSTRUCT*, LRESULT*);
	STDMETHOD(OnDrawItem)(DRAWITEMSTRUCT*, LRESULT*);
};

OBJECT_ENTRY_AUTO(__uuidof(Thumb), CThumb)
