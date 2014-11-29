setlocal

set zip="%ProgramFiles(x86)%\7-Zip\7z.exe"
if exist %zip% goto zip
set zip="%ProgramW6432%\7-Zip\7z.exe"
if exist %zip% goto zip
echo The 7z utility is missing. Please go to https://sourceforge.net/projects/sevenzip/ and install 7-Zip.
exit /b 1
:zip

set dst="..\redist\sagethumbs_%1_source.7z"
del /q %dst% 2>nul
cd ..
md "..\redist"
%zip% a -t7z -y -mx=9 -r -x!.svn -x!setup\*.exe -x!Win32 -x!x64 -x!ipch -x!*.po_ -x!*.7z -x!*.log -x!*.bak -x!*.tmp -x!*.sdf -x!*.suo -x!*.ncb -x!*.user -x!*.opensdf %dst% .\
exit /b 0