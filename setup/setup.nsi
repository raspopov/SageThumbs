!define TITLE		"SageThumbs"
!define VERSION		"2.0.0.13"
!define COMPANY		"Cherubic Software"
!define FILENAME	"sagethumbs_${VERSION}_setup.exe"
!define COPYRIGHT	"Copyright © 2004-2012 Nikolay Raspopov"
!define URL			"http://www.cherubicsoft.com/"
!define PAD			"http://sagethumbs.googlecode.com/svn/trunk/sagethumbs.xml"

SetCompressor /SOLID lzma

!addplugindir .

!include "x64.nsh"

!include "WordFunc.nsh"
	!insertmacro "VersionCompare"

!include "XML.nsh"

!include "OnlineUpdate.nsh"
	!insertmacro "UpdatePad"

Var STARTMENU_FOLDER
!include "MUI2.nsh"
	!define MUI_ABORTWARNING
	!define MUI_HEADERIMAGE
	!define MUI_ICON "install.ico"
	!define MUI_UNICON "uninstall.ico"
	!define MUI_HEADERIMAGE_BITMAP "install.bmp"
	!define MUI_HEADERIMAGE_UNBITMAP "uninstall.bmp"
	!define MUI_COMPONENTSPAGE_NODESC
	!define MUI_FINISHPAGE_NOAUTOCLOSE
	!define MUI_UNFINISHPAGE_NOAUTOCLOSE
	!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
	!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\SageThumbs"
	!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
	!insertmacro MUI_PAGE_WELCOME
	!insertmacro MUI_PAGE_LICENSE "..\license.txt"
	!insertmacro MUI_PAGE_COMPONENTS
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER
	!insertmacro MUI_PAGE_INSTFILES
	!insertmacro MUI_PAGE_FINISH
	!insertmacro MUI_UNPAGE_WELCOME
	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES
	!insertmacro MUI_UNPAGE_FINISH
	!insertmacro MUI_LANGUAGE "English"
	!insertmacro MUI_LANGUAGE "Russian"
	!insertmacro MUI_LANGUAGE "French"
	!insertmacro MUI_LANGUAGE "German"
	!insertmacro MUI_LANGUAGE "Swedish"
	!insertmacro MUI_LANGUAGE "Italian"
	!insertmacro MUI_LANGUAGE "PortugueseBR"
	!insertmacro MUI_LANGUAGE "SimpChinese"
	!insertmacro MUI_LANGUAGE "Spanish"

!macro InstallSageThumb _SRC _DST
	SetOutPath ${_DST}
	File /oname=SageThumbs.dll.tmp		"${_SRC}\SageThumbs.dll"
	File /oname=libgfl340.dll.tmp		"${_SRC}\libgfl340.dll"
	File /oname=libgfle340.dll.tmp		"${_SRC}\libgfle340.dll"
	File /oname=sqlite3.dll.tmp			"${_SRC}\sqlite3.dll"
	Rename /REBOOTOK "${_DST}\SageThumbs.dll.tmp"		"${_DST}\SageThumbs.dll"
	Rename /REBOOTOK "${_DST}\libgfl340.dll.tmp"		"${_DST}\libgfl340.dll"
	Rename /REBOOTOK "${_DST}\libgfle340.dll.tmp"		"${_DST}\libgfle340.dll"
	Rename /REBOOTOK "${_DST}\sqlite3.dll.tmp"			"${_DST}\sqlite3.dll"
!macroend

!macro FreeSageThumbs _DST
	IfFileExists "${_DST}\SageThumbs.dll" 0 +7
	ExecWait '"$SYSDIR\regsvr32.exe" /s /u "${_DST}\SageThumbs.dll"'
	System::Call 'kernel32::CreateEventA(i 0, i 1, i 0, t "SageThumbsWatch") i .r1'
	StrCmp $R1 0 +3
	System::Call 'kernel32::SetEvent(i r1)'
	Sleep 1000
	Delete "${_DST}\SageThumbs.dll"
!macroend

