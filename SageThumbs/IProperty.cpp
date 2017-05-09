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

#include "stdafx.h"
#include "SageThumbs.h"
#include "Thumb.h"

#ifdef _DEBUG

// Well-known Property Set Format IDs
static const struct
{
	FMTID	fmtid;
	LPCSTR	name;
}
sets[] =
{
	{ FMTID_SummaryInformation,				"FMTID_SummaryInformation" },
	{ FMTID_DocSummaryInformation,			"FMTID_DocSummaryInformation" },
	{ FMTID_UserDefinedProperties,			"FMTID_UserDefinedProperties" },
	{ FMTID_DiscardableInformation,			"FMTID_DiscardableInformation" },
	{ FMTID_ImageSummaryInformation,		"FMTID_ImageSummaryInformation" },			// System.Image.*
	{ FMTID_AudioSummaryInformation,		"FMTID_AudioSummaryInformation" },
	{ FMTID_VideoSummaryInformation,		"FMTID_VideoSummaryInformation" },
	{ FMTID_MediaFileSummaryInformation,	"FMTID_MediaFileSummaryInformation" },
	{ FMTID_ImageProperties,				"FMTID_ImageProperties" },					// System.Photo.*
	{ FMTID_MUSIC,							"FMTID_MUSIC" },
	{ FMTID_DRM,							"FMTID_DRM" },
	{ FMTID_RecordedTVSummaryInformation,	"FMTID_RecordedTVSummaryInformation" }
};

CStringA FormatName(REFFMTID fmtid)
{
	for ( int i = 0; i < _countof( sets ); ++i )
	{
		if ( IsEqualIID( fmtid, sets[ i ].fmtid ) )
		{
			return sets[ i ].name;
		}
	}
	
	// {...}
	LPOLESTR szGUID = NULL;
	if ( FAILED( StringFromIID( fmtid, &szGUID ) ) )
		return CStringA();
	CStringA name( (LPCSTR)CW2A( szGUID ) );
	CoTaskMemFree( szGUID );
	return name;
}

CStringA PropName(REFPROPERTYKEY key)
{
	CStringA sPropName;
	sPropName.Format( "%u", key.pid );
	/*CComPtr< IPropertyDescription > pDesc;
	if ( SUCCEEDED( PSGetPropertyDescription( key, IID_PPV_ARGS( &pDesc ) ) ) )
	{
		LPWSTR szPropName = NULL;
		if ( SUCCEEDED( pDesc->GetCanonicalName( &szPropName ) ) )
		{
			sPropName.Format( "%s(%d)", (LPCSTR)CW2A( szPropName ), key.pid );
			CoTaskMemFree( szPropName );
		}
		pDesc.Release();
	}*/
	return FormatName( key.fmtid ) + ":" + sPropName;
}

#else // _DEBUG

#define FormatName __noop
#define PropName __noop

#endif // _DEBUG

// IPropertyStoreCapabilities

STDMETHODIMP CThumb::IsPropertyWritable(
	/* [in] */ __RPC__in REFPROPERTYKEY key)
{
	key;
	ATLTRACE( "CThumb - IPropertyStoreCapabilities::IsPropertyWritable(\"%s\") : S_FALSE\n", (LPCSTR)PropName( key ) );
	return S_FALSE;
}

// IPropertyStore

static const struct 
{
	PROPERTYKEY key;
	VARTYPE		vt;
} props[] =
{
	{ PKEY_ItemTypeText,				VT_BSTR	},
	{ PKEY_FileDescription,				VT_BSTR	},
	{ PKEY_DRM_IsProtected,				VT_BOOL	},
	{ PKEY_Image_Dimensions,			VT_BSTR	},
	{ PKEY_Image_HorizontalSize,		VT_UI4	},
	{ PKEY_Image_VerticalSize,			VT_UI4	},
	{ PKEY_Image_ResolutionUnit,		VT_I2	},
	{ PKEY_Image_HorizontalResolution,	VT_R8	},
	{ PKEY_Image_VerticalResolution,	VT_R8	},
	{ PKEY_Image_BitDepth,				VT_UI4	},
	{ PKEY_Image_Compression,			VT_UI2	},
	{ PKEY_Image_CompressionText,		VT_BSTR	},
	{ PKEY_PerceivedType,				VT_I4	}
//	{ PKEY_Kind,						VT_ARRAY | VT_BSTR	}
};

