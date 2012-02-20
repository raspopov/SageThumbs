!define TITLE		"SageThumbs Repair Utility"
!define VERSION		"2.0.0.15"
!define COMPANY		"Cherubic Software"
!define FILENAME	"repair.exe"
!define COPYRIGHT	"Copyright © 2004-2012 Nikolay Raspopov"
!define URL			"http://www.cherubicsoft.com/"
!define PAD			"http://sagethumbs.googlecode.com/svn/trunk/sagethumbs.xml"

SetCompressor /SOLID lzma

Caption "${TITLE}"
Name "${TITLE}"
Icon "repair.ico"
InstallButtonText "Repair"
SubCaption 3 " : Repairing..."
Page custom nsDialogsPage
Page instfiles
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
BrandingText " "
SetOverwrite Try
OutFile "${FILENAME}"
InstallDir "$PROGRAMFILES\SageThumbs"
InstallDirRegKey HKCU "Software\SageThumbs" "Install"
ShowInstDetails show
RequestExecutionLevel admin

!include "x64.nsh"
!include "nsDialogs.nsh"
!include "LogicLib.nsh"

Function nsDialogsPage
	nsDialogs::Create 1018
	${NSD_CreateLabel} 0 0 100% 14u "This utility will repair SageThumbs registration."
	nsDialogs::Show
FunctionEnd

Section
	SectionIn RO

	DetailPrint "Cleaning 32-bit..."
	IfFileExists "$INSTDIR\32\SageThumbs.dll" +3 0
	DetailPrint "Repair 32-bit failed! Missed file: $INSTDIR\32\SageThumbs.dll"
	Goto +3
	ExecWait '"$SYSDIR\regsvr32.exe" /s /u "$INSTDIR\32\SageThumbs.dll"'
	ExecWait '"$SYSDIR\regsvr32.exe" /s /u "$INSTDIR\32\SageThumbs.dll"'

	${If} ${RunningX64}
		DetailPrint "Cleaning 64-bit..."
		IfFileExists "$INSTDIR\64\SageThumbs.dll" +3 0
		DetailPrint "Repair 64-bit failed! Missed file: $INSTDIR\64\SageThumbs.dll"
		Goto +3
		ExecWait '"$SYSDIR\regsvr32.exe" /s /u "$INSTDIR\64\SageThumbs.dll"'
		ExecWait '"$SYSDIR\regsvr32.exe" /s /u "$INSTDIR\64\SageThumbs.dll"'
	${EndIf}

	IfFileExists "$INSTDIR\32\SageThumbs.dll" 0 +5
	DetailPrint "Repairing 32-bit..."
	ExecWait '"$SYSDIR\regsvr32.exe" /s "$INSTDIR\32\SageThumbs.dll"' $0
	IntCmp $0 0 +3
	DetailPrint "Repair 32-bit failed!"
	Goto +2
	DetailPrint "Repair 32-bit succeded."

	${If} ${RunningX64}
		IfFileExists "$INSTDIR\64\SageThumbs.dll" 0 +5
		DetailPrint "Repairing 64-bit..."
		ExecWait '"$SYSDIR\regsvr32.exe" /s "$INSTDIR\64\SageThumbs.dll"' $0
		IntCmp $0 0 +3
		DetailPrint "Repair 64-bit failed!"
		Goto +2
		DetailPrint "Repair 64-bit succeded."
	${EndIf}

SectionEnd

Function .onInit
	SetShellVarContext all
	SetRegView 64
FunctionEnd

!appendfile "repair.trg" "[${__TIMESTAMP__}] ${TITLE} ${VERSION} ${FILENAME}$\n"
