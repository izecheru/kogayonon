@echo off
setlocal

:: First arg = source dir, second arg = destination dir
set SRC=H:\Git\kogayonon\resources
set DST=H:\Git\kogayonon\out\build\x64-Debug\bin\resources

echo Copying resources from %SRC% to %DST% (excluding 'models')

:: Create destination if missing
if not exist "%DST%" mkdir "%DST%"

:: Use robocopy to copy everything except models/
robocopy "%SRC%" "%DST%" /E /XD "%SRC%\models"

if %ERRORLEVEL% GEQ 8 (
    echo Robocopy failed with error %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo Copy completed successfully
endlocal
