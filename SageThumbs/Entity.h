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

class CEntity
{
public:
	CEntity();
	~CEntity();

	// Загрузка информации об изображении из базы или из файла
	HRESULT LoadInfo(const CString& sFilename);

	// Загрузка превьюшки из базы или из файла
	HRESULT LoadImage(const CString& sFilename, UINT cx, UINT cy);

#ifdef ISTREAM_ENABLED
	HRESULT LoadInfo(IStream* pStream);
	HRESULT LoadImage(IStream* pStream, UINT cx, UINT cy);
#endif // ISTREAM_ENABLED

	// Получение битмэпа
	HBITMAP GetImage(UINT cx, UINT cy);

	// Получение иконки
	HICON GetIcon(UINT cx);

	inline operator bool() const
	{
		return ( m_hGflBitmap != NULL );
	}

	inline bool IsInfoAvailable() const
	{
		return m_bInfoLoaded;
	}

	inline UINT Width() const
	{
		return m_hGflBitmap ? m_hGflBitmap->Width : 0;
	}

	inline UINT Height() const
	{
		return m_hGflBitmap ? m_hGflBitmap->Height : 0;
	}

	inline UINT ImageWidth() const
	{
		return m_ImageInfo.Width;
	}

	inline UINT ImageHeight() const
	{
		return m_ImageInfo.Height;
	}

	inline UINT ImageXdpi() const
	{
		return m_ImageInfo.Xdpi;
	}

	inline UINT ImageYdpi() const
	{
		return m_ImageInfo.Ydpi;
	}

	inline UINT ImageBitDepth() const
	{
		return m_ImageInfo.ComponentsPerPixel * m_ImageInfo.BitsPerComponent;
	}

	inline CString ImageDescription() const
	{
		return (LPCTSTR)CA2T( m_ImageInfo.Description );
	}

	inline GFL_COMPRESSION ImageCompression() const
	{
		return m_ImageInfo.Compression;
	}

	inline CString ImageCompressionDescription() const
	{
		return (LPCTSTR)CA2T( m_ImageInfo.CompressionDescription );
	}

	inline void GetLastWriteTime(FILETIME* pDateStamp) const
	{
		*pDateStamp = m_FileData.ftLastWriteTime;
	}

	CString GetTitleString() const;
	CString GetInfoTipString() const;
	CString GetMenuTipString() const;

	// Расчёт размеров изображения исходя из заданных размеров подложки
	void CalcSize(UINT& tx, UINT& ty, UINT width, UINT height);

protected:
	GFL_FILE_INFORMATION	m_ImageInfo;		// Image info
	WIN32_FIND_DATA			m_FileData;			// Image file info
	volatile bool			m_bDatabaseUsed;	// Image info loaded from database
	volatile bool			m_bInfoLoaded;		// Image info loaded successfully
	GFL_BITMAP*				m_hGflBitmap;		// Loaded thumbnail
	CComAutoCriticalSection m_pSection;
};
