@echo off
setlocal

echo Cleaning...
call clean.cmd

echo Compiling...
call "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86
msbuild vc10\SageThumbs.sln /nologo /v:m /t:Rebuild /p:Configuration=Release /p:Platform=x64 /fl /flp:Summary;Verbosity=normal;LogFile=build.log

exit /b 0