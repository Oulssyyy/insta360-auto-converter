@echo off
setlocal

REM Insta360 Batch Converter - Windows Launch Script
REM No more file duplication! Detection based on output directory comparison.

echo 🎯 Insta360 Auto Converter - Smart Detection
echo =============================================

REM Configuration
set INPUT_DIR=/mnt/c/Users/stf039/Pictures/insta360
set OUTPUT_DIR=/mnt/c/Users/stf039/Pictures/output
set CONFIG_DIR=/mnt/c/Users/stf039/Pictures/config

echo 📁 Creating directories...
if not exist "C:\Users\stf039\Pictures\output" mkdir "C:\Users\stf039\Pictures\output"
if not exist "C:\Users\stf039\Pictures\config" mkdir "C:\Users\stf039\Pictures\config"

REM Copy config if not exists
if not exist "C:\Users\stf039\Pictures\config\config.json" (
    echo 📋 Creating default configuration...
    copy config.json "C:\Users\stf039\Pictures\config\"
)

echo 📂 Directories:
echo    Input:  %INPUT_DIR%
echo    Output: %OUTPUT_DIR%
echo    Config: %CONFIG_DIR%
echo.

REM Check if input directory exists
if not exist "C:\Users\stf039\Pictures\insta360" (
    echo ❌ Input directory not found: C:\Users\stf039\Pictures\insta360
    echo    Please create it and add your .insv/.insp files
    pause
    exit /b 1
)

echo 🔨 Building Docker image...
docker-compose -f docker-compose-wsl.yml build

echo.
echo 🚀 Starting batch processor...
echo    - No more file duplication in processed folder!
echo    - Detection based on output directory comparison
echo    - Will automatically add 360° EXIF metadata
echo.

REM Launch the batch processor
docker-compose -f docker-compose-wsl.yml up

echo.
echo ✅ Batch processor stopped
pause