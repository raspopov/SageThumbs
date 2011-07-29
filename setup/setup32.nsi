!define BITS		"32"
!define TITLE		"SageThumbs ${BITS}-bit"
!define VERSION		"2.0.0.5"
!define COMPANY		"Cherubic Software"
!define FILENAME	"sagethumbs_${VERSION}_${BITS}_setup.exe"
!define COPYRIGHT	"Copyright © 2004-2011 Nikolay Raspopov"

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
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder ${BITS}"
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\license.txt"
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

Name "${TITLE}"
VIProductVersion "${VERSION}"
VIAddVersionKey ProductName "${TITLE}"
VIAddVersionKey ProductVersion "${VERSION}"
VIAddVersionKey OriginalFilename "${FILENAME}"
VIAddVersionKey FileDescription "${TITLE}"
VIAddVersionKey FileVersion "${VERSION}"
VIAddVersionKey CompanyName "${COMPANY}"
VIAddVersionKey LegalCopyright "${COPYRIGHT}"
CRCCheck On
XPStyle On
BrandingText "${COPYRIGHT}"
SetOverwrite Try
OutFile "${FILENAME}"
InstallDir "$PROGRAMFILES${BITS}\SageThumbs"
InstallDirRegKey HKCU "Software\SageThumbs" "Install${BITS}"
ShowInstDetails show
ShowUninstDetails show
RequestExecutionLevel admin

Function .onInit
	SetShellVarContext all
	SetRegView ${BITS}
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "myMutex777") i .r1 ?e'
	Pop $R0
	StrCmp $R0 0 +2
	Abort
FunctionEnd

Function un.onInit
	SetShellVarContext all
	SetRegView ${BITS}
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "myMutex777") i .r1 ?e'
	Pop $R0
	StrCmp $R0 0 +2
	Abort
FunctionEnd

Function FreeSageThumbs
	ExecWait '"$SYSDIR\regsvr32.exe" /s /u "$INSTDIR\SageThumbs.dll"'
	System::Call 'kernel32::CreateEventA(i 0, i 1, i 0, t "SageThumbsWatch") i .r1'
	StrCmp $R1 0 no_event
 	System::Call 'kernel32::SetEvent(i r1)'
  	Sleep 1000
no_event:
	Delete "$INSTDIR\SageThumbs.dll"
FunctionEnd

Function un.FreeSageThumbs
	ExecWait '"$SYSDIR\regsvr32.exe" /s /u "$INSTDIR\SageThumbs.dll"'
	System::Call 'kernel32::CreateEventA(i 0, i 1, i 0, t "SageThumbsWatch") i .r1'
	StrCmp $R1 0 no_event
 	System::Call 'kernel32::SetEvent(i r1)'
  	Sleep 1000
no_event:
	Delete "$INSTDIR\SageThumbs.dll"
FunctionEnd

Section "${TITLE}"
	SetOutPath $INSTDIR	
	IfFileExists "$INSTDIR\SageThumbs.dll" unreg ok
unreg:
	Call FreeSageThumbs
	IfErrors wait1 ok
wait1:
	Call FreeSageThumbs
	IfErrors wait2 ok
wait2:
	Call FreeSageThumbs
	IfErrors wait3 ok
wait3:
	Call FreeSageThumbs
	IfErrors wait4 ok
wait4:
	Call FreeSageThumbs
ok:
	Delete "$INSTDIR\SageThumbs19.dll"
	Delete "$INSTDIR\SageThumbs0c.dll"
	Delete "$INSTDIR\SageThumbs07.dll"
	Delete "$INSTDIR\libgfl340.dll"
	Delete "$INSTDIR\libgfle340.dll"
	Delete "$INSTDIR\sqlite3.dll"
	
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

	ReadRegStr $0 HKCU "Software\SageThumbs" "Database"
	StrCmp $0 "" +2
	Delete /REBOOTOK "$0"
	Delete /REBOOTOK "$APPDATA\SageThumbs.db3"
	Delete /REBOOTOK "$WINDIR\SageThumbs.db3"
	Delete /REBOOTOK "$INSTDIR\SageThumbs.db3"

	Delete "$INSTDIR\*.tmp"
	File /oname=SageThumbs.dll.tmp "..\Win32\release\SageThumbs.dll"
	File /oname=SageThumbs19.dll.tmp "..\Win32\release\SageThumbs19.dll"
	File /oname=SageThumbs0c.dll.tmp "..\Win32\release\SageThumbs0c.dll"
	File /oname=SageThumbs07.dll.tmp "..\Win32\release\SageThumbs07.dll"
	File /oname=libgfl340.dll.tmp "..\Win32\Release\libgfl340.dll"
	File /oname=libgfle340.dll.tmp "..\Win32\Release\libgfle340.dll"
	File /oname=sqlite3.dll.tmp "..\Win32\Release\sqlite3.dll"
	File "..\license.txt"
	File "..\readme.txt"
	Rename /REBOOTOK "$INSTDIR\SageThumbs.dll.tmp" "$INSTDIR\SageThumbs.dll"
	Rename /REBOOTOK "$INSTDIR\SageThumbs19.dll.tmp" "$INSTDIR\SageThumbs19.dll"
	Rename /REBOOTOK "$INSTDIR\SageThumbs0c.dll.tmp" "$INSTDIR\SageThumbs0c.dll"
	Rename /REBOOTOK "$INSTDIR\SageThumbs07.dll.tmp" "$INSTDIR\SageThumbs07.dll"
	Rename /REBOOTOK "$INSTDIR\libgfl340.dll.tmp" "$INSTDIR\libgfl340.dll"
	Rename /REBOOTOK "$INSTDIR\libgfle340.dll.tmp" "$INSTDIR\libgfle340.dll"
	Rename /REBOOTOK "$INSTDIR\sqlite3.dll.tmp" "$INSTDIR\sqlite3.dll"
	IfRebootFlag reboot 0
	ExecWait '"$SYSDIR\regsvr32.exe" /s "$INSTDIR\SageThumbs.dll"'
	goto done
