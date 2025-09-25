#!/bin/bash

# Insta360 Batch Converter - WSL Launch Script
# Dual Mode: Single Run or Continuous Watch

# Parse command line arguments
WATCH_MODE=false
SHOW_HELP=false

for arg in "$@"; do
    case $arg in
        -w|--watch)
            WATCH_MODE=true
            shift
            ;;
        -h|--help)
            SHOW_HELP=true
            shift
            ;;
        *)
            ;;
    esac
done

# Show help if requested
if [ "$SHOW_HELP" = true ]; then
    echo "🎯 Insta360 Auto Converter - Dual Mode"
    echo "======================================"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "OPTIONS:"
    echo "  (no args)     Single Run Mode - Process all files once and exit"
    echo "  -w, --watch   Watch Mode - Continuous monitoring for new files"
    echo "  -h, --help    Show this help message"
    echo ""
    echo "EXAMPLES:"
    echo "  $0                # Single batch processing"
    echo "  $0 --watch        # Continuous monitoring"
    echo "  $0 -w             # Continuous monitoring (short form)"
    echo ""
    exit 0
fi

# Display mode
if [ "$WATCH_MODE" = true ]; then
    echo "🔄 Insta360 Auto Converter - WATCH MODE"
    echo "======================================="
    echo "Continuous monitoring enabled - will detect new files automatically"
else
    echo "🎯 Insta360 Auto Converter - SINGLE RUN MODE"
    echo "============================================="
    echo "Single batch processing - will process all files once and exit"
fi

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
if ! docker-compose -f docker-compose-wsl.yml build; then
    echo "❌ Docker build failed. Retrying with --no-cache..."
    docker-compose -f docker-compose-wsl.yml build --no-cache
fi

echo ""

# Launch based on selected mode
if [ "$WATCH_MODE" = true ]; then
    echo "� Starting WATCH MODE..."
    echo "   - Continuous monitoring for new files"
    echo "   - Native resolution: 11904x5952 (70.9MP)"
    echo "   - Automatic 360° EXIF metadata injection"
    echo "   - Press Ctrl+C to stop"
    echo ""
    
    # Clean up any existing watch container
    docker rm -f insta360-watch-mode 2>/dev/null || true
    
    # Run watch mode with Docker
    docker run -it --rm \
        -v "$INPUT_DIR":/data/input \
        -v "$OUTPUT_DIR":/data/output \
        -v "$CONFIG_DIR":/data/config \
        -e DISPLAY=:99 \
        --name insta360-watch-mode \
        --entrypoint=/bin/bash \
        insta360-auto-converter:latest \
        -c "Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render -noreset & sleep 2 && echo 'Watch mode starting...' && /app/build/insta360_batch_processor /data/input /data/output /data/config/config.json --watch"
else
    echo "🎯 Starting SINGLE RUN MODE..."
    echo "   - Process all existing files once"
    echo "   - Native resolution: 11904x5952 (70.9MP)"  
    echo "   - Automatic 360° EXIF metadata injection"
    echo "   - Will exit after processing"
    echo ""
    
    # Run single mode with Docker Compose
    docker-compose -f docker-compose-wsl.yml up
fi

echo ""
echo "✅ Batch processor stopped"