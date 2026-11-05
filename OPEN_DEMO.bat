@echo off
title FinTrack - Demo Mode (No Server Required)
echo.
echo  ================================================================
echo       FINTRACK - SMART PERSONAL FINANCE MANAGEMENT SYSTEM
echo       Opening in DEMO MODE (No server required)
echo  ================================================================
echo.
echo  Demo credentials:
echo    Email   : demo@fintrack.com
echo    Password: Demo@123
echo.
echo  Opening FinTrack login page...
start "" "%~dp0frontend\login.html"
echo.
echo  Done! The app opened in your browser.
echo  All features work with built-in demo data.
timeout /t 3 /nobreak >nul