VARTYPE GetPropType(REFPROPERTYKEY key)
{
	for ( int i = 0; i < _countof( props ); ++i )
	{
		if ( IsEqualPropertyKey( key, props[ i ].key ) )
		{
			return props[ i ].vt;
		}
	}
	return VT_EMPTY;
}

STDMETHODIMP CThumb::GetCount(
	/* [out] */ __RPC__out DWORD* cProps)
{
	if ( ! cProps )
	{
		ATLTRACE( "CThumb - IPropertyStore::GetCount() : E_POINTER\n" );
		return E_POINTER;
	}

	*cProps = _countof( props );

	ATLTRACE( "CThumb - IPropertyStore::GetCount() : S_OK (%u)\n", *cProps );
	return S_OK;
}

STDMETHODIMP CThumb::GetAt(
	/* [in] */ DWORD iProp,
	/* [out] */ __RPC__out PROPERTYKEY* pkey)
{
	if ( ! pkey )
	{
		ATLTRACE( "CThumb - IPropertyStore::GetAt(%u) : E_POINTER\n", iProp );
		return E_POINTER;
	}

	if ( iProp >= _countof( props ) )
	{
		ATLTRACE( "CThumb - IPropertyStore::GetAt(%u) : E_INVALIDARG\n", iProp );
		return E_INVALIDARG;
	}

	*pkey = props[ iProp ].key;

	ATLTRACE( "CThumb - IPropertyStore::GetAt(%u) : S_OK\n", iProp );
	return S_OK;
}

STDMETHODIMP CThumb::GetValue(
	/* [in] */ __RPC__in REFPROPERTYKEY key,
	/* [out] */ __RPC__out PROPVARIANT* pv)
{
	if ( ! pv )
	{
		ATLTRACE( "CThumb - IPropertyStore::GetValue() : E_POINTER\n" );
		return E_POINTER;
	}

	PropVariantInit( pv );

	if ( ! m_sFilename.IsEmpty() )
	{
		m_Preview.LoadInfo( m_sFilename );
	}
#ifdef ISTREAM_ENABLED
	else if ( m_pStream )
	{
		m_Preview.LoadInfo( m_pStream );
	}
#endif // ISTREAM_ENABLED

	if ( ! m_Preview.IsInfoAvailable() )
	{
		ATLTRACE( "CThumb - IPropertyStore::GetValue(\"%s\") : S_OK (no info)\n", (LPCSTR)PropName( key ) );
		return S_OK;
	}

	pv->vt = GetPropType( key );

	if ( IsEqualPropertyKey( key, PKEY_ItemTypeText ) ||
		 IsEqualPropertyKey( key, PKEY_FileDescription ) )
	{
		pv->bstrVal = m_Preview.ImageDescription().AllocSysString();
	}
	else if ( IsEqualPropertyKey( key, PKEY_DRM_IsProtected ) )
	{
		pv->boolVal = VARIANT_FALSE;
	}
	else if ( IsEqualPropertyKey( key, PKEY_Image_Dimensions ) )
	{
		CString sDimensions;
		sDimensions.Format( _T("%u x %u"), m_Preview.ImageWidth(), m_Preview.ImageHeight() );
		pv->bstrVal = sDimensions.AllocSysString();
	}
	else if ( IsEqualPropertyKey( key, PKEY_Image_HorizontalSize ) )
	{
		pv->ulVal = m_Preview.ImageWidth();
	}
	else if ( IsEqualPropertyKey( key, PKEY_Image_VerticalSize ) )
	{
		pv->ulVal = m_Preview.ImageHeight();
	}
	else if ( IsEqualPropertyKey( key, PKEY_Image_ResolutionUnit ) )
	{
		pv->iVal = ( m_Preview.ImageYdpi() || m_Preview.ImageXdpi() ) ? 2 : 1;	// inches or no units
	}
	else if ( IsEqualPropertyKey( key, PKEY_Image_HorizontalResolution ) )
	{
		if ( m_Preview.ImageXdpi() )
		{
			pv->dblVal = (double)m_Preview.ImageXdpi();
		}
		else
			pv->vt = VT_EMPTY;
	}
	else if ( IsEqualPropertyKey( key, PKEY_Image_VerticalResolution ) )
	{
		if ( m_Preview.ImageYdpi() || m_Preview.ImageXdpi() )
		{
			pv->dblVal = (double) ( m_Preview.ImageYdpi() ? m_Preview.ImageYdpi() : m_Preview.ImageXdpi() );
		}
		else
			pv->vt = VT_EMPTY;
	}
	else if ( IsEqualPropertyKey( key, PKEY_Image_BitDepth ) )
	{
		pv->ulVal = m_Preview.ImageBitDepth();
	}
	else if ( IsEqualPropertyKey( key, PKEY_Image_Compression ) )
	{
		switch ( m_Preview.ImageCompression() )
		{
		case GFL_NO_COMPRESSION:
			pv->uiVal = IMAGE_COMPRESSION_UNCOMPRESSED;
			break;
		case GFL_LZW:
		case GFL_LZW_PREDICTOR:
			pv->uiVal = IMAGE_COMPRESSION_LZW;
			break;
		case GFL_JPEG:
			pv->uiVal = IMAGE_COMPRESSION_JPEG;
			break;
		case GFL_CCITT_FAX3:
		case GFL_CCITT_FAX3_2D:
			pv->uiVal = IMAGE_COMPRESSION_CCITT_T3;
			break;
		case GFL_CCITT_FAX4:
			pv->uiVal = IMAGE_COMPRESSION_CCITT_T4;
			break;
		case GFL_ZIP:
		case GFL_RLE:
		case GFL_SGI_RLE:
		case GFL_CCITT_RLE:
		case GFL_WAVELET:
		default:
			pv->uiVal = IMAGE_COMPRESSION_PACKBITS;
		}
	}
	else if ( IsEqualPropertyKey( key, PKEY_Image_CompressionText ) )
	{
		pv->bstrVal = m_Preview.ImageCompressionDescription().AllocSysString();
	}
	else if ( IsEqualPropertyKey( key, PKEY_PerceivedType ) )
	{
		pv->lVal = PERCEIVED_TYPE_IMAGE;
	}
	//else if ( IsEqualPropertyKey( key, PKEY_Kind ) )
	//{
	//	pv->vt = VT_ARRAY | VT_BSTR;
	//	pv-> = CString( KIND_PICTURE ).AllocSysString();
	//}

	ATLTRACE( "CThumb - IPropertyStore::GetValue(\"%s\") : S_OK%s\n", (LPCSTR)PropName( key ), ( ( pv->vt == VT_EMPTY) ? " (empty)" : "" ) );
	return S_OK;
}

