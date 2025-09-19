# Insta360 SDK Validation Script (PowerShell)
# This script checks if the SDK is properly installed

Write-Host "🔍 Validating Insta360 SDK installation..." -ForegroundColor Cyan

$sdkDir = "./sdk"
$errors = 0

# Check if SDK directory exists
if (-Not (Test-Path $sdkDir)) {
    Write-Host "❌ SDK directory not found: $sdkDir" -ForegroundColor Red
    $errors++
} else {
    Write-Host "✅ SDK directory found" -ForegroundColor Green
}

# Check include directory
$includeDir = "$sdkDir/include"
if (-Not (Test-Path $includeDir)) {
    Write-Host "❌ Include directory not found: $includeDir" -ForegroundColor Red
    $errors++
} else {
    Write-Host "✅ Include directory found" -ForegroundColor Green
}

# Check required header files
$requiredHeaders = @(
    "ins_common.h",
    "ins_stitcher.h",
    "ins_realtime_stitcher.h",
    "ins_version.h"
)

foreach ($header in $requiredHeaders) {
    $headerPath = "$includeDir/$header"
    if (-Not (Test-Path $headerPath)) {
        Write-Host "❌ Missing header file: $headerPath" -ForegroundColor Red
        $errors++
    } else {
        Write-Host "✅ Header file found: $header" -ForegroundColor Green
    }
}

# Check lib directory
$libDir = "$sdkDir/lib"
if (-Not (Test-Path $libDir)) {
    Write-Host "❌ Lib directory not found: $libDir" -ForegroundColor Red
    $errors++
} else {
    Write-Host "✅ Lib directory found" -ForegroundColor Green
}

# Check main library file
$mainLib = "$libDir/libMediaSDK.so"
if (-Not (Test-Path $mainLib)) {
    Write-Host "❌ Main library not found: $mainLib" -ForegroundColor Red
    $errors++
} else {
    Write-Host "✅ Main library found: libMediaSDK.so" -ForegroundColor Green
    
    # Check library size (should be substantial)
    $size = (Get-Item $mainLib).Length
    if ($size -gt 1000000) {
        $sizeMB = [math]::Round($size / 1024 / 1024, 1)
        Write-Host "✅ Library size looks correct: $sizeMB MB" -ForegroundColor Green
    } else {
        $sizeKB = [math]::Round($size / 1024, 1)
        Write-Host "⚠️  Library size seems small: $sizeKB KB - might be corrupted" -ForegroundColor Yellow
    }
}

# Check for common SDK problems
Write-Host ""
Write-Host "🔧 Checking for common issues..." -ForegroundColor Cyan

# Check if old/incorrect headers were referenced in main.cpp
if (Test-Path "app/main.cpp") {
    $content = Get-Content "app/main.cpp" -Raw
    if ($content -match "ins_media\.h|ins_video_stitcher\.h|ins_photo_stitcher\.h") {
        Write-Host "⚠️  Old/incorrect header references found in main.cpp" -ForegroundColor Yellow
        Write-Host "   These should be replaced with ins_stitcher.h and ins_common.h" -ForegroundColor Yellow
        $errors++
    } else {
        Write-Host "✅ No incorrect header references in main.cpp" -ForegroundColor Green
    }
}

# Final result
Write-Host ""
if ($errors -eq 0) {
    Write-Host "🎉 SDK validation successful! All required files are present." -ForegroundColor Green
    Write-Host "You can now run: docker build -t insta360-auto-converter ." -ForegroundColor Green
    exit 0
} else {
    Write-Host "💥 SDK validation failed with $errors error(s)." -ForegroundColor Red
    Write-Host ""
    Write-Host "To fix:" -ForegroundColor Yellow
    Write-Host "1. Download the Insta360 Media SDK" -ForegroundColor Yellow
    Write-Host "2. Extract it and copy the contents to ./sdk/" -ForegroundColor Yellow
    Write-Host "3. Ensure the directory structure matches the README.md" -ForegroundColor Yellow
    exit 1
}
