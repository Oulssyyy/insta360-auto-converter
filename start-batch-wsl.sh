#!/bin/bash

# Insta360 Batch Converter - WSL Launch Script
# No more file duplication! Detection based on output directory comparison.

echo "🎯 Insta360 Auto Converter - Smart Detection"
echo "============================================="

# Configuration
INPUT_DIR="/mnt/c/Users/stf039/Pictures/insta360"
OUTPUT_DIR="/mnt/c/Users/stf039/Pictures/output"
CONFIG_DIR="/mnt/c/Users/stf039/Pictures/config"

# Create necessary directories
echo "📁 Creating directories..."
mkdir -p "$OUTPUT_DIR"
mkdir -p "$CONFIG_DIR"

# Copy config if not exists
if [ ! -f "$CONFIG_DIR/config.json" ]; then
    echo "📋 Creating default configuration..."
    cp config.json "$CONFIG_DIR/"
fi

echo "📂 Directories:"
echo "   Input:  $INPUT_DIR"
echo "   Output: $OUTPUT_DIR"
echo "   Config: $CONFIG_DIR"
echo ""

# Check if input directory exists
if [ ! -d "$INPUT_DIR" ]; then
    echo "❌ Input directory not found: $INPUT_DIR"
    echo "   Please create it and add your .insv/.insp files"
    exit 1
fi

echo "🔨 Building Docker image..."
docker-compose -f docker-compose-wsl.yml build

echo ""
echo "🚀 Starting batch processor..."
echo "   - No more file duplication in processed folder!"
echo "   - Detection based on output directory comparison"
echo "   - Will automatically add 360° EXIF metadata"
echo ""

# Launch the batch processor
docker-compose -f docker-compose-wsl.yml up

echo ""
echo "✅ Batch processor stopped"