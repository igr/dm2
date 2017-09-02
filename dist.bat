@echo off
SET version=1.22
echo.
echo DM2 v%version%
echo.

del /Q dist\*

echo [prepare]
copy /Y dm2\Release\* release\
copy /Y dm2\*.txt release\doc\
copy /Y language\*.lang release\language\

echo [release]
cd release
zip -9 -q -r ../dist/dm2-%version%.zip *.*
cd..
echo done.
echo.

echo [source]
zip -9 -q -r dist/dm2-all-%version%.zip release dm2-plugins\*.* dm2\*.* -xi *.ncb *.opt *.plg *\.svn\* dm2\DM2???\Release\* dm2\DM2???\Debug\*
echo done.
echo.

set version=

