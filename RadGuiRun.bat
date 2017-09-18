@title %*
@ConIcon %1
@call %*
@echo off
echo.
echo Exitcode: %ERRORLEVEL%
ConFlash
if /I "%RADGUI_PAUSE%" == "true" pause
