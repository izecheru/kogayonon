@echo off
setlocal enabledelayedexpansion

echo Formatting all C++ files...

for /r %%f in (*.cpp *.hpp *.c *.h) do (
    set "file=%%f"
    set "exclude=0"
    
    echo !file! | findstr /i /c:"dependencies" >nul && set exclude=1
    echo !file! | findstr /i /c:"build" >nul && set exclude=1
    echo !file! | findstr /i /c:"_deps" >nul && set exclude=1
    
    if !exclude! equ 0 (
        echo Formatting: !file!
        clang-format -i -style=file "!file!"
    )
)

echo Formatting complete!
pause
