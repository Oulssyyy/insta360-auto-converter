#!/bin/bash

# Script to analyze EXIF metadata of converted images

echo "🔍 Analyzing EXIF metadata of converted image..."
echo "================================================"

IMAGE_PATH="/mnt/c/Users/stf039/Pictures/output/IMG_20250827_162943_00_001.jpg"

# Run a temporary container with exiftool
docker run --rm \
  -v /mnt/c/Users/stf039/Pictures/output:/images \
  alpine:latest sh -c "
    apk add --no-cache exiftool
    echo 'EXIF data for: IMG_20250827_162943_00_001.jpg'
    echo '=============================================='
    exiftool -a -G1 /images/IMG_20250827_162943_00_001.jpg
    
    echo ''
    echo '360° Related Tags:'
    echo '=================='
    exiftool -G1 -ProjectionType -FullPanoWidthPixels -FullPanoHeightPixels /images/IMG_20250827_162943_00_001.jpg
    
    echo ''
    echo 'XMP GPano Tags:'
    echo '==============='
    exiftool -G1 -xmp-gpano:all /images/IMG_20250827_162943_00_001.jpg
"