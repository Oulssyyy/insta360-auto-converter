@echo off
REM Insta360 Auto Converter - Windows Launcher
REM Calls the WSL bash script with proper arguments

setlocal

REM Parse arguments
set WATCH_MODE=false
set SHOW_HELP=false

:parse_args
if "%~1"=="" goto end_parse
if "%~1"=="-w" set WATCH_MODE=true
if "%~1"=="--watch" set WATCH_MODE=true
if "%~1"=="-h" set SHOW_HELP=true
if "%~1"=="--help" set SHOW_HELP=true
shift
goto parse_args
:end_parse

REM Show help
if "%SHOW_HELP%"=="true" (
    echo 🎯 Insta360 Auto Converter - Dual Mode
    echo ======================================
    echo.
    echo Usage: %~n0 [OPTIONS]
    echo.
    echo OPTIONS:
    echo   ^(no args^)     Single Run Mode - Process all files once and exit
    echo   -w, --watch   Watch Mode - Continuous monitoring for new files
    echo   -h, --help    Show this help message
    echo.
    echo EXAMPLES:
    echo   %~n0                # Single batch processing
    echo   %~n0 --watch        # Continuous monitoring  
    echo   %~n0 -w             # Continuous monitoring ^(short form^)
    echo.
    goto end
)

REM Display mode
if "%WATCH_MODE%"=="true" (
    echo 🔄 Launching WATCH MODE...
    echo Continuous file monitoring enabled
    echo.
    wsl bash -c "cd /mnt/c/Users/stf039/Documents/GitHub/insta360-auto-converter && ./start-batch-wsl.sh --watch"
) else (
    echo 🎯 Launching SINGLE RUN MODE...
    echo Single batch processing
    echo.
    wsl bash -c "cd /mnt/c/Users/stf039/Documents/GitHub/insta360-auto-converter && ./start-batch-wsl.sh"
)

:end
endlocal