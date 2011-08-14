!define SRC32		"..\Win32\release\"
!define SRC64		"..\x64\release\"

!define TITLE		"SageThumbs"
!define VERSION		"2.0.0.9"
!define COMPANY		"Cherubic Software"
!define FILENAME	"sagethumbs_${VERSION}_setup.exe"
!define COPYRIGHT	"Copyright © 2004-2011 Nikolay Raspopov"
!define URL			"http://www.cherubicsoft.com/"
!define PAD			"http://sagethumbs.googlecode.com/svn/trunk/sagethumbs.xml"

SetCompressor /SOLID lzma

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
	Call FreeSageThumbs32
	IfErrors wait32_1 ok32
wait32_1:
	Call FreeSageThumbs32
	IfErrors wait32_2 ok32
wait32_2:
	Call FreeSageThumbs32
	IfErrors wait32_3 ok32
wait32_3:
	Call FreeSageThumbs32
	IfErrors wait32_4 ok32
wait32_4:
	Call FreeSageThumbs32
ok32:
	Delete "$INSTDIR\32\SageThumbs1d.dll"
	Delete "$INSTDIR\32\SageThumbs19.dll"
	Delete "$INSTDIR\32\SageThumbs16.dll"
	Delete "$INSTDIR\32\SageThumbs10.dll"
	Delete "$INSTDIR\32\SageThumbs0c.dll"
	Delete "$INSTDIR\32\SageThumbs07.dll"
	Delete "$INSTDIR\32\libgfl340.dll"
	Delete "$INSTDIR\32\libgfle340.dll"
	Delete "$INSTDIR\32\sqlite3.dll"
	Delete "$INSTDIR\32\*.tmp"

# Trying to unregister, unload and delete SageThumbs 64-bit
	${If} ${RunningX64}
	Call FreeSageThumbs64
	IfErrors wait64_1 ok64
wait64_1:
	Call FreeSageThumbs64
	IfErrors wait64_2 ok64
wait64_2:
	Call FreeSageThumbs64
	IfErrors wait64_3 ok64
wait64_3:
	Call FreeSageThumbs64
	IfErrors wait64_4 ok64
wait64_4:
	Call FreeSageThumbs64
ok64:
	Delete "$INSTDIR\64\SageThumbs1d.dll"
	Delete "$INSTDIR\64\SageThumbs19.dll"
	Delete "$INSTDIR\64\SageThumbs16.dll"
	Delete "$INSTDIR\64\SageThumbs10.dll"
	Delete "$INSTDIR\64\SageThumbs0c.dll"
	Delete "$INSTDIR\64\SageThumbs07.dll"
	Delete "$INSTDIR\64\libgfl340.dll"
	Delete "$INSTDIR\64\libgfle340.dll"
	Delete "$INSTDIR\64\sqlite3.dll"
	Delete "$INSTDIR\64\*.tmp"
	${EndIf}

