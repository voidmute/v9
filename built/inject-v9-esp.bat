@echo off
cd /d "%~dp0"
if not exist "v9.dll" (
  echo v9.dll not found. Run v9\build.bat first.
  pause
  exit /b 1
)
echo Injecting v9 ESP...
"%~dp0v9injector.exe" PenguinHotel-Win64-Shipping.exe "%~dp0v9.dll"
if %ERRORLEVEL% EQU 0 (
  echo OK. INSERT = menu, END = unload
) else (
  echo Failed. Start the game first or run as Administrator.
)
pause