Name "${TITLE}"
VIProductVersion "${VERSION}"
VIAddVersionKey ProductName "${TITLE}"
VIAddVersionKey ProductVersion "${VERSION}"
VIAddVersionKey OriginalFilename "${FILENAME}"
VIAddVersionKey FileDescription "${TITLE}"
VIAddVersionKey FileVersion "${VERSION}"
VIAddVersionKey CompanyName "${COMPANY}"
VIAddVersionKey LegalCopyright "${COPYRIGHT}"
VIAddVersionKey Comments "${URL}"
CRCCheck On
XPStyle On
BrandingText "${COPYRIGHT}"
SetOverwrite Try
OutFile "${FILENAME}"
InstallDir "$PROGRAMFILES\SageThumbs"
InstallDirRegKey HKCU "Software\SageThumbs" "Install"
ShowInstDetails show
ShowUninstDetails show
RequestExecutionLevel admin

Section "Check for new version before install"

	SetOutPath $TEMP
	${UpdatePad} "${PAD}" "${VERSION}" $0 $1
	StrCmp $0 "" 0 new
	DetailPrint "No updates available"
	Goto end
new:
	DetailPrint "Found new version: $1"
	MessageBox MB_YESNO|MB_ICONQUESTION|MB_SETFOREGROUND "Found new version: $1$\n$\nDo you want to download and install it instead?" IDYES update
	Goto end
update:
	DetailPrint "Downloading $0..."
	StrCpy $3 "$EXEDIR\sagethumbs_$1_setup.exe"
	ClearErrors
	CreateDirectory "$EXEDIR\_test"
	IfErrors 0 +2
	StrCpy $3 "$TEMP\sagethumbs_$1_setup.exe"
	RMDir "$EXEDIR\_test"
	Delete "$3"
	NSISdl::download $0 $3
	Pop $2
	StrCmp $2 "cancel" end
	StrCmp $2 "success" run
	DetailPrint "Download failed: $2"
	MessageBox MB_OK|MB_ICONEXCLAMATION|MB_SETFOREGROUND "Download failed: $2"
    ExecShell "open" "$0"
    Quit

run:
    Exec '$3'
	Quit

end:
	ClearErrors

SectionEnd

Section "!${TITLE}"
	SectionIn RO
	SetOutPath $TEMP

# Trying to unregister, unload and delete SageThumbs 32-bit
	!insertmacro "FreeSageThumbs" "$INSTDIR\32"
	IfErrors 0 ok32
	!insertmacro "FreeSageThumbs" "$INSTDIR\32"
	IfErrors 0 ok32
	!insertmacro "FreeSageThumbs" "$INSTDIR\32"
	IfErrors 0 ok32
	!insertmacro "FreeSageThumbs" "$INSTDIR\32"
	IfErrors 0 ok32
	!insertmacro "FreeSageThumbs" "$INSTDIR\32"
ok32:
	Delete "$INSTDIR\32\SageThumbs*.dll"
	Delete "$INSTDIR\32\libgfl*.dll"
	Delete "$INSTDIR\32\sqlite3.dll"
	Delete "$INSTDIR\32\*.tmp"

# Trying to unregister, unload and delete SageThumbs 64-bit
	${If} ${RunningX64}
	!insertmacro "FreeSageThumbs" "$INSTDIR\64"
	IfErrors 0 ok64
	!insertmacro "FreeSageThumbs" "$INSTDIR\64"
	IfErrors 0 ok64
	!insertmacro "FreeSageThumbs" "$INSTDIR\64"
	IfErrors 0 ok64
	!insertmacro "FreeSageThumbs" "$INSTDIR\64"
	IfErrors 0 ok64
	!insertmacro "FreeSageThumbs" "$INSTDIR\64"
ok64:
	Delete "$INSTDIR\64\SageThumbs*.dll"
	Delete "$INSTDIR\64\libgfl*.dll"
	Delete "$INSTDIR\64\sqlite3.dll"
	Delete "$INSTDIR\64\*.tmp"
	${EndIf}