# Clean very old files
	Delete /REBOOTOK "$INSTDIR\SageThumbs.dll"
	Delete /REBOOTOK "$INSTDIR\SageThumbs19.dll"
	Delete /REBOOTOK "$INSTDIR\SageThumbs0c.dll"
	Delete /REBOOTOK "$INSTDIR\SageThumbs07.dll"
	Delete /REBOOTOK "$INSTDIR\libgfl220.dll"
	Delete /REBOOTOK "$INSTDIR\libgfle220.dll"
	Delete /REBOOTOK "$INSTDIR\libgfl240.dll"
	Delete /REBOOTOK "$INSTDIR\libgfle240.dll"
	Delete /REBOOTOK "$INSTDIR\libgfl254.dll"
	Delete /REBOOTOK "$INSTDIR\libgfle254.dll"
	Delete /REBOOTOK "$INSTDIR\libgfl290.dll"
	Delete /REBOOTOK "$INSTDIR\libgfle290.dll"
	Delete /REBOOTOK "$INSTDIR\libgfl311.dll"
	Delete /REBOOTOK "$INSTDIR\libgfle311.dll"
	Delete /REBOOTOK "$INSTDIR\libgfl340.dll"
	Delete /REBOOTOK "$INSTDIR\libgfle340.dll"
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
	SetOutPath $INSTDIR\32
	File /oname=SageThumbs.dll.tmp		"${SRC32}SageThumbs.dll"
	File /oname=SageThumbs1d.dll.tmp	"${SRC32}SageThumbs1d.dll"
	File /oname=SageThumbs19.dll.tmp	"${SRC32}SageThumbs19.dll"
	File /oname=SageThumbs16.dll.tmp	"${SRC32}SageThumbs16.dll"
	File /oname=SageThumbs10.dll.tmp	"${SRC32}SageThumbs10.dll"
	File /oname=SageThumbs0c.dll.tmp	"${SRC32}SageThumbs0c.dll"
	File /oname=SageThumbs07.dll.tmp	"${SRC32}SageThumbs07.dll"
	File /oname=libgfl340.dll.tmp		"${SRC32}libgfl340.dll"
	File /oname=libgfle340.dll.tmp		"${SRC32}libgfle340.dll"
	File /oname=sqlite3.dll.tmp			"${SRC32}sqlite3.dll"
	Rename /REBOOTOK "$INSTDIR\32\SageThumbs.dll.tmp"	"$INSTDIR\32\SageThumbs.dll"
	Rename /REBOOTOK "$INSTDIR\32\SageThumbs1d.dll.tmp"	"$INSTDIR\32\SageThumbs1d.dll"
	Rename /REBOOTOK "$INSTDIR\32\SageThumbs19.dll.tmp"	"$INSTDIR\32\SageThumbs19.dll"
	Rename /REBOOTOK "$INSTDIR\32\SageThumbs16.dll.tmp"	"$INSTDIR\32\SageThumbs16.dll"
	Rename /REBOOTOK "$INSTDIR\32\SageThumbs10.dll.tmp"	"$INSTDIR\32\SageThumbs10.dll"
	Rename /REBOOTOK "$INSTDIR\32\SageThumbs0c.dll.tmp"	"$INSTDIR\32\SageThumbs0c.dll"
	Rename /REBOOTOK "$INSTDIR\32\SageThumbs07.dll.tmp"	"$INSTDIR\32\SageThumbs07.dll"
	Rename /REBOOTOK "$INSTDIR\32\libgfl340.dll.tmp"	"$INSTDIR\32\libgfl340.dll"
	Rename /REBOOTOK "$INSTDIR\32\libgfle340.dll.tmp"	"$INSTDIR\32\libgfle340.dll"
	Rename /REBOOTOK "$INSTDIR\32\sqlite3.dll.tmp"		"$INSTDIR\32\sqlite3.dll"
	IfRebootFlag reboot32 0
	ExecWait '"$SYSDIR\regsvr32.exe" /s "$INSTDIR\32\SageThumbs.dll"'
	goto done32
reboot32:
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "SageThumbs32" "regsvr32.exe /s $\"$INSTDIR\32\SageThumbs.dll$\""
done32:

