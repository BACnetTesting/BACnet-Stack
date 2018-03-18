@echo off

rem Indent the C and H files with specific coding standard
rem requires 'astyle.exe' from MSYS (MinGW).

rem call :treeProcess
call :safedirectories
goto :eof

:safedirectories
cd src
call :treeProcess
cd ../demo
call :treeProcess
cd ../include
call :treeProcess
cd ../ConnectExUtil
call :treeProcess
cd ..
goto :eof

:treeProcess
rem perform the indent on all the files of this subdirectory:
for %%f in (*.c) do (
rem    \programs\astyle.exe "%%f" -o "%%f" %OPTIONS%
    \programs\astyle.exe --style=kr --indent=spaces=4 "%%f"
echo %%f
)
for %%f in (*.h) do (
rem   indent.exe "%%f" -o "%%f" %OPTIONS%
    \programs\astyle.exe --style=kr --indent=spaces=4 "%%f"
 echo %%f
)

rem loop over all directories and sub directories
for /D %%d in (*) do (
		cd %%d
		call :treeProcess
		cd ..
)
exit /b


