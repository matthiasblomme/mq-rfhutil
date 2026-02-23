@echo off
REM Quick build script for mq-rfhutil
REM This script provides easy access to MSBuild commands
REM Supports both Win32 (32-bit) and x64 (64-bit) builds

set MSBUILD="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"

if "%1"=="" goto usage
if "%1"=="rfhutil" goto rfhutil
if "%1"=="rfhutil-x64" goto rfhutil_x64
if "%1"=="client" goto client
if "%1"=="client-x64" goto client_x64
if "%1"=="safe" goto safe
if "%1"=="safe-x64" goto safe_x64
if "%1"=="all" goto all
if "%1"=="all-x64" goto all_x64
if "%1"=="all-both" goto all_both
goto usage

:rfhutil
echo Building RFHUtil (server mode, Win32)...
%MSBUILD% RFHUtil.sln /t:RFHUtil:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
goto end

:rfhutil_x64
echo Building RFHUtil (server mode, x64)...
%MSBUILD% RFHUtil.sln /t:RFHUtil:Rebuild /p:Configuration=Release /p:Platform=x64 /v:minimal
goto end

:client
echo Building Client (rfhutilc, Win32)...
%MSBUILD% RFHUtil.sln /t:Client:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
goto end

:client_x64
echo Building Client (rfhutilc, x64)...
%MSBUILD% RFHUtil.sln /t:Client:Rebuild /p:Configuration=Release /p:Platform=x64 /v:minimal
goto end

:safe
echo Building Client Safe Mode (rfhutilc-safe, Win32)...
%MSBUILD% RFHUtil.sln /t:Client:Rebuild /p:Configuration=ReleaseSafe /p:Platform=Win32 /v:minimal
goto end

:safe_x64
echo Building Client Safe Mode (rfhutilc-safe, x64)...
%MSBUILD% RFHUtil.sln /t:Client:Rebuild /p:Configuration=ReleaseSafe /p:Platform=x64 /v:minimal
goto end

:all
echo Building all projects (Win32)...
%MSBUILD% RFHUtil.sln /t:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
goto end

:all_x64
echo Building all projects (x64)...
%MSBUILD% RFHUtil.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 /v:minimal
goto end

:all_both
echo =====================================
echo Building All Configurations
echo =====================================
echo.
echo Building Win32 RFHUtil...
%MSBUILD% RFHUtil.sln /t:RFHUtil:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
echo.
echo Building Win32 Client...
%MSBUILD% RFHUtil.sln /t:Client:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:minimal
echo.
echo Building Win32 Client Safe Mode...
%MSBUILD% RFHUtil.sln /t:Client:Rebuild /p:Configuration=ReleaseSafe /p:Platform=Win32 /v:minimal
echo.
echo Building x64 RFHUtil...
%MSBUILD% RFHUtil.sln /t:RFHUtil:Rebuild /p:Configuration=Release /p:Platform=x64 /v:minimal
echo.
echo Building x64 Client...
%MSBUILD% RFHUtil.sln /t:Client:Rebuild /p:Configuration=Release /p:Platform=x64 /v:minimal
echo.
echo Building x64 Client Safe Mode...
%MSBUILD% RFHUtil.sln /t:Client:Rebuild /p:Configuration=ReleaseSafe /p:Platform=x64 /v:minimal
echo.
echo =====================================
echo Build Complete!
echo =====================================
echo.
echo Output locations:
echo   Win32: bin\Release\
echo   x64:   bin\Release\x64\
goto end

:usage
echo Usage: build.cmd [command]
echo.
echo Win32 (32-bit) Commands:
echo   rfhutil      - Build rfhutil.exe (server mode, Win32)
echo   client       - Build rfhutilc.exe (client mode, Win32)
echo   safe         - Build rfhutilc-safe.exe (browse-only mode, Win32)
echo   all          - Build all projects (Win32)
echo.
echo x64 (64-bit) Commands:
echo   rfhutil-x64  - Build rfhutil.exe (server mode, x64)
echo   client-x64   - Build rfhutilc.exe (client mode, x64)
echo   safe-x64     - Build rfhutilc-safe.exe (browse-only mode, x64)
echo   all-x64      - Build all projects (x64)
echo.
echo Combined Commands:
echo   all-both     - Build all projects for both Win32 and x64
echo.
echo Examples:
echo   build.cmd rfhutil        (Build 32-bit server version)
echo   build.cmd rfhutil-x64    (Build 64-bit server version)
echo   build.cmd client         (Build 32-bit client version)
echo   build.cmd client-x64     (Build 64-bit client version)
echo   build.cmd all-both       (Build everything)
goto end

:end