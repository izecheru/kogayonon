@echo off
setlocal

:: First arg = source dir, second arg = destination dir
set SRC=D:\Github\kogayonon\resources
set DST=D:\Github\kogayonon\out\build\x64-debug\bin\resources

echo Copying resources from %SRC% to %DST%
:: Create destination if missing
if not exist "%DST%" mkdir "%DST%"

:: Use robocopy to copy everything except models/
::robocopy "%SRC%" "%DST%" /E /XD "%SRC%\models"
robocopy "%SRC%" "%DST%" /E

if %ERRORLEVEL% GEQ 8 (
    echo Robocopy failed with error %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo Copy completed successfully
endlocal