@echo off
if "%~1"=="" goto prompt
:next
if "%~1"=="" goto done
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Build-TextureCache.ps1" -Archive "%~f1"
if errorlevel 1 goto done
shift
goto next
:prompt
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Build-TextureCache.ps1"
:done
pause