STDMETHODIMP CThumb::SetValue(
	/* [in] */ __RPC__in REFPROPERTYKEY key,
	/* [in] */ __RPC__in REFPROPVARIANT /* propvar */)
{
	key;
	ATLTRACE( "CThumb - IPropertyStore::SetValue(\"%s\") : E_INVALIDARG\n", (LPCSTR)PropName(key) );
	return E_INVALIDARG;
}

STDMETHODIMP CThumb::Commit()
{
	ATLTRACE( "CThumb - IPropertyStore::Commit() : E_FAIL\n" );

	return E_FAIL;
}

// IPropertySetStorage

STDMETHODIMP CThumb::Create(
	/* [in] */ __RPC__in REFFMTID rfmtid,
	/* [unique][in] */ __RPC__in_opt const CLSID* /* pclsid */,
	/* [in] */ DWORD /* grfFlags */,
	/* [in] */ DWORD /* grfMode */,
	/* [out] */ __RPC__deref_out_opt IPropertyStorage** /* ppprstg */)
{
	rfmtid;

	ATLTRACE( "CThumb - IPropertySetStorage::Create(\"%s\") : STG_E_ACCESSDENIED\n", (LPCSTR)FormatName( rfmtid ) );
	return STG_E_ACCESSDENIED;
}

