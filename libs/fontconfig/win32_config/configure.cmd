@echo off
call ..\..\..\envbuild.cmd
pushd ..\src
set LD=link
set CC=cl
set CXX=cl


sh -c "./configure --help" > ..\win32_config\configure.txt


sh -c "./configure --disable-docs --disable-dependency-tracking --disable-shared --with-freetype-config=../../freetype/src --enable-libxml2 LIBXML2_CFLAGS=xx LIBXML2_LIBS=yy"



pause
