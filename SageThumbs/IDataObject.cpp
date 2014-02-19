/*
SageThumbs - Thumbnail image shell extension.

Copyright (C) Nikolay Raspopov, 2004-2014.

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

// IDataObject

//#define PRINT_FORMAT1(fmt) if(pformatetcIn->cfFormat==RegisterClipboardFormat(fmt)) \
//	{ ATLTRACE ("%s\n", #fmt); } else
//#define PRINT_FORMAT2(fmt) if(pformatetcIn->cfFormat==(fmt)) \
//	{ ATLTRACE ("%s\n", #fmt); } else
//#define PRINT_FORMAT_END \
//	{ ATLTRACE ("no CF_\n"); }
//#define PRINT_FORMAT_ALL \
//	{ \
//		TCHAR fm [128]; fm [0] = _T('\0');\
//		GetClipboardFormatName (pformatetcIn->cfFormat, fm, sizeof (fm)); \
//		ATLTRACE ("0x%08x \"%s\" ", pformatetcIn->cfFormat, fm);\
//	} \
//	PRINT_FORMAT1(CFSTR_SHELLIDLIST) \
//	PRINT_FORMAT1(CFSTR_SHELLIDLISTOFFSET) \
//	PRINT_FORMAT1(CFSTR_NETRESOURCES) \
//	PRINT_FORMAT1(CFSTR_FILEDESCRIPTORA) \
//	PRINT_FORMAT1(CFSTR_FILEDESCRIPTORW) \
//	PRINT_FORMAT1(CFSTR_FILECONTENTS) \
//	PRINT_FORMAT1(CFSTR_FILENAMEA) \
//	PRINT_FORMAT1(CFSTR_FILENAMEW) \
//	PRINT_FORMAT1(CFSTR_PRINTERGROUP) \
//	PRINT_FORMAT1(CFSTR_FILENAMEMAPA) \
//	PRINT_FORMAT1(CFSTR_FILENAMEMAPW) \
//	PRINT_FORMAT1(CFSTR_SHELLURL) \
//	PRINT_FORMAT1(CFSTR_INETURLA) \
//	PRINT_FORMAT1(CFSTR_INETURLW) \
//	PRINT_FORMAT1(CFSTR_PREFERREDDROPEFFECT) \
//	PRINT_FORMAT1(CFSTR_PERFORMEDDROPEFFECT) \
//	PRINT_FORMAT1(CFSTR_PASTESUCCEEDED) \
//	PRINT_FORMAT1(CFSTR_INDRAGLOOP) \
//	PRINT_FORMAT1(CFSTR_DRAGCONTEXT) \
//	PRINT_FORMAT1(CFSTR_MOUNTEDVOLUME) \
//	PRINT_FORMAT1(CFSTR_PERSISTEDDATAOBJECT) \
//	PRINT_FORMAT1(CFSTR_TARGETCLSID) \
//	PRINT_FORMAT1(CFSTR_LOGICALPERFORMEDDROPEFFECT) \
//	PRINT_FORMAT1(CFSTR_AUTOPLAY_SHELLIDLISTS) \
//	PRINT_FORMAT1(CF_RTF) \
//	PRINT_FORMAT1(CF_RTFNOOBJS) \
//	PRINT_FORMAT1(CF_RETEXTOBJ) \
//	PRINT_FORMAT2(CF_TEXT) \
//	PRINT_FORMAT2(CF_BITMAP) \
//	PRINT_FORMAT2(CF_METAFILEPICT) \
//	PRINT_FORMAT2(CF_SYLK) \
//	PRINT_FORMAT2(CF_DIF) \
//	PRINT_FORMAT2(CF_TIFF) \
//	PRINT_FORMAT2(CF_OEMTEXT) \
//	PRINT_FORMAT2(CF_DIB) \
//	PRINT_FORMAT2(CF_PALETTE) \
//	PRINT_FORMAT2(CF_PENDATA) \
//	PRINT_FORMAT2(CF_RIFF) \
//	PRINT_FORMAT2(CF_WAVE) \
//	PRINT_FORMAT2(CF_UNICODETEXT) \
//	PRINT_FORMAT2(CF_ENHMETAFILE) \
//	PRINT_FORMAT2(CF_HDROP) \
//	PRINT_FORMAT2(CF_LOCALE) \
//	PRINT_FORMAT2(CF_DIBV5) \
//	PRINT_FORMAT2(CF_MAX) \
//	PRINT_FORMAT2(CF_OWNERDISPLAY) \
//	PRINT_FORMAT2(CF_DSPTEXT) \
//	PRINT_FORMAT2(CF_DSPBITMAP) \
//	PRINT_FORMAT2(CF_DSPMETAFILEPICT) \
//	PRINT_FORMAT2(CF_DSPENHMETAFILE) \
//	PRINT_FORMAT2(CF_PRIVATEFIRST) \
//	PRINT_FORMAT2(CF_PRIVATELAST) \
//	PRINT_FORMAT2(CF_GDIOBJFIRST) \
//	PRINT_FORMAT2(CF_GDIOBJLAST) \
//	PRINT_FORMAT_END
//
//STDMETHODIMP CThumb::GetData (
//	/* [unique][in] */ FORMATETC* pformatetcIn,
//	/* [out] */ STGMEDIUM* /*pmedium*/)
//{
//	ATLTRACE ("IDataObject::GetData() : ");
//	PRINT_FORMAT_ALL
//	return E_INVALIDARG;
//	/*startActualLoad ();
//	stopActualLoad ();
//	if (m_Status == LS_LOADED) {
//	pmedium->tymed = TYMED_HGLOBAL;
//	pmedium->hGlobal = GlobalAlloc (GHND, sizeof (CLSID));
//	pmedium->pUnkForRelease = NULL;
//	char* dst = (char*) GlobalLock (pmedium->hGlobal);
//	CLSID clsid = { 0x4A34B3E3,0xF50E,0x4FF6,0x89,0x79,0x7E,0x41,0x76,0x46,0x6F,0xF2 };
//	CopyMemory (dst, &clsid, sizeof (CLSID));
//	GlobalUnlock (pmedium->hGlobal);
//	return S_OK;
//	}
//	return E_FAIL;*/
//}
//
//STDMETHODIMP CThumb::GetDataHere (
//	/* [unique][in] */ FORMATETC* /* pformatetc */,
//	/* [out][in] */ STGMEDIUM* /* pmedium */)
//{
//	ATLTRACENOTIMPL ("IDataObject::GetDataHere");
//}
//
//STDMETHODIMP CThumb::QueryGetData (
//	/* [unique][in] */ FORMATETC* pformatetcIn)
//{
//	ATLTRACE ("IDataObject::QueryGetData() : ");
//	PRINT_FORMAT_ALL
//	return E_INVALIDARG;
//}
//
//STDMETHODIMP CThumb::GetCanonicalFormatEtc (
//	/* [unique][in] */ FORMATETC* /* pformatectIn */,
//	/* [out] */ FORMATETC* /* pformatetcOut */)
//{
//	ATLTRACENOTIMPL ("IDataObject::GetCanonicalFormatEtc");
//}
//
//STDMETHODIMP CThumb::SetData (
//	/* [unique][in] */ FORMATETC* pformatetcIn,
//	/* [unique][in] */ STGMEDIUM* /*pmedium*/,
//	/* [in] */ BOOL fRelease)
//{
//	ATLTRACE ("IDataObject::SetData(fRelease=%d) : ", fRelease);
//	PRINT_FORMAT_ALL
//	/*FILEDESCRIPTOR* src = (FILEDESCRIPTOR*) GlobalLock (pmedium->hGlobal);
//	SIZE_T len = GlobalSize (pmedium->hGlobal);
//	GlobalUnlock (pmedium->hGlobal);
//	if (fRelease)
//		GlobalFree (pmedium->hGlobal);*/
//	return E_NOTIMPL;
//}
//
//static FORMATETC fmt [1] = {
//	{ CF_BITMAP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL }
//};
//
//class FORMATETCCopy
//{
//public:
//	static void init (FORMATETC* /*p*/)
//	{
//	}
//	static HRESULT copy (FORMATETC* pTo, const FORMATETC* pFrom)
//	{
//		CopyMemory (pTo, pFrom, sizeof (FORMATETC));
//		return S_OK;
//	}
//	static void destroy (FORMATETC* /*p*/)
//	{
//	}
//};
//
//STDMETHODIMP CThumb::EnumFormatEtc (
//	/* [in] */ DWORD dwDirection,
//	/* [out] */ IEnumFORMATETC** ppenumFormatEtc)
//{
//	ATLTRACE ("IDataObject::EnumFormatEtc(dwDirection=%d) : ", dwDirection);
//	if (!ppenumFormatEtc) {
//		ATLTRACE("E_POINTER\n");
//		return E_POINTER;
//	}
//	// Создание объекта-перечислителя
//	typedef CComEnum < IEnumFORMATETC, &IID_IEnumFORMATETC,
//		FORMATETC, FORMATETCCopy > EnumFORMATETCType;
//	typedef CComObject < EnumFORMATETCType > EnumFORMATETC;
//	EnumFORMATETC* pEnum = NULL;
//	EnumFORMATETC::CreateInstance (&pEnum);
//	pEnum->Init ((FORMATETC*) (&fmt[0]), (FORMATETC*) (&fmt[1]), NULL);
//	ATLTRACE("S_OK\n");
//	return pEnum->QueryInterface (IID_IEnumFORMATETC, (void**) ppenumFormatEtc);
//}
//
//STDMETHODIMP CThumb::DAdvise (
//	/* [in] */ FORMATETC* /* pformatetc */,
//	/* [in] */ DWORD /* advf */,
//	/* [unique][in] */ IAdviseSink* /* pAdvSink */,
//	/* [out] */ DWORD* /* pdwConnection */)
//{
//	ATLTRACENOTIMPL ("IDataObject::DAdvise");
//}
//
//STDMETHODIMP CThumb::DUnadvise (
//	/* [in] */ DWORD /* dwConnection */)
//{
//	ATLTRACENOTIMPL ("IDataObject::DUnadvise");
//}
//
//STDMETHODIMP CThumb::EnumDAdvise (
//	/* [out] */ IEnumSTATDATA** /* ppenumAdvise */)
//{
//	ATLTRACENOTIMPL ("IDataObject::EnumDAdvise");
//}