# Clean very old files
	Delete /REBOOTOK "$INSTDIR\SageThumbs*.dll"
	Delete /REBOOTOK "$INSTDIR\libgfl*.dll"
	Delete /REBOOTOK "$INSTDIR\sqlite3.dll"
	Delete /REBOOTOK "$INSTDIR\*.tmp"
	SetRegView 32
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs 32-bit"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs 64-bit"
	Delete "$SMPROGRAMS\SageThumbs 32-bit\*.*"
	RmDir  "$SMPROGRAMS\SageThumbs 32-bit"
	SetRegView 64
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs 32-bit"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs 64-bit"
	Delete "$SMPROGRAMS\SageThumbs 64-bit\*.*"
	RmDir  "$SMPROGRAMS\SageThumbs 64-bit"
	${If} ${RunningX64}
	RmDir /r /REBOOTOK "$PROGRAMFILES64\SageThumbs"
	${EndIf}

# Clean old database
	ReadRegStr $0 HKCU "Software\SageThumbs" "Database"
	StrCmp $0 "" +2
	Delete /REBOOTOK "$0"
	Delete /REBOOTOK "$APPDATA\SageThumbs.db3"
	Delete /REBOOTOK "$WINDIR\SageThumbs.db3"
	Delete /REBOOTOK "$INSTDIR\SageThumbs.db3"

# Install SageThumbs 32-bit
	!insertmacro "InstallSageThumb" "..\Win32\release" "$INSTDIR\32"

# Install SageThumbs 64-bit
	${If} ${RunningX64}
	!insertmacro "InstallSageThumb" "..\x64\release" "$INSTDIR\64"
	${EndIf}

# Install common files and uninstaller
	SetOutPath $INSTDIR
	File "..\SageThumbs\SageThumbs.dll.pot"
	File "..\license.txt"
	File "..\readme.txt"
	WriteUninstaller "Uninst.exe"
	WriteRegStr HKCU "Software\SageThumbs" "Install" $INSTDIR
	DeleteRegKey  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs"
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs" "DisplayName" "${TITLE} ${VERSION}"
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs" "UninstallString" "$INSTDIR\Uninst.exe"
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs" "InstallLocation" "$INSTDIR"
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs" "DisplayIcon" "$INSTDIR\32\SageThumbs.dll"
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs" "DisplayVersion" "${VERSION}"
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs" "Publisher" "${COMPANY}"
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs" "URLInfoAbout" "${URL}"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs" "NoRepair" 1

# Install start menu items
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
	CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
	CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\SageThumbs 32-bit Options.lnk" "control.exe" "$\"$INSTDIR\32\SageThumbs.dll$\"" "$INSTDIR\32\SageThumbs.dll" 0
	${If} ${RunningX64}
	CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\SageThumbs 64-bit Options.lnk" "control.exe" "$\"$INSTDIR\64\SageThumbs.dll$\"" "$INSTDIR\64\SageThumbs.dll" 0
	${EndIf}
	CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Read Me.lnk" "$INSTDIR\readme.txt" "" "$INSTDIR\readme.txt" 0
	CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\License.lnk" "$INSTDIR\license.txt" "" "$INSTDIR\license.txt" 0
	CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall SageThumbs.lnk" "$INSTDIR\Uninst.exe" "" "$INSTDIR\Uninst.exe" 0
	!insertmacro MUI_STARTMENU_WRITE_END
	SetOutPath "$SMPROGRAMS\$STARTMENU_FOLDER"
	File "..\SageThumbs Online.url"

# Registration
	IfRebootFlag reboot 0
	ExecWait '"$SYSDIR\regsvr32.exe" /s "$INSTDIR\32\SageThumbs.dll"'
	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "SageThumbs32"
	${If} ${RunningX64}
	ExecWait '"$SYSDIR\regsvr32.exe" /s "$INSTDIR\64\SageThumbs.dll"'
	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "SageThumbs64"
	${EndIf}
	goto done
reboot:
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "SageThumbs32" "regsvr32.exe /s $\"$INSTDIR\32\SageThumbs.dll$\""
	${If} ${RunningX64}
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "SageThumbs64" "regsvr32.exe /s $\"$INSTDIR\64\SageThumbs.dll$\""
	${EndIf}
done:

SectionEnd

Section "Uninstall"
	SetOutPath $TEMP

