# Running Insta360 Converter on WSL Ubuntu

This is likely to fix the OpenCV issues we're encountering with Windows Docker Desktop.

## Setup on WSL Ubuntu

### 1. Copy project to WSL
```bash
# From Windows, copy the project to WSL
# In PowerShell:
wsl
cd ~
cp -r /mnt/c/Users/stf039/Documents/GitHub/insta360-auto-converter ./
cd insta360-auto-converter
```

### 2. Build the Docker image in WSL
```bash
# Build the image
sudo docker build -t insta360-auto-converter .

# This should work much better in WSL than Windows Docker Desktop
```

### 3. Test with your files
```bash
# Create directories for testing
mkdir -p ~/test-input ~/test-output

# Copy a test file from Windows
cp /mnt/c/Users/stf039/Pictures/insta360/IMG_20250827_162943_00_001.insp ~/test-input/

# Run the converter
sudo docker run --rm \
  -v ~/test-input:/data/input:ro \
  -v ~/test-output:/data/output \
  insta360-auto-converter \
  "/data/input/IMG_20250827_162943_00_001.insp" "/data/output"
```

### 4. Check results
```bash
# Check if output file was created
ls -la ~/test-output/
```

## Why WSL is Better

1. **Native Linux environment** - The SDK is designed for Linux
2. **Better graphics support** - OpenGL/OpenCV work more reliably
3. **No Windows Docker layer** - Eliminates compatibility issues
4. **Proper X11 support** - Xvfb works correctly for headless operation

## Access Files from Windows

After conversion, you can access the output files from Windows at:
```
\\wsl$\Ubuntu\home\[your-username]\test-output\
```

Would you like to try this approach? It should resolve the OpenCV crash we've been seeing.
