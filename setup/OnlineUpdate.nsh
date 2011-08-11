/*

  OnlineUpdate 0.2
  by Jan T. Sott
  
  Let the installer check for newer versions (using RSS, PAD or INI files)
  
  Usage in script:
  1. !include "OnlineUpdate.nsh"
  2. !insertmacro UpdateFunction
  3. [Section|Function]
     ${VersionFunction} "Param1" "Param2" "..." $URL $Version
     [SectionEnd|FunctionEnd]
     
  Requirements:
  1. NSISdl (built-in)
  2. XML Plugin http://nsis.sourceforge.net/XML_plug-in
  3. WordFunc.nsh (VersionCompare) for UpdatePad
  
  Thanks to Kichik and Instructor :)
     
*/

!ifndef ONLINEUPDATE_INCLUDED
!define ONLINEUPDATE_INCLUDED

!verbose push
!verbose 3
!ifndef _ONLINEUPDATE_VERBOSE
	!define _ONLINEUPDATE_VERBOSE 3
!endif
!verbose ${_ONLINEUPDATE_VERBOSE}
!define ONLINEUPDATE_VERBOSE `!insertmacro ONLINEUPDATE_VERBOSE`
!define _ONLINEUPDATE_UN
!define _ONLINEUPDATE_S
!verbose pop

!macro ONLINEUPDATE_VERBOSE _VERBOSE
	!verbose push
	!verbose 3
	!undef _ONLINEUPDATE_VERBOSE
	!define _ONLINEUPDATE_VERBOSE ${_VERBOSE}
	!verbose pop
!macroend

!macro UpdateXmlCall _URL _PATH _RESULT_URL _RESUTL_VERSION
       !verbose push
       !verbose ${_ONLINEUPDATE_VERBOSE}
       Push `${_URL}`
       Push `${_PATH}`
       Call UpdateXml
       Pop `${_RESUTL_VERSION}`
       Pop `${_RESULT_URL}`
       !verbose pop
!macroend

!macro UpdatePadCall _URL _LOCALVERSION _RESULT_URL _RESUTL_VERSION
       !verbose push
       !verbose ${_ONLINEUPDATE_VERBOSE}
       Push `${_URL}`
       Push `${_LOCALVERSION}`
       Call UpdatePad
       Pop `${_RESUTL_VERSION}`
       Pop `${_RESULT_URL}`
       !verbose pop
!macroend

!macro UpdateXml
	!ifndef ${_ONLINEUPDATE_UN}UpdateXml${_ONLINEUPDATE_S}
		!verbose push
		!verbose ${_ONLINEUPDATE_VERBOSE}
		!define ${_ONLINEUPDATE_UN}UpdateXml${_ONLINEUPDATE_S} `!insertmacro ${_ONLINEUPDATE_UN}UpdateXml${_ONLINEUPDATE_S}Call`

		Function  UpdateXml
			  Exch $1
			  Exch
			  Exch $0

			  Push $2
			  Push $3

			  ;download XML file
			  GetTempFileName $3
			  NSISdl::download $0 $3
			  Pop $2

			  ;parse XML file
			  ${xml::LoadFile} $3 $2
			  ${xml::GotoPath} $1 $2
			  ${xml::GetText} $0 $1

			  ;delete temp file
			  Delete $3

			  ;unload
			  ${xml::NodeHandle} $2
			  ${xml::Unload}

			  Pop $3
			  Pop $2

			  Exch $0
			  Exch
			  Exch $1
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro UpdatePad
	!ifndef ${_ONLINEUPDATE_UN}UpdatePad${_ONLINEUPDATE_S}
		!verbose push
		!verbose ${_ONLINEUPDATE_VERBOSE}
		!define ${_ONLINEUPDATE_UN}UpdatePad${_ONLINEUPDATE_S} `!insertmacro ${_ONLINEUPDATE_UN}UpdatePad${_ONLINEUPDATE_S}Call`

		Function  UpdatePad
			  Exch $1
			  Exch
			  Exch $0

			  Push $2
			  Push $3
			  Push $4

			  ;download XML file
			  GetTempFileName $4
			  NSISdl::download $0 $4
			  Pop $2

			  ;parse XML file
			  ${xml::LoadFile} $4 $2
			  ${xml::GotoPath} "/XML_DIZ_INFO/Program_Info/Program_Version" $2
			  ${xml::GetText} $3 $2

			  ;compare Version
			  ${VersionCompare} $1 $3 $0
			  StrCpy $1 $3
			  StrCmp $0 "2" 0 NoUpdate
			  ${xml::GotoPath} "/XML_DIZ_INFO/Web_Info/Download_URLs/Primary_Download_URL" $2
			  ${xml::GetText} $0 $2
			  Goto +2

			  NoUpdate:
			  StrCpy $0 ""
			  
			  ;delete temp file
			  Delete $4

			  ;unload
			  ${xml::NodeHandle} $2
			  ${xml::Unload}

			  Pop $4
			  Pop $3
			  Pop $2

			  Exch $0
			  Exch 
			  Exch $1
		FunctionEnd

		!verbose pop
	!endif
!macroend


!endif