// IImageDecodeFilter

//STDMETHODIMP CThumb::Initialize(IImageDecodeEventSink* pEventSink)
//{
//	if ( ! pEventSink )
//	{
//		ATLTRACE( "CThumb - IImageDecodeFilter::Initialize() : E_POINTER\n" );
//		return E_POINTER;
//	}
//
//	m_pEventSink = pEventSink;
//
//	DWORD dwEvents = 0;
//	ULONG nFormats = 0;
//    BFID *pFormats = NULL;
//	HRESULT hr = m_pEventSink->OnBeginDecode( &dwEvents, &nFormats, &pFormats );
//	if (FAILED (hr))
//	{
//		ATLTRACE( "CThumb - IImageDecodeFilter : OnBeginDecode error 0x%08x\n", hr);
//		m_pEventSink.Release();
//		return hr;
//	}
//	ATLTRACE( "CThumb - IImageDecodeFilter : OnBeginDecode returns: events=0x%08x, formats=%d\n", dwEvents, nFormats);
//	ULONG i = 0;
//	bool bOk = false;
//	for ( ; i < nFormats; ++i )
//	{
//		if ( IsEqualGUID( BFID_MONOCHROME, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_MONOCHROME\n" );
//		}
//		else if ( IsEqualGUID( BFID_RGB_4, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGB_4\n" );
//		}
//		else if ( IsEqualGUID( BFID_RGB_8, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGB_8\n" );
//		}
//		else if ( IsEqualGUID( BFID_RGB_555, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGB_555\n" );
//		}
//		else if ( IsEqualGUID( BFID_RGB_565, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGB_565\n" );
//		}
//		else if ( IsEqualGUID( BFID_RGB_24, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGB_24\n" );
//			bOk = true;
//		}
//		else if ( IsEqualGUID( BFID_RGB_32, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGB_32\n" );
//		}
//		else if ( IsEqualGUID( BFID_RGBA_32, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_RGBA_32\n" );
//		}
//		else if ( IsEqualGUID( BFID_GRAY_8, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_GRAY_8\n" );
//		}
//		else if ( IsEqualGUID( BFID_GRAY_16, pFormats[ i ] ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found format BFID_GRAY_16\n" );
//		}
//		else
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter : Found unknown format\n" );
//		}
//	}
//	CoTaskMemFree( pFormats );
//
//	if ( ! bOk )
//	{
//		ATLTRACE( "CThumb - IImageDecodeFilter : OnBeginDecode cannot find RGB_24 format\n");
//		return E_FAIL;
//	}
//
//	return S_OK;
//}
//
//STDMETHODIMP CThumb::Process(IStream* pStream)
//{
//	HRESULT hr;
//
//	const ULONG chunk = 1024;
//	ULONG total = 0;
//	CAtlArray< unsigned char > data;
//	for (;;)
//	{
//		if ( ! data.SetCount( total + chunk ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter::Process() : Out of memory\n" );
//			return E_OUTOFMEMORY;
//		}
//
//		ULONG readed = 0;
//		hr = pStream->Read( data.GetData() + total, chunk, &readed );
//		total += readed;
//		if ( FAILED( hr ) )
//		{
//			ATLTRACE( "CThumb - IImageDecodeFilter::Process() : Read error 0x%08x\n", hr );
//			return hr;
//		}
//		if ( hr == S_FALSE )
//			break;
//	}
//
//	hr = m_pEventSink->OnBitsComplete();
//	ATLTRACE( "CThumb - IImageDecodeFilter::Process() : Readed %u bytes\n", total );
//
//	GFL_BITMAP* hGflBitmap = NULL;
//	hr = _Module.LoadBitmapFromMemory( data.GetData (), total, &hGflBitmap );
//	if ( FAILED( hr ) )
//	{
//		ATLTRACE( "CThumb - IImageDecodeFilter::Process() : Load error 0x%08x\n", hr );
//		return hr;
//	}
//	ATLTRACE( "CThumb - IImageDecodeFilter::Process() : Loaded as %dx%d bitmap (%d bpl)\n", hGflBitmap->Width, hGflBitmap->Height, hGflBitmap->BytesPerLine );
//
//	CComPtr< IDirectDrawSurface > pIDirectDrawSurface;
//	hr = m_pEventSink->GetSurface( hGflBitmap->Width, hGflBitmap->Height,
//		BFID_RGB_24, 1, IMGDECODE_HINT_TOPDOWN | IMGDECODE_HINT_FULLWIDTH,
//		(IUnknown**) &pIDirectDrawSurface );
//	if (FAILED (hr))
//	{
//		ATLTRACE ("CThumb - IImageDecodeFilter::Process() : m_spEventSink->GetSurface error 0x%08x\n", hr );
//		_Module.FreeBitmap( hGflBitmap );
//		return hr;
//	}
//
//	DDSURFACEDESC desc = { sizeof( DDSURFACEDESC ) };
//	RECT rc = { 0, 0, hGflBitmap->Width, hGflBitmap->Height };
//	hr = pIDirectDrawSurface->Lock( &rc, &desc, DDLOCK_WAIT, NULL );
//	if (FAILED (hr))
//	{
//		ATLTRACE ("CThumb - IImageDecodeFilter::Process() : pIDirectDrawSurface->Lock error 0x%08x\n", hr);
//		_Module.FreeBitmap( hGflBitmap );
//		return hr;
//	}
//
//	for ( int line = 0; line < hGflBitmap->Height; ++line )
//	{
//		char* dst = (char*)desc.lpSurface + line * desc.lPitch;
//		char* src = (char*)hGflBitmap->Data + line * hGflBitmap->BytesPerLine;
//		for ( int p = 0; p < hGflBitmap->Width; ++p, dst += 3, src += hGflBitmap->BytesPerPixel )
//		{
//			// RGB -> BGR
//			dst[0] = src[2];
//			dst[1] = src[1];
//			dst[2] = src[0];
//		}
//	}
//
//	hr = pIDirectDrawSurface->Unlock( &desc );
//	if (FAILED (hr))
//	{
//		ATLTRACE ("CThumb - IImageDecodeFilter::Process() : pIDirectDrawSurface->Unlock error 0x%08x\n", hr);
//		_Module.FreeBitmap( hGflBitmap );
//		return hr;
//	}
//
//	m_pEventSink->OnDecodeComplete( S_OK );
//
//	ATLTRACE( "CThumb - IImageDecodeFilter::Process() : OK\n" );
//	_Module.FreeBitmap( hGflBitmap );
//	return hr;
//}
//
//STDMETHODIMP CThumb::Terminate(HRESULT hrStatus)
//{
//	if ( m_pEventSink )
//	{
//		m_pEventSink->OnDecodeComplete( hrStatus );
//		m_pEventSink.Release();
//	}
//
//	return S_OK;
//}
