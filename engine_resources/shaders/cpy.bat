@echo off
setlocal

:: First arg = source dir, second arg = destination dir
set SRC=F:\github\kogayonon\engine_resources
set DST=F:\github\kogayonon\build\x64-debug\bin\engine_resources

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
