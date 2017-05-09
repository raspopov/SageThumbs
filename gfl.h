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

#include "gfl\libgfl.h"
#include "gfl\libgfle.h"

#if UNICODE
	#define gflLoadBitmapT			gflLoadBitmapW
	#define gflSaveBitmapT			gflSaveBitmapW
	#define gflLoadThumbnailT		gflLoadThumbnailW
	#define gflSetPluginsPathnameT	gflSetPluginsPathnameW
	#define gflGetFileInformationT	gflGetFileInformationW
#else
	#pragma warning( disable: 4127 )
	#define gflLoadBitmapT			gflLoadBitmap
	#define gflSaveBitmapT			gflSaveBitmap
	#define gflLoadThumbnailT		gflLoadThumbnail
	#define gflSetPluginsPathnameT	gflSetPluginsPathname
	#define gflGetFileInformationT	gflGetFileInformation
#endif
