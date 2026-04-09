@echo off
chcp 65001 >nul
title Cprog 手动同步
color 0B

set GIT=D:\Tools\Git\bin\git.exe
set PROJECT=D:\我的项目\Cprog

cd /d "%PROJECT%"

echo ========================================
echo   Cprog 手动同步
echo ========================================
echo.

echo [1/4] 检查文件更改...
%GIT% status --short

echo.
echo [2/4] 添加更改...
%GIT% add -A

echo.
echo [3/4] 提交更改...
set TIMESTAMP=%date:~0,4%-%date:~5,2%-%date:~8,2% %time:~0,2%:%time:~3,2%:%time:~6,2%
%GIT% commit -m "Manual sync: %TIMESTAMP%"

echo.
echo [4/4] 推送到 GitHub...
%GIT% push origin main

if %ERRORLEVEL% == 0 (
    echo.
    echo ✅ 同步成功！
) else (
    echo.
    echo ❌ 同步失败，请检查网络连接
)

echo.
pause
