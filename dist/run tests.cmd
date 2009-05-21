@echo off
set PATH=%PATH%;%CD%\bin
jshost -u .\tests\qa.js -dir ./tests arg1 arg2 arg3
pause
