@echo off
call ..\..\..\envbuild.cmd

set LD=link
set CC=cl
set CXX=cl

pushd ..\src
sh ./configure --help > ..\win32_config\configuration.txt
pause
