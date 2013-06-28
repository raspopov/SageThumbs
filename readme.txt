
SageThumbs Shell Extension

========================================================================

Features:

  * Extended thumbnail image view of explorer folder
  * Thumbnail image in explorer context menu (right-click menu)
  * Extended info tips
  * Support 162 image formats (224 extensions) via GFL Library
  * Support additional 26 image formats via XnView plugins (if installed)
  * Send by mail support
  * One-click conversion to popular image formats support
  * Wallpaper selection support
  * Copy to clipboard support

Please read carefully the following information. 
	As SageThumbs is a Windows Explorer context menu extension,
	there is no an executable file that you should launch to bring it up.
	To use it, just open any folder with image files in Windows Explorer,
	and then right-click a file you want to preview. You'll see the
	thumbnail immediately in the context menu. 

========================================================================

System Requirements:

  * Windows XP/2003/Vista/2008/7/8 32/64-bit
  * About 5 Mb of disk space + additional space for cache

========================================================================

External plugins:

	SageThumbs can use external XnView plugins, it tries to automatically
	detect XnView installation folder or if failed it will use predefined
	folder. Typically this folder is "C:\Program files\XnView\PlugIns\"
	for 32-bit SageThumbs on 32-bit Windows or 64-bit SageThumbs on 64-bit
	Windows and "C:\Program Files (x86)\XnView\PlugIns\" for 32-bit
	SageThumbs on 64-bit Windows. So if you have no XnView installed you
	can create this folder manually and unpack plugins to it. Just don't
	forget that you need same "bit capacity" for plugins and SageThumbs.

========================================================================

Translation:

SageThumbs can be easily translated to other languages.

  * Download and install PoEdit: http://www.poedit.net/download.php
  * Copy "SageThumbs.dll.pot" file (can be found in SageThumbs installation
    folder) under new name "SageThumbs.dll.XX.po" near original files i.e. near
    "32\SageThumbs.dll" and "64\SageThumbs.dll". Where "XX" is 2-digit hexadecimal
    code of language, see: http://msdn.microsoft.com/en-us/library/windows/desktop/dd318693.aspx
    For example Russian has code 0x19 so its a "SageThumbs.dll.19.po".
  * Open new file in PoEdit, translate and save as UTF-8 (its by default)
  * Open SageThumbs options and select new language in it
  * E-mail file to me: raspopov@cherubicsoft.com
    It will be definitely included in next version.

========================================================================

Versions:

2.0.0.16
  * Added user-defined file extensions
  * PNG-files now disabled by default under Windows 8
  * Fixed misplaced thumbnail in context menu under classic themes
  * Fixed copy to clipboard function
  * Fixed uninstallation minor bugs
  * Added fixer of registry keys bad security rights (fixes incompatibility with Windows PhotoViewer)
  * Added Turkish translation
  * Added Hungarian translation
  * Added Persian (Farsi) translation
  * Updated translations

2.0.0.15
  * Fixed "Use thumbnails as icons in Explorer" option sometimes broken in Windows 7 (thanks to uhfath)
  * Minor interface fixes
  * Updated translations

2.0.0.14
  * Improved "compatible" mode (added invalid extension detection)
  * Fixed PaintShopPro extensions (pspbrush, pspframe and pspimage)
  * Fixed shifted icon in context menu on Windows XP
  * Added new option "Show preview in submenu"
  * Added "SageThumbs Repair Utility"
  * Added Polish translation
  * Updated installer

2.0.0.13
  * New user-friendly translation method using PoEdit
  * Shortened main menu text (without "32/64-bit")
  * SageThumbs now works in "compatible" mode (overwrite no other shell extensions)
  * Excluded "vst" extension by default
  * Dropped support for Windows 2000
  * Added Indonesian translation

2.0.0.12
  * "Maximum size of image file" option limits image thumbnail generation only
  * Added Spanish translation

2.0.0.11
  * Added new option "Show type overlay on thumbnails"
  * Separated 32-bit and 64-bit plugins folders
  * Fixed some issues under UAC
  * Added Chinese translation

2.0.0.10
  * Improved/optimized image property information (Windows Vista and above)
  * Added progress dialogs for long operations
  * Added Italian translation

2.0.0.9
  * Added support for image properties
  * Improved working with image files associations in Windows Registry
  * Cache performance optimizations
  * Added Swedish translation
  * Added Portuguese translation
  * Added new version checker to installer

2.0.0.8
  * Combined 32/64-bit distributive
  * Added "JPEG quality" and "PNG compression" convert options
  * Added "Convert to PNG" context menu item
  * Optimized options dialog

2.0.0.7
  * Added option "Enable Windows thumbnail cache" (as Win7 "black thumbnail" bug workaround)
  * Added support for work under restricted account privileges (UAC)
  * Performance optimizations
  * Fixed SageThumbs modules stuck inside parent process
  * Excluded by default "wmz" extension (used by "Windows Media Player Skin Package")