reboot:
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "SageThumbs" "regsvr32.exe /s $\"$INSTDIR\SageThumbs.dll$\""
done:
	WriteUninstaller "Uninst.exe"
	WriteRegStr HKCU "Software\SageThumbs" "Install${BITS}" $INSTDIR
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "DisplayName" "${TITLE} ${VERSION}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "UninstallString" "$INSTDIR\Uninst.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "DisplayIcon" "$INSTDIR\SageThumbs.dll,0"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "DisplayVersion" "${VERSION}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "Publisher" "${COMPANY}"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "NoRepair" 1
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
	CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
	CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\SageThumbs Options.lnk" "control.exe" "$\"$INSTDIR\SageThumbs.dll$\"" "$INSTDIR\SageThumbs.dll" 0
	CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Read Me.lnk" "$INSTDIR\readme.txt" "" "$INSTDIR\readme.txt" 0
	CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\License.lnk" "$INSTDIR\license.txt" "" "$INSTDIR\license.txt" 0
	CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall SageThumbs.lnk" "$INSTDIR\Uninst.exe" "" "$INSTDIR\Uninst.exe" 0
	!insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section "Uninstall"
	SetOutPath $TEMP

	Call un.FreeSageThumbs
	IfErrors wait1 ok
wait1:
	Call un.FreeSageThumbs
	IfErrors wait2 ok
wait2:
	Call un.FreeSageThumbs
	IfErrors wait3 ok
wait3:
	Call un.FreeSageThumbs
	IfErrors wait4 ok
wait4:
	Call un.FreeSageThumbs
	IfErrors fail ok
fail:
	Delete /REBOOTOK "$INSTDIR\SageThumbs.dll"
ok:
	Delete /REBOOTOK "$INSTDIR\SageThumbs19.dll"
	Delete /REBOOTOK "$INSTDIR\SageThumbs0c.dll"
	Delete /REBOOTOK "$INSTDIR\SageThumbs07.dll"
	Delete /REBOOTOK "$INSTDIR\libgfl340.dll"
	Delete /REBOOTOK "$INSTDIR\libgfle340.dll"
	Delete /REBOOTOK "$INSTDIR\sqlite3.dll"
	Delete /REBOOTOK "$INSTDIR\license.txt"
	Delete /REBOOTOK "$INSTDIR\readme.txt"
	Delete "$INSTDIR\*.tmp"

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

	ReadRegStr $0 HKCU "Software\SageThumbs" "Database"
	StrCmp $0 "" +2
	Delete /REBOOTOK "$0"
	Delete /REBOOTOK "$APPDATA\SageThumbs.db3"
	Delete /REBOOTOK "$WINDIR\SageThumbs.db3"
	Delete /REBOOTOK "$INSTDIR\SageThumbs.db3"
	
	!insertmacro MUI_STARTMENU_GETFOLDER Application $STARTMENU_FOLDER
	Delete "$SMPROGRAMS\$STARTMENU_FOLDER\*.*"
	RmDir  "$SMPROGRAMS\$STARTMENU_FOLDER"

	Delete /REBOOTOK "$INSTDIR\Uninst.exe"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}"
	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "${TITLE}"
	RMDir /REBOOTOK "$INSTDIR"
SectionEnd

!appendfile "setup${BITS}.trg" "[${__TIMESTAMP__}] ${TITLE} ${VERSION} ${FILENAME}$\n"