STDMETHODIMP CThumb::Open(
	/* [in] */ __RPC__in REFFMTID rfmtid,
	/* [in] */ DWORD grfMode,
	/* [out] */ __RPC__deref_out_opt IPropertyStorage** ppprstg)
{
	grfMode;

	if ( ! ppprstg )
	{
		ATLTRACE( "CThumb - IPropertySetStorage::Open() : E_POINTER\n" );
		return E_POINTER;
	}
	*ppprstg = NULL;

	if ( IsEqualIID( rfmtid, FMTID_ImageSummaryInformation ) )
	{
		ATLTRACE( "CThumb - IPropertySetStorage::Open(\"%s\",0x%08x) : S_OK\n", (LPCSTR)FormatName( rfmtid ), grfMode );
		return QueryInterface( IID_IPropertyStorage, (void**)ppprstg );
	}

	ATLTRACE( "CThumb - IPropertySetStorage::Open(\"%s\",0x%08x) : STG_E_FILENOTFOUND\n", (LPCSTR)FormatName( rfmtid ), grfMode );
	return STG_E_FILENOTFOUND;
}

STDMETHODIMP CThumb::Delete(
	/* [in] */ __RPC__in REFFMTID rfmtid)
{
	rfmtid;

	ATLTRACE( "CThumb - IPropertySetStorage::Delete(\"%s\") : STG_E_ACCESSDENIED\n", (LPCSTR)FormatName( rfmtid ) );
	return STG_E_ACCESSDENIED;
}

typedef std::list< STATPROPSETSTG > STATPROPSETSTGList;
class __declspec(novtable) STATPROPSETSTGCopy
{
public:
	static inline void init(STATPROPSETSTG* p) throw()
	{
		ATLTRACE( "CThumb - STATPROPSETSTGCopy::init()\n" );
		ZeroMemory( p, sizeof( STATPROPSETSTG ) );
	}
	static inline HRESULT copy(STATPROPSETSTG* pTo, const STATPROPSETSTG* pFrom) throw()
	{
		ATLTRACE( "CThumb - STATPROPSETSTGCopy::copy()\n" );
		CopyMemory( pTo, pFrom, sizeof( STATPROPSETSTG ) );
		return S_OK;
	}
	static inline void destroy(STATPROPSETSTG* p) throw()
	{
		ATLTRACE( "CThumb - STATPROPSETSTGCopy::destroy()\n" );
		p;
	}
};
typedef CComEnumOnSTLExt < IEnumSTATPROPSETSTG, &IID_IEnumSTATPROPSETSTG, STATPROPSETSTG, STATPROPSETSTGCopy, STATPROPSETSTGList > STATPROPSETSTGEnum;
typedef CComObject < STATPROPSETSTGEnum > CEnumSTATPROPSETSTG;

STDMETHODIMP CThumb::Enum(
	/* [out] */ __RPC__deref_out_opt IEnumSTATPROPSETSTG** ppenum)
{
	if ( ! ppenum )
	{
		ATLTRACE( "CThumb - IPropertySetStorage::Enum() : E_POINTER\n" );
		return E_POINTER;
	}

	ATLTRACE( "CThumb - IPropertySetStorage::Enum()\n" );

	static const STATPROPSETSTG sps = { FMTID_ImageSummaryInformation };
	STATPROPSETSTGList lst;
	lst.push_back( sps );

	CEnumSTATPROPSETSTG* pEnum = NULL;
	CEnumSTATPROPSETSTG::CreateInstance( &pEnum );
	pEnum->Init( NULL, lst );
	return pEnum->QueryInterface( IID_IEnumSTATPROPSETSTG, (void**)ppenum );
}

// IPropertyStorage

STDMETHODIMP CThumb::ReadMultiple(
	/* [in] */ ULONG cpspec,
	/* [size_is][in] */ __RPC__in_ecount_full(cpspec) const PROPSPEC rgpspec[ ],
	/* [size_is][out] */ __RPC__out_ecount_full(cpspec) PROPVARIANT rgpropvar[])
{
	if ( ! rgpspec || ! rgpropvar )
	{
		ATLTRACE( "CThumb - IPropertyStorage::ReadMultiple() : E_POINTER\n" );
		return E_POINTER;
	}

	HRESULT hr = S_OK;
	for ( ULONG i = 0; i < cpspec; ++i )
	{
		if ( rgpspec[ i ].ulKind == PRSPEC_PROPID )
		{
			
			PROPERTYKEY key = { PKEY_Image_Dimensions.fmtid, rgpspec[ i ].propid };
			if ( SUCCEEDED( GetValue( key, &rgpropvar[ i ] ) ) && ( rgpropvar[ i ].vt != VT_EMPTY ) )
			{
				// Ok
				ATLTRACE( "CThumb - IPropertyStorage::ReadMultiple() : #%u(%u). ID=%d : S_OK\n", i, cpspec, rgpspec[ i ].propid );
			}
			else
			{
				ATLTRACE( "CThumb - IPropertyStorage::ReadMultiple() : #%u(%u). ID=%d : ERROR_NOT_SUPPORTED\n", i, cpspec, rgpspec[ i ].propid );
				hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
			}
		}
		else
		{
			ATLTRACE( "CThumb - IPropertyStorage::ReadMultiple() : #%u(%u). STR=\"%s\" : STG_E_INVALIDPARAMETER\n", i, cpspec, (LPCSTR)CW2A( rgpspec[ i ].lpwstr ) );
			hr = STG_E_INVALIDPARAMETER;
		}
	}

	return hr;
}

