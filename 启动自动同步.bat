@echo off
chcp 65001 >nul
title Cprog 自动同步工具
color 0A

echo ========================================
echo   Cprog 自动同步工具
echo ========================================
echo.
echo 正在启动自动同步...
echo 按 Ctrl+C 停止同步
echo.

powershell -ExecutionPolicy Bypass -File "%~dp0auto-sync.ps1"

pause
