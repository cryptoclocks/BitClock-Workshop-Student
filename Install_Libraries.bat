@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem Install libraries bundled with this workshop package for Arduino IDE on Windows.
rem Double-click this file, then reopen Arduino IDE.

set "PACKAGE_DIR=%~dp0"
set "SOURCE_DIR=%PACKAGE_DIR%bundled-libraries"
set "DESTINATION_DIR=%USERPROFILE%\Documents\Arduino\libraries"

if not exist "%SOURCE_DIR%" (
  echo ไม่พบโฟลเดอร์ bundled-libraries
  pause
  exit /b 1
)

if not exist "%DESTINATION_DIR%" mkdir "%DESTINATION_DIR%"

set /a INSTALLED=0
set /a SKIPPED=0
for /D %%D in ("%SOURCE_DIR%\*") do (
  if exist "%DESTINATION_DIR%\%%~nxD" (
    echo ข้าม %%~nxD ^(มีอยู่แล้ว^)
    set /a SKIPPED+=1
  ) else (
    robocopy "%%~fD" "%DESTINATION_DIR%\%%~nxD" /E /NFL /NDL /NJH /NJS /NC /NS >nul
    if errorlevel 8 (
      echo ติดตั้ง %%~nxD ไม่สำเร็จ
      pause
      exit /b 1
    )
    echo ติดตั้ง %%~nxD
    set /a INSTALLED+=1
  )
)

echo.
echo เสร็จแล้ว: ติดตั้ง !INSTALLED! ไลบรารี, ข้าม !SKIPPED! ไลบรารี
echo ปิดและเปิด Arduino IDE ใหม่ แล้วเลือก Board: ESP32 Dev Module
pause