2.0.0.6
  * Fixed Windows 7 "black thumbnail" bug
  * Project converted to MS VS2010 (if compiled minimum system requirements will be Windows XP)
  * Optimized distributive file (smaller)

2.0.0.5
  * Added "Maximum size of image file" option to prevent processing of very large images (performance)
  * Added "Prefer image file embedded thumbnails" option (performance)
  * Added "Use thumbnails as icons in Explorer" option
  * Added Windows Cleanup utility support
  * Improved thumbnail cache structure (performance)
  * SQLite library updated to version 3.7.7.1
  * Minor bugfixes and optimizations
  * Opened project on Google Code: http://code.google.com/p/sagethumbs/

2.0.0.4
  * Added German translation
  * Minor bugfixes (disabled unfinished yet code for Explorer's preview plane)

2.0.0.3
  * Fixed thumbnail aspect ratio
  * Fixed thumbnail quality
  * Changed SQLite database schema (database engine updated)
  * GFL Library updated to version 3.40 (now Unicode)
  * Minor bugfixes

2.0.0.2
  * More complete support for Vista (menu, thumbnails, control panel)
  * Added options to turn off context menu and/or Explorer thumbnails
  * Added option to clear cache database
  * By default .ico and .cur file extensions excluded
  * Performance optimization
  * Minor bugfixes

2.0.0.1
  * Added 64-bit OS support
  * GFL Library upgraded to version 3.11
  * Minor bugfixes

1.0.0.13
  * GFL Library upgraded to version 2.54
  * Added support for multiply files operations (converting, e-mailing etc.)
  * Added more descriptive version information in About box
  * Added multi language support (selectable)
  * Fixed very long file paths
  * Modified installation process (makes attempt to avoid reboots)

1.0.0.12
  * Fixed missing preview (one missing registry key)

1.0.0.11
  * Fixed AppID registration (may be useful for some security maniacs...)
  * Fixed missed additional image extensions (doh!)
  * Added image information cache (SQLite 3.0.8 database) - performance boost!

1.0.0.10
  * Fixed installation procedure
  * Fixed shell registration
  * Fixed "experimental ColumnHandler support in 1.0.0.9" bug 
  * Fixed "duplicate context menu item insertion in Explorer File menu" bug
  * Fixed "missing uppercase file extensions" bug
  * Fixed "right-click double selection" bug
  * Now GFL thread-safe (more stable)
  * Added extensions cache for performance
  * Added Options dialog shortcut in Start Menu and Control Panel
  * Added handled extension selection in Options dialog

1.0.0.9
  * Added copy to clipboard support
  * Added wallpaper selection support (centered, tiled, stretched)
  * Added send by mail support (original image and thumbnail image)
  * Added one click conversion to JPG, GIF, BMP image formats

1.0.0.8
  * Fixed missed default menu items and "Open With..." submenu
  * Fixed lost preview on Explorer left panel

1.0.0.7
  * GFL Library upgraded to version 2.20:
  * Added GFL_ALLOCATEBITMAP_CALLBACK, GFL_PROGRESS_CALLBACK & GFL_WANTCANCEL_CALLBACK callback 
  * Added gflSetIPTCValue & gflRemoveIPTCValue to change IPTC value 
  * Added gflLoadIPTC, gflSaveIPTC, gflBitmapSetIPTC 
  * Added gflJPEGGetComment, gflJPEGSetComment, gflPNGGetComment, gflPNGSetComment 
  * Added Support of long pathname on Windows NT (with \\?\) 
  * Added gflGetFileInformationFromMemory, gflLoadBitmapFromMemory, gflLoadThumbnailFromMemory 
  * Fixed Many bugs & improved speed 

1.0.0.6
  * Added Russian language
  * Extended image info (info tips)
  * Fixed black borders around thumbnail

1.0.0.5
  * Added Options dialog
  * Changed menu colors to Windows default menu colors
  * Now isolation-aware enabled
  * Fixed crash during Windows shutdown or logoff in 32-bit Fractal Image Decoder Library: "First-chance exception at 0x1101fe06 (deco_32.dll) in explorer.exe: 0xC0000005: Access violation reading location 0x02ca4bff."
  * Included Windows Fax/Image Viewer switch
  * Included some minor registry fixes for XnView image file types

1.0.0.4
  * Corrected installation/uninstallation procedure

1.0.0.3
  * First release

========================================================================

License:

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

========================================================================

Copyrights:

SageThumbs
Copyright (C) Nikolay Raspopov, 2004-2013.
E-Mail: raspopov@cherubicsoft.com
Web site: http://www.cherubicsoft.com/projects/sagethumbs

GFL Library, GFL SDK and XnView
Copyright (C) Pierre-E Gougelet, 1991-2011.
E-Mail: webmaster@xnview.com
Web site: http://www.xnview.com/