# Install SageThumbs 64-bit
	${If} ${RunningX64}
	SetOutPath $INSTDIR\64
	File /oname=SageThumbs.dll.tmp		"${SRC64}SageThumbs.dll"
	File /oname=SageThumbs1d.dll.tmp	"${SRC64}SageThumbs1d.dll"
	File /oname=SageThumbs19.dll.tmp	"${SRC64}SageThumbs19.dll"
	File /oname=SageThumbs16.dll.tmp	"${SRC64}SageThumbs16.dll"
	File /oname=SageThumbs10.dll.tmp	"${SRC64}SageThumbs10.dll"
	File /oname=SageThumbs0c.dll.tmp	"${SRC64}SageThumbs0c.dll"
	File /oname=SageThumbs07.dll.tmp	"${SRC64}SageThumbs07.dll"
	File /oname=libgfl340.dll.tmp		"${SRC64}libgfl340.dll"
	File /oname=libgfle340.dll.tmp		"${SRC64}libgfle340.dll"
	File /oname=sqlite3.dll.tmp			"${SRC64}sqlite3.dll"
	Rename /REBOOTOK "$INSTDIR\64\SageThumbs.dll.tmp"	"$INSTDIR\64\SageThumbs.dll"
	Rename /REBOOTOK "$INSTDIR\64\SageThumbs1d.dll.tmp"	"$INSTDIR\64\SageThumbs1d.dll"
	Rename /REBOOTOK "$INSTDIR\64\SageThumbs19.dll.tmp"	"$INSTDIR\64\SageThumbs19.dll"
	Rename /REBOOTOK "$INSTDIR\64\SageThumbs16.dll.tmp"	"$INSTDIR\64\SageThumbs16.dll"
	Rename /REBOOTOK "$INSTDIR\64\SageThumbs10.dll.tmp"	"$INSTDIR\64\SageThumbs10.dll"
	Rename /REBOOTOK "$INSTDIR\64\SageThumbs0c.dll.tmp"	"$INSTDIR\64\SageThumbs0c.dll"
	Rename /REBOOTOK "$INSTDIR\64\SageThumbs07.dll.tmp"	"$INSTDIR\64\SageThumbs07.dll"
	Rename /REBOOTOK "$INSTDIR\64\libgfl340.dll.tmp"	"$INSTDIR\64\libgfl340.dll"
	Rename /REBOOTOK "$INSTDIR\64\libgfle340.dll.tmp"	"$INSTDIR\64\libgfle340.dll"
	Rename /REBOOTOK "$INSTDIR\64\sqlite3.dll.tmp"		"$INSTDIR\64\sqlite3.dll"
	IfRebootFlag reboot64 0
	ExecWait '"$SYSDIR\regsvr32.exe" /s "$INSTDIR\64\SageThumbs.dll"'
	goto done64
reboot64:
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "SageThumbs64" "regsvr32.exe /s $\"$INSTDIR\64\SageThumbs.dll$\""
done64:
	${EndIf}

# Install common files and uninstaller
	SetOutPath $INSTDIR
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

SectionEnd

Section "Uninstall"
	SetOutPath $TEMP

# Trying to unregister, unload and delete SageThumbs 32-bit
	Call un.FreeSageThumbs32
	IfErrors wait32_1 ok32
wait32_1:
	Call un.FreeSageThumbs32
	IfErrors wait32_2 ok32
wait32_2:
	Call un.FreeSageThumbs32
	IfErrors wait32_3 ok32
wait32_3:
	Call un.FreeSageThumbs32
	IfErrors wait32_4 ok32
wait32_4:
	Call un.FreeSageThumbs32
	IfErrors fail32 ok32
fail32:
	Delete /REBOOTOK "$INSTDIR\32\SageThumbs.dll"
ok32:
	Delete /REBOOTOK "$INSTDIR\32\SageThumbs1d.dll"
	Delete /REBOOTOK "$INSTDIR\32\SageThumbs19.dll"
	Delete /REBOOTOK "$INSTDIR\32\SageThumbs16.dll"
	Delete /REBOOTOK "$INSTDIR\32\SageThumbs10.dll"
	Delete /REBOOTOK "$INSTDIR\32\SageThumbs0c.dll"
	Delete /REBOOTOK "$INSTDIR\32\SageThumbs07.dll"
	Delete /REBOOTOK "$INSTDIR\32\libgfl340.dll"
	Delete /REBOOTOK "$INSTDIR\32\libgfle340.dll"
	Delete /REBOOTOK "$INSTDIR\32\sqlite3.dll"
	Delete /REBOOTOK "$INSTDIR\32\*.tmp"
	RMDir  /REBOOTOK "$INSTDIR\32\"
	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "SageThumbs32"

