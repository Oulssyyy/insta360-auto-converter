FROM ubuntu:22.04

# Set timezone and prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Install dependencies optimized for Synology NAS and headless environment
RUN apt-get update && apt-get install -y \
    build-essential cmake git pkg-config \
    # Graphics and OpenGL libraries for headless operation
    mesa-utils libgl1-mesa-dev libglu1-mesa-dev \
    libegl1-mesa-dev libgles2-mesa-dev \
    # X11 libraries for headless graphics
    xvfb x11-utils \
    # Image processing libraries
    libopencv-dev libpng-dev libpng16-16 libjpeg-dev libtiff5-dev \
    # Multimedia libraries
    ffmpeg libavcodec-dev libavformat-dev libswscale-dev \
    # Vulkan support
    libvulkan1 vulkan-tools \
    # Additional dependencies for Insta360 SDK
    libtbb-dev libssl-dev zlib1g-dev \
    # JSON library for configuration
    libjsoncpp-dev \
    # EXIF library for 360 metadata
    libexiv2-dev \
    # Cleanup
    && rm -rf /var/lib/apt/lists/*

# Set up environment for headless operation
ENV DISPLAY=:99
ENV LIBGL_ALWAYS_INDIRECT=1
ENV MESA_GL_VERSION_OVERRIDE=4.5

# Create working directories
WORKDIR /app
RUN mkdir -p /data/input /data/output /data/processed

# Copy application code
COPY app /app

# Copy SDK
COPY sdk /sdk

# Build the project with optimizations for NAS environment
RUN cmake -S /app -B /app/build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-O3 -DHEADLESS_MODE=1" \
    && cmake --build /app/build --config Release

# Set up library paths for runtime
RUN echo '/sdk/lib' >> /etc/ld.so.conf.d/sdk.conf && ldconfig

# Set environment variables for library paths
ENV LD_LIBRARY_PATH="/sdk/lib:$LD_LIBRARY_PATH"

# Create startup script for NAS environment
COPY <<EOF /app/start.sh
#!/bin/bash
# Start Xvfb for headless graphics
Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render -noreset &
XVFB_PID=\$!

# Wait for Xvfb to start
sleep 2

# Run the converter
/app/build/insta360_converter "\$@"

# Cleanup
kill \$XVFB_PID 2>/dev/null || true
EOF

RUN chmod +x /app/start.sh

# Create batch processor scripts for different modes
COPY <<EOF /app/batch_single.sh
#!/bin/bash
# Single run mode - process all files once and exit
# Start Xvfb for headless graphics
Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render -noreset &
XVFB_PID=\$!

# Wait for Xvfb to start
sleep 2

echo "Starting Insta360 Batch Processor in SINGLE RUN mode"
echo "This will process all files once and exit"

# Run the batch processor in single mode
/app/build/insta360_batch_processor "\$@"

# Cleanup
kill \$XVFB_PID 2>/dev/null || true
EOF

COPY <<EOF /app/batch_watch.sh
#!/bin/bash
# Watch mode - continuously monitor for new files
# Start Xvfb for headless graphics
Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render -noreset &
XVFB_PID=\$!

# Wait for Xvfb to start
sleep 2

echo "Starting Insta360 Batch Processor in WATCH mode"
echo "This will continuously monitor for new files"
echo "Press Ctrl+C to stop"

# Run the batch processor in watch mode
/app/build/insta360_batch_processor "\$@" --watch

# Cleanup
kill \$XVFB_PID 2>/dev/null || true
EOF

RUN chmod +x /app/batch_single.sh /app/batch_watch.sh

# Entry point optimized for Synology NAS
ENTRYPOINT ["/app/start.sh"]
CMD ["/data/input/sample.insv", "/data/output"]
