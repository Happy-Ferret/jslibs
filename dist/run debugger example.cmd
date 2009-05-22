@echo off
set PATH=%PATH%;%CD%\bin
start bin/debugger.xul
jshost bin/debugger.js examples/helloworld.js
