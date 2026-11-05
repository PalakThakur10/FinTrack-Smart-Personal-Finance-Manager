@echo off
title FinTrack - C++ Backend Server
color 0A

echo.
echo  ================================================================
echo       FINTRACK - SMART PERSONAL FINANCE MANAGEMENT SYSTEM
echo       C++ OOPS Backend + REST APIs + In-Memory Storage
echo  ================================================================
echo.

cd /d "%~dp0"

echo  [1/3] Checking backend binary...
if not exist "backend\fintrack_server.exe" (
    echo  [!] Server not compiled. Run build.bat first.
    echo  [!] Meanwhile, opening in demo mode...
    start "" "%~dp0frontend\login.html"
    pause
    exit /b 1
)

echo  [2/3] Killing any existing process on port 8080...
for /f "tokens=5" %%a in ('netstat -aon 2^>nul ^| findstr ":8080 "') do (
    taskkill /PID %%a /F >nul 2>&1
)

echo  [3/3] Starting FinTrack C++ Server...
start "FinTrack C++ Server" /MIN cmd /c "cd /d %~dp0 && backend\fintrack_server.exe 8080"

echo  Waiting for server to initialize...
timeout /t 3 /nobreak >nul

echo.
echo  ================================================================
echo   SERVER RUNNING: http://localhost:8080
echo.
echo   DEMO LOGIN:
echo     Email   : demo@fintrack.com
echo     Password: Demo@123
echo  ================================================================
echo.
start "" "http://localhost:8080/login.html"
echo  [!] Keep this window open to keep the server running.
echo  [!] Press any key to stop the server and exit.
echo.
pause
taskkill /FI "WINDOWTITLE eq FinTrack C++ Server" /F >nul 2>&1
