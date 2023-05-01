::Force change to batch file's directory first
::(Starts in system32 on some computers)
cd /d %~dp0
@echo off
cls

::Obtain Windows and appropriate system folder
set dirwin=%WINDIR%
if "%PROGRAMFILES(X86)%"=="" (set dirsys=system32) else (set dirsys=syswow64)

::The following path variables will all end with a backslash
set dirdest=%dirwin%\%dirsys%\
set dirsource=%~dp0

set exitcode=0
call :regfull msvbvm60.dll
if %exitcode% NEQ 0 goto :EOF
call :regfull MSCOMM32.OCX
if %exitcode% NEQ 0 goto :EOF

echo.
echo - Success! Press any key to exit...
goto :EOF


:regfull
::Built-in subroutine version of old regfull.bat, thanks to DJ3520
set filesource=%1
echo.
echo SOURCE FILE     = %filesource%
echo SOURCE DIR      = %dirsource%
echo DESTINATION DIR = %dirdest%
::Change to destination directory to see if file exists
::(exist "dir\file" fails on Windows NT/XP).
cd /d %dirdest%

if exist %filesource% (
    echo - File already exists in destination; skipping copying.
    ::Change back to working directory.
    cd /d %dirsource%
) else (
    echo - Copying %filesource% to %dirdest%
    ::Change back to working directory before copying...
    cd /d %dirsource%
    copy "%dirsource%%filesource%" "%dirdest%%filesource%"
)

echo - Adding info for %filesource% to the Windows registry...
regsvr32.exe /s "%dirdest%%filesource%"
set exitcode=%ERRORLEVEL%
echo   (Exit code is %exitcode%)
if %exitcode% NEQ 0 goto failstart
goto :EOF

:failstart
::Always show this text as an error header
echo.
echo --------------------------------
echo - Failed to sort %filesource%  -
echo --------------------------------
::Optionally show appropriate error regarding missing file
if %exitcode% EQU 3 goto failnotfound

:failelse
::Always show this text as an error footer
echo Please make sure you run this BAT file with admin privileges... 
echo - Right-click this BAT file and choose 'Run as administrator'.
echo - Alternatively, log on to Windows using an administrator account
echo   and run this BAT file.
echo.
pause
::Calling code now looks at exitcode returned by regsvr32, instead of our own.
goto :EOF

:failnotfound
echo That DLL/OCX file could not be seen in the same folder as this BAT file.
echo Please make sure that the whole Bawami archive is extracted to the
echo same folder (the DLL/OCX file, this BAT file, and BaWaMI.exe must be in
echo the same folder). Afterwards:
echo.
goto failelse
