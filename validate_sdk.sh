#!/bin/bash

# Insta360 SDK Validation Script
# This script checks if the SDK is properly installed

echo "🔍 Validating Insta360 SDK installation..."

SDK_DIR="./sdk"
ERRORS=0

# Check if SDK directory exists
if [ ! -d "$SDK_DIR" ]; then
    echo "❌ SDK directory not found: $SDK_DIR"
    ERRORS=$((ERRORS + 1))
else
    echo "✅ SDK directory found"
fi

# Check include directory
INCLUDE_DIR="$SDK_DIR/include"
if [ ! -d "$INCLUDE_DIR" ]; then
    echo "❌ Include directory not found: $INCLUDE_DIR"
    ERRORS=$((ERRORS + 1))
else
    echo "✅ Include directory found"
fi

# Check required header files
REQUIRED_HEADERS=(
    "ins_common.h"
    "ins_stitcher.h"
    "ins_realtime_stitcher.h"
    "ins_version.h"
)

for header in "${REQUIRED_HEADERS[@]}"; do
    if [ ! -f "$INCLUDE_DIR/$header" ]; then
        echo "❌ Missing header file: $INCLUDE_DIR/$header"
        ERRORS=$((ERRORS + 1))
    else
        echo "✅ Header file found: $header"
    fi
done

# Check lib directory
LIB_DIR="$SDK_DIR/lib"
if [ ! -d "$LIB_DIR" ]; then
    echo "❌ Lib directory not found: $LIB_DIR"
    ERRORS=$((ERRORS + 1))
else
    echo "✅ Lib directory found"
fi

# Check main library file
MAIN_LIB="$LIB_DIR/libMediaSDK.so"
if [ ! -f "$MAIN_LIB" ]; then
    echo "❌ Main library not found: $MAIN_LIB"
    ERRORS=$((ERRORS + 1))
else
    echo "✅ Main library found: libMediaSDK.so"
    
    # Check library size (should be substantial)
    SIZE=$(stat -f%z "$MAIN_LIB" 2>/dev/null || stat -c%s "$MAIN_LIB" 2>/dev/null)
    if [ "$SIZE" -gt 1000000 ]; then
        echo "✅ Library size looks correct: $(($SIZE / 1024 / 1024)) MB"
    else
        echo "⚠️  Library size seems small: $(($SIZE / 1024)) KB - might be corrupted"
    fi
fi

# Check for common SDK problems
echo ""
echo "🔧 Checking for common issues..."

# Check if old/incorrect headers were referenced in main.cpp
if [ -f "app/main.cpp" ]; then
    if grep -q "ins_media.h\|ins_video_stitcher.h\|ins_photo_stitcher.h" app/main.cpp; then
        echo "⚠️  Old/incorrect header references found in main.cpp"
        echo "   These should be replaced with ins_stitcher.h and ins_common.h"
        ERRORS=$((ERRORS + 1))
    else
        echo "✅ No incorrect header references in main.cpp"
    fi
fi

# Final result
echo ""
if [ $ERRORS -eq 0 ]; then
    echo "🎉 SDK validation successful! All required files are present."
    echo "You can now run: docker build -t insta360-auto-converter ."
    exit 0
else
    echo "💥 SDK validation failed with $ERRORS error(s)."
    echo ""
    echo "To fix:"
    echo "1. Download the Insta360 Media SDK"
    echo "2. Extract it and copy the contents to ./sdk/"
    echo "3. Ensure the directory structure matches the README.md"
    exit 1
fi
