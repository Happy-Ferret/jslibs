@echo off
set PATH=%PATH%;%CD%\bin
jshost -u ./tests/qa.js -repeat 3 -dir ./tests
echo.
pause
