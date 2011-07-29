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

	// Получение битмэпа
	HBITMAP GetImage(UINT cx, UINT cy) const;

	// Получение иконки
	HICON GetIcon(UINT cx) const;

	inline operator bool() const
	{
		return ( m_hGflBitmap != NULL );
	}

	inline UINT Width() const
	{
		return m_hGflBitmap ? m_hGflBitmap->Width : 0;
	}

	inline UINT Height() const
	{
		return m_hGflBitmap ? m_hGflBitmap->Height : 0;
	}

	inline void GetLastWriteTime(FILETIME* pDateStamp) const
	{
		*pDateStamp = m_FileData.ftLastWriteTime;
	}

	inline LPCTSTR GetTitleString() const
	{
		return m_TitleString;
	}

	inline LPCTSTR GetInfoTipString() const
	{
		return m_InfoTipString;
	}

	inline LPCTSTR GetMenuTipString() const
	{
		return m_MenuTipString;
	}

	// Расчёт размеров изображения исходя из заданных размеров подложки
	void CalcSize(UINT& tx, UINT& ty, UINT width, UINT height) const;

protected:
	CString					m_sName;			// Имя файла
	CString					m_sPath;			// Путь файла
	WIN32_FIND_DATA			m_FileData;			// Данные о файле на диске
	GFL_FILE_INFORMATION	m_ImageInfo;		// Информация о изображении
	GFL_BITMAP*				m_hGflBitmap;		// Загруженный эскиз (NULL - не загружен)
	CString					m_TitleString;		// Текст подписи под эскизом
	CString					m_InfoTipString;	// Текст для всплывающей подсказки
	CString					m_MenuTipString;	// Текст для подсказки пункта меню
};