# Trying to unregister, unload and delete SageThumbs 64-bit
	${If} ${RunningX64}
	Call un.FreeSageThumbs64
	IfErrors wait64_1 ok64
wait64_1:
	Call un.FreeSageThumbs64
	IfErrors wait64_2 ok64
wait64_2:
	Call un.FreeSageThumbs64
	IfErrors wait64_3 ok64
wait64_3:
	Call un.FreeSageThumbs64
	IfErrors wait64_4 ok64
wait64_4:
	Call un.FreeSageThumbs64
	IfErrors fail64 ok64
fail64:
	Delete /REBOOTOK "$INSTDIR\64\SageThumbs.dll"
ok64:
	Delete /REBOOTOK "$INSTDIR\64\SageThumbs1d.dll"
	Delete /REBOOTOK "$INSTDIR\64\SageThumbs19.dll"
	Delete /REBOOTOK "$INSTDIR\64\SageThumbs16.dll"
	Delete /REBOOTOK "$INSTDIR\64\SageThumbs10.dll"
	Delete /REBOOTOK "$INSTDIR\64\SageThumbs0c.dll"
	Delete /REBOOTOK "$INSTDIR\64\SageThumbs07.dll"
	Delete /REBOOTOK "$INSTDIR\64\libgfl340.dll"
	Delete /REBOOTOK "$INSTDIR\64\libgfle340.dll"
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

Function FreeSageThumbs64
	IfFileExists "$INSTDIR\64\SageThumbs.dll" unreg ok
unreg:
	ExecWait '"$SYSDIR\regsvr32.exe" /s /u "$INSTDIR\64\SageThumbs.dll"'
	System::Call 'kernel32::CreateEventA(i 0, i 1, i 0, t "SageThumbsWatch") i .r1'
	StrCmp $R1 0 no_event
 	System::Call 'kernel32::SetEvent(i r1)'
  	Sleep 1000
no_event:
	Delete "$INSTDIR\64\SageThumbs.dll"
ok:
FunctionEnd

Function un.FreeSageThumbs64
	IfFileExists "$INSTDIR\64\SageThumbs.dll" unreg ok
unreg:
	ExecWait '"$SYSDIR\regsvr32.exe" /s /u "$INSTDIR\64\SageThumbs.dll"'
	System::Call 'kernel32::CreateEventA(i 0, i 1, i 0, t "SageThumbsWatch") i .r1'
	StrCmp $R1 0 no_event
 	System::Call 'kernel32::SetEvent(i r1)'
  	Sleep 1000
no_event:
	Delete "$INSTDIR\64\SageThumbs.dll"
ok:
FunctionEnd

Function FreeSageThumbs32
	IfFileExists "$INSTDIR\32\SageThumbs.dll" unreg ok
unreg:
	ExecWait '"$SYSDIR\regsvr32.exe" /s /u "$INSTDIR\32\SageThumbs.dll"'
	System::Call 'kernel32::CreateEventA(i 0, i 1, i 0, t "SageThumbsWatch") i .r1'
	StrCmp $R1 0 no_event
 	System::Call 'kernel32::SetEvent(i r1)'
  	Sleep 1000
no_event:
	Delete "$INSTDIR\32\SageThumbs.dll"
ok:
FunctionEnd

Function un.FreeSageThumbs32
	IfFileExists "$INSTDIR\32\SageThumbs.dll" unreg ok
unreg:
	ExecWait '"$SYSDIR\regsvr32.exe" /s /u "$INSTDIR\32\SageThumbs.dll"'
	System::Call 'kernel32::CreateEventA(i 0, i 1, i 0, t "SageThumbsWatch") i .r1'
	StrCmp $R1 0 no_event
 	System::Call 'kernel32::SetEvent(i r1)'
  	Sleep 1000
no_event:
	Delete "$INSTDIR\32\SageThumbs.dll"
ok:
FunctionEnd

!appendfile "setup.trg" "[${__TIMESTAMP__}] ${TITLE} ${VERSION} ${FILENAME}$\n"
