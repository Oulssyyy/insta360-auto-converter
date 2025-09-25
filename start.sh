#!/bin/bash

# 🎯 Insta360 Auto Converter - Script Unifié pour WSL/Ubuntu
# Modes: Single Run ou Watch Continu avec jonctions optimisées

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INPUT_DIR="/mnt/c/Users/stf039/Pictures/insta360"
OUTPUT_DIR="/mnt/c/Users/stf039/Pictures/output"
CONFIG_DIR="/mnt/c/Users/stf039/Pictures/config"

# Variables de contrôle
WATCH_MODE=false
SHOW_HELP=false

# Fonction d'aide
show_help() {
    cat << EOF
🎯 Insta360 Auto Converter - Unified Ubuntu/WSL Script
======================================================

✨ NEW: Junction optimization with OPTFLOW algorithm
   Eliminates blur at extremities for quality identical to Insta360 Studio

USAGE: $0 [OPTIONS]

OPTIONS:
  (no args)     Single Mode - Process all files once and exit
  -w, --watch   Watch Mode - Continuous monitoring for new files
  -h, --help    Show this help

EXAMPLES:
  $0                # Single batch processing
  $0 --watch        # Continuous monitoring
  $0 -w             # Continuous monitoring (short form)

QUALITY:
  • Native resolution: 11904x5952 (70.9MP)
  • OPTFLOW algorithm for perfect junctions
  • 360° metadata preserved for Synology Photos
  • EnableStitchFusion to eliminate edge blur
EOF
}

# Parse des arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -w|--watch)
            WATCH_MODE=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo "❌ Unknown argument: $1"
            echo "Use -h or --help for help"
            exit 1
            ;;
    esac
done

# Display mode
echo "🚀 Insta360 Auto Converter - $([ "$WATCH_MODE" = true ] && echo "WATCH MODE" || echo "SINGLE MODE")"
echo "=================================================================="
if [ "$WATCH_MODE" = true ]; then
    echo "🔄 Continuous monitoring enabled - automatic detection of new files"
else
    echo "🎯 Single batch processing - processes all files and exits"
fi
echo ""

# Create directories
echo "📁 Creating directories..."
mkdir -p "$OUTPUT_DIR" "$CONFIG_DIR"

# Default configuration
if [ ! -f "$CONFIG_DIR/config.json" ]; then
    echo "📋 Creating default configuration..."
    cp "$SCRIPT_DIR/config.json" "$CONFIG_DIR/"
fi

echo "📂 Configured directories:"
echo "   Input:  $INPUT_DIR"
echo "   Output: $OUTPUT_DIR"
echo "   Config: $CONFIG_DIR"
echo ""

# Check input directory
if [ ! -d "$INPUT_DIR" ]; then
    echo "❌ Input directory not found: $INPUT_DIR"
    echo "💡 Create the directory or modify INPUT_DIR in the script"
    exit 1
fi

# Cleanup function
cleanup() {
    echo ""
    echo "🧹 Cleaning up containers..."
    if [ "$WATCH_MODE" = true ]; then
        docker stop insta360-watch-mode 2>/dev/null || true
        docker rm -f insta360-watch-mode 2>/dev/null || true
    else
        docker stop insta360-single-run 2>/dev/null || true
        docker rm -f insta360-single-run 2>/dev/null || true
    fi
    echo "✅ Cleanup completed"
}

# Trap for automatic cleanup
trap cleanup EXIT INT TERM

# Build Docker image if necessary
echo "🔧 Checking Docker image..."
if ! docker image inspect insta360-auto-converter:latest >/dev/null 2>&1; then
    echo "🏗️ Building Docker image..."
    cd "$SCRIPT_DIR"
    docker-compose build
else
    echo "✅ Docker image ready"
fi

# Configuration Docker commune
DOCKER_VOLUMES=(
    -v "$INPUT_DIR:/data/input:ro"
    -v "$OUTPUT_DIR:/data/output"
    -v "$CONFIG_DIR:/data/config"
)

DOCKER_ENV=(
    -e DISPLAY=:99
    -e LIBGL_ALWAYS_INDIRECT=1
    -e MESA_GL_VERSION_OVERRIDE=4.5
)

# Launch according to mode
if [ "$WATCH_MODE" = true ]; then
    echo "🔄 Starting WATCH mode..."
    echo "   Press Ctrl+C to stop"
    echo ""
    
    docker run --rm -it \
        --name insta360-watch-mode \
        "${DOCKER_VOLUMES[@]}" \
        "${DOCKER_ENV[@]}" \
        --memory=4g \
        --cpus=2 \
        --entrypoint="" \
        insta360-auto-converter:latest \
        bash -c "Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render -noreset & sleep 2 && /app/build/insta360_batch_processor /data/input /data/output /data/config/config.json --watch; kill \$! 2>/dev/null || true"
else
    echo "🎯 Starting SINGLE mode..."
    echo ""
    
    docker run --rm \
        --name insta360-single-run \
        "${DOCKER_VOLUMES[@]}" \
        "${DOCKER_ENV[@]}" \
        --memory=4g \
        --cpus=2 \
        --entrypoint="" \
        insta360-auto-converter:latest \
        bash -c "Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render -noreset & sleep 2 && /app/build/insta360_batch_processor /data/input /data/output /data/config/config.json; kill \$! 2>/dev/null || true"
    
    echo ""
    echo "✅ Processing completed!"
    echo "📁 Converted files available in: $OUTPUT_DIR"
fi