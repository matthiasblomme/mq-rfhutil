@echo off
REM Quick build script for mq-rfhutil
REM This script provides easy access to MSBuild commands

set MSBUILD="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"

if "%1"=="" goto usage
if "%1"=="rfhutil" goto rfhutil
if "%1"=="client" goto client
if "%1"=="safe" goto safe
if "%1"=="all" goto all
goto usage

:rfhutil
echo Building RFHUtil (server mode)...
%MSBUILD% RFHUtil.sln /t:RFHUtil:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
goto end

:client
echo Building Client (rfhutilc)...
%MSBUILD% RFHUtil.sln /t:Client:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
goto end

:safe
echo Building Client Safe Mode (rfhutilc-safe)...
%MSBUILD% RFHUtil.sln /t:Client:Rebuild /p:Configuration=ReleaseSafe /p:Platform=Win32 /v:minimal
goto end

:all
echo Building all projects...
%MSBUILD% RFHUtil.sln /t:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
goto end

:usage
echo Usage: build.cmd [rfhutil^|client^|safe^|all]
echo.
echo   rfhutil  - Build rfhutil.exe (server mode)
echo   client   - Build rfhutilc.exe (client mode)
echo   safe     - Build rfhutilc-safe.exe (browse-only mode)
echo   all      - Build all projects
echo.
echo Examples:
echo   build.cmd rfhutil
echo   build.cmd client
echo   build.cmd safe
echo   build.cmd all
goto end

:end