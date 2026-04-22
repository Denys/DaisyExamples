@echo off
setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0build_host.ps1" %*
exit /b %errorlevel%
