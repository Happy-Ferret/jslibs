@echo off
set PATH=%PATH%;%CD%\bin
cd examples
start debugger.xul
jshost debugger.js helloworld.js
