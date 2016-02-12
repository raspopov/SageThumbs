@echo off
setlocal

for /F "usebackq tokens=3*" %%A IN (`reg query "HKEY_CURRENT_USER\SOFTWARE\7-Zip" /v Path`) do set appdir=%%A %%B
if exist "%appdir%" set app="%appdir%7z.exe"
if exist %app% goto found
set app="%ProgramFiles%\7-Zip\7z.exe"
if exist %app% goto found
set app="%ProgramFiles(x86)%\7-Zip\7z.exe"
if exist %app% goto found
set app="%ProgramW6432%\7-Zip\7z.exe"
if exist %app% goto found
set app="7z.exe"

:found
set dst="..\redist\sagethumbs_%1_source.7z"
del /q %dst% 2>nul
cd ..
md "..\redist"
%app% a -t7z -y -mx=9 -r -x!.svn -x!.vs -x!setup\*.exe -x!Debug -x!Release -x!Win32 -x!x64 -x!ipch -x!*.po_ -x!*.7z -x!*.log -x!*.bak -x!*.tmp -x!*.sdf -x!*.db -x!*.opendb -x!*.suo -x!*.ncb -x!*.user -x!*.opensdf -- %dst% .\
if %errorlevel% equ 9009 echo The 7-Zip utility is missing. Please go to http://www.7-zip.org/ and install 7-Zip.
exit /b %errorlevel%