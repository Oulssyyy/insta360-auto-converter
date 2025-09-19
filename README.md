# Insta360 Auto Converter

A Docker-based application for automatically converting Insta360 camera files (.insv video and .insp photo files) into standard formats.

🏠 **Perfect for Synology NAS!** - See [SYNOLOGY_SETUP.md](SYNOLOGY_SETUP.md) for complete NAS deployment guide.

## Features

✅ **Automated Batch Processing** - Monitor directories and auto-convert new files  
✅ **Synology NAS Optimized** - Designed specifically for NAS environments  
✅ **Single File Conversion** - Convert individual files on demand  
✅ **Headless Operation** - No GUI required, perfect for servers  
✅ **Configuration Management** - JSON-based configuration with sensible defaults  
✅ **Progress Tracking** - Avoid reprocessing files with smart tracking  

## Quick Start Options

### 🏠 Synology NAS (Recommended)

See **[SYNOLOGY_SETUP.md](SYNOLOGY_SETUP.md)** for complete setup guide.

### 💻 Standard Docker

## Prerequisites

- **Linux system** (Ubuntu/Debian preferred) or **Windows with WSL2**
- Docker (GPU support optional)
- Insta360 Media SDK (see installation instructions below)

### Platform Compatibility

⚠️ **Linux Only**: This application is designed to run on Linux systems. 

**Windows Users**: You must use Windows Subsystem for Linux (WSL2) to run this application:
1. Install [WSL2](https://docs.microsoft.com/en-us/windows/wsl/install)
2. Install Docker Desktop with WSL2 backend
3. Run all commands from within your WSL2 environment

## SDK Installation

1. Download the Insta360 Media SDK from the official Insta360 developer portal
2. Extract the SDK and copy its contents to the `sdk/` directory in this project:
   ```
   sdk/
   ├── bin/
   ├── include/
   │   ├── ins_common.h
   │   ├── ins_stitcher.h
   │   ├── ins_realtime_stitcher.h
   │   └── ins_version.h
   └── lib/
       ├── libMediaSDK.so
       └── [other SDK libraries]
   ```

## Build

```bash
docker build -t insta360-auto-converter .
```

## Usage

Convert a video file:
```bash
docker run --rm --gpus all \
  -v /path/to/input:/data/input \
  -v /path/to/output:/data/output \
  insta360-auto-converter \
  /data/input/video.insv /data/output
```

Convert a photo file:
```bash
docker run --rm --gpus all \
  -v /path/to/input:/data/input \
  -v /path/to/output:/data/output \
  insta360-auto-converter \
  /data/input/photo.insp /data/output
```

## Features

- Automatic detection of file type (video .insv or photo .insp)
- GPU acceleration support
- H.265 encoding for videos
- Flow state stabilization
- Direction lock
- Image fusion for photos
- Configurable output resolution and bitrate

## Troubleshooting

### Missing SDK Headers
If you encounter "ins_media.h: No such file or directory" errors, ensure that:

1. The SDK is properly extracted to the `sdk/` directory
2. The `sdk/include/` directory contains the required header files
3. The `sdk/lib/` directory contains `libMediaSDK.so`

### Build Errors
- Ensure you have the latest Docker version with BuildKit support
- Make sure the SDK files have proper permissions
- Check that all SDK dependencies are included

## SDK Header Files Used

This project uses the following SDK headers:
- `ins_stitcher.h` - Contains VideoStitcher and ImageStitcher classes
- `ins_common.h` - Contains common types, enums, and callbacks

Note: The project previously referenced non-existent headers like `ins_media.h`, `ins_video_stitcher.h`, and `ins_photo_stitcher.h`. These have been corrected to use the actual SDK headers.
