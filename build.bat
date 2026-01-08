@echo off
setlocal

:: UPDATE YOUR NDK PATH HERE
set "NDK_ROOT=UPDATE YOUR NDK PATH HERE"
set "LLVM_AR=%NDK_ROOT%\toolchains\llvm\prebuilt\windows-x86_64\bin\llvm-ar.exe"

echo ----------------------------------------------------
echo         BUILD KEYAUTH FAT STATIC LIBRARY
echo ----------------------------------------------------

echo [*] Building Core Library...
call ndk-build.cmd -C KeyAuth_Library -B
if %ERRORLEVEL% NEQ 0 goto :error

echo [*] Merging all dependencies into 1 file...
powershell -ExecutionPolicy Bypass -File "%~dp0build_logic.ps1"
if %ERRORLEVEL% NEQ 0 goto :error

echo [*] Updating Prebuilt and Example...
mkdir prebuilt\arm64-v8a 2>nul
copy /Y libkeyauth.a prebuilt\arm64-v8a\
copy /Y KeyAuth_Library\jni\include\KeyAuth.hpp prebuilt\
xcopy /E /I /Y KeyAuth_Library\jni\include\Json prebuilt\Json

mkdir KeyAuth_Example\jni\libs\arm64-v8a 2>nul
copy /Y libkeyauth.a KeyAuth_Example\jni\libs\arm64-v8a\
copy /Y KeyAuth_Library\jni\include\KeyAuth.hpp KeyAuth_Example\jni\include\
xcopy /E /I /Y KeyAuth_Library\jni\include\Json KeyAuth_Example\jni\include\Json

echo [*] Verifying build with Example Project...
call ndk-build.cmd -C KeyAuth_Example -B
if %ERRORLEVEL% NEQ 0 goto :error

echo.
echo [+] SUCCESS! Fat library is located at: prebuilt\arm64-v8a\libkeyauth.a
goto :end

:error
echo.
echo [!] AN ERROR OCCURRED DURING BUILD!
pause
exit /b 1

:end
pause