# Trying to unregister, unload and delete SageThumbs 32-bit
	!insertmacro "FreeSageThumbs" "$INSTDIR\32"
	IfErrors 0 ok32
	!insertmacro "FreeSageThumbs" "$INSTDIR\32"
	IfErrors 0 ok32
	!insertmacro "FreeSageThumbs" "$INSTDIR\32"
	IfErrors 0 ok32
	!insertmacro "FreeSageThumbs" "$INSTDIR\32"
	IfErrors 0 ok32
	!insertmacro "FreeSageThumbs" "$INSTDIR\32"
	IfErrors 0 ok32
	Delete /REBOOTOK "$INSTDIR\32\SageThumbs.dll"
ok32:
	Delete /REBOOTOK "$INSTDIR\32\SageThumbs*.dll"
	Delete /REBOOTOK "$INSTDIR\32\libgfl*.dll"
	Delete /REBOOTOK "$INSTDIR\32\sqlite3.dll"
	Delete /REBOOTOK "$INSTDIR\32\*.tmp"
	RMDir  /REBOOTOK "$INSTDIR\32\"
	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "SageThumbs32"

# Trying to unregister, unload and delete SageThumbs 64-bit
	${If} ${RunningX64}
	!insertmacro "FreeSageThumbs" "$INSTDIR\64"
	IfErrors 0 ok64
	!insertmacro "FreeSageThumbs" "$INSTDIR\64"
	IfErrors 0 ok64
	!insertmacro "FreeSageThumbs" "$INSTDIR\64"
	IfErrors 0 ok64
	!insertmacro "FreeSageThumbs" "$INSTDIR\64"
	IfErrors 0 ok64
	!insertmacro "FreeSageThumbs" "$INSTDIR\64"
	IfErrors 0 ok64
	Delete /REBOOTOK "$INSTDIR\64\SageThumbs.dll"
ok64:
	Delete /REBOOTOK "$INSTDIR\64\SageThumbs*.dll"
	Delete /REBOOTOK "$INSTDIR\64\libgfl*.dll"
	Delete /REBOOTOK "$INSTDIR\64\sqlite3.dll"
	Delete /REBOOTOK "$INSTDIR\64\*.tmp"
	RMDir  /REBOOTOK "$INSTDIR\64\"
	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "SageThumbs64"
	${EndIf}

# Delete database
	ReadRegStr $0 HKCU "Software\SageThumbs" "Database"
	StrCmp $0 "" +2
	Delete /REBOOTOK "$0"
	Delete /REBOOTOK "$APPDATA\SageThumbs.db3"
	Delete /REBOOTOK "$WINDIR\SageThumbs.db3"
	Delete /REBOOTOK "$INSTDIR\SageThumbs.db3"
	
# Delete start menu items
	!insertmacro MUI_STARTMENU_GETFOLDER Application $STARTMENU_FOLDER
	Delete "$SMPROGRAMS\$STARTMENU_FOLDER\*.*"
	RmDir  "$SMPROGRAMS\$STARTMENU_FOLDER"

# Delete common files and uninstaller
	Delete /REBOOTOK "$INSTDIR\SageThumbs.dll.pot"
	Delete /REBOOTOK "$INSTDIR\license.txt"
	Delete /REBOOTOK "$INSTDIR\readme.txt"
	Delete /REBOOTOK "$INSTDIR\Uninst.exe"
	RMDir  /REBOOTOK "$INSTDIR"
	SetRegView 32
	DeleteRegKey HKCU "Software\SageThumbs"
	DeleteRegKey HKLM "Software\SageThumbs"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs"
	SetRegView 64
	DeleteRegKey HKCU "Software\SageThumbs"
	DeleteRegKey HKLM "Software\SageThumbs"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SageThumbs"

SectionEnd

Function .onInit
	SetShellVarContext all
	SetRegView 64
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${TITLE}.${VERSION}") i .r1 ?e'
	Pop $R0
	StrCmp $R0 0 +2
	Abort
FunctionEnd

Function un.onInit
	SetShellVarContext all
	SetRegView 64
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${TITLE}.${VERSION}") i .r1 ?e'
	Pop $R0
	StrCmp $R0 0 +2
	Abort
FunctionEnd

!appendfile "setup.trg" "[${__TIMESTAMP__}] ${TITLE} ${VERSION} ${FILENAME}$\n"