STDMETHODIMP CThumb::WriteMultiple(
	/* [in] */ ULONG cpspec,
	/* [size_is][in] */ __RPC__in_ecount_full(cpspec) const PROPSPEC /* rgpspec */ [ ],
	/* [size_is][in] */ __RPC__in_ecount_full(cpspec) const PROPVARIANT /* rgpropvar */ [ ],
	/* [in] */ PROPID /* propidNameFirst */)
{
	cpspec;
	ATLTRACENOTIMPL( _T("CThumb - IPropertyStorage::WriteMultiple()") );
}

STDMETHODIMP CThumb::DeleteMultiple(
	/* [in] */ ULONG cpspec,
	/* [size_is][in] */ __RPC__in_ecount_full(cpspec) const PROPSPEC /* rgpspec */ [])
{
	cpspec;
	ATLTRACENOTIMPL( _T("CThumb - IPropertyStorage::DeleteMultiple()") );
}

STDMETHODIMP CThumb::ReadPropertyNames(
	/* [in] */ ULONG cpropid,
	/* [size_is][in] */ __RPC__in_ecount_full(cpropid) const PROPID /* rgpropid */ [],
	/* [size_is][out] */ __RPC__out_ecount_full(cpropid) LPOLESTR /* rglpwstrName */ [])
{
	cpropid;
	ATLTRACENOTIMPL( _T("CThumb - IPropertyStorage::ReadPropertyNames()") );
}

STDMETHODIMP CThumb::WritePropertyNames(
	/* [in] */ ULONG cpropid,
	/* [size_is][in] */ __RPC__in_ecount_full(cpropid) const PROPID /* rgpropid */ [],
	/* [size_is][in] */ __RPC__in_ecount_full(cpropid) const LPOLESTR /* rglpwstrName */ [])
{
	cpropid;
	ATLTRACENOTIMPL( _T("CThumb - IPropertyStorage::WritePropertyNames()") );
}

STDMETHODIMP CThumb::DeletePropertyNames(
	/* [in] */ ULONG cpropid,
	/* [size_is][in] */ __RPC__in_ecount_full(cpropid) const PROPID /* rgpropid */ [])
{
	cpropid;
	ATLTRACENOTIMPL( _T("CThumb - IPropertyStorage::DeletePropertyNames()") );
}

STDMETHODIMP CThumb::Commit(
	/* [in] */ DWORD /* grfCommitFlags */)
{
	ATLTRACENOTIMPL( _T("CThumb - IPropertyStorage::Commit()") );
}

STDMETHODIMP CThumb::Revert()
{
	ATLTRACENOTIMPL( _T("CThumb - IPropertyStorage::Revert()") );
}

typedef std::list< STATPROPSTG > STATPROPSTGList;
class __declspec(novtable) STATPROPSTGCopy
{
public:
	static inline void init(STATPROPSTG* p) throw()
	{
		ATLTRACE( "CThumb - STATPROPSTGCopy::init()\n" );
		ZeroMemory( p, sizeof( STATPROPSTG ) );
	}
	static inline HRESULT copy(STATPROPSTG* pTo, const STATPROPSTG* pFrom) throw()
	{
		ATLTRACE( "CThumb - STATPROPSTGCopy::copy()\n" );
		CopyMemory( pTo, pFrom, sizeof( STATPROPSTG ) );
		return S_OK;
	}
	static inline void destroy(STATPROPSTG* p) throw()
	{
		ATLTRACE( "CThumb - STATPROPSTGCopy::destroy()\n" );
		p;
	}
};
typedef CComEnumOnSTLExt < IEnumSTATPROPSTG, &IID_IEnumSTATPROPSTG, STATPROPSTG, STATPROPSTGCopy, STATPROPSTGList > STATPROPSTGEnum;
typedef CComObject < STATPROPSTGEnum > CEnumSTATPROPSTG;

