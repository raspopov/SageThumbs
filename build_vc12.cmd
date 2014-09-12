@echo off
setlocal

echo Cleaning...
call clean.cmd

echo Compiling...
call "%VS120COMNTOOLS%..\..\VC\vcvarsall.bat" x86
msbuild vc12\SageThumbs.sln /nologo /v:m /t:Rebuild /p:Configuration=Release /p:Platform=x64 /fl /flp:Summary;Verbosity=normal;LogFile=build.log

exit /b 0