STDMETHODIMP CThumb::Enum(
	/* [out] */ __RPC__deref_out_opt IEnumSTATPROPSTG** ppenum)
{
	if ( ! ppenum )
	{
		ATLTRACE( "CThumb - IPropertyStorage::Enum() : E_POINTER\n" );
		return E_POINTER;
	}

	ATLTRACE( "CThumb - IPropertyStorage::Enum()\n" );

	STATPROPSTGList lst;
	for ( int i = 0; i < _countof( props ); ++i )
	{
		if ( IsEqualIID( props[ i ].key.fmtid, FMTID_ImageSummaryInformation ) )
		{
			STATPROPSTG stats = { NULL, props[ i ].key.pid, props[ i ].vt };
			lst.push_back( stats );
		}
	}

	CEnumSTATPROPSTG* pEnum = NULL;
	CEnumSTATPROPSTG::CreateInstance( &pEnum );
	pEnum->Init( NULL, lst );
	return pEnum->QueryInterface( IID_IEnumSTATPROPSTG, (void**)ppenum );
}

STDMETHODIMP CThumb::SetTimes(
	/* [in] */ __RPC__in const FILETIME* /* pctime */,
	/* [in] */ __RPC__in const FILETIME* /* patime */,
	/* [in] */ __RPC__in const FILETIME* /* pmtime */)
{
	ATLTRACENOTIMPL( _T("CThumb - IPropertyStorage::SetTimes()") );
}

STDMETHODIMP CThumb::SetClass(
	/* [in] */ __RPC__in REFCLSID /* clsid */)
{
	ATLTRACENOTIMPL( _T("CThumb - IPropertyStorage::SetClass()") );
}

STDMETHODIMP CThumb::Stat(
	/* [out] */ __RPC__out STATPROPSETSTG* /* pstatpsstg */)
{
	ATLTRACENOTIMPL( _T("CThumb - IPropertyStorage::Stat()") );
}

// INamedPropertyStore

//STDMETHODIMP CThumb::GetNamedValue(
//	/* [string][in] */ __RPC__in_string LPCWSTR pszName,
//	/* [out] */ __RPC__out PROPVARIANT* ppropvar)
//{
//	if ( ! ppropvar )
//	{
//		ATLTRACE( "CThumb - IPropertyStore::GetValue() : E_POINTER\n" );
//		return E_POINTER;
//	}
//	PropVariantInit( ppropvar );
//
//	ATLTRACE( "CThumb - INamedPropertyStore::GetNamedValue(\"%s\") : S_OK\n", (LPCSTR)CW2A( pszName ) );
//	return S_OK;
//}
//
//STDMETHODIMP CThumb::SetNamedValue(
//	/* [string][in] */ __RPC__in_string LPCWSTR pszName,
//	 /* [in] */ __RPC__in REFPROPVARIANT /* propvar */)
//{
//	ATLTRACE( "CThumb - INamedPropertyStore::SetNamedValue(\"%s\") : STG_E_ACCESSDENIED\n", (LPCSTR)CW2A( pszName ) );
//	return STG_E_ACCESSDENIED;
//}
//
//STDMETHODIMP CThumb::GetNameCount(
//	/* [out] */ __RPC__out DWORD* pdwCount)
//{
//	if ( ! pdwCount )
//	{
//		ATLTRACE( "CThumb - INamedPropertyStore::GetNameCount() : E_POINTER\n" );
//		return E_POINTER;
//	}
//	*pdwCount = 0;
//
//	ATLTRACE( "CThumb - INamedPropertyStore::GetNameCount : S_OK (%u)\n", *pdwCount );
//	return S_OK;
//}
//
//STDMETHODIMP CThumb::GetNameAt(
//	/* [in] */ DWORD /* iProp */,
//	/* [out] */ __RPC__deref_out_opt BSTR* /* pbstrName */)
//{
//	ATLTRACE( "CThumb - INamedPropertyStore::GetNameAt() : STG_E_ACCESSDENIED\n" );
//	return E_NOTIMPL;
//}
